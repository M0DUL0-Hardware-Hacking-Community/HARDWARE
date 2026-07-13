#ifndef WIFI_SCANNER_NETWORK_CATALOG_HPP
#define WIFI_SCANNER_NETWORK_CATALOG_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace wifi_scanner {

inline constexpr std::size_t kBssidSize{6U};
// ponytail: fixed RAM ceiling; raise only when field logs report capacity limits.
inline constexpr std::size_t kMaximumUniqueNetworks{128U};

using Bssid = std::array<std::uint8_t, kBssidSize>;

enum class CatalogStatus : std::uint8_t {
  inserted,
  updated,
  full,
  invalid_bssid,
  invalid_sweep,
  invalid_state
};

struct CatalogUpdate final {
  CatalogStatus status;
  std::size_t index;
};

struct CatalogEntry final {
  Bssid bssid;
  std::int8_t strongest_rssi;
  std::uint32_t first_sweep;
  std::uint32_t last_sweep;
  std::uint32_t observation_count;
};

struct NetworkCatalog final {
  std::array<CatalogEntry, kMaximumUniqueNetworks> entries;
  std::size_t count;
  std::uint32_t dropped_observations;
};

[[nodiscard]] CatalogUpdate observe(NetworkCatalog& catalog, const Bssid& bssid, std::int8_t rssi,
                                    std::uint32_t sweep) noexcept;

} // namespace wifi_scanner

#endif
