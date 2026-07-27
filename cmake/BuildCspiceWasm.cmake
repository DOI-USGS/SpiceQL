# BuildCspiceWasm.cmake
#
# Provides a CSPICE::cspice target for the WebAssembly (Emscripten) build, built
# as a STATIC library and linked directly into the SpiceQL Embind module.
#
# The native build gets cspice from a conda package (find_package(cspice)); that
# host binary can't be linked into WASM. The AndrewAnnex/cspice-cmake-spiceypy
# recipe (submodules/cspice-cmake, the same recipe conda-forge uses) builds
# cspice as an Emscripten SIDE_MODULE for pyodide-style dynamic linking. That
# dynamic-linking model is brittle with Asyncify + Embind (runtime symbol
# resolution failures), so instead we reuse only the recipe's NAIF source
# download + f2c source patches and compile a STATIC archive ourselves. Static
# linking yields one self-contained .wasm with no runtime side-module loading.
#
# The f2c patches are load-bearing, not cosmetic: NAIF's f2c-translated C
# declares support routines like s_copy/s_cat with an `int` return at every call
# site (`/* Subroutine */ int s_copy(...)`) while defining them as `void`. Native
# linkers ignore the mismatched return register, but WebAssembly type-checks
# every call, so wasm-ld emits a `signature_mismatch:s_copy` trampoline that
# traps ("unreachable") on the first CSPICE call. The recipe's
# cspice_apply_patches() normalizes those prototypes to match the definitions,
# which is what makes the module actually run. We therefore apply the recipe's
# patches (via its cmake/CspiceCommon.cmake) rather than reimplementing them, so
# this build stays in sync with conda-forge / SpiceyPy.
#
# Source: submodules/cspice-cmake (git submodule pointing at
#   https://github.com/AndrewAnnex/cspice-cmake-spiceypy)

set(CSPICE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules/cspice-cmake"
    CACHE PATH "Path to the cspice-cmake-spiceypy recipe")

if(NOT EXISTS "${CSPICE_CMAKE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "cspice-cmake recipe not found at ${CSPICE_CMAKE_DIR}.\n"
    "Initialize the submodule first:\n"
    "  git submodule update --init submodules/cspice-cmake")
endif()

include(FetchContent)

# NAIF CSPICE N0067 source archive (matches the recipe's Emscripten selection:
# the 32-bit Linux GCC package). URL + hash copied from the recipe so we fetch
# the identical, verified source.
set(CSPICE_VERSION "N0067")
FetchContent_Declare(cspice_src
  URL "https://naif.jpl.nasa.gov/pub/naif/misc/toolkit_${CSPICE_VERSION}/C/PC_Linux_GCC_32bit/packages/cspice.tar.Z"
  URL_HASH SHA256=33d75cd94acf6546e53d7ebc4e7d3d6d42ac27c83cb0d8f04c91a8b50c1149e3
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(cspice_src)

set(CSPICE_SRC_PRISTINE "${cspice_src_SOURCE_DIR}")
if(NOT EXISTS "${CSPICE_SRC_PRISTINE}/include/SpiceUsr.h")
  # Some extractors nest the payload under a cspice/ directory.
  if(EXISTS "${CSPICE_SRC_PRISTINE}/cspice/include/SpiceUsr.h")
    set(CSPICE_SRC_PRISTINE "${CSPICE_SRC_PRISTINE}/cspice")
  else()
    message(FATAL_ERROR "CSPICE headers not found under ${CSPICE_SRC_PRISTINE}")
  endif()
endif()

# Prepare a patched working copy of the source in the build tree (never touch the
# pristine FetchContent download). cspice_apply_patches() edits files in place, so
# we copy first and guard the copy+patch with a marker so it runs exactly once per
# build tree (patches are only idempotent on a fresh copy).
include("${CSPICE_CMAKE_DIR}/cmake/CspiceCommon.cmake")

set(CSPICE_SOURCE_ROOT "${CMAKE_BINARY_DIR}/cspice-src")
set(CSPICE_PREPARED_MARKER "${CMAKE_BINARY_DIR}/cspice_source_prepared")
if(NOT EXISTS "${CSPICE_PREPARED_MARKER}")
  message(STATUS "Preparing patched CSPICE working copy at ${CSPICE_SOURCE_ROOT}")
  file(REMOVE_RECURSE "${CSPICE_SOURCE_ROOT}")
  file(COPY "${CSPICE_SRC_PRISTINE}/" DESTINATION "${CSPICE_SOURCE_ROOT}")
  cspice_apply_patches("${CSPICE_SOURCE_ROOT}")
  file(WRITE "${CSPICE_PREPARED_MARKER}" "prepared")
else()
  message(STATUS "CSPICE working copy already prepared, skipping copy + patch.")
endif()

set(CSPICE_SRC_ROOT "${CSPICE_SOURCE_ROOT}")

file(GLOB CSPICE_SOURCES CONFIGURE_DEPENDS "${CSPICE_SRC_ROOT}/src/cspice/*.c")
if(NOT CSPICE_SOURCES)
  message(FATAL_ERROR "No CSPICE .c sources under ${CSPICE_SRC_ROOT}/src/cspice")
endif()

add_library(cspice_static STATIC ${CSPICE_SOURCES})

target_include_directories(cspice_static PUBLIC "${CSPICE_SRC_ROOT}/include")

# CSPICE is f2c-translated C. Silence its warnings and allow the old-style
# implicit declarations. We deliberately do NOT add setjmp/longjmp flags to the
# CSPICE compile: the final module links with -fwasm-exceptions (see
# bindings/wasm/CMakeLists.txt), and this configuration is verified to work end
# to end, including furnsh_c and SPICE error recovery (a raised SPICE error
# unwinds to a catchable JS Error and the module stays usable afterward).
#
# Historical note: an earlier "unreachable inside furnsh_c" trap was attributed
# to longjmp codegen, but the real cause was the f2c prototype mismatch fixed by
# the source patches above (a signature_mismatch:s_copy trampoline traps on the
# first CSPICE call, which happens to be on the furnsh path). Don't re-add
# longjmp flags to chase that symptom.
target_compile_options(cspice_static PRIVATE
  -w
  -Wno-implicit-int
  -Wno-implicit-function-declaration
  -fno-strict-aliasing)
target_compile_definitions(cspice_static PRIVATE NON_UNIX_STDIO)

# Namespaced target so the rest of the build refers to CSPICE::cspice exactly as
# the native find_package(cspice) case does.
if(NOT TARGET CSPICE::cspice)
  add_library(CSPICE::cspice ALIAS cspice_static)
endif()

message(STATUS "CSPICE (WASM) will be built STATIC from ${CSPICE_SRC_ROOT}")
