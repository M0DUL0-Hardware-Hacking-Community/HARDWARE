#include "power10/firmware/device_app.hpp"

#include "power10/platform/device_io.hpp"

namespace power10::firmware {

CycleStatus run_device_cycle(DeviceState& state) noexcept {
  SensorReading reading{};
  const platform::IoStatus read_status{platform::read_sensor(reading)};
  if (read_status != platform::IoStatus::ok) {
    return CycleStatus::sensor_error;
  }

  Telemetry telemetry{};
  const DeviceStatus data_status{prepare_telemetry(reading, state, telemetry)};
  if (data_status != DeviceStatus::ok) {
    return CycleStatus::data_error;
  }

  const platform::IoStatus publish_status{platform::publish_telemetry(telemetry)};
  if (publish_status != platform::IoStatus::ok) {
    return CycleStatus::publish_error;
  }
  return CycleStatus::ok;
}

} // namespace power10::firmware
