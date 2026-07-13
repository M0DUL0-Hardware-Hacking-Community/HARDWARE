#ifndef POWER10_DEVICE_HPP
#define POWER10_DEVICE_HPP

#include <cstdint>

namespace power10 {

struct SensorReading final {
  std::int32_t temperature_millicelsius;
  std::uint32_t humidity_millipercent;
};

struct Telemetry final {
  std::uint32_t sequence;
  SensorReading reading;
};

struct DeviceState final {
  std::uint32_t next_sequence;
};

enum class DeviceStatus : std::uint8_t {
  ok,
  invalid_temperature,
  invalid_humidity,
  sequence_exhausted
};

// On failure, leaves state and telemetry unchanged. On success, consumes one sequence.
[[nodiscard]] DeviceStatus prepare_telemetry(const SensorReading& reading, DeviceState& state,
                                             Telemetry& telemetry) noexcept;

} // namespace power10

#endif
