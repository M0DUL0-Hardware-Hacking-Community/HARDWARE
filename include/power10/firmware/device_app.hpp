#ifndef POWER10_FIRMWARE_DEVICE_APP_HPP
#define POWER10_FIRMWARE_DEVICE_APP_HPP

#include "power10/device.hpp"

namespace power10::firmware {

enum class CycleStatus { ok, sensor_error, data_error, publish_error };

[[nodiscard]] CycleStatus run_device_cycle(DeviceState& state) noexcept;

}  // namespace power10::firmware

#endif
