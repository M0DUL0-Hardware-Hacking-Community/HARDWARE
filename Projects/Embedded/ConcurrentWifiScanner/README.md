# Bounded Passive Wi-Fi Scanner

A C11 ESP32 application that passively surveys the 2.4 GHz channels permitted by
the configured regulatory country, blinks an activity LED during each scan, and
prints one detailed report after the bounded run. All project application and host
test sources are native C; there are no C++ application sources or language features.

The ESP32 has one Wi-Fi radio, so it cannot listen to every channel at the same
instant. It scans the allowed channels sequentially while a separate LED task runs
concurrently. The scanner never associates, authenticates, injects frames,
deauthenticates clients, or intentionally sends probe requests.

## Architecture

```text
ConcurrentWifiScanner/
├── src/main.c                    # ESP-IDF scan, LED, lifecycle, and reporting
├── src/network_catalog.c/.h     # Fixed-capacity BSSID aggregation
├── src/security_assessment.c/.h # Deterministic beacon-security labels
├── tests/                       # Native C11 host test
├── platformio.ini               # ESP32 DevKit V1 build configuration
└── POWER_OF_TEN.md              # Application-rule evidence and deviations
```

`app_main` owns the Wi-Fi driver, scan results, catalog, and final report. One
statically allocated FreeRTOS task controls the LED from event bits. Scan data uses
fixed-capacity storage and is never shared with the LED task.

## Hardware and configuration

The PlatformIO environment targets an ESP32 DOIT DevKit V1 using ESP-IDF. GPIO 2 is
configured as an active-high activity LED; confirm that pin and polarity against the
exact board schematic before connecting an external LED, and use a suitable
current-limiting resistor.

The regulatory country is `MY`. Change `COUNTRY_CODE` in `src/main.c` only when the
board will operate in another country, and use that country's supported ESP-IDF
country code. This setting determines which 2.4 GHz channels the radio may scan.

## Scan behavior

- Run three passive all-channel sweeps with 360 ms dwell per channel.
- Blink GPIO 2 every 150 ms during a sweep and turn it off between sweeps.
- Retrieve every driver scan record individually.
- Merge repeated observations by BSSID in a fixed 128-entry catalog.
- Preserve the latest metadata, strongest RSSI, first and last sweep, and sighting
  count.
- Print the catalog only after scanning stops, including partial results after an
  error.
- Leave the LED off after success and request an on state after runtime scan or LED
  failure. Failures before GPIO/task initialization cannot guarantee an LED signal.

"Complete" means every BSS observed during the three sweeps fit in the catalog. It
does not guarantee that every network physically in range transmitted a beacon
during the observation window. A hidden network can be identified by BSSID while
still advertising an empty SSID.

## Report contents

The final summary shows the effective scanner country and channel range, passive
dwell, completed sweeps, detected and retrieved records, unique and duplicate
BSSIDs, hidden SSIDs, invalid records, dropped observations, and completion state.

Each retained BSSID includes its escaped SSID, signal history, channel, bandwidth,
receiving antenna, authentication mode, pairwise and group ciphers, WPS flag,
every public ESP-IDF AP-record capability, FTM and VHT fields, advertised country
fields, HE fields, and a passive security assessment. Every continuation line repeats
the network number, and the scanner country appears in the final line of the report.
`scanner_country=MY` is the ESP32 regulatory setting;
`advertised_country` is the optional Country Information Element sent by that AP.
`not-advertised` means the AP omitted it; `invalid` means its two code bytes were not
uppercase ISO letters. The scanner does not invent a country for either case.

The security assessment is a beacon-level heuristic, not proof that a network is
safe or vulnerable. Raw information elements, client discovery, vendor lookup,
5 GHz capture, and packet injection are outside this project's scope.

## Host validation

Build and run the strict ISO C11 catalog and security test before using a board:

```sh
cmake -S tests -B build/tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

The firmware component uses GNU C11 because ESP-IDF's Xtensa headers require GNU
inline assembly; project application code itself stays within C11.

## Build, flash, and monitor

```sh
pio run
pio run --target upload
pio device monitor
```

`platformio.ini` deliberately omits `upload_port` and `monitor_port`, so PlatformIO
auto-detects the connected serial device on macOS, Linux, and Windows. If several
compatible serial devices are attached, select one for that invocation instead of
hardcoding it in the project:

```sh
pio device list
pio run --target upload --upload-port <port>
pio device monitor --port <port> --baud 115200
```

Exit the serial monitor with `Ctrl+]`. See `POWER_OF_TEN.md` for the application
rules and the documented ESP-IDF and FreeRTOS boundary exceptions.
