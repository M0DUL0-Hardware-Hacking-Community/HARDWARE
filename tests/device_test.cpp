#include "device_test.hpp"

#include "power10/device.hpp"

#include <cstdint>
#include <iostream>
#include <limits>

namespace {

int report_failure(const char* const message) {
  if (message == nullptr) {
    return 1;
  }
  std::cerr << message << '\n';
  return 1;
}

}  // namespace

int run_device_tests() {
  power10::DeviceState state{};
  power10::Telemetry telemetry{};
  constexpr power10::SensorReading valid{25'000, 50'000U};
  if (power10::prepare_telemetry(valid, state, telemetry) != power10::DeviceStatus::ok) {
    return report_failure("valid sensor data was rejected");
  }
  if ((telemetry.sequence != 0U) || (state.next_sequence != 1U)) {
    return report_failure("telemetry sequence was not advanced");
  }

  constexpr power10::SensorReading invalid_temperature{125'001, 50'000U};
  if (power10::prepare_telemetry(invalid_temperature, state, telemetry) !=
      power10::DeviceStatus::invalid_temperature) {
    return report_failure("invalid temperature was accepted");
  }

  state.next_sequence = std::numeric_limits<std::uint32_t>::max();
  if (power10::prepare_telemetry(valid, state, telemetry) !=
      power10::DeviceStatus::sequence_exhausted) {
    return report_failure("exhausted sequence was accepted");
  }
  return 0;
}
