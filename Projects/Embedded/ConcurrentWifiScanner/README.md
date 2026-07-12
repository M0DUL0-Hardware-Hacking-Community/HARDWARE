# Concurrent Wi-Fi Scanner

An incremental ESP32 learning project that will monitor the active Wi-Fi
connection, scan nearby networks, and publish bounded telemetry to a local MQTT
broker. Application code follows the repository's C++ interpretation of Gerard
J. Holzmann's Power of Ten rules.

## Milestone 1: verified ESP-IDF heartbeat

This milestone deliberately contains no Wi-Fi, MQTT, or user-created tasks. It
first verifies that the ESP32 DevKit V1 can build and run a native ESP-IDF C++20
application. The application logs chip information and ten one-second
heartbeats before returning from `app_main`.

The bootloader reports 4 MiB of physical flash. `sdkconfig.defaults` records that
size explicitly so the binary header and the detected hardware agree.

Power of Ten properties introduced here:

- the heartbeat loop has a fixed upper bound;
- all application storage has a fixed size;
- no recursion, heap allocation, exceptions, or preprocessor macros are added
  by the application;
- the fallible flash query is checked explicitly;
- runtime assertions document important timing and bound assumptions;
- strict compiler warnings are treated as errors for the application component.
- GCC's interprocedural static analyzer runs on the application component.
- exceptions and RTTI are disabled for the application component.

ESP-IDF, FreeRTOS, and their drivers are third-party boundary code and are not
claimed to comply with the project's application-level rules.

## Milestone 2: bounded nearby-network scanner

The application now performs three active scans, separated by five seconds. Each
scan reports up to the 20 strongest nearby networks with SSID, RSSI, channel, and
authentication mode. Hidden networks are reported without inventing a name.

The ESP32-D0WD radio detects 2.4 GHz Wi-Fi only; 5 GHz networks cannot appear in
these results. Scan records use a fixed-capacity application array. ESP-IDF keeps
an internal scan list during each scan and releases it when the application
retrieves the records; that allocation is third-party boundary behavior.

## Milestone 3: concurrent activity and passive security assessment

Three statically allocated application tasks cooperate through a bounded queue
and event group:

- the scanner task runs on CPU 0 and produces three fixed-capacity reports;
- the logger task runs on CPU 1 and consumes reports without touching the radio;
- the LED task runs on CPU 1 and exclusively owns GPIO 2.

The LED blinks rapidly while scanning, flashes twice when a report is ready,
blinks slowly between scans, remains on for an error, and turns off when all
bounded work completes.

Each observation receives a passive assessment based only on advertised fields.
Open networks and WEP are critical findings; legacy WPA or TKIP are high;
WPA2/WPA3 transition mode and advertised WPS are medium. A low result means only
that no obvious issue appears in beacon/probe metadata. It does not prove that a
network is secure. The scanner performs no authentication, injection,
deauthentication, password testing, or exploitation.

FreeRTOS task entry points are required function pointers at the RTOS boundary.
They are isolated and validate their context, forming a documented exception to
Power of Ten rule 1. All application task, stack, queue, and event-group storage
is static, and every application loop has a visible upper bound.

## Build, upload, and monitor

```sh
pio run
pio run --target upload
pio device monitor
```

Exit the serial monitor with `Ctrl+]`.

## Host tests

The passive security classifier is independent of ESP-IDF and has bounded host
tests covering all classification branches:

```sh
cmake -S tests -B build/tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

See `POWER_OF_TEN.md` for the rule-by-rule compliance evidence and documented
ESP-IDF/FreeRTOS boundary exceptions.
