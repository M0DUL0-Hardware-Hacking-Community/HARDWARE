#include "network_catalog.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static bool valid_bssid(const uint8_t bssid[WIFI_SCANNER_BSSID_SIZE]) {
  bool nonzero = false;
  size_t index;
  for (index = 0U; index < WIFI_SCANNER_BSSID_SIZE; ++index) {
    nonzero = nonzero || (bssid[index] != 0U);
  }
  return nonzero && ((bssid[0] & 1U) == 0U);
}

wifi_scanner_catalog_update_t wifi_scanner_observe(wifi_scanner_network_catalog_t* catalog,
                                                   const uint8_t bssid[WIFI_SCANNER_BSSID_SIZE],
                                                   const int8_t rssi, const uint32_t sweep) {
  const size_t invalid_index = WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS;
  size_t index;

  if ((bssid == NULL) || !valid_bssid(bssid)) {
    return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_BSSID, invalid_index};
  }
  if (sweep == 0U) {
    return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_SWEEP, invalid_index};
  }
  if ((catalog == NULL) || (catalog->count > WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS)) {
    return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_STATE, invalid_index};
  }

  /* ponytail: bounded linear lookup; replace only if this fixed catalog grows materially. */
  for (index = 0U; index < catalog->count; ++index) {
    wifi_scanner_catalog_entry_t* entry = &catalog->entries[index];
    if (memcmp(entry->bssid, bssid, WIFI_SCANNER_BSSID_SIZE) == 0) {
      if (sweep < entry->last_sweep) {
        return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_SWEEP, invalid_index};
      }
      if (entry->observation_count == UINT32_MAX) {
        return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_STATE, invalid_index};
      }
      entry->strongest_rssi = rssi > entry->strongest_rssi ? rssi : entry->strongest_rssi;
      entry->last_sweep = sweep;
      ++entry->observation_count;
      return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_UPDATED, index};
    }
  }

  if (catalog->count == WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS) {
    if (catalog->dropped_observations == UINT32_MAX) {
      return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INVALID_STATE, invalid_index};
    }
    ++catalog->dropped_observations;
    return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_FULL, invalid_index};
  }

  index = catalog->count;
  memcpy(catalog->entries[index].bssid, bssid, WIFI_SCANNER_BSSID_SIZE);
  catalog->entries[index].strongest_rssi = rssi;
  catalog->entries[index].first_sweep = sweep;
  catalog->entries[index].last_sweep = sweep;
  catalog->entries[index].observation_count = 1U;
  ++catalog->count;
  return (wifi_scanner_catalog_update_t){WIFI_SCANNER_CATALOG_INSERTED, index};
}
