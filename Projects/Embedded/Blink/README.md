# Portable Microcontroller Blink

A hardware-independent, non-blocking blink state machine with a thin platform
adapter. The blink behavior does not depend on Arduino, an RTOS, a processor
architecture, a GPIO register layout, or a vendor SDK.

No single firmware binary can run on every microcontroller. Each MCU family still
requires its compiler, startup code, linker script, GPIO implementation, clock, and
flashing tool. Portability here means the behavior in `blink_core` remains unchanged;
only the small adapter is replaced.

## Architecture

```text
Blink/
├── lib/blink_core/src/       # Portable C++ state machine
├── src/main.cpp              # Arduino GPIO/time adapter
├── targets/pico-sdk/         # Native RP2040 and RP2350 adapter
├── tests/                    # Native host tests
├── CMakeLists.txt            # SDK-independent host build
└── platformio.ini            # Example Arduino-capable targets
```

The core uses fixed-size values, performs no dynamic allocation, throws no
exceptions, and handles a wrapping 32-bit millisecond counter. It returns explicit
status values. The normal update path is non-blocking.

## Host validation

Run the portable logic tests before using a board:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## PlatformIO examples

The included Arduino adapter requires each target environment to define
`BLINK_LED_PIN`; it never silently guesses a pin. Example environments are provided
for ESP32 Dev Module, Arduino Uno, STM32 Nucleo F401RE, and Raspberry Pi Pico
RP2040:

```sh
pio run -e esp32dev
pio run -e uno
pio run -e nucleo_f401re
pio run -e pico_rp2040
```

Build and upload one environment with:

```sh
pio run -e esp32dev --target upload
```

Confirm the configured LED pin and whether the LED is active-high on your exact
board. Some boards require the GPIO levels in `apply_output()` to be inverted. Check
the board schematic before attaching an external LED. Always use a suitable
current-limiting resistor.

PlatformIO compiles application sources from `src/` and discovers private libraries
under `lib/`. See its [project structure documentation](https://docs.platformio.org/en/latest/core/quickstart.html).
For installation, USB permissions, environment selection, and troubleshooting, see
the repository's [PlatformIO setup guide](../../../docs/platformio-setup.md).

## Raspberry Pi Pico RP2040 and Pico 2 RP2350

Use the official Raspberry Pi Pico C/C++ SDK for both boards. SDK 2.0.0 or newer is
required for RP2350 support. Clone the SDK and expose its location:

```sh
git clone --branch master https://github.com/raspberrypi/pico-sdk.git
export PICO_SDK_PATH="$PWD/pico-sdk"
```

Build the same adapter for the original Raspberry Pi Pico RP2040:

```sh
cmake -S targets/pico-sdk -B build/pico-rp2040 -DPICO_BOARD=pico
cmake --build build/pico-rp2040
```

Build it for the Raspberry Pi Pico 2 RP2350 Arm cores:

```sh
cmake -S targets/pico-sdk -B build/pico2-rp2350 -DPICO_BOARD=pico2
cmake --build build/pico2-rp2350
```

`PICO_BOARD=pico2` selects the RP2350 platform automatically. Use separate build
directories because RP2040 and RP2350 CMake caches are not interchangeable. The
generated UF2 files are normally:

```text
build/pico-rp2040/blink_pico.uf2
build/pico2-rp2350/blink_pico.uf2
```

To flash either board, hold `BOOTSEL` while connecting it and copy the appropriate
UF2 to the mounted `RPI-RP2` volume. Alternatively, use a compatible `picotool`:

```sh
picotool load -f build/pico-rp2040/blink_pico.uf2
picotool reboot
```

The official Pico SDK uses `PICO_BOARD=pico2` for Raspberry Pi Pico 2 and supports
both RP2040 and RP2350 from the same source tree. See the
[Pico SDK](https://github.com/raspberrypi/pico-sdk) and
[Pico examples](https://github.com/raspberrypi/pico-examples) repositories.

For complete Windows, macOS, Linux, CI, compiler, SDK, `picotool`, and troubleshooting
instructions, see the repository's
[Pico development environment guide](../../../docs/raspberry-pi-pico-setup.md).

## Porting to another SDK

Keep `lib/blink_core/src/blink_controller.*` unchanged and write an adapter that:

1. Configures a safe GPIO as an output.
2. Supplies a monotonically wrapping millisecond counter to `blink::update()`.
3. Applies `Output::led_on` only when `Output::changed` is true.
4. Checks every returned `blink::Status` and drives the LED to a safe state on error.
5. Calls `blink::update()` regularly from the main loop, task, or scheduler.

Examples of target-specific functions include ESP-IDF `gpio_set_level()`, STM32 HAL
`HAL_GPIO_WritePin()`, Pico SDK `gpio_put()`, Zephyr `gpio_pin_set_dt()`, and bare-metal
register writes. Those APIs belong in separate adapters; they do not belong in the
portable state machine.

See [Adding other microcontroller platforms](../../../docs/adding-microcontroller-platforms.md)
for the adapter structure, vendor environment links, and support checklist.

## Flashing

Flashing remains target-specific. PlatformIO uses `pio run -e <environment> -t upload`
for configured boards. Vendor-native projects use tools such as `idf.py`, `picotool`,
OpenOCD, STM32CubeProgrammer, `avrdude`, or a debug probe. See the repository's
[flashing and deployment guide](../../../docs/flashing-and-deployment.md).
