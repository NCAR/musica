const assert = require('node:assert');
const { describe, it } = require('node:test');
const musica = require('@ncar/musica');

describe('Version Tests', () => {
  it('should return the MUSICA version', async (t) => {
    const wasm = require('../../index.js');
    if (!wasm.hasWasm) {
      t.skip();
      return;
    }
    const version = await musica.getVersion();
    console.log(`MUSICA Version: ${version}`);
    assert.ok(typeof version === 'string' && version.length > 0, 'Invalid MUSICA version string');
  });

  it('should return the MICM version',  async (t) => {
    const wasm = require('../../index.js');
    if (!wasm.hasWasm) {
      t.skip();
      return;
    }
    const micmVersion = await musica.getMicmVersion();
    console.log(`MICM Version: ${micmVersion}`);
    assert.ok(typeof micmVersion === 'string' && micmVersion.length > 0, 'Invalid MICM version string');
  });
});
