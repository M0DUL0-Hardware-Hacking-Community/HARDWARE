#ifndef POWER10_BOUNDED_STATISTICS_HPP
#define POWER10_BOUNDED_STATISTICS_HPP

#include <cstddef>
#include <cstdint>
#include <span>

namespace power10 {

inline constexpr std::size_t kMaximumSamples{64U};

enum class Status : std::uint8_t { ok, empty_input, too_many_samples };

struct Statistics final {
  int minimum;
  int maximum;
  int mean;
};

// Leaves output unchanged when the input size is invalid.
[[nodiscard]] Status calculate_statistics(std::span<const int> samples,
                                          Statistics& output) noexcept;

} // namespace power10

#endif
