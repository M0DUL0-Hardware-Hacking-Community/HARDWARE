#include "power10/device.hpp"

#include <cstdint>
#include <limits>

namespace power10 {
namespace {

constexpr std::int32_t kMinimumTemperature{-40'000};
constexpr std::int32_t kMaximumTemperature{125'000};
constexpr std::uint32_t kMaximumHumidity{100'000U};

} // namespace

DeviceStatus prepare_telemetry(const SensorReading& reading, DeviceState& state,
                               Telemetry& telemetry) noexcept {
  if ((reading.temperature_millicelsius < kMinimumTemperature) ||
      (reading.temperature_millicelsius > kMaximumTemperature)) {
    return DeviceStatus::invalid_temperature;
  }
  if (reading.humidity_millipercent > kMaximumHumidity) {
    return DeviceStatus::invalid_humidity;
  }
  if (state.next_sequence == std::numeric_limits<std::uint32_t>::max()) {
    return DeviceStatus::sequence_exhausted;
  }

  telemetry = Telemetry{state.next_sequence, reading};
  ++state.next_sequence;
  return DeviceStatus::ok;
}

} // namespace power10
