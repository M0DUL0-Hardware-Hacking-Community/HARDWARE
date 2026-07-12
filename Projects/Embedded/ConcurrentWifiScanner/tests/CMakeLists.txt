cmake_minimum_required(VERSION 3.25)

project(concurrent_wifi_scanner_tests LANGUAGES CXX)

enable_testing()

add_executable(
  security_assessment_tests
  security_assessment_test.cpp
  ../src/security_assessment.cpp
)
target_include_directories(security_assessment_tests PRIVATE ../src)
target_compile_features(security_assessment_tests PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(security_assessment_tests PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(
    security_assessment_tests
    PRIVATE -Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow
  )
endif()

add_test(NAME security_assessment COMMAND security_assessment_tests)
