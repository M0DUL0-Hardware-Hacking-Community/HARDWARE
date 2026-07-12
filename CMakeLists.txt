cmake_minimum_required(VERSION 3.25)

project(power10_iot VERSION 0.2.0 LANGUAGES CXX)

option(POWER10_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)
option(POWER10_BUILD_HOST_APP "Build the host IoT simulator" ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warnings INTERFACE)
if(MSVC)
  target_compile_options(project_warnings INTERFACE /W4 /WX /permissive-)
else()
  target_compile_options(
    project_warnings
    INTERFACE
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

if(POWER10_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
endif()

add_subdirectory(src)

if(POWER10_BUILD_HOST_APP)
  add_subdirectory(platform/host)
  add_subdirectory(firmware)
  add_subdirectory(apps/linux_gateway)
endif()

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
