/**
 * Shared setup for the SpiceQL WASM test suite (node:test).
 *
 * Loads the Emscripten module, injects the ENV/FS setup that bindings/wasm/
 * spiceql.js does, and mounts the repo's bundled test kernels into the virtual
 * filesystem. Each node:test file runs in its own process, so each imports this
 * and calls loadModule() once.
 *
 * The built artifacts live in <repo>/build-wasm/bindings/wasm by default (see
 * the WASM build commands in bindings/wasm/tests/README.md); override with
 * SPICEQL_WASM_DIR. If they are missing, moduleAvailable() returns a reason
 * string so suites skip cleanly rather than failing on a checkout that hasn't
 * built the WASM target.
 */

import { existsSync, readFileSync } from 'node:fs';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { dirname, join, resolve } from 'node:path';

const here = dirname(fileURLToPath(import.meta.url));
export const repoRoot = resolve(here, '..', '..', '..');

/** Directory holding spiceql_wasm.{js,wasm,data}. */
export const moduleDir = process.env.SPICEQL_WASM_DIR
  ? resolve(process.env.SPICEQL_WASM_DIR)
  : resolve(repoRoot, 'build-wasm', 'bindings', 'wasm');

const modulePath = join(moduleDir, 'spiceql_wasm.js');

/** Bundled test kernels (see SpiceQL/tests/data). */
export const kernels = {
  lsk: resolve(repoRoot, 'SpiceQL', 'tests', 'data', 'naif0012.tls'),
  sclk: resolve(repoRoot, 'SpiceQL', 'tests', 'data', 'lro_clkcor_2020184_v00.tsc'),
};

/** LRO spacecraft clock id, used by the SCLK conversions. */
export const LRO_SCLK_ID = -85;

/**
 * @returns {false|string} false if the module can be loaded, otherwise a
 *   human-readable reason to pass as the `skip` option of describe().
 */
export function moduleUnavailable() {
  if (!existsSync(modulePath)) {
    return `WASM module not built at ${modulePath} — build the WASM target (see bindings/wasm/tests/README.md) or set SPICEQL_WASM_DIR`;
  }
  return false;
}

/**
 * Load and initialize the module and mount the bundled kernels.
 * Mirrors the ENV/FS wiring in bindings/wasm/spiceql.js.
 * @returns {Promise<object>} the raw Emscripten module (bound api.h functions
 *   plus FS/ENV).
 */
export async function loadModule() {
  const createSpiceQL = (await import(pathToFileURL(modulePath).href)).default;
  const Module = await createSpiceQL({ locateFile: (p) => join(moduleDir, p) });

  // CONDA_PREFIX/SPICEQL_DEV_DB are set for parity with the JS wrapper; the WASM
  // build no longer depends on them for config paths (see getConfigDirectory).
  Module.ENV.SPICEQL_DEV_DB = 'true';
  Module.ENV.CONDA_PREFIX = '/spiceql';
  Module.ENV.SPICEROOT = '/kernels';
  Module.FS.mkdirTree('/kernels');

  mountKernel(Module, '/kernels/naif0012.tls', kernels.lsk);
  mountKernel(Module, '/kernels/lro_clkcor_2020184_v00.tsc', kernels.sclk);

  return Module;
}

/**
 * Build the raw-CSPICE `naifspice` namespace on a loaded Module by importing the
 * opt-in entry point from bindings/wasm/naifspice.js — the same call a library
 * user makes. loadModule() returns the raw Emscripten Module, which
 * loadNaifspice() also accepts.
 * @returns {Promise<object>} the naifspice namespace
 */
export async function loadNaifspice(Module) {
  const { loadNaifspice: build } = await import(
    pathToFileURL(resolve(here, '..', 'naifspice.js')).href);
  return build(Module);
}

/** Write a host file into the module's virtual FS. */
export function mountKernel(Module, vpath, hostPath) {
  const slash = vpath.lastIndexOf('/');
  if (slash > 0) Module.FS.mkdirTree(vpath.slice(0, slash));
  Module.FS.writeFile(vpath, new Uint8Array(readFileSync(hostPath)));
  return vpath;
}

/** Options that use only the local LSK (no search, no network). */
export const localLskOpts = {
  searchKernels: false,
  useWeb: false,
  kernelList: ['/kernels/naif0012.tls'],
};

/** Options that use the local LSK + LRO SCLK. */
export const localSclkOpts = {
  searchKernels: false,
  useWeb: false,
  kernelList: ['/kernels/naif0012.tls', '/kernels/lro_clkcor_2020184_v00.tsc'],
};
