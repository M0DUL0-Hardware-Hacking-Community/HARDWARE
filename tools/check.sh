#!/usr/bin/env sh
set -eu

root_dir=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root_dir"

check_format() {
  find apps firmware include platform src tests \
    Projects/Embedded/Blink/src \
    Projects/Embedded/Blink/targets/pico-sdk \
    Projects/Embedded/ConcurrentWifiScanner/src \
    Projects/Embedded/ConcurrentWifiScanner/tests \
    -type f \( -name '*.c' -o -name '*.h' \) \
    -exec "$@" --dry-run --Werror {} +
}

if command -v clang-format >/dev/null 2>&1; then
  check_format clang-format
elif command -v xcrun >/dev/null 2>&1 && xcrun --find clang-format >/dev/null 2>&1; then
  check_format xcrun clang-format
fi

cmake --preset dev
cmake --build --preset dev
ctest --preset dev

scanner_dir=Projects/Embedded/ConcurrentWifiScanner
cmake -S "$scanner_dir/tests" -B "$scanner_dir/build/tests" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$scanner_dir/build/tests"
ctest --test-dir "$scanner_dir/build/tests" --output-on-failure

if ! command -v pio >/dev/null 2>&1; then
  printf '%s\n' 'PlatformIO is required to validate the embedded firmware.' >&2
  exit 1
fi
pio run --project-dir Projects/Embedded/Blink
pio run --project-dir "$scanner_dir"

if command -v cppcheck >/dev/null 2>&1; then
  cppcheck --project=build/dev/compile_commands.json --enable=warning,style,performance,portability \
    --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem
  cppcheck --project="$scanner_dir/build/tests/compile_commands.json" \
    --enable=warning,style,performance,portability --error-exitcode=1 --inline-suppr \
    --suppress=missingIncludeSystem
  cppcheck Projects/Embedded/Blink/src/main.c Projects/Embedded/Blink/targets/pico-sdk/main.c \
    --force --std=c11 --enable=warning,style,performance,portability --error-exitcode=1 \
    --suppress=missingInclude --suppress=missingIncludeSystem -DBLINK_LED_PIN=2 \
    -DBLINK_LED_ACTIVE_LOW=0 -DBLINK_HALF_PERIOD_MS=500 \
    -DPICO_DEFAULT_LED_PIN=25 -DGPIO_OUT=1
fi
