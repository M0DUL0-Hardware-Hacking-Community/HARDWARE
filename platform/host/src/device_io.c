#include "power10/platform/device_io.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

power10_io_status power10_read_sensor(power10_sensor_reading* reading) {
  if (reading == NULL) {
    return POWER10_IO_INVALID_ARGUMENT;
  }
  reading->temperature_millicelsius = INT32_C(24500);
  reading->humidity_millipercent = UINT32_C(55000);
  return POWER10_IO_OK;
}

power10_io_status power10_publish_telemetry(const power10_telemetry* telemetry) {
  if (telemetry == NULL) {
    return POWER10_IO_INVALID_ARGUMENT;
  }
  const int written =
      printf("sequence=%" PRIu32 " temperature_mc=%" PRId32 " humidity_mp=%" PRIu32 "\n",
             telemetry->sequence, telemetry->reading.temperature_millicelsius,
             telemetry->reading.humidity_millipercent);
  if ((written < 0) || (fflush(stdout) != 0)) {
    return POWER10_IO_TRANSPORT_UNAVAILABLE;
  }
  return POWER10_IO_OK;
}
