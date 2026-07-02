/**
 * Initialize the MUSICA WASM module
 * Must be called before using any WASM functionality
 * @returns {Promise<Object>} The initialized WASM module
 */
export function initModule(): Promise<Object>;
/**
 * Get the initialized WASM module
 * @returns {Object} WASM module
 */
export function getBackend(): Object;
