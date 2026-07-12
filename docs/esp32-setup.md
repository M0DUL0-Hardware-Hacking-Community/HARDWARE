# ESP32 Development Environment

ESP32 development in this repository can use PlatformIO with Arduino today. A future
native target can use ESP-IDF without changing the portable Blink state machine.

Official references:

- [ESP-IDF Installation Manager](https://docs.espressif.com/projects/idf-im-cli/en/latest/installation.html)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/)
- [`idf.py` command reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-py.html)
- [PlatformIO setup](platformio-setup.md)

## PlatformIO workflow

Install PlatformIO using the repository guide, then run from
`Projects/Embedded/Blink`:

```sh
pio run -e esp32dev
pio run -e esp32dev --target upload
```

The generic ESP32 Dev Module environment configures GPIO 2 for the example LED.
Confirm this mapping against the exact board schematic. ESP32 boards vary, and some
onboard LEDs are active-low, addressable RGB devices, or absent.

PlatformIO produces the firmware under `.pio/build/esp32dev/`. Use `pio device list`
to inspect ports and `--upload-port` when more than one device is connected.

## Native ESP-IDF workflow

Use Espressif's current Installation Manager on Windows, macOS, or Linux. It installs
the selected ESP-IDF release, Python environment, CMake/Ninja integration, compiler,
OpenOCD, and flashing tools together.

On Windows, run the manager from 64-bit PowerShell. After installation, start the
generated IDF PowerShell shortcut. On macOS and Linux, source the activation script
created in the selected installation directory; source it rather than executing it.

Verify the activated environment:

```sh
idf.py --version
python --version
cmake --version
```

Within an ESP-IDF project:

```sh
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `esp32` with the actual chip, such as `esp32c3` or `esp32s3`, and replace the
port for the host operating system. Exit the serial monitor with `Ctrl+]`.

The current Blink directory is a PlatformIO project, not an ESP-IDF project. Native
ESP-IDF support requires a `targets/esp-idf/` project whose `app_main()` adapter uses
ESP-IDF GPIO and time APIs while linking `blink_core`.

## USB and boot mode

- Use a USB data cable.
- Install the USB-UART/JTAG driver required by the exact development board.
- On Linux, add the narrow udev/serial permissions recommended by the toolchain.
- If automatic reset fails, hold `BOOT`, start upload, then release when writing begins.
- Do not assume GPIO 2 is safe on a custom board or every ESP32 module.

## Release discipline

- Pin an ESP-IDF or PlatformIO platform version.
- Record the exact ESP32 target and board revision.
- Preserve `sdkconfig.defaults` or checked configuration inputs.
- Verify partition layout, flash size, secure boot, and flash encryption separately.
- Test recovery after interrupted writes and failed over-the-air updates.
