// Backend loader for MUSICA - supports both Node.js addon and WASM
// This module provides a unified interface that works with either backend

const wasm = require('./wasm/index.js');

let backend = null;
let backendType = null;

/**
 * Initialize the backend (WASM or Node.js addon)
 * @returns {Promise<Object>} The initialized backend
 */
async function initBackend() {
  if (backend) {
    return { backend, backendType };
  }

  // Try WASM first if available
  if (wasm.hasWasm) {
    try {
      const wasmModule = await wasm.initModule();
      backend = wasmModule;
      backendType = 'wasm';
      console.log('Using WASM backend');
      return { backend, backendType };
    } catch (error) {
      console.warn('WASM initialization failed, falling back to Node.js addon:', error.message);
    }
  }

  // Fall back to Node.js addon
  try {
    const addon = require('./load_addon');
    backend = addon;
    backendType = 'node';
    console.log('Using Node.js addon backend');
    return { backend, backendType };
  } catch (error) {
    throw new Error('No backend available. Please build either the WASM module or Node.js addon.');
  }
}

/**
 * Get the current backend type
 * @returns {string|null} 'wasm' or 'node' or null if not initialized
 */
function getBackendType() {
  return backendType;
}

/**
 * Get the current backend
 * @returns {Object|null} The backend module or null if not initialized
 */
function getBackend() {
  return backend;
}

/**
 * Reset the backend (for testing purposes)
 */
function resetBackend() {
  backend = null;
  backendType = null;
}

module.exports = {
  initBackend,
  getBackendType,
  getBackend,
  resetBackend,
};
