# Firmware Flashing and Device Deployment

This repository separates portable application logic from board-specific SDKs and
deployment tools. It does not use one universal flashing command because ESP32
microcontrollers, Raspberry Pi Linux computers, and Raspberry Pi Pico devices have
different build and boot models.

## Deployment overview

| Target | Build environment | Deployment method |
| --- | --- | --- |
| ESP32 family | ESP-IDF | Flash over USB/UART with `idf.py` |
| Raspberry Pi 4/5/Zero | Native or cross-compiled Linux | Copy and run a Linux executable |
| Raspberry Pi Pico/Pico W | Pico SDK | Copy a UF2 or use `picotool` |
| Other microcontrollers | Vendor SDK/toolchain | Bootloader, programmer, or debug probe |

The existing `cmake/toolchains/arm-none-eabi.cmake` is only a baseline for generic
ARM bare-metal projects. A deployable target also needs the correct SDK, compiler,
startup code, linker configuration, board definitions, and platform adapter. It is
not suitable for ESP32 variants, which can use Xtensa or RISC-V processors.

## Recommended target layout

```text
apps/
    linux_gateway/
firmware/
include/
platform/
    host/
    esp32/
    raspberry_pi/
    pico/
src/
targets/
    esp32/
    pico/
```

Portable code remains in `include/`, `src/`, and `firmware/`. A platform adapter
implements `power10/platform/device_io.hpp` using the target SDK. Each
`targets/<board>/` directory owns:

- Vendor SDK initialization
- Startup and linker configuration
- Board and pin definitions
- Network and persistent-storage initialization
- The platform adapter implementation
- Build, flash, monitor, and debug configuration

SDK headers and conditional compilation should remain inside the relevant platform
and target directories. They should not leak into portable domain code.

## ESP32 with ESP-IDF

Use Espressif's ESP-IDF toolchain and project format. An ESP32 target can be arranged
as follows:

```text
targets/esp32/
├── CMakeLists.txt
├── sdkconfig.defaults
└── main/
    ├── CMakeLists.txt
    ├── app_main.cpp
    └── device_io.cpp
```

`device_io.cpp` implements the platform interface with ESP-IDF sensor, network, and
transport APIs. `app_main.cpp` initializes the device and invokes the bounded
firmware cycle. The ESP-IDF component can compile the portable sources from `src/`
and `firmware/`.

After installing and activating ESP-IDF:

```sh
cd targets/esp32
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `esp32` with the actual target, such as `esp32c3` or `esp32s3`. The `flash`
command builds when necessary, writes the firmware, and `monitor` displays serial
output. Exit the ESP-IDF monitor with `Ctrl+]`.

Common serial device names are:

| Host | Typical port |
| --- | --- |
| Linux | `/dev/ttyUSB0` or `/dev/ttyACM0` |
| macOS | `/dev/cu.usbserial-*` or `/dev/cu.usbmodem*` |
| Windows | `COM3`, `COM4`, or another Device Manager port |

Linux users may need membership in the distribution's serial-access group, commonly
`dialout`, followed by a logout and login. Do not solve serial permissions by
running the entire development toolchain as root.

See the official [ESP-IDF `idf.py` documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-py.html).

## Raspberry Pi Linux computers

Raspberry Pi 4, 5, Zero, and similar boards are Linux computers. Normally, Raspberry
Pi OS is written to an SD card or other boot device once. Application updates are
then deployed as Linux executables rather than firmware images.

Use Raspberry Pi Imager to prepare the boot device and optionally configure the
hostname, user, network, and SSH access. See the official
[Raspberry Pi installation guide](https://www.raspberrypi.com/documentation/computers/getting-started.html).

### Build directly on the Pi

```sh
ssh pi@your-pi.local
git clone <repository-url>
cd <repository-directory>

cmake -S . -B build/pi \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF
cmake --build build/pi
./build/pi/apps/linux_gateway/power10_iot_sim
```

### Deploy a previously built executable

When the executable was built for the Pi's processor and Linux ABI:

```sh
scp build/pi/apps/linux_gateway/power10_iot_sim \
  pi@your-pi.local:/opt/power10/
```

A binary built on an x86-64 computer will not run on an ARM Raspberry Pi. Either
build directly on the Pi or configure a matching Linux cross-compiler and sysroot.
For production, install the executable as a `systemd` service with a dedicated,
unprivileged user, explicit restart limits, and only the device permissions it needs.

GPIO, I2C, SPI, UART, and networking implementations belong in
`platform/raspberry_pi/`. Keep Linux-specific code out of the portable core.

## Raspberry Pi Pico and Pico W

Pico boards are microcontrollers, not Linux computers. Use the Raspberry Pi Pico SDK
to produce an ELF and UF2 firmware image. A target can be arranged as:

```text
targets/pico/
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── main.cpp
└── device_io.cpp
```

There are two common deployment methods:

1. Hold `BOOTSEL` while connecting or resetting the Pico.
2. Copy the generated `.uf2` file to the mounted `RPI-RP2` storage device.

Alternatively, use `picotool` with a supported connection:

```sh
picotool load build/pico/power10_firmware.uf2
picotool reboot
```

Exact options depend on the Pico generation, connection, and installed `picotool`
version. Consult the official
[Getting Started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
guide when setting up the SDK and programmer.

This repository's [Pico development environment guide](raspberry-pi-pico-setup.md)
covers installation on Windows, macOS, Linux, and headless build machines.

## Adding another device family

For an STM32, Nordic nRF, RP2040/RP2350, or another MCU:

1. Install the vendor-supported SDK and compiler.
2. Add `targets/<family>/` with the SDK entry point and build integration.
3. Add `platform/<family>/` implementing `device_io.hpp`.
4. Link or include the portable `power10` sources without modifying their behavior.
5. Add a documented CMake preset or vendor build command.
6. Add separate commands for build, flash, monitor, and debug.
7. Test portable code on the host and hardware-dependent code on the actual board.

Do not assume that a successful host build proves correct timing, memory use,
electrical configuration, watchdog behavior, or network recovery on real hardware.

## Production considerations

- Never commit Wi-Fi passwords, private keys, certificates, or production tokens.
- Verify firmware authenticity and use secure boot when the target supports it.
- Use encrypted transport and authenticate the remote endpoint.
- Bound connection attempts, retries, message sizes, waits, and firmware loops.
- Validate all data crossing sensor, storage, network, and update boundaries.
- Maintain a recovery image or rollback mechanism for over-the-air updates.
- Record the exact SDK, compiler, board revision, partition map, and build version.
- Test power loss during storage writes and firmware updates.
- Treat physical flashing and production provisioning as separate operations.

The portable core should remain independently testable on Linux and macOS. Hardware
tests should run against each supported board before releasing firmware.

## Development environment guides

- [Environment index](development-environments.md)
- [PlatformIO](platformio-setup.md)
- [ESP32 and ESP-IDF](esp32-setup.md)
- [Arduino AVR and Uno](arduino-avr-setup.md)
- [STM32 and Nucleo](stm32-setup.md)
- [Raspberry Pi Pico and Pico 2](raspberry-pi-pico-setup.md)
- [Adding other microcontroller platforms](adding-microcontroller-platforms.md)
