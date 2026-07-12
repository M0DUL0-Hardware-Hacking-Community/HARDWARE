#ifndef WIFI_SCANNER_SECURITY_ASSESSMENT_HPP
#define WIFI_SCANNER_SECURITY_ASSESSMENT_HPP

#include <cstdint>

namespace wifi_scanner {

enum class Authentication : std::uint8_t {
  open,
  wep,
  wpa,
  wpa2,
  wpa_wpa2,
  wpa2_enterprise,
  wpa3,
  wpa2_wpa3,
  wapi,
  owe,
  unknown
};

enum class Cipher : std::uint8_t { other, tkip, tkip_ccmp };
enum class RiskLevel : std::uint8_t { low, medium, high, critical };
enum class Finding : std::uint8_t {
  none,
  open_network,
  obsolete_wep,
  legacy_wpa,
  transition_mode,
  legacy_tkip,
  wps_enabled,
  unknown_security
};

struct AdvertisedSecurity final {
  Authentication authentication;
  Cipher pairwise_cipher;
  bool wps_enabled;
};

struct SecurityAssessment final {
  RiskLevel risk;
  Finding finding;
};

[[nodiscard]] SecurityAssessment assess_security(const AdvertisedSecurity& advertised) noexcept;

} // namespace wifi_scanner

#endif
