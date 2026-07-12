#include "power10/bounded_statistics.hpp"

#include "device_test.hpp"

#include <array>
#include <iostream>

namespace {

int report_failure(const char* const message) {
  if (message == nullptr) {
    return 1;
  }
  std::cerr << message << '\n';
  return 1;
}

int test_valid_samples() {
  constexpr std::array samples{-2, 4, 10};
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

int test_invalid_sizes() {
  power10::Statistics output{};
  constexpr std::array<int, 0U> empty{};
  if (power10::calculate_statistics(empty, output) != power10::Status::empty_input) {
    return report_failure("empty input was accepted");
  }
  constexpr std::array<int, power10::kMaximumSamples + 1U> oversized{};
  if (power10::calculate_statistics(oversized, output) != power10::Status::too_many_samples) {
    return report_failure("oversized input was accepted");
  }
  return 0;
}

}  // namespace

int main() {
  const int valid_result{test_valid_samples()};
  if (valid_result != 0) {
    return valid_result;
  }
  const int invalid_result{test_invalid_sizes()};
  if (invalid_result != 0) {
    return invalid_result;
  }
  return run_device_tests();
}
