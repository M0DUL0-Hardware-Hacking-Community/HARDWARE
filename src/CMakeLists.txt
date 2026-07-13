add_library(power10 STATIC bounded_statistics.c device.c)
target_include_directories(power10 PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(power10 PRIVATE project_warnings)
