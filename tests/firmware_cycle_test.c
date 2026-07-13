#include "power10/firmware/device_app.h"
#include "power10/platform/device_io.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum io_mode { IO_SUCCESS, IO_SENSOR_FAILURE, IO_DATA_FAILURE, IO_PUBLISH_FAILURE } io_mode;

// Bounded test-fake state; this translation unit is never linked into production.
static io_mode mode = IO_SUCCESS;
static bool publish_called = false;
static power10_telemetry published = {0};

typedef struct test_case {
  io_mode mode;
  power10_cycle_status expected_status;
  uint32_t expected_sequence;
  bool expected_publish;
} test_case;

static const test_case test_cases[] = {
    {IO_SUCCESS, POWER10_CYCLE_OK, 1U, true},
    {IO_SENSOR_FAILURE, POWER10_CYCLE_SENSOR_ERROR, 0U, false},
    {IO_DATA_FAILURE, POWER10_CYCLE_DATA_ERROR, 0U, false},
    {IO_PUBLISH_FAILURE, POWER10_CYCLE_PUBLISH_ERROR, 1U, true},
};

power10_io_status power10_read_sensor(power10_sensor_reading* reading) {
  if (reading == NULL) {
    return POWER10_IO_INVALID_ARGUMENT;
  }
  if (mode == IO_SENSOR_FAILURE) {
    return POWER10_IO_SENSOR_UNAVAILABLE;
  }
  *reading = mode == IO_DATA_FAILURE ? (power10_sensor_reading){125001, UINT32_C(50000)}
                                     : (power10_sensor_reading){24500, UINT32_C(55000)};
  return POWER10_IO_OK;
}

power10_io_status power10_publish_telemetry(const power10_telemetry* telemetry) {
  if (telemetry == NULL) {
    return POWER10_IO_INVALID_ARGUMENT;
  }
  publish_called = true;
  published = *telemetry;
  return mode == IO_PUBLISH_FAILURE ? POWER10_IO_TRANSPORT_UNAVAILABLE : POWER10_IO_OK;
}

int main(void) {
  for (size_t index = 0U; index < (sizeof(test_cases) / sizeof(test_cases[0])); ++index) {
    mode = test_cases[index].mode;
    publish_called = false;
    published = (power10_telemetry){0};
    power10_device_state state = {0};
    const power10_cycle_status status = power10_run_device_cycle(&state);
    if ((status != test_cases[index].expected_status) ||
        (state.next_sequence != test_cases[index].expected_sequence) ||
        (publish_called != test_cases[index].expected_publish)) {
      return (int)(index + 1U);
    }
    if (publish_called &&
        ((published.sequence != 0U) || (published.reading.temperature_millicelsius != 24500) ||
         (published.reading.humidity_millipercent != UINT32_C(55000)))) {
      return (int)(index + 5U);
    }
  }
  return power10_run_device_cycle(NULL) == POWER10_CYCLE_INVALID_ARGUMENT ? 0 : 9;
}
