#!/usr/bin/env bash
# Reproducible GCC C++20 module build: DAG order, partitions, implementation,
# two importers, a C ABI consumer, plus an explicitly compiled header unit.
set -Eeuo pipefail
SOURCE_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$(mktemp -d "${TMPDIR:-/tmp}/allmight-m017-modules.XXXXXX")"
trap 'rm -rf -- "$BUILD_DIR"' EXIT
cd "$BUILD_DIR"
CXX="${CXX:-g++}"
CC="${CC:-gcc}"
CPP_FLAGS=(-std=c++20 -fmodules-ts -O1 -Wall -Wextra -Wpedantic -Werror -I "$SOURCE_DIR")

# Compiled module interfaces (CMIs) require an acyclic, ordered dependency
# graph. Compile partition BEFORE the primary interface that re-exports it.
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/allmight_asset_query_rules.cppm" -o rules.o
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/allmight_asset_query.cppm" -o interface.o
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/allmight_asset_query_impl.cpp" -o implementation.o
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/asset_query_c_api.cpp" -o c_api.o
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/asset_query_secondary.cpp" -o secondary.o
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/asset_query_module_lab.cpp" -o main_consumer.o
"$CXX" rules.o interface.o implementation.o c_api.o secondary.o main_consumer.o -o module_consumer
./module_consumer

# A plain C11 consumer knows nothing about a C++ module or C++ name mangling.
"$CC" -std=c11 -Wall -Wextra -Wpedantic -Werror -I "$SOURCE_DIR" \
  -c "$SOURCE_DIR/asset_query_c_consumer.c" -o plain_c.o
"$CXX" rules.o interface.o implementation.o c_api.o plain_c.o -o c_consumer
./c_consumer

# A header unit needs its OWN CMI. It is not automatically created by selecting
# -std=c++20; this GCC-specific invocation builds it before the import.
"$CXX" "${CPP_FLAGS[@]}" -x c++-user-header asset_query_interop.hpp
"$CXX" "${CPP_FLAGS[@]}" -x c++ -c "$SOURCE_DIR/asset_query_header_unit_lab.cpp" -o header_consumer.o
"$CXX" rules.o interface.o implementation.o header_consumer.o -o header_consumer
./header_consumer
printf '%s\n' 'build_module_lab.sh: all module and C boundary checks passed'
