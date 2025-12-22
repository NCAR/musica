const assert = require('node:assert');
const { describe, it } = require('node:test');
const { hasWasm, musica_wasm } = require('../../util/testUtils.js');

describe('WASM Version Tests', {skip: !hasWasm}, () => {
  it('should return the MUSICA version from WASM', async () => {
    const version = await musica_wasm.getVersion();
    console.log(`MUSICA Version (WASM): ${version}`);
    assert.ok(typeof version === 'string' && version.length > 0, 'Invalid MUSICA version string from WASM');
  });

  it('should return the MICM version from WASM', {skip: !hasWasm}, async () => {
    const micmVersion = await musica_wasm.getMicmVersion();
    console.log(`MICM Version (WASM): ${micmVersion}`);
    assert.ok(typeof micmVersion === 'string' && micmVersion.length > 0, 'Invalid MICM version string from WASM');
  });
});
