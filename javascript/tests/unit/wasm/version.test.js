const { assert } = require('console');
const { describe, it } = require('node:test');

describe('WASM Version Tests', () => {
  it('should return the MUSICA version from WASM', async () => {
    try {
      // Try to load the WASM module
      const musica = require('../../../wasm/index.js');
      const version = await musica.getVersion();
      console.log(`MUSICA Version (WASM): ${version}`);
      assert.ok(typeof version === 'string' && version.length > 0, 'Invalid MUSICA version string from WASM');
    } catch (error) {
      if (error.message.includes('WASM module not found')) {
        console.log('WASM module not built yet. This is expected if you have not run the WASM build.');
      } else {
        throw error;
      }
    }
  });

  it('should return the MICM version from WASM', async () => {
    try {
      const musica = require('../../../wasm/index.js');
      const micmVersion = await musica.getMicmVersion();
      console.log(`MICM Version (WASM): ${micmVersion}`);
      assert.ok(typeof micmVersion === 'string' && micmVersion.length > 0, 'Invalid MICM version string from WASM');
    } catch (error) {
      if (error.message.includes('WASM module not found')) {
        console.log('WASM module not built yet. This is expected if you have not run the WASM build.');
        console.log(WASM_BUILD_INSTRUCTIONS);
      } else {
        throw error;
      }
    }
  });
});
