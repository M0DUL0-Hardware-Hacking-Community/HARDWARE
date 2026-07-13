#include "power10/device.h"

#include <stddef.h>
#include <stdint.h>

static const int32_t minimum_temperature = -40000;
static const int32_t maximum_temperature = 125000;
static const uint32_t maximum_humidity = UINT32_C(100000);

power10_device_status power10_prepare_telemetry(const power10_sensor_reading* reading,
                                                power10_device_state* state,
                                                power10_telemetry* telemetry) {
  if ((reading == NULL) || (state == NULL) || (telemetry == NULL)) {
    return POWER10_DEVICE_INVALID_ARGUMENT;
  }
  if ((reading->temperature_millicelsius < minimum_temperature) ||
      (reading->temperature_millicelsius > maximum_temperature)) {
    return POWER10_DEVICE_INVALID_TEMPERATURE;
  }
  if (reading->humidity_millipercent > maximum_humidity) {
    return POWER10_DEVICE_INVALID_HUMIDITY;
  }
  if (state->next_sequence == UINT32_MAX) {
    return POWER10_DEVICE_SEQUENCE_EXHAUSTED;
  }

  const power10_telemetry prepared = {state->next_sequence, *reading};
  *telemetry = prepared;
  ++state->next_sequence;
  return POWER10_DEVICE_OK;
}
