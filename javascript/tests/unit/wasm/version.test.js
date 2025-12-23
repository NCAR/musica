const assert = require('node:assert');
const { describe, it } = require('node:test');
const { musica_wasm } = require('../../util/testUtils.js');

describe('WASM Version Tests', () => {
  it('should return the MUSICA version from WASM', async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }
    const version = await musica_wasm.getVersion();
    console.log(`MUSICA Version (WASM): ${version}`);
    assert.ok(typeof version === 'string' && version.length > 0, 'Invalid MUSICA version string from WASM');
  });

  it('should return the MICM version from WASM',  async (t) => {
    if (!musica_wasm.hasWasm) {
      t.skip();
      return;
    }
    const micmVersion = await musica_wasm.getMicmVersion();
    console.log(`MICM Version (WASM): ${micmVersion}`);
    assert.ok(typeof micmVersion === 'string' && micmVersion.length > 0, 'Invalid MICM version string from WASM');
  });
});
