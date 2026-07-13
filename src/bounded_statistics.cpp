#include "power10/bounded_statistics.hpp"

#include <cstdint>
#include <limits>

namespace power10 {

static_assert(std::numeric_limits<int>::digits <= 31,
              "the fixed int64 accumulator requires int to be at most 32 bits");

Status calculate_statistics(const std::span<const int> samples, Statistics& output) noexcept {
  if (samples.empty()) {
    return Status::empty_input;
  }
  if (samples.size() > kMaximumSamples) {
    return Status::too_many_samples;
  }

  int minimum{samples.front()};
  int maximum{samples.front()};
  std::int64_t sum{0};
  for (std::size_t index{0U}; index < kMaximumSamples; ++index) {
    if (index >= samples.size()) {
      break;
    }
    const int sample{samples[index]};
    minimum = sample < minimum ? sample : minimum;
    maximum = sample > maximum ? sample : maximum;
    sum += sample;
  }

  const auto count{static_cast<std::int64_t>(samples.size())};
  const std::int64_t mean{sum / count};
  output = Statistics{minimum, maximum, static_cast<int>(mean)};
  return Status::ok;
}

} // namespace power10
