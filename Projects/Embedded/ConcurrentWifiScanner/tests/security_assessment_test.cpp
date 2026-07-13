#include "network_catalog.hpp"
#include "security_assessment.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace {

struct SecurityTestCase final {
  wifi_scanner::AdvertisedSecurity advertised;
  wifi_scanner::SecurityAssessment expected;
};

constexpr std::array<SecurityTestCase, 9U> kSecurityTestCases{{
    {{wifi_scanner::Authentication::open, true, true, true},
     {wifi_scanner::RiskLevel::critical, wifi_scanner::Finding::open_network}},
    {{wifi_scanner::Authentication::wep, true, true, true},
     {wifi_scanner::RiskLevel::critical, wifi_scanner::Finding::obsolete_wep}},
    {{wifi_scanner::Authentication::legacy_wpa, true, true, true},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_wpa}},
    {{wifi_scanner::Authentication::transition, true, true, true},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_tkip}},
    {{wifi_scanner::Authentication::unknown, false, false, true},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::unknown_security}},
    {{wifi_scanner::Authentication::transition, false, true, true},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::unknown_security}},
    {{wifi_scanner::Authentication::transition, false, false, true},
     {wifi_scanner::RiskLevel::medium, wifi_scanner::Finding::transition_mode}},
    {{wifi_scanner::Authentication::modern, false, false, true},
     {wifi_scanner::RiskLevel::medium, wifi_scanner::Finding::wps_enabled}},
    {{wifi_scanner::Authentication::modern, false, false, false},
     {wifi_scanner::RiskLevel::low, wifi_scanner::Finding::none}},
}};

int test_security_assessment() {
  for (std::size_t index{0U}; index < kSecurityTestCases.size(); ++index) {
    const wifi_scanner::SecurityAssessment actual{
        wifi_scanner::assess_security(kSecurityTestCases[index].advertised)};
    if ((actual.risk != kSecurityTestCases[index].expected.risk) ||
        (actual.finding != kSecurityTestCases[index].expected.finding)) {
      return static_cast<int>(index + 1U);
    }
  }
  return 0;
}

int test_network_catalog() {
  static_assert(wifi_scanner::kMaximumUniqueNetworks <= 256U);
  constexpr std::size_t kInvalidIndex{wifi_scanner::kMaximumUniqueNetworks};
  wifi_scanner::NetworkCatalog catalog{};
  constexpr wifi_scanner::Bssid first{0x02U, 0U, 0U, 0U, 0U, 0U};

  wifi_scanner::CatalogUpdate update{wifi_scanner::observe(catalog, first, -70, 1U)};
  if ((update.status != wifi_scanner::CatalogStatus::inserted) || (update.index != 0U)) {
    return 20;
  }
  update = wifi_scanner::observe(catalog, first, -40, 2U);
  if ((update.status != wifi_scanner::CatalogStatus::updated) || (update.index != 0U) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].first_sweep != 1U) ||
      (catalog.entries[0].last_sweep != 2U) || (catalog.entries[0].observation_count != 2U)) {
    return 21;
  }
  update = wifi_scanner::observe(catalog, first, -50, 2U);
  if ((update.status != wifi_scanner::CatalogStatus::updated) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].last_sweep != 2U) ||
      (catalog.entries[0].observation_count != 3U)) {
    return 22;
  }
  update = wifi_scanner::observe(catalog, first, -80, 3U);
  if ((update.status != wifi_scanner::CatalogStatus::updated) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].last_sweep != 3U) ||
      (catalog.entries[0].observation_count != 4U)) {
    return 23;
  }

  for (std::size_t index{1U}; index < wifi_scanner::kMaximumUniqueNetworks; ++index) {
    const wifi_scanner::Bssid bssid{0x02U, 0U, 0U, 0U, 0U, static_cast<std::uint8_t>(index)};
    const wifi_scanner::CatalogUpdate inserted{wifi_scanner::observe(catalog, bssid, -60, 1U)};
    if ((inserted.status != wifi_scanner::CatalogStatus::inserted) || (inserted.index != index)) {
      return 24;
    }
  }
  constexpr wifi_scanner::Bssid overflow{0x02U, 0U, 0U, 0U, 1U, 0U};
  update = wifi_scanner::observe(catalog, overflow, -50, 4U);
  if ((update.status != wifi_scanner::CatalogStatus::full) || (update.index != kInvalidIndex) ||
      (catalog.count != wifi_scanner::kMaximumUniqueNetworks) ||
      (catalog.dropped_observations != 1U)) {
    return 25;
  }
  update = wifi_scanner::observe(catalog, first, -30, 4U);
  if ((update.status != wifi_scanner::CatalogStatus::updated) ||
      (catalog.entries[0].strongest_rssi != -30)) {
    return 26;
  }
  update = wifi_scanner::observe(catalog, first, -20, 3U);
  if ((update.status != wifi_scanner::CatalogStatus::invalid_sweep) ||
      (update.index != kInvalidIndex)) {
    return 27;
  }
  return 0;
}

int test_catalog_rejections() {
  constexpr std::size_t kInvalidIndex{wifi_scanner::kMaximumUniqueNetworks};
  constexpr wifi_scanner::Bssid valid{0x02U, 0U, 0U, 0U, 0U, 0U};
  constexpr std::array invalid_bssids{wifi_scanner::Bssid{},
                                      wifi_scanner::Bssid{0x03U, 0U, 0U, 0U, 0U, 0U}};
  wifi_scanner::NetworkCatalog catalog{};
  for (std::size_t index{0U}; index < invalid_bssids.size(); ++index) {
    const auto update{wifi_scanner::observe(catalog, invalid_bssids[index], -20, 1U)};
    if ((update.status != wifi_scanner::CatalogStatus::invalid_bssid) ||
        (update.index != kInvalidIndex) || (catalog.count != 0U)) {
      return static_cast<int>(40U + index);
    }
  }
  const auto update{wifi_scanner::observe(catalog, valid, -20, 0U)};
  if ((update.status != wifi_scanner::CatalogStatus::invalid_sweep) ||
      (update.index != kInvalidIndex) || (catalog.count != 0U)) {
    return 42;
  }
  return 0;
}

int test_catalog_overflow_guards() {
  constexpr std::size_t kInvalidIndex{wifi_scanner::kMaximumUniqueNetworks};
  constexpr wifi_scanner::Bssid valid{0x02U, 0U, 0U, 0U, 0U, 0U};
  wifi_scanner::NetworkCatalog catalog{};
  catalog.count = wifi_scanner::kMaximumUniqueNetworks + 1U;
  auto update{wifi_scanner::observe(catalog, valid, -20, 1U)};
  if ((update.status != wifi_scanner::CatalogStatus::invalid_state) ||
      (update.index != kInvalidIndex) ||
      (catalog.count != wifi_scanner::kMaximumUniqueNetworks + 1U)) {
    return 50;
  }

  catalog = {};
  catalog.count = 1U;
  catalog.entries[0] = {valid, -50, 1U, 1U, std::numeric_limits<std::uint32_t>::max()};
  update = wifi_scanner::observe(catalog, valid, -20, 1U);
  if ((update.status != wifi_scanner::CatalogStatus::invalid_state) ||
      (update.index != kInvalidIndex) ||
      (catalog.entries[0].observation_count != std::numeric_limits<std::uint32_t>::max())) {
    return 51;
  }

  catalog = {};
  catalog.count = wifi_scanner::kMaximumUniqueNetworks;
  catalog.dropped_observations = std::numeric_limits<std::uint32_t>::max();
  for (std::size_t index{0U}; index < wifi_scanner::kMaximumUniqueNetworks; ++index) {
    const wifi_scanner::Bssid bssid{0x04U, 0U, 0U, 0U, 0U, static_cast<std::uint8_t>(index)};
    catalog.entries[index] = {bssid, -50, 1U, 1U, 1U};
  }
  update = wifi_scanner::observe(catalog, valid, -20, 1U);
  if ((update.status != wifi_scanner::CatalogStatus::invalid_state) ||
      (update.index != kInvalidIndex) ||
      (catalog.dropped_observations != std::numeric_limits<std::uint32_t>::max())) {
    return 52;
  }
  return 0;
}

} // namespace

int main() {
  const int security_result{test_security_assessment()};
  if (security_result != 0) {
    return security_result;
  }
  const int catalog_result{test_network_catalog()};
  if (catalog_result != 0) {
    return catalog_result;
  }
  const int rejection_result{test_catalog_rejections()};
  return rejection_result == 0 ? test_catalog_overflow_guards() : rejection_result;
}
