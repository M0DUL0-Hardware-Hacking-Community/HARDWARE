#ifndef WIFI_SCANNER_SECURITY_ASSESSMENT_H
#define WIFI_SCANNER_SECURITY_ASSESSMENT_H

#include <stdbool.h>

typedef enum {
  WIFI_SCANNER_AUTHENTICATION_OPEN,
  WIFI_SCANNER_AUTHENTICATION_WEP,
  WIFI_SCANNER_AUTHENTICATION_LEGACY_WPA,
  WIFI_SCANNER_AUTHENTICATION_TRANSITION,
  WIFI_SCANNER_AUTHENTICATION_MODERN,
  WIFI_SCANNER_AUTHENTICATION_UNKNOWN
} wifi_scanner_authentication_t;

typedef enum {
  WIFI_SCANNER_RISK_LOW,
  WIFI_SCANNER_RISK_MEDIUM,
  WIFI_SCANNER_RISK_HIGH,
  WIFI_SCANNER_RISK_CRITICAL
} wifi_scanner_risk_level_t;

typedef enum {
  WIFI_SCANNER_FINDING_NONE,
  WIFI_SCANNER_FINDING_OPEN_NETWORK,
  WIFI_SCANNER_FINDING_OBSOLETE_WEP,
  WIFI_SCANNER_FINDING_LEGACY_WPA,
  WIFI_SCANNER_FINDING_TRANSITION_MODE,
  WIFI_SCANNER_FINDING_LEGACY_TKIP,
  WIFI_SCANNER_FINDING_WPS_ENABLED,
  WIFI_SCANNER_FINDING_UNKNOWN_SECURITY
} wifi_scanner_finding_t;

typedef struct {
  wifi_scanner_authentication_t authentication;
  bool legacy_tkip;
  bool unknown_cipher;
  bool wps_enabled;
} wifi_scanner_advertised_security_t;

typedef struct {
  wifi_scanner_risk_level_t risk;
  wifi_scanner_finding_t finding;
} wifi_scanner_security_assessment_t;

wifi_scanner_security_assessment_t
wifi_scanner_assess_security(wifi_scanner_advertised_security_t advertised);

#endif
