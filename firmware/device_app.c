#include "power10/firmware/device_app.h"

#include "power10/platform/device_io.h"

#include <stddef.h>

power10_cycle_status power10_run_device_cycle(power10_device_state* state) {
  if (state == NULL) {
    return POWER10_CYCLE_INVALID_ARGUMENT;
  }

  power10_sensor_reading reading = {0};
  if (power10_read_sensor(&reading) != POWER10_IO_OK) {
    return POWER10_CYCLE_SENSOR_ERROR;
  }

  power10_telemetry telemetry = {0};
  if (power10_prepare_telemetry(&reading, state, &telemetry) != POWER10_DEVICE_OK) {
    return POWER10_CYCLE_DATA_ERROR;
  }

  if (power10_publish_telemetry(&telemetry) != POWER10_IO_OK) {
    return POWER10_CYCLE_PUBLISH_ERROR;
  }
  return POWER10_CYCLE_OK;
}
