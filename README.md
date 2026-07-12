# HARDWARE

Hardware hacking resources and a portable C++20 environment for IoT firmware,
device simulation, testing, and deployment.

## Repository Areas

- [Course materials](Course-Materials/)
- [Hardware setup guides](Setup/)
- [Very easy exercises](Very-Easy/)
- [Easy exercises](Easy/)
- [Medium exercises](Medium/)
- [Hard exercises](Hard/)
- [Firmware flashing and device deployment](docs/flashing-and-deployment.md)
- [Microcontroller development environments](docs/development-environments.md)
- [PlatformIO development environment](docs/platformio-setup.md)
- [IoT architecture](docs/iot-architecture.md)
- [Safety-critical coding standard](docs/coding-standard.md)

## Projects

- [Portable microcontroller Blink](Projects/Embedded/Blink/)
- [Concurrent ESP32 Wi-Fi Scanner](Projects/Embedded/ConcurrentWifiScanner/)

The difficulty directories contain Hack The Box Hardware, ICS, and satellite
challenges. Standalone embedded development tutorials belong under `Projects/`.

## C++ and IoT Development

The repository includes a strict, dependency-free C++20 IoT project structured
around Gerard J. Holzmann's *The Power of 10: Rules for Developing Safety-Critical
Code*. The original paper targets C; [the coding standard](docs/coding-standard.md)
defines this project's C++ interpretation.

The project keeps portable domain logic separate from firmware, platform adapters,
board targets, protocols, configuration, and host applications.

### Build and test

Prerequisites are CMake 3.25+ and a C++20 compiler. CMake selects the platform's
native build backend automatically. Make, Ninja, Xcode, and Visual Studio can also
be selected explicitly with CMake's `-G` option.

```sh
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Run the host device simulator with:

```sh
./build/dev/apps/linux_gateway/power10_iot_sim
```

For static analysis, install `clang-tidy` and configure with
`cmake --preset analysis`. The `tools/check.sh` helper runs formatting checks, the
strict build, tests, and `cppcheck` when those tools are installed. It is POSIX
`sh` compatible and runs on Linux and macOS without shell-specific extensions.

The presets contain no absolute paths or fixed generator. For a manual out-of-tree
build on any platform:

```sh
cmake -S . -B build/local -DBUILD_TESTING=ON
cmake --build build/local --config Debug
ctest --test-dir build/local -C Debug --output-on-failure
```

### Project layout

- `include/power10/`: public interfaces
- `src/`: platform-independent device and domain logic
- `firmware/`: bounded device-cycle orchestration
- `platform/`: host, board, RTOS, and transport adapters
- `apps/linux_gateway/`: native IoT simulator and gateway executable
- `tests/`: dependency-free unit tests
- `protocols/`: transport-neutral telemetry contracts
- `config/`: non-secret configuration templates
- `cmake/toolchains/`: cross-compilation definitions
- `docs/`: architecture, coding standard, checklist, and source paper
- `tools/`: local quality checks
- `.github/workflows/`: continuous integration

See the [flashing and deployment guide](docs/flashing-and-deployment.md) for ESP32,
Raspberry Pi Linux, Pico/Pico W, cross-compilation, and deployment workflows.

Production code must not depend on code under `tests/`. Add each new production
module to `src/CMakeLists.txt` and its tests to `tests/CMakeLists.txt`.
