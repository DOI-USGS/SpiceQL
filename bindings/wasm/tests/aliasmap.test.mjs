/**
 * Alias-map surface in the WASM build. The default map is loaded from the
 * preloaded /spiceql/SpiceQL/aliasMap.json (no HDF5, no env reliance).
 */

import { describe, it, before, beforeEach } from 'node:test';
import assert from 'node:assert/strict';

import { loadModule, moduleUnavailable } from './helper.mjs';

describe('alias map', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  // getAliasMap/setAliasMap mutate a process-global singleton; restore the
  // bundled default before each test so ordering never matters.
  let defaultMap;
  before(() => { defaultMap = M.getAliasMap(); });
  beforeEach(() => { M.setAliasMap(defaultMap); });

  it('getAliasMap returns a non-empty object', () => {
    const m = M.getAliasMap();
    assert.equal(typeof m, 'object');
    assert.ok(Object.keys(m).length > 0);
  });

  it('resolves a known alias to its canonical spiceql name', () => {
    // aliasMap.json groups 'AMICA'/'HAYABUSA_AMICA' under the 'amica' key.
    assert.equal(M.getSpiceqlName('AMICA'), 'amica');
    assert.equal(M.getSpiceqlName('HAYABUSA_AMICA'), 'amica');
  });

  it('getSpiceqlName is case-insensitive', () => {
    assert.equal(M.getSpiceqlName('amica'), 'amica');
  });

  it('returns empty string for an unknown alias', () => {
    assert.equal(M.getSpiceqlName('definitely-not-a-mission-xyz'), '');
  });

  it('addAliasKey adds a resolvable alias', () => {
    M.addAliasKey('myalias123', 'lro');
    assert.equal(M.getSpiceqlName('myalias123'), 'lro');
  });

  it('setAliasMap replaces the map wholesale', () => {
    M.setAliasMap({ foo: ['BAR', 'BAZ'] });
    assert.equal(M.getSpiceqlName('BAR'), 'foo');
    assert.equal(M.getSpiceqlName('BAZ'), 'foo');
    // Entries from the default map are gone.
    assert.equal(M.getSpiceqlName('AMICA'), '');
  });

  it('setAliasMap rejects a non-object argument', () => {
    assert.throws(() => M.setAliasMap([1, 2, 3]), /must be a JSON object/);
  });
});
