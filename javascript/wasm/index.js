// WASM module wrapper for MUSICA
// This provides a JavaScript-friendly interface to the WASM module
// NOTE: The WASM module file is expected to be named 'musica.js' as configured
// in javascript/CMakeLists.txt (OUTPUT_NAME "musica")

let musicaModule = null;

const isNodeEnv = typeof process !== 'undefined' && process.versions && process.versions.node;

function WasmBuilt() {
  if (!isNodeEnv) return;
  const path = require('path');
  const fs = require('fs');
  const wasmFile = path.resolve(__dirname, 'musica.wasm');
  const jsFile = path.resolve(__dirname, 'musica.js');
  return fs.existsSync(wasmFile) && fs.existsSync(jsFile);
}

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
    // Try to load the module factory
    try {
      const createMusicaModule = require('./musica.js');
      musicaModule = await createMusicaModule();
    } catch (error) {
      throw new Error('MUSICA WASM module not found. Please build the WASM module first. Error: ' + error.message);
    }
  } else {
    // Browser environment - requires musica.js to be loaded via script tag first
    if (typeof createMusicaModule === 'undefined') {
      throw new Error('Browser environment detected. Please load musica.js via script tag before using this module. See example.html for a working example.');
    }
    musicaModule = await createMusicaModule();
  }

  // Wrap MICM and State to provide a consistent API
  if (musicaModule.MICM) {
    const OriginalMICM = musicaModule.MICM;
    musicaModule.MICM = {
      fromConfigPath: (configPath, solverType = 1) => {
        return OriginalMICM.fromConfigPath(configPath, solverType);
      },
      fromConfigString: (configString, solverType = 1) => {
        return OriginalMICM.fromConfigString(configString, solverType);
      }
    };
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
  getMicmVersion,
  hasWasm: WasmBuilt(),
};