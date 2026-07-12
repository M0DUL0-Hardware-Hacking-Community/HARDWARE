# Microcontroller Development Environments

Use this index to choose a toolchain for a board before building or flashing
firmware. Prefer one managed environment per project and avoid putting multiple
SDK-owned Python packages into the same global Python installation.

## Current Blink targets

| Target | Unified workflow | Vendor-native workflow |
| --- | --- | --- |
| ESP32 Dev Module | [PlatformIO](platformio-setup.md) | [ESP-IDF](esp32-setup.md) |
| Arduino Uno / AVR | [PlatformIO](platformio-setup.md) | [Arduino CLI](arduino-avr-setup.md) |
| STM32 Nucleo F401RE | [PlatformIO](platformio-setup.md) | [STM32Cube](stm32-setup.md) |
| Raspberry Pi Pico RP2040 | PlatformIO or Pico SDK | [Pico SDK](raspberry-pi-pico-setup.md) |
| Raspberry Pi Pico 2 RP2350 | Pico SDK | [Pico SDK](raspberry-pi-pico-setup.md) |

The portable Blink state machine is independent of these tools. PlatformIO currently
builds its Arduino adapter. Pico SDK builds a native GPIO adapter for RP2040 and
RP2350. ESP-IDF, STM32 HAL, Arduino CLI, Zephyr, and other native environments need
their own thin adapter before they can build this repository's portable logic.

## Guides

- [PlatformIO on Windows, macOS, and Linux](platformio-setup.md)
- [ESP32 with PlatformIO and ESP-IDF](esp32-setup.md)
- [Arduino AVR and Arduino Uno](arduino-avr-setup.md)
- [STM32 and Nucleo boards](stm32-setup.md)
- [Raspberry Pi Pico RP2040 and Pico 2 RP2350](raspberry-pi-pico-setup.md)
- [Adding other microcontroller families](adding-microcontroller-platforms.md)
- [Flashing and deployment](flashing-and-deployment.md)

## Toolchain rules

- Record the board identifier, SDK version, compiler version, and programmer.
- Pin versions for CI and released firmware.
- Keep separate build directories for different processors or ABI configurations.
- Do not run IDEs, compilers, or package managers as root.
- Install Linux USB/udev rules narrowly instead of making devices world-writable.
- Verify a host test, cross-build, flash, reboot, and hardware behavior separately.
- Never treat a successful compile as proof that pin mapping or electrical wiring is safe.
