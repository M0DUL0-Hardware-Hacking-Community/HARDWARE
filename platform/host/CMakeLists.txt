add_library(power10_platform STATIC src/device_io.cpp)
target_include_directories(power10_platform PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(power10_platform PRIVATE project_warnings)
