#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <stdint.h>

int main() {
  constexpr uint32_t kHalfPeriodMilliseconds{500U};

  gpio_init(PICO_DEFAULT_LED_PIN);
  gpio_put(PICO_DEFAULT_LED_PIN, false);
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
  for (;;) {
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(kHalfPeriodMilliseconds);
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(kHalfPeriodMilliseconds);
  }
}
