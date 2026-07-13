#include <array>
#include <cinttypes>
#include <cstddef>
#include <cstdint>

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
#include "network_catalog.hpp"
#include "nvs_flash.h"
#include "security_assessment.hpp"

namespace {

constexpr char kLogTag[]{"wifi_scanner"};
constexpr char kCountryCode[]{"MY"};
constexpr gpio_num_t kActivityLed{GPIO_NUM_2};
constexpr std::uint32_t kScanSweepCount{3U};
constexpr std::uint32_t kPassiveDwellMilliseconds{360U};
constexpr std::uint32_t kInterSweepDelayMilliseconds{5'000U};
constexpr std::uint32_t kLedTickMilliseconds{150U};
constexpr std::uint32_t kLedDeadlineMilliseconds{60'000U};
constexpr std::uint32_t kLedShutdownTimeoutMilliseconds{5'000U};
constexpr std::uint32_t kLedStackDepth{3'072U};
constexpr std::uint32_t kMaximumLedTicks{(kLedDeadlineMilliseconds / kLedTickMilliseconds) + 1U};

constexpr EventBits_t kScanningBit{1U << 0U};
constexpr EventBits_t kFailureBit{1U << 1U};
constexpr EventBits_t kRunDoneBit{1U << 2U};
constexpr EventBits_t kLedDoneBit{1U << 3U};

static_assert(sizeof(kCountryCode) == 3U);
static_assert(kScanSweepCount > 0U);
static_assert(kPassiveDwellMilliseconds > 0U);
static_assert(kPassiveDwellMilliseconds <= 1'500U);
static_assert(kLedTickMilliseconds > 0U);
static_assert(kScanSweepCount <= (UINT32_MAX / UINT16_MAX));

struct ScanResults final {
  wifi_scanner::NetworkCatalog catalog;
  std::array<wifi_ap_record_t, wifi_scanner::kMaximumUniqueNetworks> records;
  wifi_country_t configured_country;
  std::uint32_t completed_sweeps;
  std::uint32_t detected_observations;
  std::uint32_t retrieved_observations;
  std::uint32_t duplicate_observations;
  std::uint32_t invalid_observations;
  bool country_valid;
};

struct LedContext final {
  EventGroupHandle_t events;
};

struct RuntimeStorage final {
  StaticEventGroup_t event_control;
  StaticTask_t led_control;
  std::array<StackType_t, kLedStackDepth> led_stack;
  LedContext led_context;
  ScanResults results;
};

struct WifiResources final {
  esp_netif_t* station_netif;
  bool event_loop_created;
  bool wifi_initialized;
  bool wifi_started;
};

const char* risk_name(const wifi_scanner::RiskLevel risk) noexcept {
  switch (risk) {
  case wifi_scanner::RiskLevel::low:
    return "low";
  case wifi_scanner::RiskLevel::medium:
    return "medium";
  case wifi_scanner::RiskLevel::high:
    return "high";
  case wifi_scanner::RiskLevel::critical:
    return "critical";
  }
  return "invalid";
}

const char* finding_name(const wifi_scanner::Finding finding) noexcept {
  switch (finding) {
  case wifi_scanner::Finding::none:
    return "no obvious beacon-level issue";
  case wifi_scanner::Finding::open_network:
    return "unencrypted open network";
  case wifi_scanner::Finding::obsolete_wep:
    return "obsolete WEP security";
  case wifi_scanner::Finding::legacy_wpa:
    return "legacy WPA permitted";
  case wifi_scanner::Finding::transition_mode:
    return "WPA2/WPA3 transition mode";
  case wifi_scanner::Finding::legacy_tkip:
    return "legacy TKIP cipher advertised";
  case wifi_scanner::Finding::wps_enabled:
    return "WPS capability advertised";
  case wifi_scanner::Finding::unknown_security:
    return "unknown authentication or cipher";
  }
  return "invalid assessment";
}

wifi_scanner::Authentication map_authentication(const wifi_auth_mode_t mode) noexcept {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return wifi_scanner::Authentication::open;
  case WIFI_AUTH_WEP:
    return wifi_scanner::Authentication::wep;
  case WIFI_AUTH_WPA_PSK:
  case WIFI_AUTH_WPA_WPA2_PSK:
  case WIFI_AUTH_WPA_ENTERPRISE:
    return wifi_scanner::Authentication::legacy_wpa;
  case WIFI_AUTH_WPA2_WPA3_PSK:
  case WIFI_AUTH_WPA2_WPA3_ENTERPRISE:
    return wifi_scanner::Authentication::transition;
  case WIFI_AUTH_WPA2_PSK:
  case WIFI_AUTH_WPA2_ENTERPRISE:
  case WIFI_AUTH_WPA3_PSK:
  case WIFI_AUTH_WAPI_PSK:
  case WIFI_AUTH_OWE:
  case WIFI_AUTH_WPA3_ENT_192:
  case WIFI_AUTH_DPP:
  case WIFI_AUTH_WPA3_ENTERPRISE:
    return wifi_scanner::Authentication::modern;
  default:
    return wifi_scanner::Authentication::unknown;
  }
}

bool is_legacy_tkip(const wifi_cipher_type_t cipher) noexcept {
  return (cipher == WIFI_CIPHER_TYPE_TKIP) || (cipher == WIFI_CIPHER_TYPE_TKIP_CCMP);
}

bool is_unknown_cipher(const wifi_cipher_type_t cipher) noexcept {
  return cipher == WIFI_CIPHER_TYPE_UNKNOWN;
}

wifi_scanner::AdvertisedSecurity
make_advertised_security(const wifi_ap_record_t& access_point) noexcept {
  const bool legacy_tkip{is_legacy_tkip(access_point.pairwise_cipher) ||
                         is_legacy_tkip(access_point.group_cipher)};
  const bool unknown_cipher{is_unknown_cipher(access_point.pairwise_cipher) ||
                            is_unknown_cipher(access_point.group_cipher)};
  return {map_authentication(access_point.authmode), legacy_tkip, unknown_cipher,
          access_point.wps != 0U};
}

const char* authentication_name(const wifi_auth_mode_t mode) noexcept {
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

const char* cipher_name(const wifi_cipher_type_t cipher) noexcept {
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

const char* secondary_channel_name(const wifi_second_chan_t channel) noexcept {
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

const char* bandwidth_name(const wifi_bandwidth_t bandwidth) noexcept {
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

const char* antenna_name(const wifi_ant_t antenna) noexcept {
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

std::array<char, 129U> escaped_ssid(const wifi_ap_record_t& access_point) noexcept {
  static_assert(sizeof(access_point.ssid) == 33U);
  constexpr char kHexDigits[]{"0123456789ABCDEF"};
  std::array<char, 129U> output{};
  std::size_t output_index{0U};
  for (std::size_t input_index{0U}; input_index < 32U; ++input_index) {
    const std::uint8_t byte{access_point.ssid[input_index]};
    if (byte == 0U) {
      break;
    }
    if ((byte >= 32U) && (byte <= 126U) && (byte != static_cast<std::uint8_t>('\\'))) {
      output[output_index] = static_cast<char>(byte);
      ++output_index;
    } else {
      output[output_index] = '\\';
      output[output_index + 1U] = 'x';
      output[output_index + 2U] = kHexDigits[byte >> 4U];
      output[output_index + 3U] = kHexDigits[byte & 0x0FU];
      output_index += 4U;
    }
  }
  configASSERT(output_index < output.size());
  output[output_index] = '\0';
  return output;
}

wifi_scanner::Bssid bssid_from_record(const wifi_ap_record_t& access_point) noexcept {
  wifi_scanner::Bssid bssid{};
  for (std::size_t index{0U}; index < wifi_scanner::kBssidSize; ++index) {
    bssid[index] = access_point.bssid[index];
  }
  return bssid;
}

void set_event_bits(const EventGroupHandle_t events, const EventBits_t bits) noexcept {
  configASSERT(events != nullptr);
  configASSERT(bits != 0U);
  (void)xEventGroupSetBits(events, bits); // Returned bits are a racy cross-core snapshot.
}

void clear_event_bits(const EventGroupHandle_t events, const EventBits_t bits) noexcept {
  configASSERT(events != nullptr);
  configASSERT(bits != 0U);
  (void)xEventGroupClearBits(events, bits); // Only app_main owns these level bits.
}

esp_err_t set_led(const bool enabled) noexcept {
  return gpio_set_level(kActivityLed, enabled ? 1U : 0U);
}

esp_err_t initialize_led() noexcept {
  esp_err_t status{gpio_reset_pin(kActivityLed)};
  if (status == ESP_OK) {
    status = gpio_set_direction(kActivityLed, GPIO_MODE_OUTPUT);
  }
  if (status == ESP_OK) {
    status = set_led(false);
  }
  return status;
}

esp_err_t process_record(const wifi_ap_record_t& access_point, const std::uint32_t sweep,
                         ScanResults& results) noexcept {
  const wifi_scanner::CatalogUpdate update{wifi_scanner::observe(
      results.catalog, bssid_from_record(access_point), access_point.rssi, sweep)};
  switch (update.status) {
  case wifi_scanner::CatalogStatus::inserted:
  case wifi_scanner::CatalogStatus::updated:
    if (update.index >= results.records.size()) {
      return ESP_ERR_INVALID_STATE;
    }
    results.records[update.index] = access_point;
    if (update.status == wifi_scanner::CatalogStatus::updated) {
      ++results.duplicate_observations;
    }
    return ESP_OK;
  case wifi_scanner::CatalogStatus::full:
    return ESP_OK;
  case wifi_scanner::CatalogStatus::invalid_bssid:
    ++results.invalid_observations;
    return ESP_OK;
  case wifi_scanner::CatalogStatus::invalid_sweep:
  case wifi_scanner::CatalogStatus::invalid_state:
    return ESP_ERR_INVALID_STATE;
  }
  return ESP_ERR_INVALID_STATE;
}

esp_err_t collect_scan_results(const std::uint32_t sweep, ScanResults& results) noexcept {
  std::uint16_t detected{0U};
  esp_err_t status{esp_wifi_scan_get_ap_num(&detected)};
  if (status == ESP_OK) {
    results.detected_observations += detected;
    for (std::uint32_t index{0U}; index < UINT16_MAX; ++index) {
      if (index >= detected) {
        break;
      }
      wifi_ap_record_t access_point{};
      status = esp_wifi_scan_get_ap_record(&access_point);
      if (status != ESP_OK) {
        break;
      }
      ++results.retrieved_observations;
      status = process_record(access_point, sweep, results);
      if (status != ESP_OK) {
        break;
      }
    }
  }

  const esp_err_t clear_status{esp_wifi_clear_ap_list()};
  if (clear_status != ESP_OK) {
    ESP_LOGE(kLogTag, "AP-list cleanup failed: %s", esp_err_to_name(clear_status));
  }
  return status == ESP_OK ? clear_status : status;
}

esp_err_t perform_scan(const std::uint32_t sweep, ScanResults& results) noexcept {
  configASSERT((sweep > 0U) && (sweep <= kScanSweepCount));
  wifi_scan_config_t configuration{};
  configuration.show_hidden = true;
  configuration.scan_type = WIFI_SCAN_TYPE_PASSIVE;
  configuration.scan_time.passive = kPassiveDwellMilliseconds;
  const esp_err_t status{esp_wifi_scan_start(&configuration, true)};
  if (status != ESP_OK) {
    return status;
  }
  return collect_scan_results(sweep, results);
}

esp_err_t run_scans(const EventGroupHandle_t events, ScanResults& results) noexcept {
  esp_err_t status{ESP_OK};
  const TickType_t inter_sweep_delay{pdMS_TO_TICKS(kInterSweepDelayMilliseconds)};
  for (std::uint32_t sweep{1U}; (sweep <= kScanSweepCount) && (status == ESP_OK); ++sweep) {
    const std::uint32_t detected_before{results.detected_observations};
    ESP_LOGI(kLogTag, "Passive sweep %" PRIu32 "/%" PRIu32 " started", sweep, kScanSweepCount);
    set_event_bits(events, kScanningBit);
    status = perform_scan(sweep, results);
    clear_event_bits(events, kScanningBit);
    if (status == ESP_OK) {
      ++results.completed_sweeps;
      ESP_LOGI(kLogTag, "Passive sweep %" PRIu32 " observed %" PRIu32 " BSS record(s)", sweep,
               results.detected_observations - detected_before);
    }
    if ((xEventGroupGetBits(events) & kFailureBit) != 0U) {
      status = ESP_FAIL;
    }
    if ((status == ESP_OK) && (sweep < kScanSweepCount)) {
      vTaskDelay(inter_sweep_delay);
    }
  }
  return status;
}

void log_observation(const std::size_t index, const wifi_scanner::CatalogEntry& observed,
                     const wifi_ap_record_t& access_point) noexcept {
  configASSERT(index < wifi_scanner::kMaximumUniqueNetworks);
  configASSERT(access_point.primary <= 14U);
  const std::array<char, 129U> ssid{escaped_ssid(access_point)};
  const char* const displayed_ssid{ssid.front() == '\0' ? "<hidden>" : ssid.data()};
  const wifi_scanner::SecurityAssessment assessment{
      wifi_scanner::assess_security(make_advertised_security(access_point))};

  ESP_LOGI(kLogTag,
           "network=%u bssid=" MACSTR " ssid=%s strongest_rssi=%d latest_rssi=%d "
           "first_sweep=%" PRIu32 " last_sweep=%" PRIu32 " sightings=%" PRIu32,
           static_cast<unsigned>(index + 1U), MAC2STR(access_point.bssid), displayed_ssid,
           static_cast<int>(observed.strongest_rssi), static_cast<int>(access_point.rssi),
           observed.first_sweep, observed.last_sweep, observed.observation_count);
  ESP_LOGI(kLogTag,
           "network=%u rf primary=%u secondary=%s bandwidth=%s antenna=%s auth=%s pairwise=%s "
           "group=%s WPS=%s risk=%s finding=%s",
           static_cast<unsigned>(index + 1U), static_cast<unsigned>(access_point.primary),
           secondary_channel_name(access_point.second), bandwidth_name(access_point.bandwidth),
           antenna_name(access_point.ant), authentication_name(access_point.authmode),
           cipher_name(access_point.pairwise_cipher), cipher_name(access_point.group_cipher),
           access_point.wps != 0U ? "yes" : "no", risk_name(assessment.risk),
           finding_name(assessment.finding));
}

void log_capabilities(const std::size_t index, const wifi_ap_record_t& access_point) noexcept {
  configASSERT(index < wifi_scanner::kMaximumUniqueNetworks);
  const std::array<char, 3U> advertised_country{access_point.country.cc[0],
                                                access_point.country.cc[1], '\0'};
  const bool country_absent{(advertised_country[0] == '\0') && (advertised_country[1] == '\0')};
  const bool country_valid{(advertised_country[0] >= 'A') && (advertised_country[0] <= 'Z') &&
                           (advertised_country[1] >= 'A') && (advertised_country[1] <= 'Z')};
  const char* const displayed_country{
      country_absent ? "not-advertised" : (country_valid ? advertised_country.data() : "invalid")};

  ESP_LOGI(kLogTag,
           "network=%u capabilities 11b=%u 11g=%u 11n=%u 11a=%u 11ac=%u 11ax=%u lr=%u "
           "FTM_responder=%u FTM_initiator=%u VHT_center1=%u VHT_center2=%u",
           static_cast<unsigned>(index + 1U), static_cast<unsigned>(access_point.phy_11b),
           static_cast<unsigned>(access_point.phy_11g), static_cast<unsigned>(access_point.phy_11n),
           static_cast<unsigned>(access_point.phy_11a),
           static_cast<unsigned>(access_point.phy_11ac),
           static_cast<unsigned>(access_point.phy_11ax), static_cast<unsigned>(access_point.phy_lr),
           static_cast<unsigned>(access_point.ftm_responder),
           static_cast<unsigned>(access_point.ftm_initiator),
           static_cast<unsigned>(access_point.vht_ch_freq1),
           static_cast<unsigned>(access_point.vht_ch_freq2));
  ESP_LOGI(kLogTag,
           "network=%u advertised_country=%s advertised_country_start=%u "
           "advertised_country_channels=%u advertised_country_max_power=%d HE_color=%u "
           "HE_partial=%u HE_disabled=%u HE_bssid_index=%u",
           static_cast<unsigned>(index + 1U), displayed_country,
           static_cast<unsigned>(access_point.country.schan),
           static_cast<unsigned>(access_point.country.nchan),
           static_cast<int>(access_point.country.max_tx_power),
           static_cast<unsigned>(access_point.he_ap.bss_color),
           static_cast<unsigned>(access_point.he_ap.partial_bss_color),
           static_cast<unsigned>(access_point.he_ap.bss_color_disabled),
           static_cast<unsigned>(access_point.he_ap.bssid_index));
}

std::size_t count_hidden_networks(const ScanResults& results) noexcept {
  std::size_t hidden_count{0U};
  for (std::size_t index{0U}; index < wifi_scanner::kMaximumUniqueNetworks; ++index) {
    if (index >= results.catalog.count) {
      break;
    }
    if (results.records[index].ssid[0] == 0U) {
      ++hidden_count;
    }
  }
  return hidden_count;
}

const char* report_state(const esp_err_t status, const ScanResults& results) noexcept {
  if ((status != ESP_OK) || (results.completed_sweeps != kScanSweepCount) ||
      (results.invalid_observations != 0U)) {
    return "incomplete";
  }
  if (results.catalog.dropped_observations != 0U) {
    return "capacity-limited";
  }
  return "complete";
}

void log_report(const esp_err_t status, const ScanResults& results) noexcept {
  const char country0{results.country_valid ? results.configured_country.cc[0] : '?'};
  const char country1{results.country_valid ? results.configured_country.cc[1] : '?'};
  const unsigned first_channel{results.country_valid ? results.configured_country.schan : 0U};
  const unsigned channel_count{results.country_valid ? results.configured_country.nchan : 0U};
  const unsigned last_channel{channel_count == 0U ? 0U : first_channel + channel_count - 1U};

  for (std::size_t index{0U}; index < wifi_scanner::kMaximumUniqueNetworks; ++index) {
    if (index >= results.catalog.count) {
      break;
    }
    log_observation(index, results.catalog.entries[index], results.records[index]);
    log_capabilities(index, results.records[index]);
  }
  ESP_LOGI(kLogTag,
           "Summary: detected=%" PRIu32 " retrieved=%" PRIu32 " unique=%u duplicates=%" PRIu32
           " hidden=%u dropped=%" PRIu32 " invalid=%" PRIu32,
           results.detected_observations, results.retrieved_observations,
           static_cast<unsigned>(results.catalog.count), results.duplicate_observations,
           static_cast<unsigned>(count_hidden_networks(results)),
           results.catalog.dropped_observations, results.invalid_observations);
  ESP_LOGI(kLogTag,
           "Final passive survey report: state=%s error=%s scanner_country=%c%c "
           "scanner_channels=%u-%u dwell_ms=%" PRIu32 " sweeps=%" PRIu32 "/%" PRIu32,
           report_state(status, results), esp_err_to_name(status), country0, country1,
           first_channel, last_channel, kPassiveDwellMilliseconds, results.completed_sweeps,
           kScanSweepCount);
}

void led_task(void* parameter) noexcept {
  if (parameter == nullptr) {
    vTaskDelete(nullptr);
    return;
  }
  auto& context{*static_cast<LedContext*>(parameter)};
  if (context.events == nullptr) {
    vTaskDelete(nullptr);
    return;
  }

  esp_err_t status{ESP_OK};
  bool led_enabled{false};
  bool run_finished{false};
  const TickType_t delay{pdMS_TO_TICKS(kLedTickMilliseconds)};
  for (std::uint32_t tick{0U}; (tick < kMaximumLedTicks) && (status == ESP_OK); ++tick) {
    const EventBits_t bits{xEventGroupGetBits(context.events)};
    if ((bits & kRunDoneBit) != 0U) {
      led_enabled = (bits & kFailureBit) != 0U;
      status = set_led(led_enabled);
      run_finished = true;
      break;
    }
    if ((bits & kFailureBit) != 0U) {
      led_enabled = true;
      status = set_led(true);
    } else if ((bits & kScanningBit) != 0U) {
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
    ESP_LOGE(kLogTag, "LED task failed: %s", esp_err_to_name(status));
    set_event_bits(context.events, kFailureBit);
    const esp_err_t led_status{set_led(true)};
    if (led_status != ESP_OK) {
      ESP_LOGE(kLogTag, "LED error indication failed: %s", esp_err_to_name(led_status));
    }
  }
  set_event_bits(context.events, kLedDoneBit);
  vTaskDelete(nullptr);
}

esp_err_t initialize_nvs() noexcept {
  esp_err_t status{nvs_flash_init()};
  if ((status != ESP_ERR_NVS_NO_FREE_PAGES) && (status != ESP_ERR_NVS_NEW_VERSION_FOUND)) {
    return status;
  }
  status = nvs_flash_erase();
  if (status != ESP_OK) {
    return status;
  }
  return nvs_flash_init();
}

esp_err_t initialize_scanner(WifiResources& resources, ScanResults& results) noexcept {
  esp_err_t status{esp_netif_init()}; // Process-wide; ESP-IDF does not support deinit.
  if (status != ESP_OK) {
    return status;
  }
  status = esp_event_loop_create_default();
  if (status != ESP_OK) {
    return status;
  }
  resources.event_loop_created = true;
  resources.station_netif = esp_netif_create_default_wifi_sta();
  if (resources.station_netif == nullptr) {
    return ESP_ERR_NO_MEM;
  }
  wifi_init_config_t initialization = WIFI_INIT_CONFIG_DEFAULT();
  status = esp_wifi_init(&initialization);
  if (status != ESP_OK) {
    return status;
  }
  resources.wifi_initialized = true;
  status = esp_wifi_set_country_code(kCountryCode, false);
  if (status == ESP_OK) {
    status = esp_wifi_set_mode(WIFI_MODE_STA);
  }
  if (status == ESP_OK) {
    status = esp_wifi_start();
  }
  if (status != ESP_OK) {
    return status;
  }
  resources.wifi_started = true;
  status = esp_wifi_get_country(&results.configured_country);
  results.country_valid = status == ESP_OK;
  return status;
}

void log_cleanup_error(const char* const operation, const esp_err_t status) noexcept {
  configASSERT(operation != nullptr);
  configASSERT(operation[0] != '\0');
  if (status != ESP_OK) {
    ESP_LOGE(kLogTag, "%s failed during cleanup: %s", operation, esp_err_to_name(status));
  }
}

void shutdown_scanner(WifiResources& resources) noexcept {
  if (resources.wifi_started) {
    log_cleanup_error("Wi-Fi stop", esp_wifi_stop());
  }
  if (resources.wifi_initialized) {
    log_cleanup_error("Wi-Fi deinitialize", esp_wifi_deinit());
  }
  if (resources.station_netif != nullptr) {
    esp_netif_destroy_default_wifi(resources.station_netif);
  }
  if (resources.event_loop_created) {
    log_cleanup_error("Event loop delete", esp_event_loop_delete_default());
  }
}

bool initialize_runtime(RuntimeStorage& storage) noexcept {
  storage.led_context.events = xEventGroupCreateStatic(&storage.event_control);
  return storage.led_context.events != nullptr;
}

bool start_led_task(RuntimeStorage& storage) noexcept {
  const TaskHandle_t led{
      xTaskCreateStaticPinnedToCore(led_task, "activity_led", kLedStackDepth, &storage.led_context,
                                    4U, storage.led_stack.data(), &storage.led_control, 1)};
  return led != nullptr;
}

esp_err_t wait_for_led(const EventGroupHandle_t events) noexcept {
  const EventBits_t completion{xEventGroupWaitBits(events, kLedDoneBit, pdFALSE, pdTRUE,
                                                   pdMS_TO_TICKS(kLedShutdownTimeoutMilliseconds))};
  return (completion & kLedDoneBit) != 0U ? ESP_OK : ESP_ERR_TIMEOUT;
}

void log_completion(const esp_err_t status, const ScanResults& results) noexcept {
  if (status != ESP_OK) {
    ESP_LOGE(kLogTag, "Passive Wi-Fi survey failed: %s", esp_err_to_name(status));
  } else if (results.catalog.dropped_observations != 0U) {
    ESP_LOGW(kLogTag, "Passive Wi-Fi survey ended with a capacity-limited report");
  } else {
    ESP_LOGI(kLogTag, "Passive Wi-Fi survey complete");
  }
}

} // namespace

extern "C" void app_main() {
  static RuntimeStorage storage{};
  WifiResources wifi_resources{};
  bool nvs_initialized{false};
  bool led_started{false};
  esp_err_t status{initialize_runtime(storage) ? ESP_OK : ESP_ERR_NO_MEM};
  if (status == ESP_OK) {
    status = initialize_nvs();
    nvs_initialized = status == ESP_OK;
  }
  if (status == ESP_OK) {
    status = initialize_scanner(wifi_resources, storage.results);
  }
  if (status == ESP_OK) {
    status = initialize_led();
  }
  if (status == ESP_OK) {
    led_started = start_led_task(storage);
    status = led_started ? ESP_OK : ESP_ERR_NO_MEM;
  }
  if (status == ESP_OK) {
    status = run_scans(storage.led_context.events, storage.results);
  }
  if ((status == ESP_OK) && (storage.results.invalid_observations != 0U)) {
    status = ESP_ERR_INVALID_RESPONSE;
  }

  if ((status != ESP_OK) && (storage.led_context.events != nullptr)) {
    set_event_bits(storage.led_context.events, kFailureBit);
  }
  if (storage.led_context.events != nullptr) {
    set_event_bits(storage.led_context.events, kRunDoneBit);
  }

  if (led_started) {
    const esp_err_t led_status{wait_for_led(storage.led_context.events)};
    if ((led_status != ESP_OK) && (status == ESP_OK)) {
      status = led_status;
    }
  }
  if ((status == ESP_OK) &&
      ((xEventGroupGetBits(storage.led_context.events) & kFailureBit) != 0U)) {
    status = ESP_FAIL;
  }
  if ((status != ESP_OK) && (storage.led_context.events != nullptr)) {
    set_event_bits(storage.led_context.events, kFailureBit);
  }

  log_report(status, storage.results);
  log_completion(status, storage.results);

  shutdown_scanner(wifi_resources);
  if (nvs_initialized) {
    log_cleanup_error("NVS deinitialize", nvs_flash_deinit());
  }
}
