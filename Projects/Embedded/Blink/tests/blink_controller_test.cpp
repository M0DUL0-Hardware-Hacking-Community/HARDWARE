#include "blink_controller.hpp"

#include <cstdint>
#include <iostream>

namespace {

int fail(const char* const message) {
  if (message == nullptr) {
    return 1;
  }
  std::cerr << message << '\n';
  return 1;
}

int test_normal_operation() {
  blink::State state{};
  blink::Output output{};
  if (blink::initialize(state, 500U, 100U, output) != blink::Status::ok) {
    return fail("initialization failed");
  }
  if (!output.changed || output.led_on) {
    return fail("initial output is incorrect");
  }
  if (blink::update(state, 599U, output) != blink::Status::ok || output.changed) {
    return fail("LED changed before the interval elapsed");
  }
  if (blink::update(state, 600U, output) != blink::Status::ok || !output.led_on) {
    return fail("LED did not change at the interval");
  }
  return 0;
}

int test_timer_wraparound() {
  blink::State state{};
  blink::Output output{};
  constexpr std::uint32_t start{0xFFFFFFF0U};
  if (blink::initialize(state, 32U, start, output) != blink::Status::ok) {
    return fail("wraparound initialization failed");
  }
  if (blink::update(state, 0x00000010U, output) != blink::Status::ok || !output.led_on) {
    return fail("timer wraparound was handled incorrectly");
  }
  return 0;
}

}  // namespace

int main() {
  const int normal_result{test_normal_operation()};
  if (normal_result != 0) {
    return normal_result;
  }
  return test_timer_wraparound();
}
