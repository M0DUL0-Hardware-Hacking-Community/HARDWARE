# AVR and Arduino Uno Development Environment

The Blink project supports Arduino Uno with PlatformIO's Atmel AVR platform.
`src/main.c` is C11 application code built with AVR GCC and avr-libc; it does not
use the Arduino framework or sketch runtime.

Official references:

- [PlatformIO setup](platformio-setup.md)
- [Arduino Uno board documentation](https://docs.arduino.cc/hardware/uno-rev3)

## PlatformIO workflow

From `Projects/Embedded/Blink`:

```sh
pio run -e uno
pio run -e uno --target upload
```

The `uno` environment writes the onboard LED through AVR port bit PB5, which the
Uno Rev3 exposes as digital pin 13. Confirm the exact board before upload; clones
can use a different USB-serial interface or bootloader.

PlatformIO installs the matching AVR GCC, avr-libc, and uploader packages in its
isolated package store. Inspect and record them with:

```sh
pio pkg list -e uno
```

Do not install a second global toolchain unless another project needs it. If one is
already installed, verify that PlatformIO is still using the pinned project packages
rather than whichever compiler happens to appear first on `PATH`.

## Hardware and upload checks

- Use a known USB data cable.
- Close serial monitors before uploading.
- Install the USB-serial driver required by a clone's interface chip.
- Never connect an LED without a current-limiting resistor.
- Respect AVR GPIO voltage and current limits from the board and MCU datasheets.
- Use the port reported by `pio device list` when automatic detection is ambiguous.

Windows uses `COM` ports. macOS commonly uses `/dev/cu.usbmodem*` or
`/dev/cu.usbserial*`; Linux commonly uses `/dev/ttyACM0` or `/dev/ttyUSB0`.

## Reproducibility

Pin the PlatformIO Atmel AVR platform, then record `pio --version`, `pio pkg list -e
uno`, the exact board revision, and the uploader. Build, upload, reset, and observe
the real LED before treating a toolchain update as verified.
