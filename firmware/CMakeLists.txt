add_library(power10_firmware STATIC device_app.c)
target_include_directories(power10_firmware PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(power10_firmware PUBLIC power10 power10_platform PRIVATE project_warnings)
