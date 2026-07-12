#include "power10/config/device_config.hpp"
#include "power10/firmware/device_app.hpp"

#include <cstdint>
#include <iostream>

int main() {
  power10::DeviceState state{};
  for (std::uint32_t cycle{0U}; cycle < power10::config::kMaximumHostCycles; ++cycle) {
    const auto status{power10::firmware::run_device_cycle(state)};
    if (status != power10::firmware::CycleStatus::ok) {
      std::cerr << "device cycle failed\n";
      return 1;
    }
  }
  return 0;
}
