cmake_minimum_required(VERSION 3.25)

project(concurrent_wifi_scanner_tests LANGUAGES CXX)

enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
  wifi_scanner_tests
  security_assessment_test.cpp
  ../src/network_catalog.cpp
  ../src/security_assessment.cpp
)
target_include_directories(wifi_scanner_tests PRIVATE ../src)
target_compile_features(wifi_scanner_tests PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(wifi_scanner_tests PRIVATE /W4 /WX /permissive-)
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
  )
endif()

add_test(NAME wifi_scanner COMMAND wifi_scanner_tests)
set_tests_properties(wifi_scanner PROPERTIES TIMEOUT 5)
