#include "power10/bounded_statistics.h"

#include <limits.h>
#include <stdint.h>

_Static_assert(INT_MAX <= (INT64_MAX / (int64_t)POWER10_MAXIMUM_SAMPLES),
               "the fixed int64 accumulator must hold every bounded sample");
_Static_assert(INT_MIN >= (INT64_MIN / (int64_t)POWER10_MAXIMUM_SAMPLES),
               "the fixed int64 accumulator must hold every bounded sample");

power10_statistics_status power10_calculate_statistics(const int* samples,
                                                       const size_t sample_count,
                                                       power10_statistics* output) {
  if (output == NULL) {
    return POWER10_STATISTICS_INVALID_ARGUMENT;
  }
  if (sample_count == 0U) {
    return POWER10_STATISTICS_EMPTY_INPUT;
  }
  if (sample_count > POWER10_MAXIMUM_SAMPLES) {
    return POWER10_STATISTICS_TOO_MANY_SAMPLES;
  }
  if (samples == NULL) {
    return POWER10_STATISTICS_INVALID_ARGUMENT;
  }

  int minimum = samples[0];
  int maximum = samples[0];
  int64_t sum = 0;
  for (size_t index = 0U; index < sample_count; ++index) {
    const int sample = samples[index];
    minimum = sample < minimum ? sample : minimum;
    maximum = sample > maximum ? sample : maximum;
    sum += sample;
  }

  output->minimum = minimum;
  output->maximum = maximum;
  output->mean = (int)(sum / (int64_t)sample_count);
  return POWER10_STATISTICS_OK;
}
