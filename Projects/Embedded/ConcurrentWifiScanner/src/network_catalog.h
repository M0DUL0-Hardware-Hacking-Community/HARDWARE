#ifndef WIFI_SCANNER_NETWORK_CATALOG_H
#define WIFI_SCANNER_NETWORK_CATALOG_H

#include <stddef.h>
#include <stdint.h>

enum {
  WIFI_SCANNER_BSSID_SIZE = 6,
  /* ponytail: fixed RAM ceiling; raise only when field logs report capacity limits. */
  WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS = 128
};

typedef uint8_t wifi_scanner_bssid_t[WIFI_SCANNER_BSSID_SIZE];

typedef enum {
  WIFI_SCANNER_CATALOG_INSERTED,
  WIFI_SCANNER_CATALOG_UPDATED,
  WIFI_SCANNER_CATALOG_FULL,
  WIFI_SCANNER_CATALOG_INVALID_BSSID,
  WIFI_SCANNER_CATALOG_INVALID_SWEEP,
  WIFI_SCANNER_CATALOG_INVALID_STATE
} wifi_scanner_catalog_status_t;

typedef struct {
  wifi_scanner_catalog_status_t status;
  size_t index;
} wifi_scanner_catalog_update_t;

typedef struct {
  wifi_scanner_bssid_t bssid;
  int8_t strongest_rssi;
  uint32_t first_sweep;
  uint32_t last_sweep;
  uint32_t observation_count;
} wifi_scanner_catalog_entry_t;

typedef struct {
  wifi_scanner_catalog_entry_t entries[WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS];
  size_t count;
  uint32_t dropped_observations;
} wifi_scanner_network_catalog_t;

wifi_scanner_catalog_update_t wifi_scanner_observe(wifi_scanner_network_catalog_t* catalog,
                                                   const uint8_t bssid[WIFI_SCANNER_BSSID_SIZE],
                                                   int8_t rssi, uint32_t sweep);

#endif
