#ifndef POWER10_PLATFORM_DEVICE_IO_H
#define POWER10_PLATFORM_DEVICE_IO_H

#include "power10/device.h"

typedef enum power10_io_status {
  POWER10_IO_OK,
  POWER10_IO_SENSOR_UNAVAILABLE,
  POWER10_IO_TRANSPORT_UNAVAILABLE,
  POWER10_IO_INVALID_ARGUMENT
} power10_io_status;

power10_io_status power10_read_sensor(power10_sensor_reading* reading);
power10_io_status power10_publish_telemetry(const power10_telemetry* telemetry);

#endif /* POWER10_PLATFORM_DEVICE_IO_H */
