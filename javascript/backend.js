let wasm = null;

async function initModule() {
  if (wasm) return wasm;
  const createMusicaModule = require('./wasm/musica.js');
  wasm = await createMusicaModule();

  // Mount the host filesystem at /host
  const { FS, NODEFS } = wasm;
  FS.mkdir('/host');
  FS.mount(NODEFS, { root: '/' }, '/host');

  return wasm;
}

function getBackend() {
  if (!wasm) {
    throw new Error("WASM not initialized. Call await initModule()");
  }
  return wasm;
}

module.exports = { initModule, getBackend };
