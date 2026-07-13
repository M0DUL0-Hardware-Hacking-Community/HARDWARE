#include "hardware/gpio.h"
#include "pico/stdlib.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef BLINK_LED_PIN
#error "Define BLINK_LED_PIN"
#endif

#ifndef BLINK_LED_ACTIVE_LOW
#error "Define BLINK_LED_ACTIVE_LOW as 0 or 1"
#endif

#ifndef BLINK_HALF_PERIOD_MS
#error "Define BLINK_HALF_PERIOD_MS"
#endif

#define LED_LEVEL(enabled, active_low) (((enabled) != (active_low)) ? 1U : 0U)

_Static_assert((BLINK_LED_ACTIVE_LOW == 0) || (BLINK_LED_ACTIVE_LOW == 1),
               "BLINK_LED_ACTIVE_LOW must be 0 or 1");
_Static_assert(BLINK_HALF_PERIOD_MS > 0, "BLINK_HALF_PERIOD_MS must be positive");
_Static_assert(BLINK_HALF_PERIOD_MS <= UINT32_MAX, "BLINK_HALF_PERIOD_MS must fit in uint32_t");
_Static_assert((BLINK_LED_PIN >= 0) && (BLINK_LED_PIN < NUM_BANK0_GPIOS),
               "BLINK_LED_PIN is outside the Pico GPIO range");
_Static_assert(LED_LEVEL(0, 0) == 0U, "active-high off must be low");
_Static_assert(LED_LEVEL(1, 0) == 1U, "active-high on must be high");
_Static_assert(LED_LEVEL(0, 1) == 1U, "active-low off must be high");
_Static_assert(LED_LEVEL(1, 1) == 0U, "active-low on must be low");

static void write_led(bool enabled) {
  gpio_put(BLINK_LED_PIN, LED_LEVEL(enabled, BLINK_LED_ACTIVE_LOW));
}

int main(void) {
  gpio_init(BLINK_LED_PIN);
  write_led(false);
  gpio_set_dir(BLINK_LED_PIN, GPIO_OUT);

  for (;;) {
    write_led(true);
    sleep_ms(BLINK_HALF_PERIOD_MS);
    write_led(false);
    sleep_ms(BLINK_HALF_PERIOD_MS);
  }
}
