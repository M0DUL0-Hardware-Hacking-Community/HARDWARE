add_executable(power10_tests bounded_statistics_test.c device_test.c)
target_link_libraries(power10_tests PRIVATE power10 project_warnings)
add_test(NAME power10_tests COMMAND power10_tests)
set_tests_properties(power10_tests PROPERTIES TIMEOUT 5)

if(TARGET power10_firmware)
  add_executable(power10_firmware_tests firmware_cycle_test.c)
  target_link_libraries(power10_firmware_tests PRIVATE power10_firmware project_warnings)
  add_test(NAME power10_firmware_tests COMMAND power10_firmware_tests)
  set_tests_properties(power10_firmware_tests PROPERTIES TIMEOUT 5)
endif()

if(TARGET power10_iot_sim)
  add_test(NAME power10_iot_sim COMMAND power10_iot_sim)
  add_test(NAME power10_iot_sim_output COMMAND power10_iot_sim)
  set_tests_properties(power10_iot_sim PROPERTIES TIMEOUT 5)
  set_tests_properties(
    power10_iot_sim_output
    PROPERTIES TIMEOUT 5
               PASS_REGULAR_EXPRESSION
               "sequence=9 temperature_mc=24500 humidity_mp=55000"
  )
endif()
