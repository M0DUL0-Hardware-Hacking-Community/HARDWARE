#include "blink_controller.hpp"

namespace blink {

Status initialize(State& state, const std::uint32_t interval_milliseconds,
                  const std::uint32_t now_milliseconds, Output& output) noexcept {
  if (interval_milliseconds == 0U) {
    return Status::invalid_interval;
  }

  state = State{now_milliseconds, interval_milliseconds, false, true};
  output = Output{true, false};
  return Status::ok;
}

Status update(State& state, const std::uint32_t now_milliseconds, Output& output) noexcept {
  if (!state.initialized) {
    return Status::not_initialized;
  }
  if (state.interval_milliseconds == 0U) {
    return Status::invalid_interval;
  }

  output = Output{false, state.led_on};
  const std::uint32_t elapsed{now_milliseconds - state.last_transition_milliseconds};
  if (elapsed >= state.interval_milliseconds) {
    state.led_on = !state.led_on;
    state.last_transition_milliseconds = now_milliseconds;
    output = Output{true, state.led_on};
  }
  return Status::ok;
}

}  // namespace blink
