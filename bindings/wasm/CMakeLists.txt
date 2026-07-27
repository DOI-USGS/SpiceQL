# Emscripten/Embind JavaScript bindings for SpiceQL.
#
# Produces spiceql_wasm.js + spiceql_wasm.wasm exposing the api.h surface to JS.
# Only built for the WASM target (see the SPICEQL_WASM branch in the top-level
# CMakeLists.txt, which add_subdirectory()s this instead of bindings/).

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "bindings/wasm requires an Emscripten (emcmake) build.")
endif()

add_executable(spiceql_wasm spiceql_bind.cpp)

# ---- naifspice namespace generation -----------------------------------------
# The `naifspice` JS namespace exposes every CSPICE *_c function. Rather than
# hand-write ~650 wrappers, gen_naifspice.py parses CSPICE's prototype header
# into a signature table (naifspice_sigs.json, consumed at load time by
# naifspice.js) plus an export list (naifspice_exports.txt) that keeps the
# symbols from being dead-stripped. CSPICE_SRC_ROOT is set by BuildCspiceWasm
# (included before this subdirectory); its patched working copy holds SpiceZpr.h.
find_program(PYTHON3_EXECUTABLE NAMES python3 python REQUIRED)

set(NAIFSPICE_HEADER "${CSPICE_SRC_ROOT}/include/SpiceZpr.h")
set(NAIFSPICE_SIGS "${CMAKE_CURRENT_BINARY_DIR}/naifspice_sigs.json")
set(NAIFSPICE_EXPORTS "${CMAKE_CURRENT_BINARY_DIR}/naifspice_exports.txt")
set(NAIFSPICE_GEN "${CMAKE_CURRENT_SOURCE_DIR}/gen_naifspice.py")

# Generate at configure time: the linker needs the exports file, and the header
# already exists (BuildCspiceWasm copies + patches it at configure time). Re-run
# whenever the generator or header changes.
execute_process(
  COMMAND "${PYTHON3_EXECUTABLE}" "${NAIFSPICE_GEN}"
          "${NAIFSPICE_HEADER}" "${NAIFSPICE_SIGS}" "${NAIFSPICE_EXPORTS}"
          "${CSPICE_SRC_ROOT}/src/cspice"
  RESULT_VARIABLE NAIFSPICE_GEN_RESULT)
if(NOT NAIFSPICE_GEN_RESULT EQUAL 0)
  message(FATAL_ERROR "gen_naifspice.py failed (${NAIFSPICE_GEN_RESULT})")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${NAIFSPICE_GEN}" "${NAIFSPICE_HEADER}")

# Native WebAssembly exceptions (matching the SpiceQL lib) so guard() in
# spiceql_bind.cpp can turn C++ throws into JS Errors without the invoke_*
# trampolines that trap against CSPICE's f2c signatures.
target_compile_options(spiceql_wasm PRIVATE -fwasm-exceptions)

target_link_libraries(spiceql_wasm PRIVATE SpiceQL)

# The db configs / aliasMap / bundled LSK are read from the virtual FS at the
# path _SOURCE_PREFIX/SpiceQL/db (with SPICEQL_DEV_DB=true). Preload them there
# so the module is self-contained. _SOURCE_PREFIX is the repo root.
set(SPICEQL_DB_DIR "${CMAKE_SOURCE_DIR}/SpiceQL/db")
set(SPICEQL_ALIASMAP "${CMAKE_SOURCE_DIR}/SpiceQL/aliasMap.json")

# Emscripten link options. cspice is linked statically (see
# cmake/BuildCspiceWasm.cmake), so this is a single self-contained module.
# MODULARIZE + EXPORT_ES6 yield an ES module; FS/ENV let the JS wrapper mount
# kernels and inject env vars. CSPICE uses setjmp/longjmp for its error handling,
# so SUPPORT_LONGJMP must be enabled (and be compatible with the JS-exceptions
# ABI used for -fexceptions). INITIAL_MEMORY is generous because cspice + kernels
# are memory hungry.
#
# There is intentionally no ASYNCIFY/JSPI here: the remote REST transport
# (useWeb=true) is unsupported in the WASM build. Awaiting fetch() would require
# suspending the wasm stack, and neither JSPI (forces the whole Embind API async)
# nor ASYNCIFY=1 (incompatible with -fwasm-exceptions) can do that without
# breaking the synchronous API. useWeb=true throws (see api.cpp spiceAPIQuery).
# EXPORTED_FUNCTIONS lists every CSPICE *_c symbol (from the generator's
# response file, @-syntax) so the naifspice namespace can reach them; without
# this they are dead-stripped, since SpiceQL only calls a handful directly.
# getValue/setValue/stringToNewUTF8/UTF8ToString are the runtime helpers
# naifspice.js uses to marshal buffers and strings. naifspice_sigs.json is
# preloaded next to the DB configs so the wrapper can fetch it from the VFS.
target_link_options(spiceql_wasm PRIVATE
  "-lembind"
  "-fwasm-exceptions"
  "-sSUPPORT_LONGJMP=wasm"
  "-sMODULARIZE=1"
  "-sEXPORT_ES6=1"
  "-sEXPORT_NAME=createSpiceQL"
  "-sALLOW_MEMORY_GROWTH=1"
  "-sINITIAL_MEMORY=134217728"
  "-sEXPORTED_FUNCTIONS=@${NAIFSPICE_EXPORTS}"
  "-sEXPORTED_RUNTIME_METHODS=['FS','ENV','stringToNewUTF8','UTF8ToString','getValue','setValue']"
  "-sFORCE_FILESYSTEM=1"
  "--preload-file=${SPICEQL_DB_DIR}@/spiceql/SpiceQL/db"
  "--preload-file=${SPICEQL_ALIASMAP}@/spiceql/SpiceQL/aliasMap.json"
  "--preload-file=${NAIFSPICE_SIGS}@/spiceql/SpiceQL/naifspice_sigs.json")

# The link depends on the generated export list; re-link if it changes.
set_property(TARGET spiceql_wasm APPEND PROPERTY LINK_DEPENDS "${NAIFSPICE_EXPORTS}")

# The library was compiled with _SOURCE_PREFIX="/spiceql" for the WASM build
# (see top-level CMakeLists.txt), so getConfigDirectory() resolves configs under
# /spiceql/SpiceQL/db when SPICEQL_DEV_DB=true (set by the JS wrapper). The
# preload paths above match that layout.

set_target_properties(spiceql_wasm PROPERTIES OUTPUT_NAME "spiceql_wasm")

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/spiceql_wasm.js"
    "${CMAKE_CURRENT_BINARY_DIR}/spiceql_wasm.wasm"
    "${CMAKE_CURRENT_SOURCE_DIR}/spiceql.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/naifspice.js"
  DESTINATION "share/SpiceQL/wasm" OPTIONAL)
