cmake_minimum_required(VERSION 3.20)

project(portable_blink LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(blink_core STATIC lib/blink_core/src/blink_controller.cpp)
target_include_directories(blink_core PUBLIC lib/blink_core/src)

if(MSVC)
  target_compile_options(blink_core PRIVATE /W4 /WX /permissive-)
else()
  target_compile_options(blink_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(blink_tests tests/blink_controller_test.cpp)
  target_link_libraries(blink_tests PRIVATE blink_core)
  if(MSVC)
    target_compile_options(blink_tests PRIVATE /W4 /WX /permissive-)
  else()
    target_compile_options(blink_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
  add_test(NAME blink_tests COMMAND blink_tests)
endif()
