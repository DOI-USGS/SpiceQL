/**
 * Minimal JS wrapper around the SpiceQL WASM module (Embind).
 *
 * This is a thin pass-through: it loads the Emscripten module, injects the
 * environment SpiceQL expects, and re-exports the bound api.h functions. All
 * real logic lives in the compiled library; keep this layer small.
 *
 * Usage:
 *   import { loadSpiceQL } from './spiceql.js';
 *   const spiceql = await loadSpiceQL();
 *   spiceql.mountKernel('/kernels/naif0012.tls', bytes);   // furnish your own kernels
 *   const { result, kernels } = spiceql.utcToEt('2000-01-01T00:00:00',
 *                                 { searchKernels: false, kernelList: ['/kernels/naif0012.tls'] });
 *
 * The raw CSPICE toolkit (every *_c function) is available on demand by
 * importing naifspice.js and calling loadNaifspice() — it is intentionally NOT
 * built by loadSpiceQL(), so the ~650 wrappers are only generated when wanted:
 *   import { loadNaifspice } from './naifspice.js';
 *   const naif = loadNaifspice(spiceql);   // also sets spiceql.naifspice
 *   const et = naif.str2et('2000 JAN 01 12:00:00');
 *   const { state, lt } = naif.spkez(1, et, 'J2000', 'NONE', 399);
 *
 * Notes:
 *  - Kernel *search* is unavailable in the WASM build (no HDF5 inventory). Pass
 *    an explicit kernelList of virtual-FS paths with searchKernels:false, or
 *    furnish kernels yourself. searchKernels:true throws.
 *  - useWeb:true (the remote REST transport) is NOT supported in the WASM build
 *    and throws. Awaiting fetch() would require suspending the wasm stack, which
 *    through Embind forces the whole API to become async. If you need the hosted
 *    service, call the SpiceQL REST API directly from JavaScript with fetch()
 *    and use this module only for local-kernel work (useWeb:false).
 */

import createSpiceQL from './spiceql_wasm.js';

// api.h functions exposed by the Embind layer (see spiceql_bind.cpp).
const API_FUNCTIONS = [
  'getSpiceqlName', 'addAliasKey', 'getAliasMap', 'setAliasMap', 'urlEncode',
  'spiceAPIQuery',
  'getTargetStates', 'getTargetStatesRanged',
  'getTargetOrientations', 'getTargetOrientationsRanged',
  'strSclkToEt', 'doubleSclkToEt', 'doubleEtToSclk',
  'utcToEt', 'etToUtc',
  'translateNameToCode', 'translateCodeToName',
  'getFrameInfo', 'getTargetFrameInfo',
  'findMissionKeywords', 'findTargetKeywords',
  'frameTrace', 'extractExactCkTimes', 'getExactTargetOrientations',
  'searchForKernelsets',
  // Manual kernel-pool management (see spiceql_bind.cpp).
  'load', 'unload', 'getLoadedKernels', 'isLskLoaded',
];

/**
 * Load and initialize the SpiceQL WASM module.
 *
 * @param {object} [opts]
 * @param {string} [opts.spiceRoot='/kernels'] virtual-FS dir for user kernels
 * @param {object} [opts.moduleOverrides] extra Emscripten Module overrides
 * @returns {Promise<object>} the wrapped SpiceQL API
 */
export async function loadSpiceQL(opts = {}) {
  const spiceRoot = opts.spiceRoot || '/kernels';

  const Module = await createSpiceQL(opts.moduleOverrides || {});

  // Environment SpiceQL reads via getenv(). CONDA_PREFIX must be set (any value)
  // because getConfigDirectory() dereferences it unconditionally; SPICEQL_DEV_DB
  // selects the preloaded /spiceql/SpiceQL/db config path.
  Module.ENV.SPICEQL_DEV_DB = 'true';
  Module.ENV.CONDA_PREFIX = '/spiceql';
  Module.ENV.SPICEROOT = spiceRoot;

  // Ensure the user-kernel mount point exists in the virtual FS.
  try {
    Module.FS.mkdirTree(spiceRoot);
  } catch (e) {
    // already exists
  }

  const api = {
    /** Raw Emscripten module (FS, ENV, etc.) for advanced use. */
    Module,

    /**
     * Write a kernel into the virtual filesystem so CSPICE can furnish it.
     * @param {string} path absolute virtual-FS path (e.g. '/kernels/foo.bsp')
     * @param {Uint8Array|ArrayBuffer|string} data kernel bytes or text
     */
    mountKernel(path, data) {
      const slash = path.lastIndexOf('/');
      if (slash > 0) {
        Module.FS.mkdirTree(path.slice(0, slash));
      }
      const bytes = data instanceof ArrayBuffer ? new Uint8Array(data) : data;
      Module.FS.writeFile(path, bytes);
      return path;
    },

    /**
     * RAII kernel set: furnishes kernels on construction and unfurnishes them on
     * unload(). While alive, its kernels are visible to api.h calls made with
     * searchKernels:false and an empty kernelList.
     *
     * Accepts either an array of virtual-FS kernel paths (grouped by type
     * automatically) or a { type: [paths] } object.
     *
     * This is an Embind-owned C++ object, so call .unload() (or .delete()) when
     * done to free it; JS garbage collection will NOT do it for you.
     *
     * @example
     *   const ks = new spiceql.KernelSet(['/kernels/naif0012.tls']);
     *   const { result } = spiceql.utcToEt('2000-01-01T00:00:00',
     *                                      { searchKernels: false });
     *   ks.unload();
     */
    KernelSet: Module.KernelSet,
  };

  for (const name of API_FUNCTIONS) {
    if (typeof Module[name] === 'function') {
      api[name] = Module[name].bind(Module);
    }
  }

  return api;
}

export default loadSpiceQL;
