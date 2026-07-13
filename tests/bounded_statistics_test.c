#include "power10/bounded_statistics.h"

#include "device_test.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>

static int report_failure(const char* message) {
  if (message == NULL) {
    return 1;
  }
  (void)fputs(message, stderr);
  (void)fputc('\n', stderr);
  return 1;
}

static int test_valid_samples(void) {
  const int samples[] = {4, -2, 10};
  power10_statistics output = {0};
  const power10_statistics_status status = power10_calculate_statistics(samples, 3U, &output);
  if (status != POWER10_STATISTICS_OK) {
    return report_failure("valid input returned an error");
  }
  if ((output.minimum != -2) || (output.maximum != 10) || (output.mean != 4)) {
    return report_failure("valid input produced incorrect statistics");
  }
  return 0;
}

static int test_maximum_sample_count(void) {
  int samples[POWER10_MAXIMUM_SAMPLES] = {0};
  for (size_t index = 0U; index < POWER10_MAXIMUM_SAMPLES; ++index) {
    samples[index] = INT_MAX;
  }
  power10_statistics output = {0};
  const power10_statistics_status status =
      power10_calculate_statistics(samples, POWER10_MAXIMUM_SAMPLES, &output);
  if ((status != POWER10_STATISTICS_OK) || (output.minimum != INT_MAX) ||
      (output.maximum != INT_MAX) || (output.mean != INT_MAX)) {
    return report_failure("maximum-size input produced incorrect statistics");
  }
  return 0;
}

static int test_invalid_inputs(void) {
  const power10_statistics unchanged = {1, 2, 3};
  power10_statistics output = unchanged;
  if ((power10_calculate_statistics(NULL, 0U, &output) != POWER10_STATISTICS_EMPTY_INPUT) ||
      (output.minimum != unchanged.minimum) || (output.maximum != unchanged.maximum) ||
      (output.mean != unchanged.mean)) {
    return report_failure("empty input was accepted");
  }
  const int oversized[POWER10_MAXIMUM_SAMPLES + 1U] = {0};
  if ((power10_calculate_statistics(oversized, POWER10_MAXIMUM_SAMPLES + 1U, &output) !=
       POWER10_STATISTICS_TOO_MANY_SAMPLES) ||
      (output.minimum != unchanged.minimum) || (output.maximum != unchanged.maximum) ||
      (output.mean != unchanged.mean)) {
    return report_failure("oversized input was accepted");
  }
  if ((power10_calculate_statistics(NULL, 1U, &output) != POWER10_STATISTICS_INVALID_ARGUMENT) ||
      (power10_calculate_statistics(oversized, 1U, NULL) != POWER10_STATISTICS_INVALID_ARGUMENT)) {
    return report_failure("null argument was accepted");
  }
  return 0;
}

int main(void) {
  const int valid_result = test_valid_samples();
  if (valid_result != 0) {
    return valid_result;
  }
  const int maximum_result = test_maximum_sample_count();
  if (maximum_result != 0) {
    return maximum_result;
  }
  const int invalid_result = test_invalid_inputs();
  if (invalid_result != 0) {
    return invalid_result;
  }
  return run_device_tests();
}
