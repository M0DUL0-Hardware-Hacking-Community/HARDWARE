#include "blink_controller.hpp"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <cstdint>

namespace {

constexpr std::uint32_t kBlinkIntervalMilliseconds{500U};
blink::State state{};

void apply_output(const blink::Output& output) {
  if (output.changed) {
    gpio_put(PICO_DEFAULT_LED_PIN, output.led_on);
  }
}

std::uint32_t milliseconds_since_boot() {
  return to_ms_since_boot(get_absolute_time());
}

}  // namespace

int main() {
  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  blink::Output output{};
  const auto init_status{
      blink::initialize(state, kBlinkIntervalMilliseconds, milliseconds_since_boot(), output)};
  if (init_status != blink::Status::ok) {
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    return 1;
  }
  apply_output(output);

  while (true) {
    const auto update_status{blink::update(state, milliseconds_since_boot(), output)};
    if (update_status != blink::Status::ok) {
      gpio_put(PICO_DEFAULT_LED_PIN, false);
      return 1;
    }
    apply_output(output);
    tight_loop_contents();
  }
}
