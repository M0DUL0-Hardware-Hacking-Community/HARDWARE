#include "device_test.h"

#include "power10/device.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static int report_failure(const char* message) {
  if (message == NULL) {
    return 1;
  }
  (void)fputs(message, stderr);
  (void)fputc('\n', stderr);
  return 1;
}

static bool same_telemetry(const power10_telemetry* left, const power10_telemetry* right) {
  return (left->sequence == right->sequence) &&
         (left->reading.temperature_millicelsius == right->reading.temperature_millicelsius) &&
         (left->reading.humidity_millipercent == right->reading.humidity_millipercent);
}

static int test_nominal_reading(void) {
  power10_device_state state = {0};
  power10_telemetry telemetry = {0};
  const power10_sensor_reading valid = {25000, UINT32_C(50000)};
  if (power10_prepare_telemetry(&valid, &state, &telemetry) != POWER10_DEVICE_OK) {
    return report_failure("valid sensor data was rejected");
  }
  if ((telemetry.sequence != 0U) || (state.next_sequence != 1U) ||
      (telemetry.reading.temperature_millicelsius != valid.temperature_millicelsius) ||
      (telemetry.reading.humidity_millipercent != valid.humidity_millipercent)) {
    return report_failure("valid sensor data was not copied and sequenced");
  }
  return 0;
}

static int test_valid_boundaries(void) {
  power10_device_state state = {UINT32_MAX - 1U};
  power10_telemetry telemetry = {0};
  const power10_sensor_reading lower_limits = {-40000, UINT32_C(100000)};
  if ((power10_prepare_telemetry(&lower_limits, &state, &telemetry) != POWER10_DEVICE_OK) ||
      (telemetry.sequence != UINT32_MAX - 1U) || (state.next_sequence != UINT32_MAX)) {
    return report_failure("accepted lower limits or final sequence were rejected");
  }

  state = (power10_device_state){0};
  const power10_sensor_reading upper_temperature = {125000, 0U};
  if ((power10_prepare_telemetry(&upper_temperature, &state, &telemetry) != POWER10_DEVICE_OK) ||
      (telemetry.reading.temperature_millicelsius != upper_temperature.temperature_millicelsius)) {
    return report_failure("accepted upper temperature was rejected");
  }
  return 0;
}

static int test_rejections_preserve_state(void) {
  power10_device_state state = {7U};
  const power10_telemetry unchanged = {3U, {12345, UINT32_C(67890)}};
  power10_telemetry telemetry = unchanged;

  const power10_sensor_reading below_temperature = {-40001, UINT32_C(50000)};
  if ((power10_prepare_telemetry(&below_temperature, &state, &telemetry) !=
       POWER10_DEVICE_INVALID_TEMPERATURE) ||
      (state.next_sequence != 7U) || !same_telemetry(&telemetry, &unchanged)) {
    return report_failure("temperature below the minimum changed output state");
  }

  const power10_sensor_reading invalid_temperature = {125001, UINT32_C(50000)};
  if ((power10_prepare_telemetry(&invalid_temperature, &state, &telemetry) !=
       POWER10_DEVICE_INVALID_TEMPERATURE) ||
      (state.next_sequence != 7U) || !same_telemetry(&telemetry, &unchanged)) {
    return report_failure("temperature above the maximum changed output state");
  }

  const power10_sensor_reading invalid_humidity = {25000, UINT32_C(100001)};
  if ((power10_prepare_telemetry(&invalid_humidity, &state, &telemetry) !=
       POWER10_DEVICE_INVALID_HUMIDITY) ||
      (state.next_sequence != 7U) || !same_telemetry(&telemetry, &unchanged)) {
    return report_failure("invalid humidity changed output state");
  }

  state.next_sequence = UINT32_MAX;
  const power10_sensor_reading valid = {25000, UINT32_C(50000)};
  if ((power10_prepare_telemetry(&valid, &state, &telemetry) !=
       POWER10_DEVICE_SEQUENCE_EXHAUSTED) ||
      (state.next_sequence != UINT32_MAX) || !same_telemetry(&telemetry, &unchanged)) {
    return report_failure("exhausted sequence changed output state");
  }
  if ((power10_prepare_telemetry(NULL, &state, &telemetry) != POWER10_DEVICE_INVALID_ARGUMENT) ||
      (power10_prepare_telemetry(&valid, NULL, &telemetry) != POWER10_DEVICE_INVALID_ARGUMENT) ||
      (power10_prepare_telemetry(&valid, &state, NULL) != POWER10_DEVICE_INVALID_ARGUMENT)) {
    return report_failure("null argument was accepted");
  }
  return 0;
}

int run_device_tests(void) {
  const int nominal_result = test_nominal_reading();
  if (nominal_result != 0) {
    return nominal_result;
  }
  const int boundary_result = test_valid_boundaries();
  return boundary_result == 0 ? test_rejections_preserve_state() : boundary_result;
}
