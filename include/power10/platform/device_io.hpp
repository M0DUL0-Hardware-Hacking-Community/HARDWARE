#ifndef POWER10_PLATFORM_DEVICE_IO_HPP
#define POWER10_PLATFORM_DEVICE_IO_HPP

#include "power10/device.hpp"

namespace power10::platform {

enum class IoStatus : std::uint8_t { ok, sensor_unavailable, transport_unavailable };

[[nodiscard]] IoStatus read_sensor(SensorReading& reading) noexcept;
[[nodiscard]] IoStatus publish_telemetry(const Telemetry& telemetry) noexcept;

} // namespace power10::platform

#endif
