cmake_minimum_required(VERSION 3.25)

project(concurrent_wifi_scanner_tests LANGUAGES C)

enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
  wifi_scanner_tests
  security_assessment_test.c
  ../src/network_catalog.c
  ../src/security_assessment.c
)
target_include_directories(wifi_scanner_tests PRIVATE ../src)
set_target_properties(
  wifi_scanner_tests
  PROPERTIES C_STANDARD 11
             C_STANDARD_REQUIRED ON
             C_EXTENSIONS OFF
)

if(MSVC)
  target_compile_options(wifi_scanner_tests PRIVATE /W4 /WX)
else()
  target_compile_options(
    wifi_scanner_tests
    PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Werror
      -Wconversion
      -Wshadow
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
      -Wstrict-prototypes
  )
endif()

add_test(NAME wifi_scanner COMMAND wifi_scanner_tests)
set_tests_properties(wifi_scanner PROPERTIES TIMEOUT 5)
