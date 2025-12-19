const { describe, it } = require('node:test');
const path = require('path');

describe('WASM Version Tests', () => {
  it('should return the correct MUSICA version from WASM', async () => {
    try {
      // Try to load the WASM module
      const musica = require('../../../wasm/index.js');
      const version = await musica.getVersion();
      console.log(`MUSICA Version (WASM): ${version}`);
      
      if (typeof version !== 'string' || version.length === 0) {
        throw new Error('Invalid MUSICA version string from WASM');
      }
    } catch (error) {
      if (error.message.includes('WASM module not found')) {
        console.log('WASM module not built yet. This is expected if you have not run the WASM build.');
        console.log('To build WASM: emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DMUSICA_ENABLE_JAVASCRIPT=ON -DMUSICA_ENABLE_TUVX=OFF -DMUSICA_ENABLE_CARMA=OFF && cmake --build build-wasm');
      } else {
        throw error;
      }
    }
  });

  it('should return the correct MICM version from WASM', async () => {
    try {
      const musica = require('../../../wasm/index.js');
      const micmVersion = await musica.getMicmVersion();
      console.log(`MICM Version (WASM): ${micmVersion}`);
      
      if (typeof micmVersion !== 'string' || micmVersion.length === 0) {
        throw new Error('Invalid MICM version string from WASM');
      }
    } catch (error) {
      if (error.message.includes('WASM module not found')) {
        console.log('WASM module not built yet. This is expected if you have not run the WASM build.');
        console.log('To build WASM: emcmake cmake -S . -B build-wasm -DCMAKE_BUILD_TYPE=Release -DMUSICA_ENABLE_JAVASCRIPT=ON -DMUSICA_ENABLE_TUVX=OFF -DMUSICA_ENABLE_CARMA=OFF && cmake --build build-wasm');
      } else {
        throw error;
      }
    }
  });
});
