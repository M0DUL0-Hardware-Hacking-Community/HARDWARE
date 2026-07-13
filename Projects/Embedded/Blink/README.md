# Native C Microcontroller Blink

Turn one LED on for 500 ms, turn it off for 500 ms, and repeat. Every project
source file is C; the firmware uses each target's native C API instead of Arduino
or C++.

| Target | Native API | LED mapping |
| --- | --- | --- |
| ESP32 DOIT DevKit V1 | ESP-IDF | GPIO 2 |
| Arduino Uno / ATmega328P | AVR registers and avr-libc | PB5 (board D13) |
| STM32 Nucleo F401RE | STM32Cube HAL | PA5 (LD2) |
| Raspberry Pi Pico / Pico 2 | Pico SDK | `PICO_DEFAULT_LED_PIN` |

These mappings are defaults for the named boards, not universal promises. Check the
exact schematic before flashing: an onboard LED may be absent, active-low, or an
addressable device that cannot be driven as a plain GPIO.

## Project structure

```text
Blink/
├── src/main.c                # PlatformIO native-C targets
├── targets/pico-sdk/main.c   # Native Pico SDK target
├── targets/pico-sdk/CMakeLists.txt
├── platformio.ini            # Toolchains, native frameworks, and calibration
├── POWER_OF_TEN.md
└── README.md
```

## Build with PlatformIO

Build one target or all three:

```sh
pio run -e esp32devkit_v1
pio run -e uno
pio run -e nucleo_f401re
pio run
```

Flash the connected board by selecting its environment:

```sh
pio run -e esp32devkit_v1 --target upload
```

No upload port is stored in the project; PlatformIO auto-detects it. The configured
board must match the connected hardware.

The RP2040 environment was removed from `platformio.ini` because the pinned official
PlatformIO RP2040 platform only provides an Arduino framework. Use the native Pico
SDK target instead:

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 \
  -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release
cmake --build build/pico-rp2040
```

For Pico 2, use a separate build directory and `-DPICO_BOARD=pico2`. See
`../../../docs/raspberry-pi-pico-setup.md` for toolchain and flashing setup.
The Pico CMake project enables a C++ compiler only because `pico_stdlib` builds an
SDK-owned C++ runtime source; the Blink target itself contains only `main.c`.

## Calibrate for real hardware

Each `platformio.ini` environment defines its native pin mapping,
`BLINK_LED_ACTIVE_LOW`, and `BLINK_HALF_PERIOD_MS`. Change those values only after
checking the board schematic. The Pico SDK equivalents are CMake cache options:

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 \
  -DPICO_BOARD=pico \
  -DBLINK_LED_PIN=PICO_DEFAULT_LED_PIN \
  -DBLINK_LED_ACTIVE_LOW=0 \
  -DBLINK_HALF_PERIOD_MS=500
```

## Add another microcontroller

Add a PlatformIO environment and the smallest target branch in `src/main.c` that
uses that platform's native GPIO and delay APIs. Different architectures cannot
share a firmware binary, and native SDKs do not share one portable GPIO interface.

See `POWER_OF_TEN.md` for the application-level checks and platform boundaries.
