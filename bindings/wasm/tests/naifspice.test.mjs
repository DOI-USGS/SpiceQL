/**
 * The raw-CSPICE `naifspice` namespace: every CSPICE *_c function exposed as a
 * JS function, generated from the parsed signature table (see gen_naifspice.py
 * and naifspice.js). These tests exercise each marshalling shape the generic
 * wrapper handles — scalar/array/matrix/string outputs, boolean flags, string
 * returns, the raw-pointer fallback — plus error propagation.
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert/strict';

import { loadModule, loadNaifspice, moduleUnavailable } from './helper.mjs';

describe('naifspice (raw CSPICE)', { skip: moduleUnavailable() }, () => {
  let ns;
  before(async () => {
    const M = await loadModule();      // mounts the bundled LSK at /kernels/naif0012.tls
    ns = await loadNaifspice(M);
    ns.furnsh('/kernels/naif0012.tls');
  });

  it('exposes hundreds of functions with the _c suffix dropped', () => {
    const fns = Object.keys(ns).filter((k) => typeof ns[k] === 'function');
    assert.ok(fns.length > 600, `only ${fns.length} functions`);
    assert.equal(typeof ns.spkez, 'function');
    // The CSPICE _c name is not the top-level name; it lives under raw.
    assert.equal(ns.spkez_c, undefined);
    assert.equal(typeof ns.raw.spkez_c, 'function');
  });

  it('string return: tkvrsn(TOOLKIT)', () => {
    assert.match(ns.tkvrsn('TOOLKIT'), /^CSPICE_N\d+$/);
  });

  it('single scalar output is returned directly: str2et', () => {
    const et = ns.str2et('2000 JAN 01 12:00:00');
    // Noon 2000-01-01 UTC is the J2000 epoch offset by TDB-UTC (~64.184 s).
    assert.ok(Math.abs(et - 64.184) < 0.01, `got ${et}`);
  });

  it('output string sized by a length arg: et2utc', () => {
    const et = ns.str2et('2000 JAN 01 12:00:00');
    assert.equal(ns.et2utc(et, 'ISOC', 3, 32), '2000-01-01T12:00:00.000');
  });

  it('array input + multiple scalar outputs become an object: reclat', () => {
    const { radius, longitude, latitude } = ns.reclat([1, 1, 0]);
    assert.ok(Math.abs(radius - Math.SQRT2) < 1e-12);
    assert.ok(Math.abs(longitude - Math.PI / 4) < 1e-12);
    assert.equal(latitude, 0);
  });

  it('scalar inputs + array output: vpack', () => {
    assert.deepEqual(ns.vpack(1, 2, 3), [1, 2, 3]);
  });

  it('non-void return with no outputs: vnorm', () => {
    assert.equal(ns.vnorm([3, 4, 0]), 5);
  });

  it('matrix output: pxform(J2000,J2000) is the identity', () => {
    const et = ns.str2et('2000 JAN 01 12:00:00');
    assert.deepEqual(ns.pxform('J2000', 'J2000', et),
      [[1, 0, 0], [0, 1, 0], [0, 0, 1]]);
  });

  it('string + boolean outputs: bodc2n(399) -> EARTH, found', () => {
    assert.deepEqual(ns.bodc2n(399, 32), { name: 'EARTH', found: true });
  });

  it('a signalled SPICE error throws a JS Error and the module recovers', () => {
    assert.throws(() => ns.str2et('definitely not a date'), /SPICE\(/);
    // The toolkit is reset, so a valid call afterward still works.
    assert.equal(typeof ns.str2et('2001 JAN 01'), 'number');
  });

  it('runtime-sized buffers fall back to the raw pointer form', () => {
    // bodvrd writes up to maxn doubles through a bare pointer; not ergonomic, so
    // the top-level name forwards to raw.bodvrd_c.
    assert.equal(ns.bodvrd, ns.raw.bodvrd_c);

    // Exercise the raw path with caller-managed memory (vsclg_c: s * v[ndim]).
    const { malloc, free, setValue, getValue } = ns;
    const vin = malloc(3 * 8);
    const vout = malloc(3 * 8);
    try {
      for (let i = 0; i < 3; i++) setValue(vin + i * 8, i + 1, 'double');
      ns.raw.vsclg_c(2.0, vin, 3, vout);
      const out = [0, 1, 2].map((i) => getValue(vout + i * 8, 'double'));
      assert.deepEqual(out, [2, 4, 6]);
    } finally {
      free(vin);
      free(vout);
    }
  });
});
