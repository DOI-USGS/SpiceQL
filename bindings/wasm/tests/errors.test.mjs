/**
 * Error propagation and recovery in the WASM build.
 *
 * These guard the property that made the WASM port work: a raised CSPICE error
 * (or a disabled feature) unwinds to a *catchable JS Error* with a readable
 * message — not a wasm "unreachable" trap — and the module stays usable
 * afterward. Historically the first CSPICE call trapped hard (f2c signature
 * mismatch); this suite would fail loudly if that regressed.
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert/strict';

import { loadModule, moduleUnavailable, localLskOpts, localSclkOpts, LRO_SCLK_ID } from './helper.mjs';

describe('error handling', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  it('kernel search throws with a clear "unavailable in WASM" message', () => {
    assert.throws(
      () => M.searchForKernelsets(['lro'], {}),
      /unavailable in the WASM build/,
    );
  });

  it('furnishing a missing kernel raises a catchable SPICE(NOSUCHFILE) error', () => {
    assert.throws(
      () => M.utcToEt('2000-01-01T00:00:00', {
        searchKernels: false, useWeb: false, kernelList: ['/kernels/does_not_exist.tls'],
      }),
      /NOSUCHFILE/,
    );
  });

  it('an invalid SCLK string raises a catchable SPICE error', () => {
    assert.throws(
      () => M.strSclkToEt(LRO_SCLK_ID, 'totally-not-a-sclk', localSclkOpts),
      /INVALIDSCLKSTRING|SPICE\(/,
    );
  });

  it('a caught error is a real JS Error with a message', () => {
    let err;
    try {
      M.utcToEt('2000-01-01T00:00:00', {
        searchKernels: false, useWeb: false, kernelList: ['/kernels/does_not_exist.tls'],
      });
    } catch (e) { err = e; }
    assert.ok(err instanceof Error);
    assert.equal(typeof err.message, 'string');
    assert.ok(err.message.length > 0);
  });

  it('the module is still usable after an error (SPICE state resets)', () => {
    // Trigger an error...
    assert.throws(() => M.utcToEt('2000-01-01T00:00:00', {
      searchKernels: false, useWeb: false, kernelList: ['/kernels/does_not_exist.tls'],
    }));
    // ...then a good call must still succeed.
    const r = M.utcToEt('2001-01-01T00:00:00', localLskOpts);
    assert.ok(Number.isFinite(r.result));
  });
});

describe('remote REST transport (useWeb) is unsupported', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  it('useWeb:true throws with a clear message pointing at the REST service', () => {
    // The WASM build cannot await fetch() (no JSPI/Asyncify — see api.cpp
    // spiceAPIQuery), so useWeb:true always throws rather than doing a request.
    // The message directs callers to hit the SpiceQL REST service directly.
    assert.throws(
      () => M.utcToEt('2000-01-01T00:00:00', { ...localLskOpts, useWeb: true }),
      /not supported in the WASM build|REST/i,
    );
  });
});
