#ifndef POWER10_CONFIG_DEVICE_CONFIG_HPP
#define POWER10_CONFIG_DEVICE_CONFIG_HPP

#include <cstddef>
#include <cstdint>

namespace power10::config {

inline constexpr std::size_t kDeviceIdCapacity{24U};
inline constexpr std::uint32_t kPublishPeriodMilliseconds{1'000U};
inline constexpr std::uint32_t kMaximumHostCycles{10U};

}  // namespace power10::config

#endif
