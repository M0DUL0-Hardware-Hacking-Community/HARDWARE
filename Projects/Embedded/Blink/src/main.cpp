#include <Arduino.h>

#include <stdint.h>

#ifndef BLINK_LED_PIN
#error "Define BLINK_LED_PIN in platformio.ini"
#endif

#ifndef BLINK_LED_ACTIVE_LOW
#error "Define BLINK_LED_ACTIVE_LOW as 0 or 1 in platformio.ini"
#endif

namespace {

constexpr auto kLedPin{BLINK_LED_PIN};

constexpr uint8_t led_level(const bool enabled, const bool active_low) noexcept {
  return enabled != active_low ? HIGH : LOW;
}

static_assert(led_level(false, false) == LOW, "active-high off level must be LOW");
static_assert(led_level(true, false) == HIGH, "active-high on level must be HIGH");
static_assert(led_level(false, true) == HIGH, "active-low off level must be HIGH");
static_assert(led_level(true, true) == LOW, "active-low on level must be LOW");

void write_led(const bool enabled) {
  constexpr bool kLedActiveLow{BLINK_LED_ACTIVE_LOW != 0};
  static_assert((BLINK_LED_ACTIVE_LOW == 0) || (BLINK_LED_ACTIVE_LOW == 1),
                "BLINK_LED_ACTIVE_LOW must be 0 or 1");
  digitalWrite(kLedPin, led_level(enabled, kLedActiveLow));
}

} // namespace

void setup() {
  write_led(false);
  pinMode(kLedPin, OUTPUT);
}

void loop() {
  constexpr uint32_t kHalfPeriodMilliseconds{500U};
  write_led(true);
  delay(kHalfPeriodMilliseconds);
  write_led(false);
  delay(kHalfPeriodMilliseconds);
}
