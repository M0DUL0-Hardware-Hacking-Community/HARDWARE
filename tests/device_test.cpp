#include "device_test.hpp"

#include "power10/device.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace {

int report_failure(const char* const message) {
  if (message == nullptr) {
    return 1;
  }
  std::cerr << message << '\n';
  return 1;
}

bool same_telemetry(const power10::Telemetry& left, const power10::Telemetry& right) {
  return (left.sequence == right.sequence) &&
         (left.reading.temperature_millicelsius == right.reading.temperature_millicelsius) &&
         (left.reading.humidity_millipercent == right.reading.humidity_millipercent);
}

int test_nominal_reading() {
  power10::DeviceState state{};
  power10::Telemetry telemetry{};
  constexpr power10::SensorReading valid{25'000, 50'000U};
  if (power10::prepare_telemetry(valid, state, telemetry) != power10::DeviceStatus::ok) {
    return report_failure("valid sensor data was rejected");
  }
  if ((telemetry.sequence != 0U) || (state.next_sequence != 1U) ||
      (telemetry.reading.temperature_millicelsius != valid.temperature_millicelsius) ||
      (telemetry.reading.humidity_millipercent != valid.humidity_millipercent)) {
    return report_failure("valid sensor data was not copied and sequenced");
  }
  return 0;
}

int test_valid_boundaries() {
  power10::DeviceState state{std::numeric_limits<std::uint32_t>::max() - 1U};
  power10::Telemetry telemetry{};
  constexpr power10::SensorReading lower_limits{-40'000, 100'000U};
  if ((power10::prepare_telemetry(lower_limits, state, telemetry) != power10::DeviceStatus::ok) ||
      (telemetry.sequence != std::numeric_limits<std::uint32_t>::max() - 1U) ||
      (state.next_sequence != std::numeric_limits<std::uint32_t>::max())) {
    return report_failure("accepted lower limits or final sequence were rejected");
  }

  state = {};
  constexpr power10::SensorReading upper_temperature{125'000, 0U};
  if ((power10::prepare_telemetry(upper_temperature, state, telemetry) !=
       power10::DeviceStatus::ok) ||
      (telemetry.reading.temperature_millicelsius != upper_temperature.temperature_millicelsius)) {
    return report_failure("accepted upper temperature was rejected");
  }
  return 0;
}

int test_rejections_preserve_state() {
  power10::DeviceState state{7U};
  constexpr power10::Telemetry unchanged{3U, {12'345, 67'890U}};
  power10::Telemetry telemetry{unchanged};

  constexpr power10::SensorReading below_temperature{-40'001, 50'000U};
  if ((power10::prepare_telemetry(below_temperature, state, telemetry) !=
       power10::DeviceStatus::invalid_temperature) ||
      (state.next_sequence != 7U) || !same_telemetry(telemetry, unchanged)) {
    return report_failure("temperature below the minimum changed output state");
  }

  constexpr power10::SensorReading invalid_temperature{125'001, 50'000U};
  if ((power10::prepare_telemetry(invalid_temperature, state, telemetry) !=
       power10::DeviceStatus::invalid_temperature) ||
      (state.next_sequence != 7U) || !same_telemetry(telemetry, unchanged)) {
    return report_failure("temperature above the maximum changed output state");
  }

  constexpr power10::SensorReading invalid_humidity{25'000, 100'001U};
  if ((power10::prepare_telemetry(invalid_humidity, state, telemetry) !=
       power10::DeviceStatus::invalid_humidity) ||
      (state.next_sequence != 7U) || !same_telemetry(telemetry, unchanged)) {
    return report_failure("invalid humidity changed output state");
  }

  state.next_sequence = std::numeric_limits<std::uint32_t>::max();
  constexpr power10::SensorReading valid{25'000, 50'000U};
  if ((power10::prepare_telemetry(valid, state, telemetry) !=
       power10::DeviceStatus::sequence_exhausted) ||
      (state.next_sequence != std::numeric_limits<std::uint32_t>::max()) ||
      !same_telemetry(telemetry, unchanged)) {
    return report_failure("exhausted sequence changed output state");
  }
  return 0;
}

} // namespace

int run_device_tests() {
  const int nominal_result{test_nominal_reading()};
  if (nominal_result != 0) {
    return nominal_result;
  }
  const int boundary_result{test_valid_boundaries()};
  return boundary_result == 0 ? test_rejections_preserve_state() : boundary_result;
}
