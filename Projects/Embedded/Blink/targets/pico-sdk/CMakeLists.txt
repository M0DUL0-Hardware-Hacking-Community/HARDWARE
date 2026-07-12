cmake_minimum_required(VERSION 3.20)

if(DEFINED PICO_SDK_PATH)
  set(POWER10_PICO_SDK_PATH "${PICO_SDK_PATH}")
elseif(DEFINED ENV{PICO_SDK_PATH})
  set(POWER10_PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
else()
  message(FATAL_ERROR "Set PICO_SDK_PATH as an environment variable or CMake option")
endif()

include("${POWER10_PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(blink_pico LANGUAGES C CXX ASM)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pico_sdk_init()

add_executable(
  blink_pico
  main.cpp
  "${CMAKE_CURRENT_LIST_DIR}/../../lib/blink_core/src/blink_controller.cpp"
)

target_include_directories(
  blink_pico
  PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../lib/blink_core/src"
)
target_link_libraries(blink_pico PRIVATE pico_stdlib hardware_gpio)

if(MSVC)
  target_compile_options(blink_pico PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(blink_pico PRIVATE -Wall -Wextra -Werror)
endif()

pico_enable_stdio_usb(blink_pico 0)
pico_enable_stdio_uart(blink_pico 0)
pico_add_extra_outputs(blink_pico)
