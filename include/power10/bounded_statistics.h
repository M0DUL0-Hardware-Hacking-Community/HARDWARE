#ifndef POWER10_BOUNDED_STATISTICS_H
#define POWER10_BOUNDED_STATISTICS_H

#include <stddef.h>

#define POWER10_MAXIMUM_SAMPLES 64U

typedef enum power10_statistics_status {
  POWER10_STATISTICS_OK,
  POWER10_STATISTICS_EMPTY_INPUT,
  POWER10_STATISTICS_TOO_MANY_SAMPLES,
  POWER10_STATISTICS_INVALID_ARGUMENT
} power10_statistics_status;

typedef struct power10_statistics {
  int minimum;
  int maximum;
  int mean;
} power10_statistics;

/* Leaves output unchanged when an argument or the input size is invalid. */
power10_statistics_status power10_calculate_statistics(const int* samples, size_t sample_count,
                                                       power10_statistics* output);

#endif /* POWER10_BOUNDED_STATISTICS_H */
