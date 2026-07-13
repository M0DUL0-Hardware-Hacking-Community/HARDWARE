#include "network_catalog.hpp"

#include <cstdint>
#include <limits>

namespace wifi_scanner {
namespace {

bool valid_bssid(const Bssid& bssid) noexcept {
  bool nonzero{false};
  for (std::size_t index{0U}; index < kBssidSize; ++index) {
    nonzero = nonzero || (bssid[index] != 0U);
  }
  return nonzero && ((bssid.front() & 1U) == 0U);
}

} // namespace

CatalogUpdate observe(NetworkCatalog& catalog, const Bssid& bssid, const std::int8_t rssi,
                      const std::uint32_t sweep) noexcept {
  constexpr std::size_t kInvalidIndex{kMaximumUniqueNetworks};
  if (!valid_bssid(bssid)) {
    return {CatalogStatus::invalid_bssid, kInvalidIndex};
  }
  if (sweep == 0U) {
    return {CatalogStatus::invalid_sweep, kInvalidIndex};
  }
  if (catalog.count > kMaximumUniqueNetworks) {
    return {CatalogStatus::invalid_state, kInvalidIndex};
  }

  // ponytail: bounded linear lookup; replace only if this fixed catalog grows materially.
  for (std::size_t index{0U}; index < kMaximumUniqueNetworks; ++index) {
    if (index >= catalog.count) {
      break;
    }
    CatalogEntry& entry{catalog.entries[index]};
    if (entry.bssid == bssid) {
      if (sweep < entry.last_sweep) {
        return {CatalogStatus::invalid_sweep, kInvalidIndex};
      }
      if (entry.observation_count == std::numeric_limits<std::uint32_t>::max()) {
        return {CatalogStatus::invalid_state, kInvalidIndex};
      }
      entry.strongest_rssi = rssi > entry.strongest_rssi ? rssi : entry.strongest_rssi;
      entry.last_sweep = sweep;
      ++entry.observation_count;
      return {CatalogStatus::updated, index};
    }
  }

  if (catalog.count == kMaximumUniqueNetworks) {
    if (catalog.dropped_observations == std::numeric_limits<std::uint32_t>::max()) {
      return {CatalogStatus::invalid_state, kInvalidIndex};
    }
    ++catalog.dropped_observations;
    return {CatalogStatus::full, kInvalidIndex};
  }

  const std::size_t index{catalog.count};
  catalog.entries[index] = CatalogEntry{bssid, rssi, sweep, sweep, 1U};
  ++catalog.count;
  return {CatalogStatus::inserted, index};
}

} // namespace wifi_scanner
