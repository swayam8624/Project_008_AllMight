#!/usr/bin/env bash
set -Eeuo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
build="$(mktemp -d)"
trap 'rm -rf "$build"' EXIT

compile_and_run() {
    local compiler="$1"; shift
    local mode="$1"; shift
    "$compiler" -std=c++20 -Wall -Wextra -Wpedantic -Werror \
       "$@" "$here/asset_catalog.cpp" "$here/m018_lab.cpp" \
       -o "$build/m018-$mode"
    "$build/m018-$mode"
}
# Checks use exceptions rather than assert(), including in -DNDEBUG mode.
compile_and_run g++ release -O2 -DNDEBUG
compile_and_run clang++ strict -O1
compile_and_run g++ sanitized -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer

g++ -std=c++20 -O2 -S "$here/assembly_probe.cpp" -o "$build/gcc.s"
clang++ -std=c++20 -O2 -S "$here/assembly_probe.cpp" -o "$build/clang.s"
grep -q 'allmight_m018_sum3' "$build/gcc.s"
grep -q 'allmight_m018_sum3' "$build/clang.s"
python3 "$here/check_note_examples.py" --compiler clang++
echo "m018: multi-TU PIMPL, GCC/Clang/release/sanitizers/assembly: PASS"
