#ifndef WIFI_SCANNER_SECURITY_ASSESSMENT_HPP
#define WIFI_SCANNER_SECURITY_ASSESSMENT_HPP

#include <cstdint>

namespace wifi_scanner {

enum class Authentication : std::uint8_t { open, wep, legacy_wpa, transition, modern, unknown };

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
  bool legacy_tkip;
  bool unknown_cipher;
  bool wps_enabled;
};

struct SecurityAssessment final {
  RiskLevel risk;
  Finding finding;
};

[[nodiscard]] SecurityAssessment assess_security(const AdvertisedSecurity& advertised) noexcept;

} // namespace wifi_scanner

#endif
