#ifndef POWER10_DEVICE_H
#define POWER10_DEVICE_H

#include <stdint.h>

typedef struct power10_sensor_reading {
  int32_t temperature_millicelsius;
  uint32_t humidity_millipercent;
} power10_sensor_reading;

typedef struct power10_telemetry {
  uint32_t sequence;
  power10_sensor_reading reading;
} power10_telemetry;

typedef struct power10_device_state {
  uint32_t next_sequence;
} power10_device_state;

typedef enum power10_device_status {
  POWER10_DEVICE_OK,
  POWER10_DEVICE_INVALID_TEMPERATURE,
  POWER10_DEVICE_INVALID_HUMIDITY,
  POWER10_DEVICE_SEQUENCE_EXHAUSTED,
  POWER10_DEVICE_INVALID_ARGUMENT
} power10_device_status;

/* On failure, leaves state and telemetry unchanged. On success, consumes one sequence. */
power10_device_status power10_prepare_telemetry(const power10_sensor_reading* reading,
                                                power10_device_state* state,
                                                power10_telemetry* telemetry);

#endif /* POWER10_DEVICE_H */
