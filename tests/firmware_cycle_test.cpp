#include "power10/firmware/device_app.hpp"
#include "power10/platform/device_io.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace {

enum class IoMode : std::uint8_t { success, sensor_failure, data_failure, publish_failure };

// Bounded test-fake state; this translation unit is never linked into production.
IoMode mode{IoMode::success};
bool publish_called{false};
power10::Telemetry published{};

struct TestCase final {
  IoMode mode;
  power10::firmware::CycleStatus expected_status;
  std::uint32_t expected_sequence;
  bool expected_publish;
};

constexpr std::array<TestCase, 4U> kTestCases{{
    {IoMode::success, power10::firmware::CycleStatus::ok, 1U, true},
    {IoMode::sensor_failure, power10::firmware::CycleStatus::sensor_error, 0U, false},
    {IoMode::data_failure, power10::firmware::CycleStatus::data_error, 0U, false},
    {IoMode::publish_failure, power10::firmware::CycleStatus::publish_error, 1U, true},
}};

} // namespace

namespace power10::platform {

IoStatus read_sensor(SensorReading& reading) noexcept {
  if (mode == IoMode::sensor_failure) {
    return IoStatus::sensor_unavailable;
  }
  reading = mode == IoMode::data_failure ? SensorReading{125'001, 50'000U}
                                         : SensorReading{24'500, 55'000U};
  return IoStatus::ok;
}

IoStatus publish_telemetry(const Telemetry& telemetry) noexcept {
  publish_called = true;
  published = telemetry;
  return mode == IoMode::publish_failure ? IoStatus::transport_unavailable : IoStatus::ok;
}

} // namespace power10::platform

int main() {
  for (std::size_t index{0U}; index < kTestCases.size(); ++index) {
    mode = kTestCases[index].mode;
    publish_called = false;
    published = {};
    power10::DeviceState state{};
    const auto status{power10::firmware::run_device_cycle(state)};
    if ((status != kTestCases[index].expected_status) ||
        (state.next_sequence != kTestCases[index].expected_sequence) ||
        (publish_called != kTestCases[index].expected_publish)) {
      return static_cast<int>(index + 1U);
    }
    if (publish_called &&
        ((published.sequence != 0U) || (published.reading.temperature_millicelsius != 24'500) ||
         (published.reading.humidity_millipercent != 55'000U))) {
      return static_cast<int>(index + 5U);
    }
  }
  return 0;
}
