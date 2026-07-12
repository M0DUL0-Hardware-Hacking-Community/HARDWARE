#include "security_assessment.hpp"

#include <array>
#include <cstddef>

namespace {

struct TestCase final {
  wifi_scanner::AdvertisedSecurity advertised;
  wifi_scanner::SecurityAssessment expected;
};

constexpr std::array<TestCase, 12U> kTestCases{{
    {{wifi_scanner::Authentication::open, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::critical, wifi_scanner::Finding::open_network}},
    {{wifi_scanner::Authentication::wep, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::critical, wifi_scanner::Finding::obsolete_wep}},
    {{wifi_scanner::Authentication::wpa, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_wpa}},
    {{wifi_scanner::Authentication::wpa_wpa2, wifi_scanner::Cipher::other, true},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_wpa}},
    {{wifi_scanner::Authentication::wpa2, wifi_scanner::Cipher::tkip, false},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_tkip}},
    {{wifi_scanner::Authentication::wpa2_wpa3, wifi_scanner::Cipher::tkip_ccmp, false},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::legacy_tkip}},
    {{wifi_scanner::Authentication::wpa2_wpa3, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::medium, wifi_scanner::Finding::transition_mode}},
    {{wifi_scanner::Authentication::wpa2, wifi_scanner::Cipher::other, true},
     {wifi_scanner::RiskLevel::medium, wifi_scanner::Finding::wps_enabled}},
    {{wifi_scanner::Authentication::unknown, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::high, wifi_scanner::Finding::unknown_security}},
    {{wifi_scanner::Authentication::wpa2, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::low, wifi_scanner::Finding::none}},
    {{wifi_scanner::Authentication::wpa3, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::low, wifi_scanner::Finding::none}},
    {{wifi_scanner::Authentication::owe, wifi_scanner::Cipher::other, false},
     {wifi_scanner::RiskLevel::low, wifi_scanner::Finding::none}},
}};

} // namespace

int main() {
  static_assert(!kTestCases.empty());
  static_assert(kTestCases.size() == 12U);
  for (std::size_t index{0U}; index < kTestCases.size(); ++index) {
    const wifi_scanner::SecurityAssessment actual{
        wifi_scanner::assess_security(kTestCases[index].advertised)};
    if ((actual.risk != kTestCases[index].expected.risk) ||
        (actual.finding != kTestCases[index].expected.finding)) {
      return static_cast<int>(index + 1U);
    }
  }
  return 0;
}
