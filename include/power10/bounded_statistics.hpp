#ifndef POWER10_BOUNDED_STATISTICS_HPP
#define POWER10_BOUNDED_STATISTICS_HPP

#include <cstddef>
#include <span>

namespace power10 {

inline constexpr std::size_t kMaximumSamples{64U};

enum class Status { ok, empty_input, too_many_samples, output_out_of_range };

struct Statistics final {
  int minimum;
  int maximum;
  int mean;
};

[[nodiscard]] Status calculate_statistics(std::span<const int> samples,
                                          Statistics& output) noexcept;

}  // namespace power10

#endif
