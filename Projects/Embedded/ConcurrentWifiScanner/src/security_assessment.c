#include "security_assessment.h"

wifi_scanner_security_assessment_t
wifi_scanner_assess_security(const wifi_scanner_advertised_security_t advertised) {
  if (advertised.authentication == WIFI_SCANNER_AUTHENTICATION_OPEN) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_CRITICAL,
                                                WIFI_SCANNER_FINDING_OPEN_NETWORK};
  }
  if (advertised.authentication == WIFI_SCANNER_AUTHENTICATION_WEP) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_CRITICAL,
                                                WIFI_SCANNER_FINDING_OBSOLETE_WEP};
  }
  if (advertised.authentication == WIFI_SCANNER_AUTHENTICATION_LEGACY_WPA) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_HIGH,
                                                WIFI_SCANNER_FINDING_LEGACY_WPA};
  }
  if (advertised.legacy_tkip) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_HIGH,
                                                WIFI_SCANNER_FINDING_LEGACY_TKIP};
  }
  if ((advertised.authentication == WIFI_SCANNER_AUTHENTICATION_UNKNOWN) ||
      advertised.unknown_cipher) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_HIGH,
                                                WIFI_SCANNER_FINDING_UNKNOWN_SECURITY};
  }
  if (advertised.authentication == WIFI_SCANNER_AUTHENTICATION_TRANSITION) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_MEDIUM,
                                                WIFI_SCANNER_FINDING_TRANSITION_MODE};
  }
  if (advertised.authentication != WIFI_SCANNER_AUTHENTICATION_MODERN) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_HIGH,
                                                WIFI_SCANNER_FINDING_UNKNOWN_SECURITY};
  }
  if (advertised.wps_enabled) {
    return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_MEDIUM,
                                                WIFI_SCANNER_FINDING_WPS_ENABLED};
  }
  return (wifi_scanner_security_assessment_t){WIFI_SCANNER_RISK_LOW, WIFI_SCANNER_FINDING_NONE};
}
