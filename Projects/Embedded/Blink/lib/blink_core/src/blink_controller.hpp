#ifndef BLINK_CONTROLLER_HPP
#define BLINK_CONTROLLER_HPP

#include <cstdint>

namespace blink {

enum class Status { ok, invalid_interval, not_initialized };

struct State final {
  std::uint32_t last_transition_milliseconds;
  std::uint32_t interval_milliseconds;
  bool led_on;
  bool initialized;
};

struct Output final {
  bool changed;
  bool led_on;
};

[[nodiscard]] Status initialize(State& state, std::uint32_t interval_milliseconds,
                                std::uint32_t now_milliseconds, Output& output) noexcept;

[[nodiscard]] Status update(State& state, std::uint32_t now_milliseconds,
                            Output& output) noexcept;

}  // namespace blink

#endif
