/**
 * Name <-> code translation in the WASM build. These use CSPICE's built-in body
 * codes, so no mission kernels are needed (empty options).
 */

import { describe, it, before } from 'node:test';
import assert from 'node:assert/strict';

import { loadModule, moduleUnavailable } from './helper.mjs';

describe('name/code translation', { skip: moduleUnavailable() }, () => {
  let M;
  before(async () => { M = await loadModule(); });

  for (const [name, code] of [['moon', 301], ['earth', 399], ['mars', 499]]) {
    it(`translateNameToCode('${name}') === ${code}`, () => {
      const r = M.translateNameToCode(name, {});
      assert.equal(r.result, code);
    });
  }

  it("translateCodeToName(301) === 'MOON'", () => {
    const r = M.translateCodeToName(301, {});
    assert.equal(r.result, 'MOON');
  });

  it('name->code->name is stable for the Moon', () => {
    const code = M.translateNameToCode('moon', {}).result;
    const name = M.translateCodeToName(code, {}).result;
    assert.equal(name, 'MOON');
  });
});
