/**
 * Generic marshaller that builds the `naifspice` namespace — every CSPICE `*_c`
 * function as a JS function (e.g. `spiceql.naifspice.spkez_c`).
 *
 * There is deliberately almost no per-function code here. A build step parses
 * CSPICE's prototype header into a signature table (naifspice_sigs.json, see
 * gen_naifspice.py); this module reads that table and synthesizes each function
 * from its parameter descriptors. Two forms per function:
 *
 *   - Ergonomic (the ~470 functions the parser could classify fully): the
 *     wrapper takes the INPUT parameters in declaration order as plain JS
 *     values and returns the OUTPUT parameters. The CSPICE `_c` suffix is
 *     dropped from these top-level names —
 *       const { state, lt } = naifspice.spkez(1, et, 'J2000', 'NONE', 399);
 *       const et = naifspice.str2et('2000 JAN 01 12:00:00');   // single out
 *     Rules: 0 outputs + void  -> undefined; 1 output -> that value; several ->
 *     an object keyed by parameter name; a non-void C return joins as `return`.
 *
 *   - Raw (the ~180 functions using SpiceCell/plane/ellipse/callback types or
 *     runtime-sized buffers the marshaller can't size safely): reachable under
 *     their literal C name (with the `_c` suffix) as
 *       naifspice.raw.<fn>_c(...numericArgs)
 *     Every argument is passed straight through to the wasm export as a number
 *     (pointers are addresses from naifspice.malloc); the C return is returned.
 *     Use naifspice.malloc/free/getValue/setValue/toCString/fromCString to
 *     manage buffers. A raw-only function is ALSO present at the top level under
 *     its suffix-dropped name, simply forwarding to raw.<fn>_c (so
 *     `naifspice.wnvald === naifspice.raw.wnvald_c`).
 *
 * This module does nothing until a library user explicitly imports it and calls
 * loadNaifspice() (or buildNaifspice()); spiceql.js does NOT pull it in, so the
 * ~650 wrappers are only generated when the raw toolkit is actually wanted.
 *
 * After every call the CSPICE error state is checked (Module.naifCheckErrors,
 * bound in spiceql_bind.cpp): a signalled SPICE error becomes a thrown JS Error
 * and the toolkit is reset so the module stays usable — the same behavior the
 * high-level api.h wrappers give.
 */

// Path of the preloaded signature table in the module's virtual filesystem
// (see the --preload-file entry in bindings/wasm/CMakeLists.txt).
const SIGS_VFS_PATH = '/spiceql/SpiceQL/naifspice_sigs.json';

/** Strip CSPICE's trailing `_c` for the friendly top-level name. */
function jsName(cName) {
  return cName.endsWith('_c') ? cName.slice(0, -2) : cName;
}

// Byte size + getValue/setValue type tag for each scalar CSPICE type. SpiceInt
// and SpiceBoolean are 32-bit ints in this build (see SpiceZdf.h).
const TYPE_INFO = {
  SpiceDouble: { size: 8, gv: 'double' },
  SpiceInt: { size: 4, gv: 'i32' },
  SpiceBoolean: { size: 4, gv: 'i32' },
  SpiceChar: { size: 1, gv: 'i8' },
};

function typeInfo(t) {
  return TYPE_INFO[t] || TYPE_INFO.SpiceInt; // pointers/unknowns treated as i32
}

function flatten(value, out) {
  if (Array.isArray(value)) {
    for (const v of value) flatten(v, out);
  } else {
    out.push(value);
  }
  return out;
}

function product(dims) {
  return dims.reduce((a, b) => a * b, 1);
}

/**
 * Build the naifspice namespace from a loaded Emscripten Module and the parsed
 * signature table.
 * @param {object} Module   the Emscripten module (exports + runtime methods)
 * @param {object} sigs     parsed signature table (naifspice_sigs.json)
 * @returns {object} the naifspice namespace
 */
export function buildNaifspice(Module, sigs) {
  const { _malloc, _free, getValue, setValue, UTF8ToString, stringToNewUTF8 } = Module;

  // Signal a CSPICE error (if any) as a JS Error and reset the toolkit. Bound in
  // spiceql_bind.cpp; falls back to a no-op if an older module lacks it.
  const checkErrors = typeof Module.naifCheckErrors === 'function'
    ? () => Module.naifCheckErrors()
    : () => {};

  // Low-level helpers exposed for the raw path.
  const helpers = {
    malloc: (n) => _malloc(n),
    free: (p) => _free(p),
    getValue: (p, type) => getValue(p, type),
    setValue: (p, v, type) => setValue(p, v, type),
    /** Allocate a NUL-terminated C string; free the returned pointer yourself. */
    toCString: (s) => stringToNewUTF8(s),
    fromCString: (p) => UTF8ToString(p),
  };

  // Direct call into a wasm export. All args are numbers (pointers are
  // addresses); JS numbers coerce to the export's i32/f64 params at the
  // boundary. Returns the raw C return value.
  function rawCall(name, args) {
    const fn = Module['_' + name];
    if (typeof fn !== 'function') {
      throw new Error(`naifspice: CSPICE function ${name} is not present in this build`);
    }
    return fn(...args);
  }

  // Read a C return value into a JS value per the declared return type.
  function readReturn(ret, raw) {
    if (ret === 'void') return undefined;
    if (ret === 'SpiceBoolean') return !!raw;
    if (ret === 'SpiceChar*') return raw ? UTF8ToString(raw) : '';
    return raw; // SpiceInt / SpiceDouble
  }

  // Read an output buffer (scalar / fixed array / matrix) back into JS.
  function readOutput(p, ptr) {
    const info = typeInfo(p.type);
    if (p.kind === 'scalar') {
      const v = getValue(ptr, info.gv);
      return p.type === 'SpiceBoolean' ? !!v : v;
    }
    if (p.kind === 'array') {
      const n = product(p.dims);
      const out = new Array(n);
      for (let i = 0; i < n; i++) out[i] = getValue(ptr + i * info.size, info.gv);
      return out;
    }
    if (p.kind === 'matrix') {
      const [rows, cols] = p.dims;
      const out = new Array(rows);
      for (let r = 0; r < rows; r++) {
        out[r] = new Array(cols);
        for (let c = 0; c < cols; c++) {
          out[r][c] = getValue(ptr + (r * cols + c) * info.size, info.gv);
        }
      }
      return out;
    }
    return undefined;
  }

  // Build one ergonomic wrapper from a signature.
  function makeErgonomic(name, sig) {
    const inputs = sig.params.filter((p) => p.role === 'in');
    const outputs = sig.params.filter((p) => p.role === 'out');

    return function (...jsArgs) {
      if (jsArgs.length < inputs.length) {
        throw new Error(
          `${name}: expected ${inputs.length} argument(s) ` +
          `(${inputs.map((p) => p.name).join(', ')}), got ${jsArgs.length}`);
      }

      const toFree = [];
      // Map each parameter (in declaration order) to a wasm call argument.
      const callArgs = [];
      const outPtrs = new Map();
      let ai = 0;
      try {
        for (const p of sig.params) {
          if (p.role === 'in') {
            const v = jsArgs[ai++];
            if (p.kind === 'string') {
              const ptr = stringToNewUTF8(String(v));
              toFree.push(ptr);
              callArgs.push(ptr);
            } else if (p.kind === 'array' || p.kind === 'matrix') {
              const info = typeInfo(p.type);
              const flat = flatten(v, []);
              const ptr = _malloc(Math.max(flat.length, 1) * info.size);
              toFree.push(ptr);
              for (let i = 0; i < flat.length; i++) {
                setValue(ptr + i * info.size, flat[i], info.gv);
              }
              callArgs.push(ptr);
            } else {
              // scalar: number, or boolean -> 1/0
              callArgs.push(typeof v === 'boolean' ? (v ? 1 : 0) : v);
            }
          } else {
            // output: allocate a buffer sized from dims (or a length arg for strings)
            const info = typeInfo(p.type);
            let bytes;
            if (p.kind === 'string') {
              // Capacity comes from the input int at len_from (declaration index).
              const lenParamIndex = p.len_from;
              // Count how many inputs precede that index to find it in jsArgs.
              let inIdx = 0;
              for (let k = 0; k < lenParamIndex; k++) {
                if (sig.params[k].role === 'in') inIdx++;
              }
              const cap = Number(jsArgs[inIdx]) || 0;
              bytes = Math.max(cap, 1);
            } else {
              bytes = Math.max(product(p.dims), 1) * info.size;
            }
            const ptr = _malloc(bytes);
            toFree.push(ptr);
            outPtrs.set(p, ptr);
            callArgs.push(ptr);
          }
        }

        const rawRet = rawCall(name, callArgs);
        checkErrors(); // throws (and resets) on a signalled SPICE error

        // Assemble results.
        const retVal = readReturn(sig.ret, rawRet);
        const results = {};
        for (const p of outputs) {
          const ptr = outPtrs.get(p);
          results[p.name] = p.kind === 'string'
            ? UTF8ToString(ptr)
            : readOutput(p, ptr);
        }

        const outNames = Object.keys(results);
        const hasRet = sig.ret !== 'void';
        if (!hasRet && outNames.length === 0) return undefined;
        if (!hasRet && outNames.length === 1) return results[outNames[0]];
        if (hasRet && outNames.length === 0) return retVal;
        return hasRet ? { return: retVal, ...results } : results;
      } finally {
        for (const ptr of toFree) _free(ptr);
      }
    };
  }

  // Build one raw wrapper: pass numeric args straight through, check errors.
  function makeRaw(name) {
    return function (...args) {
      const ret = rawCall(name, args);
      checkErrors();
      return ret;
    };
  }

  const ns = { ...helpers, raw: {} };
  for (const name of Object.keys(sigs)) {
    const sig = sigs[name];
    // raw.<fn>_c keeps the literal CSPICE name; the top level drops the suffix.
    ns.raw[name] = makeRaw(name);
    ns[jsName(name)] = sig.ergonomic ? makeErgonomic(name, sig) : ns.raw[name];
  }

  // Prime CSPICE's error action to RETURN. CSPICE defaults to ABORT (which
  // exit()s the whole module on the first error); naifCheckErrors() sets it to
  // RETURN lazily on its first call. Call it once now, before any user call, so
  // errors unwind to JS Errors rather than aborting the wasm instance.
  checkErrors();
  return ns;
}

/**
 * Explicit entry point for the raw CSPICE namespace. Reads the preloaded
 * signature table from the module's virtual filesystem and builds the namespace
 * with buildNaifspice(). spiceql.js does not call this — a library user opts in
 * by importing this module, so the ~650 wrappers are generated only on demand:
 *
 *   import { loadSpiceQL } from './spiceql.js';
 *   import { loadNaifspice } from './naifspice.js';
 *   const spiceql = await loadSpiceQL();
 *   const naif = loadNaifspice(spiceql);      // also sets spiceql.naifspice
 *   const et = naif.str2et('2000 JAN 01 12:00:00');
 *
 * @param {object} spiceql the object returned by loadSpiceQL(), or a raw
 *   Emscripten Module (anything exposing `.Module`/`.FS`).
 * @returns {object} the naifspice namespace (also attached as spiceql.naifspice
 *   when a loadSpiceQL() object is passed).
 */
export function loadNaifspice(spiceql) {
  const Module = spiceql && spiceql.Module ? spiceql.Module : spiceql;
  if (!Module || !Module.FS) {
    throw new Error('loadNaifspice: pass the object from loadSpiceQL() (or an Emscripten Module)');
  }
  const sigs = JSON.parse(new TextDecoder().decode(Module.FS.readFile(SIGS_VFS_PATH)));
  const ns = buildNaifspice(Module, sigs);
  if (spiceql && spiceql.Module) spiceql.naifspice = ns; // convenience handle
  return ns;
}

export default loadNaifspice;
