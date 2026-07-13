#include "power10/bounded_statistics.hpp"

#include "device_test.hpp"

#include <array>
#include <iostream>
#include <limits>

namespace {

int report_failure(const char* const message) {
  if (message == nullptr) {
    return 1;
  }
  std::cerr << message << '\n';
  return 1;
}

int test_valid_samples() {
  constexpr std::array samples{4, -2, 10};
  power10::Statistics output{};
  const auto status{power10::calculate_statistics(samples, output)};
  if (status != power10::Status::ok) {
    return report_failure("valid input returned an error");
  }
  if ((output.minimum != -2) || (output.maximum != 10) || (output.mean != 4)) {
    return report_failure("valid input produced incorrect statistics");
  }
  return 0;
}

int test_maximum_sample_count() {
  std::array<int, power10::kMaximumSamples> samples{};
  samples.fill(std::numeric_limits<int>::max());
  power10::Statistics output{};
  const auto status{power10::calculate_statistics(samples, output)};
  if ((status != power10::Status::ok) || (output.minimum != std::numeric_limits<int>::max()) ||
      (output.maximum != std::numeric_limits<int>::max()) ||
      (output.mean != std::numeric_limits<int>::max())) {
    return report_failure("maximum-size input produced incorrect statistics");
  }
  return 0;
}

int test_invalid_sizes() {
  constexpr power10::Statistics unchanged{1, 2, 3};
  power10::Statistics output{unchanged};
  constexpr std::array<int, 0U> empty{};
  if ((power10::calculate_statistics(empty, output) != power10::Status::empty_input) ||
      (output.minimum != unchanged.minimum) || (output.maximum != unchanged.maximum) ||
      (output.mean != unchanged.mean)) {
    return report_failure("empty input was accepted");
  }
  constexpr std::array<int, power10::kMaximumSamples + 1U> oversized{};
  if ((power10::calculate_statistics(oversized, output) != power10::Status::too_many_samples) ||
      (output.minimum != unchanged.minimum) || (output.maximum != unchanged.maximum) ||
      (output.mean != unchanged.mean)) {
    return report_failure("oversized input was accepted");
  }
  return 0;
}

} // namespace

int main() {
  const int valid_result{test_valid_samples()};
  if (valid_result != 0) {
    return valid_result;
  }
  const int maximum_result{test_maximum_sample_count()};
  if (maximum_result != 0) {
    return maximum_result;
  }
  const int invalid_result{test_invalid_sizes()};
  if (invalid_result != 0) {
    return invalid_result;
  }
  return run_device_tests();
}
