/**
 * Manual kernel-pool management in the WASM build: the free load/unload
 * functions and the RAII KernelSet class. These exercise the pattern of
 * furnishing kernels once and reusing them across calls made with
 * searchKernels:false and no kernelList (which read the already-furnished pool).
 */

import { describe, it, before, beforeEach } from 'node:test';
import assert from 'node:assert/strict';

import { loadModule, moduleUnavailable, LRO_SCLK_ID } from './helper.mjs';

const LSK = '/kernels/naif0012.tls';
const SCLK = '/kernels/lro_clkcor_2020184_v00.tsc';
const LRO_SCLK_STR = '1/281199081:48971';
// A call that uses only what is already furnished (no per-call kernelList).
const poolOnly = { searchKernels: false, useWeb: false };

describe('manual kernel pool (load/unload)', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  // Each test furnishes what it needs and cleans up, so start from an empty pool.
  beforeEach(() => {
    for (const k of M.getLoadedKernels()) M.unload(k);
  });

  it('starts with an empty pool and no LSK', () => {
    assert.deepEqual(M.getLoadedKernels(), []);
    assert.equal(M.isLskLoaded(), false);
  });

  it('load() furnishes a kernel and isLskLoaded/getLoadedKernels reflect it', () => {
    M.load(LSK);
    assert.equal(M.isLskLoaded(), true);
    assert.deepEqual(M.getLoadedKernels(), [LSK]);
  });

  it('a furnished pool satisfies calls made with no kernelList', () => {
    M.load(LSK);
    M.load(SCLK);
    const r = M.strSclkToEt(LRO_SCLK_ID, LRO_SCLK_STR, poolOnly);
    assert.ok(Number.isFinite(r.result));
  });

  it('unload() removes kernels from the pool', () => {
    M.load(LSK);
    M.unload(LSK);
    assert.equal(M.isLskLoaded(), false);
    assert.deepEqual(M.getLoadedKernels(), []);
  });

  it('a pool-only call fails once its kernels are unloaded', () => {
    M.load(LSK);
    M.load(SCLK);
    M.unload(SCLK);
    assert.throws(() => M.strSclkToEt(LRO_SCLK_ID, LRO_SCLK_STR, poolOnly));
  });
});

describe('KernelSet (RAII)', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  beforeEach(() => {
    for (const k of M.getLoadedKernels()) M.unload(k);
  });

  it('constructing from an array furnishes the kernels', () => {
    const ks = new M.KernelSet([LSK, SCLK]);
    assert.equal(M.getLoadedKernels().length, 2);
    ks.delete();
  });

  it('groups kernels by type in getKernels()', () => {
    const ks = new M.KernelSet([LSK, SCLK]);
    const k = ks.getKernels();
    assert.deepEqual(k.lsk, [LSK]);
    assert.deepEqual(k.sclk, [SCLK]);
    ks.delete();
  });

  it('accepts a { type: [paths] } object as well as an array', () => {
    const ks = new M.KernelSet({ lsk: [LSK], sclk: [SCLK] });
    assert.equal(M.getLoadedKernels().length, 2);
    ks.delete();
  });

  it('kernels are usable by pool-only calls while the set is alive', () => {
    const ks = new M.KernelSet([LSK, SCLK]);
    const r = M.strSclkToEt(LRO_SCLK_ID, LRO_SCLK_STR, poolOnly);
    assert.ok(Number.isFinite(r.result));
    ks.delete();
  });

  it('unload() unfurnishes everything the set furnished', () => {
    const ks = new M.KernelSet([LSK, SCLK]);
    ks.unload();
    assert.deepEqual(M.getLoadedKernels(), []);
    // A pool-only call now fails.
    assert.throws(() => M.strSclkToEt(LRO_SCLK_ID, LRO_SCLK_STR, poolOnly));
    ks.delete();
  });

  it('load() can add more kernels to an existing set', () => {
    const ks = new M.KernelSet([LSK]);
    assert.equal(M.getLoadedKernels().length, 1);
    ks.load([SCLK]);
    assert.equal(M.getLoadedKernels().length, 2);
    assert.ok(Number.isFinite(M.strSclkToEt(LRO_SCLK_ID, LRO_SCLK_STR, poolOnly).result));
    ks.delete();
  });
});
