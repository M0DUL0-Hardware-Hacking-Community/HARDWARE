#include <array>
#include <cinttypes>
#include <cstddef>
#include <cstdint>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "security_assessment.hpp"

namespace {

constexpr char kLogTag[]{"wifi_scanner"};
constexpr gpio_num_t kActivityLed{GPIO_NUM_2};
constexpr std::size_t kMaximumNetworks{20U};
constexpr std::uint32_t kScanCycleCount{3U};
constexpr std::uint32_t kScanPeriodMilliseconds{5'000U};
constexpr std::uint32_t kLedTickMilliseconds{150U};
constexpr std::uint32_t kMaximumLedTicks{400U};
constexpr UBaseType_t kReportQueueDepth{2U};
constexpr EventBits_t kScanningBit{1U << 0U};
constexpr EventBits_t kReportReadyBit{1U << 1U};
constexpr EventBits_t kStopLedBit{1U << 2U};
constexpr EventBits_t kFailureBit{1U << 3U};
constexpr EventBits_t kScannerDoneBit{1U << 4U};
constexpr EventBits_t kLoggerDoneBit{1U << 5U};
constexpr EventBits_t kLedDoneBit{1U << 6U};
constexpr EventBits_t kAllDoneBits{kScannerDoneBit | kLoggerDoneBit | kLedDoneBit};

struct ScanReport final {
  std::array<wifi_ap_record_t, kMaximumNetworks> networks;
  std::uint32_t cycle;
  std::uint16_t detected_count;
  std::uint16_t reported_count;
};

struct SharedContext final {
  QueueHandle_t report_queue;
  EventGroupHandle_t events;
};

struct WifiResources final {
  esp_netif_t* station_netif;
  bool event_loop_created;
  bool wifi_initialized;
  bool wifi_started;
};

struct RuntimeStorage final {
  std::array<std::uint8_t, kReportQueueDepth * sizeof(ScanReport)> queue_storage;
  StaticQueue_t queue_control;
  StaticEventGroup_t event_control;
  StaticTask_t scanner_control;
  StaticTask_t logger_control;
  StaticTask_t led_control;
  std::array<StackType_t, 6'144U> scanner_stack;
  std::array<StackType_t, 4'096U> logger_stack;
  std::array<StackType_t, 3'072U> led_stack;
  SharedContext context;
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
    return "unknown authentication mode";
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
    return wifi_scanner::Authentication::wpa;
  case WIFI_AUTH_WPA2_PSK:
    return wifi_scanner::Authentication::wpa2;
  case WIFI_AUTH_WPA_WPA2_PSK:
    return wifi_scanner::Authentication::wpa_wpa2;
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return wifi_scanner::Authentication::wpa2_enterprise;
  case WIFI_AUTH_WPA3_PSK:
    return wifi_scanner::Authentication::wpa3;
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return wifi_scanner::Authentication::wpa2_wpa3;
  case WIFI_AUTH_WAPI_PSK:
    return wifi_scanner::Authentication::wapi;
  case WIFI_AUTH_OWE:
    return wifi_scanner::Authentication::owe;
  default:
    return wifi_scanner::Authentication::unknown;
  }
}

wifi_scanner::Cipher map_cipher(const wifi_cipher_type_t cipher) noexcept {
  if (cipher == WIFI_CIPHER_TYPE_TKIP) {
    return wifi_scanner::Cipher::tkip;
  }
  if (cipher == WIFI_CIPHER_TYPE_TKIP_CCMP) {
    return wifi_scanner::Cipher::tkip_ccmp;
  }
  return wifi_scanner::Cipher::other;
}

wifi_scanner::AdvertisedSecurity
make_advertised_security(const wifi_ap_record_t& access_point) noexcept {
  return {map_authentication(access_point.authmode), map_cipher(access_point.pairwise_cipher),
          access_point.wps != 0U};
}

const char* authentication_name(const wifi_auth_mode_t mode) noexcept {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return "open";
  case WIFI_AUTH_WEP:
    return "WEP";
  case WIFI_AUTH_WPA_PSK:
    return "WPA";
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA/WPA2";
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2 enterprise";
  case WIFI_AUTH_WPA3_PSK:
    return "WPA3";
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return "WPA2/WPA3";
  case WIFI_AUTH_WAPI_PSK:
    return "WAPI";
  case WIFI_AUTH_OWE:
    return "OWE";
  default:
    return "other";
  }
}

std::array<char, 33U> copy_ssid(const wifi_ap_record_t& access_point) noexcept {
  static_assert(sizeof(access_point.ssid) == 33U);
  std::array<char, 33U> ssid{};
  for (std::size_t index{0U}; index < (ssid.size() - 1U); ++index) {
    if (access_point.ssid[index] == 0U) {
      break;
    }
    ssid[index] = static_cast<char>(access_point.ssid[index]);
  }
  configASSERT(ssid.back() == '\0');
  return ssid;
}

esp_err_t perform_scan(const std::uint32_t cycle, ScanReport& report) noexcept {
  configASSERT((cycle > 0U) && (cycle <= kScanCycleCount));
  static_assert(kMaximumNetworks <= UINT16_MAX);
  wifi_scan_config_t configuration{};
  configuration.show_hidden = true;
  configuration.scan_type = WIFI_SCAN_TYPE_ACTIVE;
  esp_err_t status{esp_wifi_scan_start(&configuration, true)};
  if (status != ESP_OK) {
    return status;
  }
  status = esp_wifi_scan_get_ap_num(&report.detected_count);
  if (status != ESP_OK) {
    const esp_err_t clear_status{esp_wifi_clear_ap_list()};
    return (clear_status == ESP_OK) ? status : clear_status;
  }
  report.reported_count = report.detected_count;
  if (report.reported_count > kMaximumNetworks) {
    report.reported_count = static_cast<std::uint16_t>(kMaximumNetworks);
  }
  if (report.reported_count == 0U) {
    report.cycle = cycle;
    return esp_wifi_clear_ap_list();
  }
  status = esp_wifi_scan_get_ap_records(&report.reported_count, report.networks.data());
  if (status != ESP_OK) {
    const esp_err_t clear_status{esp_wifi_clear_ap_list()};
    return (clear_status == ESP_OK) ? status : clear_status;
  }
  report.cycle = cycle;
  return ESP_OK;
}

void set_event_bits(SharedContext& context, const EventBits_t bits) noexcept {
  configASSERT(context.events != nullptr);
  configASSERT(bits != 0U);
  const EventBits_t result{xEventGroupSetBits(context.events, bits)};
  configASSERT((result & bits) == bits);
}

void clear_event_bits(SharedContext& context, const EventBits_t bits) noexcept {
  configASSERT(context.events != nullptr);
  configASSERT(bits != 0U);
  const EventBits_t previous{xEventGroupClearBits(context.events, bits)};
  configASSERT((previous & bits) == bits);
}

void scanner_task(void* parameter) noexcept {
  if (parameter == nullptr) {
    vTaskDelete(nullptr);
    return;
  }
  auto& context{*static_cast<SharedContext*>(parameter)};
  esp_err_t status{ESP_OK};
  const TickType_t queue_timeout{pdMS_TO_TICKS(1'000U)};
  const TickType_t scan_delay{pdMS_TO_TICKS(kScanPeriodMilliseconds)};
  for (std::uint32_t cycle{1U}; (cycle <= kScanCycleCount) && (status == ESP_OK); ++cycle) {
    set_event_bits(context, kScanningBit);
    ScanReport report{};
    status = perform_scan(cycle, report);
    clear_event_bits(context, kScanningBit);
    if ((status == ESP_OK) &&
        (xQueueSend(context.report_queue, &report, queue_timeout) != pdPASS)) {
      status = ESP_ERR_TIMEOUT;
    }
    if (status == ESP_OK) {
      set_event_bits(context, kReportReadyBit);
    }
    if ((status == ESP_OK) && (cycle < kScanCycleCount)) {
      vTaskDelay(scan_delay);
    }
  }
  if (status == ESP_OK) {
    set_event_bits(context, kStopLedBit);
  } else {
    ESP_LOGE(kLogTag, "Scanner task failed: %s", esp_err_to_name(status));
    set_event_bits(context, kFailureBit);
  }
  set_event_bits(context, kScannerDoneBit);
  vTaskDelete(nullptr);
}

void log_observation(const std::uint16_t index, const wifi_ap_record_t& access_point) noexcept {
  configASSERT(index < kMaximumNetworks);
  configASSERT(access_point.primary <= 14U);
  const std::array<char, 33U> ssid{copy_ssid(access_point)};
  const char* const displayed_ssid{(ssid.front() == '\0') ? "<hidden>" : ssid.data()};
  const wifi_scanner::SecurityAssessment assessment{
      wifi_scanner::assess_security(make_advertised_security(access_point))};
  ESP_LOGI(kLogTag, "[%u] SSID=%s RSSI=%d dBm channel=%u security=%s WPS=%s risk=%s finding=%s",
           static_cast<unsigned>(index + 1U), displayed_ssid, static_cast<int>(access_point.rssi),
           static_cast<unsigned>(access_point.primary), authentication_name(access_point.authmode),
           (access_point.wps != 0U) ? "yes" : "no", risk_name(assessment.risk),
           finding_name(assessment.finding));
}

void logger_task(void* parameter) noexcept {
  if (parameter == nullptr) {
    vTaskDelete(nullptr);
    return;
  }
  auto& context{*static_cast<SharedContext*>(parameter)};
  bool failed{false};
  const TickType_t timeout{pdMS_TO_TICKS(15'000U)};
  for (std::uint32_t received{0U}; received < kScanCycleCount; ++received) {
    ScanReport report{};
    if (xQueueReceive(context.report_queue, &report, timeout) != pdPASS) {
      failed = true;
      break;
    }
    if ((report.cycle == 0U) || (report.cycle > kScanCycleCount) ||
        (report.reported_count > kMaximumNetworks) ||
        (report.reported_count > report.detected_count)) {
      failed = true;
      break;
    }
    ESP_LOGI(kLogTag, "Scan %" PRIu32 "/%" PRIu32 ": detected %u network(s)%s", report.cycle,
             kScanCycleCount, static_cast<unsigned>(report.detected_count),
             (report.detected_count > report.reported_count) ? " (showing strongest 20)" : "");
    for (std::uint16_t index{0U}; index < kMaximumNetworks; ++index) {
      if (index >= report.reported_count) {
        break;
      }
      log_observation(index, report.networks[index]);
    }
  }
  if (failed) {
    ESP_LOGE(kLogTag, "Logger task timed out waiting for a report");
    set_event_bits(context, kFailureBit);
  }
  set_event_bits(context, kLoggerDoneBit);
  vTaskDelete(nullptr);
}

esp_err_t set_led(const bool enabled) noexcept {
  static_assert(kActivityLed == GPIO_NUM_2);
  configASSERT(kLedTickMilliseconds > 0U);
  return gpio_set_level(kActivityLed, enabled ? 1U : 0U);
}

esp_err_t flash_report_ready() noexcept {
  constexpr std::uint32_t kFlashCount{2U};
  const TickType_t delay{pdMS_TO_TICKS(100U)};
  for (std::uint32_t flash{0U}; flash < kFlashCount; ++flash) {
    esp_err_t status{set_led(true)};
    if (status != ESP_OK) {
      return status;
    }
    vTaskDelay(delay);
    status = set_led(false);
    if (status != ESP_OK) {
      return status;
    }
    vTaskDelay(delay);
  }
  return ESP_OK;
}

void led_task(void* parameter) noexcept {
  if (parameter == nullptr) {
    vTaskDelete(nullptr);
    return;
  }
  auto& context{*static_cast<SharedContext*>(parameter)};
  esp_err_t status{gpio_reset_pin(kActivityLed)};
  if (status == ESP_OK) {
    status = gpio_set_direction(kActivityLed, GPIO_MODE_OUTPUT);
  }
  bool led_enabled{false};
  const TickType_t tick_delay{pdMS_TO_TICKS(kLedTickMilliseconds)};
  constexpr EventBits_t kActivityBits{kScanningBit | kReportReadyBit | kStopLedBit | kFailureBit};
  for (std::uint32_t tick{0U}; (tick < kMaximumLedTicks) && (status == ESP_OK); ++tick) {
    vTaskDelay(tick_delay);
    const EventBits_t bits{xEventGroupGetBits(context.events) & kActivityBits};
    if ((bits & kFailureBit) != 0U) {
      status = set_led(true);
      break;
    }
    if ((bits & kReportReadyBit) != 0U) {
      clear_event_bits(context, kReportReadyBit);
      status = flash_report_ready();
    } else if ((bits & kStopLedBit) != 0U) {
      status = set_led(false);
      break;
    } else if ((bits & kScanningBit) != 0U) {
      led_enabled = !led_enabled;
      status = set_led(led_enabled);
    } else if ((tick % 6U) == 0U) {
      led_enabled = !led_enabled;
      status = set_led(led_enabled);
    }
  }
  if (status != ESP_OK) {
    ESP_LOGE(kLogTag, "LED task failed: %s", esp_err_to_name(status));
    set_event_bits(context, kFailureBit);
  }
  set_event_bits(context, kLedDoneBit);
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

esp_err_t initialize_scanner(WifiResources& resources) noexcept {
  esp_err_t status{esp_netif_init()};
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
  status = esp_wifi_set_mode(WIFI_MODE_STA);
  if (status != ESP_OK) {
    return status;
  }
  status = esp_wifi_start();
  if (status == ESP_OK) {
    resources.wifi_started = true;
  }
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
  storage.context.report_queue = xQueueCreateStatic(
      kReportQueueDepth, sizeof(ScanReport), storage.queue_storage.data(), &storage.queue_control);
  storage.context.events = xEventGroupCreateStatic(&storage.event_control);
  return (storage.context.report_queue != nullptr) && (storage.context.events != nullptr);
}

bool start_tasks(RuntimeStorage& storage) noexcept {
  TaskHandle_t scanner{
      xTaskCreateStaticPinnedToCore(scanner_task, "scanner", 6'144U, &storage.context, 5U,
                                    storage.scanner_stack.data(), &storage.scanner_control, 0)};
  TaskHandle_t logger{xTaskCreateStaticPinnedToCore(logger_task, "logger", 4'096U, &storage.context,
                                                    3U, storage.logger_stack.data(),
                                                    &storage.logger_control, 1)};
  TaskHandle_t led{xTaskCreateStaticPinnedToCore(led_task, "activity_led", 3'072U, &storage.context,
                                                 4U, storage.led_stack.data(), &storage.led_control,
                                                 1)};
  return (scanner != nullptr) && (logger != nullptr) && (led != nullptr);
}

} // namespace

extern "C" void app_main() {
  configASSERT(kScanCycleCount > 0U);
  configASSERT(kReportQueueDepth > 0U);
  static RuntimeStorage storage{};
  WifiResources wifi_resources{};
  esp_err_t status{initialize_nvs()};
  if (status == ESP_OK) {
    status = initialize_scanner(wifi_resources);
  }
  if ((status == ESP_OK) && !initialize_runtime(storage)) {
    status = ESP_ERR_NO_MEM;
  }
  if ((status == ESP_OK) && !start_tasks(storage)) {
    status = ESP_ERR_NO_MEM;
  }
  EventBits_t completion{0U};
  if (status == ESP_OK) {
    completion = xEventGroupWaitBits(storage.context.events, kAllDoneBits, pdFALSE, pdTRUE,
                                     pdMS_TO_TICKS(90'000U));
    if ((completion & kAllDoneBits) != kAllDoneBits) {
      status = ESP_ERR_TIMEOUT;
    } else if ((completion & kFailureBit) != 0U) {
      status = ESP_FAIL;
    }
  }
  if (status == ESP_OK) {
    ESP_LOGI(kLogTag, "Concurrent scanning milestone complete");
  } else {
    ESP_LOGE(kLogTag, "Concurrent scanning milestone failed: %s", esp_err_to_name(status));
  }
  shutdown_scanner(wifi_resources);
  log_cleanup_error("NVS deinitialize", nvs_flash_deinit());
}
