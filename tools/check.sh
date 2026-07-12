#!/usr/bin/env sh
set -eu

root_dir=$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root_dir"

if command -v clang-format >/dev/null 2>&1; then
  find apps firmware include platform src tests -type f \( -name '*.cpp' -o -name '*.hpp' \) \
    -exec clang-format --dry-run --Werror {} +
fi

cmake --preset dev
cmake --build --preset dev
ctest --preset dev

if command -v cppcheck >/dev/null 2>&1; then
  cppcheck --project=build/dev/compile_commands.json --enable=warning,style,performance,portability \
    --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem
fi
