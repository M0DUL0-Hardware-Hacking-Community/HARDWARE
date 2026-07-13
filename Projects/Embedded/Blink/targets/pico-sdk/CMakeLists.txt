cmake_minimum_required(VERSION 3.20)

if(DEFINED PICO_SDK_PATH)
  set(BLINK_PICO_SDK_PATH "${PICO_SDK_PATH}")
elseif(DEFINED ENV{PICO_SDK_PATH})
  set(BLINK_PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
else()
  message(FATAL_ERROR "Set PICO_SDK_PATH as an environment variable or CMake option")
endif()

include("${BLINK_PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(blink_pico LANGUAGES C CXX ASM)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BLINK_LED_PIN PICO_DEFAULT_LED_PIN CACHE STRING "GPIO used by the LED")
set(BLINK_LED_ACTIVE_LOW 0 CACHE STRING "1 when the LED is active-low")
set(BLINK_HALF_PERIOD_MS 500 CACHE STRING "LED half-period in milliseconds")

pico_sdk_init()

add_executable(blink_pico main.c)
target_link_libraries(blink_pico PRIVATE pico_stdlib hardware_gpio)
target_compile_options(blink_pico PRIVATE -Wall -Wextra -Werror)
target_compile_definitions(
  blink_pico
  PRIVATE
    BLINK_LED_PIN=${BLINK_LED_PIN}
    BLINK_LED_ACTIVE_LOW=${BLINK_LED_ACTIVE_LOW}
    BLINK_HALF_PERIOD_MS=${BLINK_HALF_PERIOD_MS}
)

pico_enable_stdio_usb(blink_pico 0)
pico_enable_stdio_uart(blink_pico 0)
pico_add_extra_outputs(blink_pico)
