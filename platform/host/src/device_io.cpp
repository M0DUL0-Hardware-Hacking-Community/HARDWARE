#include "power10/platform/device_io.hpp"

#include <cstdint>
#include <iostream>

namespace power10::platform {

IoStatus read_sensor(SensorReading& reading) noexcept {
  constexpr std::int32_t kSimulatedTemperature{24'500};
  constexpr std::uint32_t kSimulatedHumidity{55'000U};
  reading = SensorReading{kSimulatedTemperature, kSimulatedHumidity};
  return IoStatus::ok;
}

IoStatus publish_telemetry(const Telemetry& telemetry) noexcept {
  std::cout << "sequence=" << telemetry.sequence
            << " temperature_mc=" << telemetry.reading.temperature_millicelsius
            << " humidity_mp=" << telemetry.reading.humidity_millipercent << '\n';
  if (!std::cout) {
    return IoStatus::transport_unavailable;
  }
  return IoStatus::ok;
}

} // namespace power10::platform
