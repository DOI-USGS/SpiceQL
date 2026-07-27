/**
 * Time-system conversions (utcToEt / etToUtc / strSclkToEt) in the WASM build.
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert/strict';

import {
  loadModule, moduleUnavailable, localLskOpts, localSclkOpts, LRO_SCLK_ID,
} from './helper.mjs';

describe('time conversions', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  it('utcToEt returns { result, kernels } with a numeric ET', () => {
    const r = M.utcToEt('2000-01-01T00:00:00', localLskOpts);
    assert.equal(typeof r.result, 'number');
    assert.ok('kernels' in r);
  });

  it('utcToEt(2000-01-01T00:00:00) is the well-known ET ~ -43135.816', () => {
    const r = M.utcToEt('2000-01-01T00:00:00', localLskOpts);
    // 2000-01-01T00:00:00 UTC precedes the J2000 ET epoch by the TT-UTC offset.
    assert.ok(Math.abs(r.result - -43135.816) < 0.01, `got ${r.result}`);
  });

  it('etToUtc round-trips back to the original UTC string', () => {
    const et = M.utcToEt('2000-01-01T00:00:00', localLskOpts).result;
    const back = M.etToUtc(et, { ...localLskOpts, format: 'ISOC', precision: 3 });
    assert.equal(typeof back.result, 'string');
    assert.ok(back.result.startsWith('2000-01-01T00:00:00'), `got ${back.result}`);
  });

  it('etToUtc reports the LSK it used in .kernels', () => {
    const et = M.utcToEt('2000-01-01T00:00:00', localLskOpts).result;
    const back = M.etToUtc(et, { ...localLskOpts, format: 'ISOC', precision: 3 });
    assert.deepEqual(back.kernels.lsk, ['/kernels/naif0012.tls']);
  });

  it('strSclkToEt converts an LRO clock string to a finite ET', () => {
    const r = M.strSclkToEt(LRO_SCLK_ID, '1/281199081:48971', localSclkOpts);
    assert.ok(Number.isFinite(r.result), JSON.stringify(r));
  });
});
