# Portable Microcontroller Blink

The fundamental embedded-hardware example: turn one LED on for 500 ms, turn it off
for 500 ms, and repeat.

The same `src/main.cpp` is compiled unchanged for ESP32, Arduino Uno/AVR, STM32
Nucleo F401RE, and Raspberry Pi Pico RP2040. PlatformIO supplies each architecture's
compiler, Arduino core, board definition, and flashing tool.

This example supports boards whose LED is controlled by a normal digital GPIO.
Addressable LEDs and LEDs wired through a wireless or I/O controller require a
board-specific library or adapter; a pin number alone cannot drive them.

## Project structure

```text
Blink/
├── src/main.cpp              # Shared Arduino application
├── targets/pico-sdk/         # Optional native Pico/Pico 2 adapter
├── platformio.ini           # Board-specific pins and toolchains
├── POWER_OF_TEN.md          # Application-rule evidence and SDK boundaries
└── README.md
```

## Build

Build one target:

```sh
pio run -e esp32devkit_v1
pio run -e uno
pio run -e nucleo_f401re
pio run -e pico_rp2040
```

Build every configured target:

```sh
pio run
```

## Flash

Connect one board and select its environment:

```sh
pio run -e esp32devkit_v1 --target upload
```

No upload port is stored in the project; PlatformIO auto-detects it. The configured
board must match the connected hardware.

The optional native Pico SDK adapter is an unverified example for projects that
cannot use an Arduino core. Its setup and build commands are documented in
`../../../docs/raspberry-pi-pico-setup.md`; validate it on the exact board before
calling that target supported.

## Add another microcontroller

If the board has an Arduino-compatible core and a digital GPIO LED, add a PlatformIO
environment with its `platform`, `board`, `BLINK_LED_PIN`, and
`BLINK_LED_ACTIVE_LOW` values. The application source does not change.

Different processor architectures cannot share one firmware binary. A board without
an Arduino-compatible core needs a small adapter for its native GPIO and delay APIs;
that hardware boundary cannot be made portable in C++ alone.

See `POWER_OF_TEN.md` for the application-level rules and documented platform
boundaries.
