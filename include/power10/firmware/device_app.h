#ifndef POWER10_FIRMWARE_DEVICE_APP_H
#define POWER10_FIRMWARE_DEVICE_APP_H

#include "power10/device.h"

typedef enum power10_cycle_status {
  POWER10_CYCLE_OK,
  POWER10_CYCLE_SENSOR_ERROR,
  POWER10_CYCLE_DATA_ERROR,
  POWER10_CYCLE_PUBLISH_ERROR,
  POWER10_CYCLE_INVALID_ARGUMENT
} power10_cycle_status;

/* A sequence prepared for publishing remains consumed when publishing fails. */
power10_cycle_status power10_run_device_cycle(power10_device_state* state);

#endif /* POWER10_FIRMWARE_DEVICE_APP_H */
