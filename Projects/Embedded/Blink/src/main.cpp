#include <Arduino.h>

#include "blink_controller.hpp"

#include <cstdint>

#ifndef BLINK_LED_PIN
#error "Define BLINK_LED_PIN in the target build configuration"
#endif

namespace {

constexpr std::uint8_t kLedPin{BLINK_LED_PIN};
constexpr std::uint32_t kBlinkIntervalMilliseconds{500U};
blink::State state{};

void apply_output(const blink::Output& output) {
  if (output.changed) {
    digitalWrite(kLedPin, output.led_on ? HIGH : LOW);
  }
}

}  // namespace

void setup() {
  pinMode(kLedPin, OUTPUT);
  blink::Output output{};
  const auto status{blink::initialize(state, kBlinkIntervalMilliseconds, millis(), output)};
  if (status != blink::Status::ok) {
    digitalWrite(kLedPin, LOW);
    return;
  }
  apply_output(output);
}

void loop() {
  blink::Output output{};
  const auto status{blink::update(state, millis(), output)};
  if (status != blink::Status::ok) {
    digitalWrite(kLedPin, LOW);
    delay(kBlinkIntervalMilliseconds);
    return;
  }
  apply_output(output);
}
