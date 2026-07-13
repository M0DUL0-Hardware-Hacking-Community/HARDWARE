#include "power10/platform/device_io.hpp"

#include <cinttypes>
#include <cstdint>
#include <cstdio>

namespace power10::platform {

IoStatus read_sensor(SensorReading& reading) noexcept {
  constexpr std::int32_t kSimulatedTemperature{24'500};
  constexpr std::uint32_t kSimulatedHumidity{55'000U};
  reading = SensorReading{kSimulatedTemperature, kSimulatedHumidity};
  return IoStatus::ok;
}

IoStatus publish_telemetry(const Telemetry& telemetry) noexcept {
  const int written{std::printf("sequence=%" PRIu32 " temperature_mc=%" PRId32
                                " humidity_mp=%" PRIu32 "\n",
                                telemetry.sequence, telemetry.reading.temperature_millicelsius,
                                telemetry.reading.humidity_millipercent)};
  if ((written < 0) || (std::fflush(stdout) != 0)) {
    return IoStatus::transport_unavailable;
  }
  return IoStatus::ok;
}

} // namespace power10::platform
