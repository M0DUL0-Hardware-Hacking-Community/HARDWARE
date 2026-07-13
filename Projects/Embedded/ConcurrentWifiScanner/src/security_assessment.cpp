#include "security_assessment.hpp"

namespace wifi_scanner {

SecurityAssessment assess_security(const AdvertisedSecurity& advertised) noexcept {
  if (advertised.authentication == Authentication::open) {
    return {RiskLevel::critical, Finding::open_network};
  }
  if (advertised.authentication == Authentication::wep) {
    return {RiskLevel::critical, Finding::obsolete_wep};
  }
  if (advertised.authentication == Authentication::legacy_wpa) {
    return {RiskLevel::high, Finding::legacy_wpa};
  }
  if (advertised.legacy_tkip) {
    return {RiskLevel::high, Finding::legacy_tkip};
  }
  if ((advertised.authentication == Authentication::unknown) || advertised.unknown_cipher) {
    return {RiskLevel::high, Finding::unknown_security};
  }
  if (advertised.authentication == Authentication::transition) {
    return {RiskLevel::medium, Finding::transition_mode};
  }
  if (advertised.wps_enabled) {
    return {RiskLevel::medium, Finding::wps_enabled};
  }
  return {RiskLevel::low, Finding::none};
}

} // namespace wifi_scanner
