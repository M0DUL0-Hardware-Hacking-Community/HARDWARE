#include "network_catalog.h"
#include "security_assessment.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  wifi_scanner_advertised_security_t advertised;
  wifi_scanner_security_assessment_t expected;
} security_test_case_t;

static const security_test_case_t SECURITY_TEST_CASES[] = {
    {{WIFI_SCANNER_AUTHENTICATION_OPEN, true, true, true},
     {WIFI_SCANNER_RISK_CRITICAL, WIFI_SCANNER_FINDING_OPEN_NETWORK}},
    {{WIFI_SCANNER_AUTHENTICATION_WEP, true, true, true},
     {WIFI_SCANNER_RISK_CRITICAL, WIFI_SCANNER_FINDING_OBSOLETE_WEP}},
    {{WIFI_SCANNER_AUTHENTICATION_LEGACY_WPA, true, true, true},
     {WIFI_SCANNER_RISK_HIGH, WIFI_SCANNER_FINDING_LEGACY_WPA}},
    {{WIFI_SCANNER_AUTHENTICATION_TRANSITION, true, true, true},
     {WIFI_SCANNER_RISK_HIGH, WIFI_SCANNER_FINDING_LEGACY_TKIP}},
    {{WIFI_SCANNER_AUTHENTICATION_UNKNOWN, false, false, true},
     {WIFI_SCANNER_RISK_HIGH, WIFI_SCANNER_FINDING_UNKNOWN_SECURITY}},
    {{WIFI_SCANNER_AUTHENTICATION_TRANSITION, false, true, true},
     {WIFI_SCANNER_RISK_HIGH, WIFI_SCANNER_FINDING_UNKNOWN_SECURITY}},
    {{WIFI_SCANNER_AUTHENTICATION_TRANSITION, false, false, true},
     {WIFI_SCANNER_RISK_MEDIUM, WIFI_SCANNER_FINDING_TRANSITION_MODE}},
    {{WIFI_SCANNER_AUTHENTICATION_MODERN, false, false, true},
     {WIFI_SCANNER_RISK_MEDIUM, WIFI_SCANNER_FINDING_WPS_ENABLED}},
    {{WIFI_SCANNER_AUTHENTICATION_MODERN, false, false, false},
     {WIFI_SCANNER_RISK_LOW, WIFI_SCANNER_FINDING_NONE}},
    {{(wifi_scanner_authentication_t)99, false, false, false},
     {WIFI_SCANNER_RISK_HIGH, WIFI_SCANNER_FINDING_UNKNOWN_SECURITY}},
};

static int test_security_assessment(void) {
  size_t index;
  for (index = 0U; index < (sizeof(SECURITY_TEST_CASES) / sizeof(SECURITY_TEST_CASES[0]));
       ++index) {
    const wifi_scanner_security_assessment_t actual =
        wifi_scanner_assess_security(SECURITY_TEST_CASES[index].advertised);
    if ((actual.risk != SECURITY_TEST_CASES[index].expected.risk) ||
        (actual.finding != SECURITY_TEST_CASES[index].expected.finding)) {
      return (int)(index + 1U);
    }
  }
  return 0;
}

static int test_network_catalog(void) {
  const size_t invalid_index = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS;
  const wifi_scanner_bssid_t first = {0x02U, 0U, 0U, 0U, 0U, 0U};
  const wifi_scanner_bssid_t overflow = {0x02U, 0U, 0U, 0U, 1U, 0U};
  wifi_scanner_network_catalog_t catalog = {0};
  wifi_scanner_catalog_update_t update;
  size_t index;
  _Static_assert(WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS <= 256,
                 "test BSSID generator needs one-byte indices");

  update = wifi_scanner_observe(&catalog, first, -70, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INSERTED) || (update.index != 0U)) {
    return 20;
  }
  update = wifi_scanner_observe(&catalog, first, -40, 2U);
  if ((update.status != WIFI_SCANNER_CATALOG_UPDATED) || (update.index != 0U) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].first_sweep != 1U) ||
      (catalog.entries[0].last_sweep != 2U) || (catalog.entries[0].observation_count != 2U)) {
    return 21;
  }
  update = wifi_scanner_observe(&catalog, first, -50, 2U);
  if ((update.status != WIFI_SCANNER_CATALOG_UPDATED) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].last_sweep != 2U) ||
      (catalog.entries[0].observation_count != 3U)) {
    return 22;
  }
  update = wifi_scanner_observe(&catalog, first, -80, 3U);
  if ((update.status != WIFI_SCANNER_CATALOG_UPDATED) ||
      (catalog.entries[0].strongest_rssi != -40) || (catalog.entries[0].last_sweep != 3U) ||
      (catalog.entries[0].observation_count != 4U)) {
    return 23;
  }

  for (index = 1U; index < WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS; ++index) {
    wifi_scanner_bssid_t bssid = {0x02U, 0U, 0U, 0U, 0U, 0U};
    wifi_scanner_catalog_update_t inserted;
    bssid[5] = (uint8_t)index;
    inserted = wifi_scanner_observe(&catalog, bssid, -60, 1U);
    if ((inserted.status != WIFI_SCANNER_CATALOG_INSERTED) || (inserted.index != index)) {
      return 24;
    }
  }
  update = wifi_scanner_observe(&catalog, overflow, -50, 4U);
  if ((update.status != WIFI_SCANNER_CATALOG_FULL) || (update.index != invalid_index) ||
      (catalog.count != WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS) ||
      (catalog.dropped_observations != 1U)) {
    return 25;
  }
  update = wifi_scanner_observe(&catalog, first, -30, 4U);
  if ((update.status != WIFI_SCANNER_CATALOG_UPDATED) ||
      (catalog.entries[0].strongest_rssi != -30)) {
    return 26;
  }
  update = wifi_scanner_observe(&catalog, first, -20, 3U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_SWEEP) || (update.index != invalid_index)) {
    return 27;
  }
  return 0;
}

static int test_catalog_rejections(void) {
  const size_t invalid_index = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS;
  const wifi_scanner_bssid_t valid = {0x02U, 0U, 0U, 0U, 0U, 0U};
  const wifi_scanner_bssid_t invalid_bssids[] = {{0}, {0x03U, 0U, 0U, 0U, 0U, 0U}};
  wifi_scanner_network_catalog_t catalog = {0};
  wifi_scanner_catalog_update_t update;
  size_t index;
  for (index = 0U; index < (sizeof(invalid_bssids) / sizeof(invalid_bssids[0])); ++index) {
    update = wifi_scanner_observe(&catalog, invalid_bssids[index], -20, 1U);
    if ((update.status != WIFI_SCANNER_CATALOG_INVALID_BSSID) || (update.index != invalid_index) ||
        (catalog.count != 0U)) {
      return (int)(40U + index);
    }
  }
  update = wifi_scanner_observe(&catalog, valid, -20, 0U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_SWEEP) || (update.index != invalid_index) ||
      (catalog.count != 0U)) {
    return 42;
  }
  update = wifi_scanner_observe(NULL, valid, -20, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_STATE) || (update.index != invalid_index)) {
    return 43;
  }
  update = wifi_scanner_observe(&catalog, NULL, -20, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_BSSID) || (update.index != invalid_index)) {
    return 44;
  }
  return 0;
}

static int test_catalog_overflow_guards(void) {
  const size_t invalid_index = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS;
  const wifi_scanner_bssid_t valid = {0x02U, 0U, 0U, 0U, 0U, 0U};
  wifi_scanner_network_catalog_t catalog = {0};
  wifi_scanner_catalog_update_t update;
  size_t index;
  catalog.count = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS + 1U;
  update = wifi_scanner_observe(&catalog, valid, -20, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_STATE) || (update.index != invalid_index) ||
      (catalog.count != WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS + 1U)) {
    return 50;
  }

  catalog = (wifi_scanner_network_catalog_t){0};
  catalog.count = 1U;
  memcpy(catalog.entries[0].bssid, valid, WIFI_SCANNER_BSSID_SIZE);
  catalog.entries[0].strongest_rssi = -50;
  catalog.entries[0].first_sweep = 1U;
  catalog.entries[0].last_sweep = 1U;
  catalog.entries[0].observation_count = UINT32_MAX;
  update = wifi_scanner_observe(&catalog, valid, -20, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_STATE) || (update.index != invalid_index) ||
      (catalog.entries[0].observation_count != UINT32_MAX)) {
    return 51;
  }

  catalog = (wifi_scanner_network_catalog_t){0};
  catalog.count = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS;
  catalog.dropped_observations = UINT32_MAX;
  for (index = 0U; index < WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS; ++index) {
    catalog.entries[index].bssid[0] = 0x04U;
    catalog.entries[index].bssid[5] = (uint8_t)index;
  }
  update = wifi_scanner_observe(&catalog, valid, -20, 1U);
  if ((update.status != WIFI_SCANNER_CATALOG_INVALID_STATE) || (update.index != invalid_index) ||
      (catalog.dropped_observations != UINT32_MAX)) {
    return 52;
  }
  return 0;
}

int main(void) {
  const int security_result = test_security_assessment();
  int result;
  if (security_result != 0) {
    return security_result;
  }
  result = test_network_catalog();
  if (result != 0) {
    return result;
  }
  result = test_catalog_rejections();
  return result == 0 ? test_catalog_overflow_guards() : result;
}
