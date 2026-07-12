#include "security_assessment.hpp"

namespace wifi_scanner {

SecurityAssessment assess_security(const AdvertisedSecurity& advertised) noexcept {
  if (advertised.authentication == Authentication::open) {
    return {RiskLevel::critical, Finding::open_network};
  }
  if (advertised.authentication == Authentication::wep) {
    return {RiskLevel::critical, Finding::obsolete_wep};
  }
  if ((advertised.authentication == Authentication::wpa) ||
      (advertised.authentication == Authentication::wpa_wpa2)) {
    return {RiskLevel::high, Finding::legacy_wpa};
  }
  if ((advertised.pairwise_cipher == Cipher::tkip) ||
      (advertised.pairwise_cipher == Cipher::tkip_ccmp)) {
    return {RiskLevel::high, Finding::legacy_tkip};
  }
  if (advertised.authentication == Authentication::wpa2_wpa3) {
    return {RiskLevel::medium, Finding::transition_mode};
  }
  if (advertised.wps_enabled) {
    return {RiskLevel::medium, Finding::wps_enabled};
  }
  if (advertised.authentication == Authentication::unknown) {
    return {RiskLevel::high, Finding::unknown_security};
  }
  return {RiskLevel::low, Finding::none};
}

} // namespace wifi_scanner
