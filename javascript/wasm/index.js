// WASM module wrapper for MUSICA
// This provides a JavaScript-friendly interface to the WASM module

let musicaModule = null;

/**
 * Initialize the MUSICA WASM module
 * This must be called before using any other functions
 * @returns {Promise<Object>} The initialized module
 */
async function initModule() {
  if (musicaModule) {
    return musicaModule;
  }
  
  // In a browser environment, the WASM files should be served from the same directory
  // In Node.js, we need to load from the filesystem
  const isNode = typeof process !== 'undefined' && process.versions && process.versions.node;
  
  if (isNode) {
    const path = require('path');
    const fs = require('fs');
    
    // Try to load the module factory
    try {
      const createMusicaModule = require('./musica.js');
      musicaModule = await createMusicaModule();
    } catch (error) {
      throw new Error('MUSICA WASM module not found. Please build the WASM module first. Error: ' + error.message);
    }
  } else {
    // Browser environment
    throw new Error('Browser environment not yet fully supported. Please load musica.js script first.');
  }
  
  return musicaModule;
}

/**
 * Get the MUSICA version
 * @returns {Promise<string>} The version string
 */
async function getVersion() {
  const module = await initModule();
  return module.getVersion();
}

/**
 * Get the MICM version
 * @returns {Promise<string>} The version string
 */
async function getMicmVersion() {
  const module = await initModule();
  return module.getMicmVersion();
}

module.exports = {
  initModule,
  getVersion,
  getMicmVersion
};
