# SpiceQL WASM tests

JavaScript test suite for the Emscripten/WASM bindings, using Node's built-in
[`node:test`](https://nodejs.org/api/test.html) runner (no external deps).

## Run

```sh
# 1. Build the WASM target (produces build-wasm/bindings/wasm/spiceql_wasm.*).
#    Clear the conda host compiler flags first — they break the emcc cross-compile.
unset CFLAGS CXXFLAGS CPPFLAGS LDFLAGS
emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release \
  -DSPICEQL_WASM=ON -DSPICEQL_BUILD_TESTS=OFF -DSPICEQL_BUILD_BINDINGS=ON
cmake --build build-wasm -j"$(getconf _NPROCESSORS_ONLN)"

# 2. Run the suite from the repo root
npm test
```

See the WebAssembly section of the top-level `README.md` for what each CMake
option does.

`npm test` runs `node --test "bindings/wasm/tests/*.test.mjs"`. Node >= 20 is
required (>= 21 for the glob form used in the `test` script).

If the WASM module hasn't been built, every suite **skips** with a message
pointing at the build command above rather than failing — so `npm test` is safe
to run on a fresh checkout.

To test a build in a non-default location, set `SPICEQL_WASM_DIR`:

```sh
SPICEQL_WASM_DIR=/path/to/bindings/wasm npm test
```

## Layout

- `helper.mjs` — loads the module, wires ENV/FS, mounts the bundled test kernels
  (`SpiceQL/tests/data`), exposes `moduleUnavailable()` for graceful skips, and
  builds the `naifspice` namespace via `loadNaifspice()`.
- `naifspice.test.mjs` — the raw-CSPICE `naifspice` namespace: each marshalling
  shape (scalar/array/matrix/string outputs, boolean flags, string returns), the
  raw-pointer fallback, and SPICE-error propagation.
- `time.test.mjs` — `utcToEt` / `etToUtc` / `strSclkToEt`.
- `frames.test.mjs` — `translateNameToCode` / `translateCodeToName`.
- `aliasmap.test.mjs` — `getSpiceqlName` / `getAliasMap` / `setAliasMap` / `addAliasKey`.
- `kernelset.test.mjs` — manual pool management: `load` / `unload` /
  `getLoadedKernels` / `isLskLoaded` and the RAII `KernelSet` class.
- `errors.test.mjs` — CSPICE errors surface as catchable JS `Error`s (not wasm
  traps), the module recovers after an error, and disabled features throw
  clearly.
