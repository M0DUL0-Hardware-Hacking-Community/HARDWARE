#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "network_catalog.h"
#include "nvs_flash.h"
#include "security_assessment.h"

static const char LOG_TAG[] = "wifi_scanner";
static const char COUNTRY_CODE[] = "MY";
static const gpio_num_t ACTIVITY_LED = GPIO_NUM_2;

enum {
  SCAN_SWEEP_COUNT = 3,
  PASSIVE_DWELL_MILLISECONDS = 360,
  INTER_SWEEP_DELAY_MILLISECONDS = 5000,
  LED_TICK_MILLISECONDS = 150,
  LED_DEADLINE_MILLISECONDS = 60000,
  LED_SHUTDOWN_TIMEOUT_MILLISECONDS = 5000,
  LED_STACK_DEPTH = 3072,
  MAXIMUM_LED_TICKS = (LED_DEADLINE_MILLISECONDS / LED_TICK_MILLISECONDS) + 1,
  ESCAPED_SSID_SIZE = 129
};

static const EventBits_t SCANNING_BIT = 1U << 0U;
static const EventBits_t FAILURE_BIT = 1U << 1U;
static const EventBits_t RUN_DONE_BIT = 1U << 2U;
static const EventBits_t LED_DONE_BIT = 1U << 3U;

_Static_assert(sizeof(COUNTRY_CODE) == 3U, "country code must contain two characters");
_Static_assert(SCAN_SWEEP_COUNT > 0, "at least one scan sweep is required");
_Static_assert(PASSIVE_DWELL_MILLISECONDS > 0, "passive dwell must be positive");
_Static_assert(PASSIVE_DWELL_MILLISECONDS <= 1500, "passive dwell exceeds ESP-IDF limit");
_Static_assert(LED_TICK_MILLISECONDS > 0, "LED tick must be positive");
_Static_assert(SCAN_SWEEP_COUNT <= (UINT32_MAX / UINT16_MAX), "observation count can overflow");

typedef struct {
  wifi_scanner_network_catalog_t catalog;
  wifi_ap_record_t records[WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS];
  wifi_country_t configured_country;
  uint32_t completed_sweeps;
  uint32_t detected_observations;
  uint32_t retrieved_observations;
  uint32_t duplicate_observations;
  uint32_t invalid_observations;
  bool country_valid;
} scan_results_t;

typedef struct {
  EventGroupHandle_t events;
} led_context_t;

typedef struct {
  StaticEventGroup_t event_control;
  StaticTask_t led_control;
  StackType_t led_stack[LED_STACK_DEPTH];
  led_context_t led_context;
  scan_results_t results;
} runtime_storage_t;

typedef struct {
  esp_netif_t* station_netif;
  bool event_loop_created;
  bool wifi_initialized;
  bool wifi_started;
} wifi_resources_t;

static const char* risk_name(const wifi_scanner_risk_level_t risk) {
  switch (risk) {
  case WIFI_SCANNER_RISK_LOW:
    return "low";
  case WIFI_SCANNER_RISK_MEDIUM:
    return "medium";
  case WIFI_SCANNER_RISK_HIGH:
    return "high";
  case WIFI_SCANNER_RISK_CRITICAL:
    return "critical";
  }
  return "invalid";
}

static const char* finding_name(const wifi_scanner_finding_t finding) {
  switch (finding) {
  case WIFI_SCANNER_FINDING_NONE:
    return "no obvious beacon-level issue";
  case WIFI_SCANNER_FINDING_OPEN_NETWORK:
    return "unencrypted open network";
  case WIFI_SCANNER_FINDING_OBSOLETE_WEP:
    return "obsolete WEP security";
  case WIFI_SCANNER_FINDING_LEGACY_WPA:
    return "legacy WPA permitted";
  case WIFI_SCANNER_FINDING_TRANSITION_MODE:
    return "WPA2/WPA3 transition mode";
  case WIFI_SCANNER_FINDING_LEGACY_TKIP:
    return "legacy TKIP cipher advertised";
  case WIFI_SCANNER_FINDING_WPS_ENABLED:
    return "WPS capability advertised";
  case WIFI_SCANNER_FINDING_UNKNOWN_SECURITY:
    return "unknown authentication or cipher";
  }
  return "invalid assessment";
}

static wifi_scanner_authentication_t map_authentication(const wifi_auth_mode_t mode) {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return WIFI_SCANNER_AUTHENTICATION_OPEN;
  case WIFI_AUTH_WEP:
    return WIFI_SCANNER_AUTHENTICATION_WEP;
  case WIFI_AUTH_WPA_PSK:
  case WIFI_AUTH_WPA_WPA2_PSK:
  case WIFI_AUTH_WPA_ENTERPRISE:
    return WIFI_SCANNER_AUTHENTICATION_LEGACY_WPA;
  case WIFI_AUTH_WPA2_WPA3_PSK:
  case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
    return WIFI_SCANNER_AUTHENTICATION_TRANSITION;
  case WIFI_AUTH_WPA2_PSK:
  case WIFI_AUTH_WPA2_ENTERPRISE:
  case WIFI_AUTH_WPA3_PSK:
  case WIFI_AUTH_WAPI_PSK:
  case WIFI_AUTH_OWE:
  case WIFI_AUTH_WPA3_ENT_192:
  case WIFI_AUTH_DPP:
  case WIFI_AUTH_WPA3_ENTERPRISE:
    return WIFI_SCANNER_AUTHENTICATION_MODERN;
  default:
    return WIFI_SCANNER_AUTHENTICATION_UNKNOWN;
  }
}

static bool is_legacy_tkip(const wifi_cipher_type_t cipher) {
  return (cipher == WIFI_CIPHER_TYPE_TKIP) || (cipher == WIFI_CIPHER_TYPE_TKIP_CCMP);
}

static bool is_unknown_cipher(const wifi_cipher_type_t cipher) {
  return cipher == WIFI_CIPHER_TYPE_UNKNOWN;
}

static wifi_scanner_advertised_security_t
make_advertised_security(const wifi_ap_record_t* access_point) {
  bool legacy_tkip;
  bool unknown_cipher;
  configASSERT(access_point != NULL);
  legacy_tkip =
      is_legacy_tkip(access_point->pairwise_cipher) || is_legacy_tkip(access_point->group_cipher);
  unknown_cipher = is_unknown_cipher(access_point->pairwise_cipher) ||
                   is_unknown_cipher(access_point->group_cipher);
  return (wifi_scanner_advertised_security_t){map_authentication(access_point->authmode),
                                              legacy_tkip, unknown_cipher, access_point->wps != 0U};
}

static const char* authentication_name(const wifi_auth_mode_t mode) {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return "open";
  case WIFI_AUTH_WEP:
    return "WEP";
  case WIFI_AUTH_WPA_PSK:
    return "WPA personal";
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2 personal";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA/WPA2 personal";
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2 enterprise";
  case WIFI_AUTH_WPA3_PSK:
    return "WPA3 personal";
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return "WPA2/WPA3 personal";
  case WIFI_AUTH_WAPI_PSK:
    return "WAPI";
  case WIFI_AUTH_OWE:
    return "OWE";
  case WIFI_AUTH_WPA3_ENT_192:
    return "WPA3 enterprise 192-bit";
  case WIFI_AUTH_DPP:
    return "DPP";
  case WIFI_AUTH_WPA3_ENTERPRISE:
    return "WPA3 enterprise";
  case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
    return "WPA2/WPA3 enterprise";
  case WIFI_AUTH_WPA_ENTERPRISE:
    return "WPA enterprise";
  case WIFI_AUTH_DUMMY_1:
  case WIFI_AUTH_DUMMY_2:
  case WIFI_AUTH_MAX:
    return "reserved";
  }
  return "unknown";
}

static const char* cipher_name(const wifi_cipher_type_t cipher) {
  switch (cipher) {
  case WIFI_CIPHER_TYPE_NONE:
    return "none";
  case WIFI_CIPHER_TYPE_WEP40:
    return "WEP40";
  case WIFI_CIPHER_TYPE_WEP104:
    return "WEP104";
  case WIFI_CIPHER_TYPE_TKIP:
    return "TKIP";
  case WIFI_CIPHER_TYPE_CCMP:
    return "CCMP";
  case WIFI_CIPHER_TYPE_TKIP_CCMP:
    return "TKIP/CCMP";
  case WIFI_CIPHER_TYPE_AES_CMAC128:
    return "AES-CMAC-128";
  case WIFI_CIPHER_TYPE_SMS4:
    return "SMS4";
  case WIFI_CIPHER_TYPE_GCMP:
    return "GCMP";
  case WIFI_CIPHER_TYPE_GCMP256:
    return "GCMP-256";
  case WIFI_CIPHER_TYPE_AES_GMAC128:
    return "AES-GMAC-128";
  case WIFI_CIPHER_TYPE_AES_GMAC256:
    return "AES-GMAC-256";
  case WIFI_CIPHER_TYPE_UNKNOWN:
    return "unknown";
  }
  return "invalid";
}

static const char* secondary_channel_name(const wifi_second_chan_t channel) {
  switch (channel) {
  case WIFI_SECOND_CHAN_NONE:
    return "none";
  case WIFI_SECOND_CHAN_ABOVE:
    return "above";
  case WIFI_SECOND_CHAN_BELOW:
    return "below";
  }
  return "invalid";
}

static const char* bandwidth_name(const wifi_bandwidth_t bandwidth) {
  switch (bandwidth) {
  case WIFI_BW20:
    return "20MHz";
  case WIFI_BW40:
    return "40MHz";
  case WIFI_BW80:
    return "80MHz";
  case WIFI_BW160:
    return "160MHz";
  case WIFI_BW80_BW80:
    return "80+80MHz";
  }
  return "invalid";
}

static const char* antenna_name(const wifi_ant_t antenna) {
  switch (antenna) {
  case WIFI_ANT_ANT0:
    return "antenna0";
  case WIFI_ANT_ANT1:
    return "antenna1";
  case WIFI_ANT_MAX:
    return "unknown";
  }
  return "invalid";
}

static void escape_ssid(const wifi_ap_record_t* access_point, char output[ESCAPED_SSID_SIZE]) {
  static const char HEX_DIGITS[] = "0123456789ABCDEF";
  size_t output_index = 0U;
  size_t input_index;
  configASSERT(access_point != NULL);
  configASSERT(output != NULL);
  _Static_assert(sizeof(access_point->ssid) == 33U, "unexpected ESP-IDF SSID size");

  for (input_index = 0U; input_index < 32U; ++input_index) {
    const uint8_t byte = access_point->ssid[input_index];
    if (byte == 0U) {
      break;
    }
    if ((byte >= 32U) && (byte <= 126U) && (byte != (uint8_t)'\\')) {
      output[output_index] = (char)byte;
      ++output_index;
    } else {
      output[output_index] = '\\';
      output[output_index + 1U] = 'x';
      output[output_index + 2U] = HEX_DIGITS[byte >> 4U];
      output[output_index + 3U] = HEX_DIGITS[byte & 0x0FU];
      output_index += 4U;
    }
  }
  configASSERT(output_index < ESCAPED_SSID_SIZE);
  output[output_index] = '\0';
}

static void set_event_bits(const EventGroupHandle_t events, const EventBits_t bits) {
  configASSERT(events != NULL);
  configASSERT(bits != 0U);
  (void)xEventGroupSetBits(events, bits); /* Returned bits are a racy cross-core snapshot. */
}

static void clear_event_bits(const EventGroupHandle_t events, const EventBits_t bits) {
  configASSERT(events != NULL);
  configASSERT(bits != 0U);
  (void)xEventGroupClearBits(events, bits); /* Only app_main owns these level bits. */
}

static esp_err_t set_led(const bool enabled) {
  return gpio_set_level(ACTIVITY_LED, enabled ? 1U : 0U);
}

static esp_err_t initialize_led(void) {
  esp_err_t status = gpio_reset_pin(ACTIVITY_LED);
  if (status == ESP_OK) {
    status = gpio_set_direction(ACTIVITY_LED, GPIO_MODE_OUTPUT);
  }
  if (status == ESP_OK) {
    status = set_led(false);
  }
  return status;
}

static esp_err_t process_record(const wifi_ap_record_t* access_point, const uint32_t sweep,
                                scan_results_t* results) {
  wifi_scanner_catalog_update_t update;
  configASSERT(access_point != NULL);
  configASSERT(results != NULL);
  update = wifi_scanner_observe(&results->catalog, access_point->bssid, access_point->rssi, sweep);
  switch (update.status) {
  case WIFI_SCANNER_CATALOG_INSERTED:
  case WIFI_SCANNER_CATALOG_UPDATED:
    if (update.index >= WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS) {
      return ESP_ERR_INVALID_STATE;
    }
    results->records[update.index] = *access_point;
    if (update.status == WIFI_SCANNER_CATALOG_UPDATED) {
      ++results->duplicate_observations;
    }
    return ESP_OK;
  case WIFI_SCANNER_CATALOG_FULL:
    return ESP_OK;
  case WIFI_SCANNER_CATALOG_INVALID_BSSID:
    ++results->invalid_observations;
    return ESP_OK;
  case WIFI_SCANNER_CATALOG_INVALID_SWEEP:
  case WIFI_SCANNER_CATALOG_INVALID_STATE:
    return ESP_ERR_INVALID_STATE;
  }
  return ESP_ERR_INVALID_STATE;
}

static esp_err_t collect_scan_results(const uint32_t sweep, scan_results_t* results) {
  uint16_t detected = 0U;
  esp_err_t status;
  configASSERT(results != NULL);
  status = esp_wifi_scan_get_ap_num(&detected);
  if (status == ESP_OK) {
    uint16_t index;
    results->detected_observations += detected;
    for (index = 0U; index < detected; ++index) {
      wifi_ap_record_t access_point = {0};
      status = esp_wifi_scan_get_ap_record(&access_point);
      if (status != ESP_OK) {
        break;
      }
      ++results->retrieved_observations;
      status = process_record(&access_point, sweep, results);
      if (status != ESP_OK) {
        break;
      }
    }
  }

  {
    const esp_err_t clear_status = esp_wifi_clear_ap_list();
    if (clear_status != ESP_OK) {
      ESP_LOGE(LOG_TAG, "AP-list cleanup failed: %s", esp_err_to_name(clear_status));
    }
    return status == ESP_OK ? clear_status : status;
  }
}

static esp_err_t perform_scan(const uint32_t sweep, scan_results_t* results) {
  wifi_scan_config_t configuration = {0};
  esp_err_t status;
  configASSERT((sweep > 0U) && (sweep <= SCAN_SWEEP_COUNT));
  configASSERT(results != NULL);
  configuration.show_hidden = true;
  configuration.scan_type = WIFI_SCAN_TYPE_PASSIVE;
  configuration.scan_time.passive = PASSIVE_DWELL_MILLISECONDS;
  status = esp_wifi_scan_start(&configuration, true);
  if (status != ESP_OK) {
    return status;
  }
  return collect_scan_results(sweep, results);
}

static esp_err_t run_scans(const EventGroupHandle_t events, scan_results_t* results) {
  esp_err_t status = ESP_OK;
  const TickType_t inter_sweep_delay = pdMS_TO_TICKS(INTER_SWEEP_DELAY_MILLISECONDS);
  uint32_t sweep;
  configASSERT(events != NULL);
  configASSERT(results != NULL);
  for (sweep = 1U; (sweep <= SCAN_SWEEP_COUNT) && (status == ESP_OK); ++sweep) {
    const uint32_t detected_before = results->detected_observations;
    ESP_LOGI(LOG_TAG, "Passive sweep %" PRIu32 "/%" PRIu32 " started", sweep,
             (uint32_t)SCAN_SWEEP_COUNT);
    set_event_bits(events, SCANNING_BIT);
    status = perform_scan(sweep, results);
    clear_event_bits(events, SCANNING_BIT);
    if (status == ESP_OK) {
      ++results->completed_sweeps;
      ESP_LOGI(LOG_TAG, "Passive sweep %" PRIu32 " observed %" PRIu32 " BSS record(s)", sweep,
               results->detected_observations - detected_before);
    }
    if ((xEventGroupGetBits(events) & FAILURE_BIT) != 0U) {
      status = ESP_FAIL;
    }
    if ((status == ESP_OK) && (sweep < SCAN_SWEEP_COUNT)) {
      vTaskDelay(inter_sweep_delay);
    }
  }
  return status;
}

static void log_observation(const size_t index, const wifi_scanner_catalog_entry_t* observed,
                            const wifi_ap_record_t* access_point) {
  char ssid[ESCAPED_SSID_SIZE];
  const char* displayed_ssid;
  wifi_scanner_security_assessment_t assessment;
  configASSERT(index < WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS);
  configASSERT(observed != NULL);
  configASSERT(access_point != NULL);
  configASSERT(access_point->primary <= 14U);
  escape_ssid(access_point, ssid);
  displayed_ssid = ssid[0] == '\0' ? "<hidden>" : ssid;
  assessment = wifi_scanner_assess_security(make_advertised_security(access_point));

  ESP_LOGI(LOG_TAG,
           "network=%u bssid=" MACSTR " ssid=%s strongest_rssi=%d latest_rssi=%d "
           "first_sweep=%" PRIu32 " last_sweep=%" PRIu32 " sightings=%" PRIu32,
           (unsigned)(index + 1U), MAC2STR(access_point->bssid), displayed_ssid,
           (int)observed->strongest_rssi, (int)access_point->rssi, observed->first_sweep,
           observed->last_sweep, observed->observation_count);
  ESP_LOGI(LOG_TAG,
           "network=%u rf primary=%u secondary=%s bandwidth=%s antenna=%s auth=%s pairwise=%s "
           "group=%s WPS=%s risk=%s finding=%s",
           (unsigned)(index + 1U), (unsigned)access_point->primary,
           secondary_channel_name(access_point->second), bandwidth_name(access_point->bandwidth),
           antenna_name(access_point->ant), authentication_name(access_point->authmode),
           cipher_name(access_point->pairwise_cipher), cipher_name(access_point->group_cipher),
           access_point->wps != 0U ? "yes" : "no", risk_name(assessment.risk),
           finding_name(assessment.finding));
}

static void log_capabilities(const size_t index, const wifi_ap_record_t* access_point) {
  char advertised_country[3];
  bool country_absent;
  bool country_valid;
  const char* displayed_country;
  configASSERT(index < WIFI_SCANNER_MAXIMUM_UNIQUE_NETWORKS);
  configASSERT(access_point != NULL);
  advertised_country[0] = access_point->country.cc[0];
  advertised_country[1] = access_point->country.cc[1];
  advertised_country[2] = '\0';
  country_absent = (advertised_country[0] == '\0') && (advertised_country[1] == '\0');
  country_valid = (advertised_country[0] >= 'A') && (advertised_country[0] <= 'Z') &&
                  (advertised_country[1] >= 'A') && (advertised_country[1] <= 'Z');
  displayed_country =
      country_absent ? "not-advertised" : (country_valid ? advertised_country : "invalid");

  ESP_LOGI(LOG_TAG,
           "network=%u capabilities 11b=%u 11g=%u 11n=%u 11a=%u 11ac=%u 11ax=%u lr=%u "
           "FTM_responder=%u FTM_initiator=%u VHT_center1=%u VHT_center2=%u",
           (unsigned)(index + 1U), (unsigned)access_point->phy_11b, (unsigned)access_point->phy_11g,
           (unsigned)access_point->phy_11n, (unsigned)access_point->phy_11a,
           (unsigned)access_point->phy_11ac, (unsigned)access_point->phy_11ax,
           (unsigned)access_point->phy_lr, (unsigned)access_point->ftm_responder,
           (unsigned)access_point->ftm_initiator, (unsigned)access_point->vht_ch_freq1,
           (unsigned)access_point->vht_ch_freq2);
  ESP_LOGI(LOG_TAG,
           "network=%u advertised_country=%s advertised_country_start=%u "
           "advertised_country_channels=%u advertised_country_max_power=%d HE_color=%u "
           "HE_partial=%u HE_disabled=%u HE_bssid_index=%u",
           (unsigned)(index + 1U), displayed_country, (unsigned)access_point->country.schan,
           (unsigned)access_point->country.nchan, (int)access_point->country.max_tx_power,
           (unsigned)access_point->he_ap.bss_color, (unsigned)access_point->he_ap.partial_bss_color,
           (unsigned)access_point->he_ap.bss_color_disabled,
           (unsigned)access_point->he_ap.bssid_index);
}

static size_t count_hidden_networks(const scan_results_t* results) {
  size_t hidden_count = 0U;
  size_t index;
  configASSERT(results != NULL);
  for (index = 0U; index < results->catalog.count; ++index) {
    if (results->records[index].ssid[0] == 0U) {
      ++hidden_count;
    }
  }
  return hidden_count;
}

static const char* report_state(const esp_err_t status, const scan_results_t* results) {
  configASSERT(results != NULL);
  if ((status != ESP_OK) || (results->completed_sweeps != SCAN_SWEEP_COUNT) ||
      (results->invalid_observations != 0U)) {
    return "incomplete";
  }
  if (results->catalog.dropped_observations != 0U) {
    return "capacity-limited";
  }
  return "complete";
}

static void log_report(const esp_err_t status, const scan_results_t* results) {
  char country0;
  char country1;
  unsigned first_channel;
  unsigned channel_count;
  unsigned last_channel;
  size_t index;
  configASSERT(results != NULL);
  country0 = results->country_valid ? results->configured_country.cc[0] : '?';
  country1 = results->country_valid ? results->configured_country.cc[1] : '?';
  first_channel = results->country_valid ? (unsigned)results->configured_country.schan : 0U;
  channel_count = results->country_valid ? (unsigned)results->configured_country.nchan : 0U;
  last_channel = channel_count == 0U ? 0U : first_channel + channel_count - 1U;

  for (index = 0U; index < results->catalog.count; ++index) {
    log_observation(index, &results->catalog.entries[index], &results->records[index]);
    log_capabilities(index, &results->records[index]);
  }
  ESP_LOGI(LOG_TAG,
           "Summary: detected=%" PRIu32 " retrieved=%" PRIu32 " unique=%u duplicates=%" PRIu32
           " hidden=%u dropped=%" PRIu32 " invalid=%" PRIu32,
           results->detected_observations, results->retrieved_observations,
           (unsigned)results->catalog.count, results->duplicate_observations,
           (unsigned)count_hidden_networks(results), results->catalog.dropped_observations,
           results->invalid_observations);
  ESP_LOGI(LOG_TAG,
           "Final passive survey report: state=%s error=%s scanner_country=%c%c "
           "scanner_channels=%u-%u dwell_ms=%" PRIu32 " sweeps=%" PRIu32 "/%" PRIu32,
           report_state(status, results), esp_err_to_name(status), country0, country1,
           first_channel, last_channel, (uint32_t)PASSIVE_DWELL_MILLISECONDS,
           results->completed_sweeps, (uint32_t)SCAN_SWEEP_COUNT);
}

static void led_task(void* parameter) {
  led_context_t* context;
  esp_err_t status = ESP_OK;
  bool led_enabled = false;
  bool run_finished = false;
  TickType_t delay;
  uint32_t tick;
  if (parameter == NULL) {
    vTaskDelete(NULL);
    return;
  }
  context = parameter;
  if (context->events == NULL) {
    vTaskDelete(NULL);
    return;
  }

  delay = pdMS_TO_TICKS(LED_TICK_MILLISECONDS);
  for (tick = 0U; (tick < MAXIMUM_LED_TICKS) && (status == ESP_OK); ++tick) {
    const EventBits_t bits = xEventGroupGetBits(context->events);
    if ((bits & RUN_DONE_BIT) != 0U) {
      led_enabled = (bits & FAILURE_BIT) != 0U;
      status = set_led(led_enabled);
      run_finished = true;
      break;
    }
    if ((bits & FAILURE_BIT) != 0U) {
      led_enabled = true;
      status = set_led(true);
    } else if ((bits & SCANNING_BIT) != 0U) {
      led_enabled = !led_enabled;
      status = set_led(led_enabled);
    } else if (led_enabled) {
      led_enabled = false;
      status = set_led(false);
    }
    vTaskDelay(delay);
  }

  if (!run_finished && (status == ESP_OK)) {
    status = ESP_ERR_TIMEOUT;
  }
  if (status != ESP_OK) {
    esp_err_t led_status;
    ESP_LOGE(LOG_TAG, "LED task failed: %s", esp_err_to_name(status));
    set_event_bits(context->events, FAILURE_BIT);
    led_status = set_led(true);
    if (led_status != ESP_OK) {
      ESP_LOGE(LOG_TAG, "LED error indication failed: %s", esp_err_to_name(led_status));
    }
  }
  set_event_bits(context->events, LED_DONE_BIT);
  vTaskDelete(NULL);
}

static esp_err_t initialize_nvs(void) {
  esp_err_t status = nvs_flash_init();
  if ((status != ESP_ERR_NVS_NO_FREE_PAGES) && (status != ESP_ERR_NVS_NEW_VERSION_FOUND)) {
    return status;
  }
  status = nvs_flash_erase();
  if (status != ESP_OK) {
    return status;
  }
  return nvs_flash_init();
}

static esp_err_t initialize_scanner(wifi_resources_t* resources, scan_results_t* results) {
  esp_err_t status;
  wifi_init_config_t initialization;
  configASSERT(resources != NULL);
  configASSERT(results != NULL);
  status = esp_netif_init(); /* Process-wide; ESP-IDF does not support deinit. */
  if (status != ESP_OK) {
    return status;
  }
  status = esp_event_loop_create_default();
  if (status != ESP_OK) {
    return status;
  }
  resources->event_loop_created = true;
  resources->station_netif = esp_netif_create_default_wifi_sta();
  if (resources->station_netif == NULL) {
    return ESP_ERR_NO_MEM;
  }
  initialization = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();
  status = esp_wifi_init(&initialization);
  if (status != ESP_OK) {
    return status;
  }
  resources->wifi_initialized = true;
  status = esp_wifi_set_country_code(COUNTRY_CODE, false);
  if (status == ESP_OK) {
    status = esp_wifi_set_mode(WIFI_MODE_STA);
  }
  if (status == ESP_OK) {
    status = esp_wifi_start();
  }
  if (status != ESP_OK) {
    return status;
  }
  resources->wifi_started = true;
  status = esp_wifi_get_country(&results->configured_country);
  results->country_valid = status == ESP_OK;
  return status;
}

static void log_cleanup_error(const char* operation, const esp_err_t status) {
  configASSERT(operation != NULL);
  configASSERT(operation[0] != '\0');
  if (status != ESP_OK) {
    ESP_LOGE(LOG_TAG, "%s failed during cleanup: %s", operation, esp_err_to_name(status));
  }
}

static void shutdown_scanner(wifi_resources_t* resources) {
  configASSERT(resources != NULL);
  if (resources->wifi_started) {
    log_cleanup_error("Wi-Fi stop", esp_wifi_stop());
  }
  if (resources->wifi_initialized) {
    log_cleanup_error("Wi-Fi deinitialize", esp_wifi_deinit());
  }
  if (resources->station_netif != NULL) {
    esp_netif_destroy_default_wifi(resources->station_netif);
  }
  if (resources->event_loop_created) {
    log_cleanup_error("Event loop delete", esp_event_loop_delete_default());
  }
}

static bool initialize_runtime(runtime_storage_t* storage) {
  configASSERT(storage != NULL);
  storage->led_context.events = xEventGroupCreateStatic(&storage->event_control);
  return storage->led_context.events != NULL;
}

static bool start_led_task(runtime_storage_t* storage) {
  TaskHandle_t led;
  configASSERT(storage != NULL);
  led = xTaskCreateStaticPinnedToCore(led_task, "activity_led", LED_STACK_DEPTH,
                                      &storage->led_context, 4U, storage->led_stack,
                                      &storage->led_control, 1);
  return led != NULL;
}

static esp_err_t wait_for_led(const EventGroupHandle_t events) {
  EventBits_t completion;
  configASSERT(events != NULL);
  completion = xEventGroupWaitBits(events, LED_DONE_BIT, pdFALSE, pdTRUE,
                                   pdMS_TO_TICKS(LED_SHUTDOWN_TIMEOUT_MILLISECONDS));
  return (completion & LED_DONE_BIT) != 0U ? ESP_OK : ESP_ERR_TIMEOUT;
}

static void log_completion(const esp_err_t status, const scan_results_t* results) {
  configASSERT(results != NULL);
  if (status != ESP_OK) {
    ESP_LOGE(LOG_TAG, "Passive Wi-Fi survey failed: %s", esp_err_to_name(status));
  } else if (results->catalog.dropped_observations != 0U) {
    ESP_LOGW(LOG_TAG, "Passive Wi-Fi survey ended with a capacity-limited report");
  } else {
    ESP_LOGI(LOG_TAG, "Passive Wi-Fi survey complete");
  }
}

void app_main(void) {
  static runtime_storage_t storage;
  wifi_resources_t wifi_resources = {0};
  bool nvs_initialized = false;
  bool led_started = false;
  esp_err_t status = initialize_runtime(&storage) ? ESP_OK : ESP_ERR_NO_MEM;
  if (status == ESP_OK) {
    status = initialize_nvs();
    nvs_initialized = status == ESP_OK;
  }
  if (status == ESP_OK) {
    status = initialize_scanner(&wifi_resources, &storage.results);
  }
  if (status == ESP_OK) {
    status = initialize_led();
  }
  if (status == ESP_OK) {
    led_started = start_led_task(&storage);
    status = led_started ? ESP_OK : ESP_ERR_NO_MEM;
  }
  if (status == ESP_OK) {
    status = run_scans(storage.led_context.events, &storage.results);
  }
  if ((status == ESP_OK) && (storage.results.invalid_observations != 0U)) {
    status = ESP_ERR_INVALID_RESPONSE;
  }

  if ((status != ESP_OK) && (storage.led_context.events != NULL)) {
    set_event_bits(storage.led_context.events, FAILURE_BIT);
  }
  if (storage.led_context.events != NULL) {
    set_event_bits(storage.led_context.events, RUN_DONE_BIT);
  }

  if (led_started) {
    const esp_err_t led_status = wait_for_led(storage.led_context.events);
    if ((led_status != ESP_OK) && (status == ESP_OK)) {
      status = led_status;
    }
  }
  if ((status == ESP_OK) &&
      ((xEventGroupGetBits(storage.led_context.events) & FAILURE_BIT) != 0U)) {
    status = ESP_FAIL;
  }
  if ((status != ESP_OK) && (storage.led_context.events != NULL)) {
    set_event_bits(storage.led_context.events, FAILURE_BIT);
  }

  log_report(status, &storage.results);
  log_completion(status, &storage.results);

  shutdown_scanner(&wifi_resources);
  if (nvs_initialized) {
    log_cleanup_error("NVS deinitialize", nvs_flash_deinit());
  }
}
