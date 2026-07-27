#!/usr/bin/env bash
#
# Build SpiceQL as a WebAssembly module + JS bindings via Emscripten.
#
# Prerequisites:
#   - Emscripten SDK activated (emcc/emcmake on PATH). See https://emscripten.org.
#   - Submodules initialized:
#       git submodule update --init --recursive
#     (this includes submodules/cspice-cmake, the CSPICE CMake recipe, which
#      FetchContent-downloads the NAIF CSPICE source at configure time).
#
# Output (in build-wasm/bindings/wasm/):
#   spiceql_wasm.js, spiceql_wasm.wasm, spiceql_wasm.data
# Use with bindings/wasm/spiceql.js.
#
# Usage:
#   scripts/build_wasm.sh [build-dir]

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${REPO_ROOT}/build-wasm}"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "error: emcmake not found. Activate the Emscripten SDK first." >&2
  exit 1
fi

# A conda environment exports host (macOS/Linux) compiler and linker flags via
# CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS (e.g. -headerpad_max_install_names,
# -dead_strip_dylibs, host -isystem/-rpath paths). Those are invalid for the
# Emscripten cross-compile and break wasm-ld, so clear them for this build.
unset CFLAGS CXXFLAGS CPPFLAGS LDFLAGS
export CFLAGS="" CXXFLAGS="" CPPFLAGS="" LDFLAGS=""

echo "==> Configuring (SPICEQL_WASM) in ${BUILD_DIR}"
emcmake cmake -S "${REPO_ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DSPICEQL_WASM=ON \
  -DSPICEQL_BUILD_TESTS=OFF \
  -DSPICEQL_BUILD_BINDINGS=ON

echo "==> Building"
cmake --build "${BUILD_DIR}" -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

echo "==> Done. Artifacts in ${BUILD_DIR}/bindings/wasm/"
ls -la "${BUILD_DIR}/bindings/wasm/" 2>/dev/null || true
