# IoT Architecture

The project separates safety-critical logic from vendor SDKs and operating systems.

- `include/power10` and `src` contain portable, deterministic domain logic.
- `firmware` coordinates one bounded read, validate, and publish cycle.
- `platform/host` implements device I/O for Linux and macOS simulation.
- `apps/linux_gateway` is a host executable for integration and gateway development.
- `platform/<target>` is where an MCU, RTOS, modem, or board adapter belongs.
- `protocols` contains transport-neutral message contracts.
- `config` contains non-secret configuration examples only.
- `cmake/toolchains` contains cross-compilation definitions.

Platform adapters implement `power10/platform/device_io.hpp`. Keep SDK headers and
conditional compilation inside the adapter; neither may leak into `src` or public
domain interfaces. The interface deliberately avoids callbacks, dynamic ownership,
and exceptions.

Device secrets, certificates, Wi-Fi credentials, and production endpoints must not
be committed. Provision them with the target's secure storage. Network transports
must authenticate peers, validate message lengths, enforce timeouts and retry bounds,
and expose failures through explicit status values.

## Cross-compilation

The ARM toolchain file is a baseline for bare-metal GCC projects. A board SDK may
provide its own toolchain file and startup/linker configuration:

```sh
cmake -S . -B build/arm \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/arm-none-eabi.cmake
cmake --build build/arm
```

Cross builds compile the portable core only. Add a target adapter and firmware entry
point in the board integration project; host tests continue to run natively.
