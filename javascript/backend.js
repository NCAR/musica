// backend.js (ESM)
let wasmModule = null;

/**
 * Initialize the MUSICA WASM module
 * Must be called before using any WASM functionality
 * @returns {Promise<Object>} The initialized WASM module
 */
export async function initModule() {
  if (wasmModule) return wasmModule;

  const isNode =
    typeof process !== 'undefined' &&
    process.versions?.node != null;

  // Dynamically import the Emscripten module in both environments
  const { default: createMusicaModule } = await import('./wasm/musica.js');
  wasmModule = await createMusicaModule();

  if (isNode) {
    const { FS, NODEFS } = wasmModule;
    FS.mkdir('/host');
    FS.mount(NODEFS, { root: '/' }, '/host');
  }

  return wasmModule;
}

/**
 * Get the initialized WASM module
 * @returns {Object} WASM module
 */
export function getBackend() {
  if (!wasmModule) {
    throw new Error('WASM not initialized. Call await initModule() first.');
  }
  return wasmModule;
}
