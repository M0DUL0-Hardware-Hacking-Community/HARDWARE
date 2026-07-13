#include <stdbool.h>
#include <stdint.h>

#if (defined(BLINK_TARGET_ESP32) + defined(BLINK_TARGET_AVR) + defined(BLINK_TARGET_STM32)) != 1
#error "Define exactly one BLINK_TARGET_* macro"
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
_Static_assert(LED_LEVEL(0, 0) == 0U, "active-high off must be low");
_Static_assert(LED_LEVEL(1, 0) == 1U, "active-high on must be high");
_Static_assert(LED_LEVEL(0, 1) == 1U, "active-low off must be high");
_Static_assert(LED_LEVEL(1, 1) == 0U, "active-low on must be low");

#if defined(BLINK_TARGET_ESP32)

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

_Static_assert((BLINK_LED_PIN >= 0) && (BLINK_LED_PIN < GPIO_NUM_MAX),
               "BLINK_LED_PIN is outside the ESP32 GPIO range");
_Static_assert(pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS) > 0,
               "BLINK_HALF_PERIOD_MS must span at least one FreeRTOS tick");

static void write_led(bool enabled) {
  ESP_ERROR_CHECK(
      gpio_set_level((gpio_num_t)BLINK_LED_PIN, LED_LEVEL(enabled, BLINK_LED_ACTIVE_LOW)));
}

void app_main(void) {
  const gpio_num_t pin = (gpio_num_t)BLINK_LED_PIN;

  ESP_ERROR_CHECK(gpio_reset_pin(pin));
  write_led(false);
  ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_OUTPUT));

  for (;;) {
    write_led(true);
    vTaskDelay(pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS));
    write_led(false);
    vTaskDelay(pdMS_TO_TICKS(BLINK_HALF_PERIOD_MS));
  }
}

#elif defined(BLINK_TARGET_AVR)

#include <avr/io.h>
#include <util/delay.h>

_Static_assert((BLINK_LED_BIT >= 0) && (BLINK_LED_BIT < 8),
               "BLINK_LED_BIT must select one AVR port bit");

static void write_led(bool enabled) {
  const uint8_t mask = (uint8_t)_BV(BLINK_LED_BIT);

  if (LED_LEVEL(enabled, BLINK_LED_ACTIVE_LOW) != 0U) {
    BLINK_LED_PORT |= mask;
  } else {
    BLINK_LED_PORT &= (uint8_t)~mask;
  }
}

int main(void) {
  write_led(false);
  BLINK_LED_DDR |= (uint8_t)_BV(BLINK_LED_BIT);

  for (;;) {
    write_led(true);
    _delay_ms(BLINK_HALF_PERIOD_MS);
    write_led(false);
    _delay_ms(BLINK_HALF_PERIOD_MS);
  }
}

#elif defined(BLINK_TARGET_STM32)

#include "stm32f4xx_hal.h"

_Static_assert((BLINK_LED_PIN >= GPIO_PIN_0) && (BLINK_LED_PIN <= GPIO_PIN_15) &&
                   ((BLINK_LED_PIN & (BLINK_LED_PIN - 1U)) == 0U),
               "BLINK_LED_PIN must select one STM32 GPIO pin");

static void write_led(bool enabled) {
  const GPIO_PinState state =
      LED_LEVEL(enabled, BLINK_LED_ACTIVE_LOW) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
  HAL_GPIO_WritePin(BLINK_LED_PORT, BLINK_LED_PIN, state);
}

int main(void) {
  if (HAL_Init() != HAL_OK) {
    __disable_irq();
    for (;;) {
    }
  }

  BLINK_LED_CLOCK_ENABLE();
  write_led(false);
  GPIO_InitTypeDef gpio = {
      .Pin = BLINK_LED_PIN,
      .Mode = GPIO_MODE_OUTPUT_PP,
      .Pull = GPIO_NOPULL,
      .Speed = GPIO_SPEED_FREQ_LOW,
  };
  HAL_GPIO_Init(BLINK_LED_PORT, &gpio);

  for (;;) {
    write_led(true);
    HAL_Delay(BLINK_HALF_PERIOD_MS);
    write_led(false);
    HAL_Delay(BLINK_HALF_PERIOD_MS);
  }
}

#endif
