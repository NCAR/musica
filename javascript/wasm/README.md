# MUSICA WASM JavaScript Interface

This directory contains the WebAssembly (WASM) interface for MUSICA, allowing it to be used in web browsers and Node.js environments where native addons are not available or desired.

## Prerequisites

To build the WASM module, you need:
- Emscripten SDK installed and activated
- CMake 3.21 or later
- All MUSICA dependencies

## Building the WASM Module

### Install Emscripten

```bash
# Download and install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### Build MUSICA WASM

From the repository root:

```bash
# Configure with Emscripten
emcmake cmake -S . -B build-wasm \
  -DCMAKE_BUILD_TYPE=Release \
  -DMUSICA_ENABLE_JAVASCRIPT=ON \
  -DMUSICA_ENABLE_TUVX=OFF \
  -DMUSICA_ENABLE_CARMA=OFF \
  -DMUSICA_ENABLE_TESTS=OFF

# Build
cmake --build build-wasm
```

The WASM files will be automatically placed in `javascript/wasm/` directory.

## Usage

### Node.js

```javascript
const musica = require('musica-addon/wasm');

(async () => {
  // Initialize the module (required before first use)
  await musica.initModule();
  
  // Get versions
  const version = await musica.getVersion();
  const micmVersion = await musica.getMicmVersion();
  
  console.log('MUSICA Version:', version);
  console.log('MICM Version:', micmVersion);
})();
```

### Browser

Copy `musica.js` and `musica.wasm` to your web server directory, then:

```html
<script src="musica.js"></script>
<script>
  createMusicaModule().then(module => {
    const version = module.getVersion();
    const micmVersion = module.getMicmVersion();
    
    console.log('MUSICA Version:', version);
    console.log('MICM Version:', micmVersion);
  });
</script>
```

See `example.html` in this directory for a complete working example.

## Differences from Node.js Native Addon

The WASM interface differs from the native Node.js addon in several ways:

1. **Asynchronous API**: All WASM functions are asynchronous and return Promises
2. **Initialization**: You must call `initModule()` before using the module
3. **Performance**: WASM may be slower than native addons for compute-intensive operations
4. **Portability**: WASM works in browsers and any JavaScript environment that supports it

## Testing

Run the WASM tests:

```bash
npm run test:unit:wasm
```

## Current Features

The WASM interface currently supports:
- ✅ Getting MUSICA version
- ✅ Getting MICM version

Additional features will be added in future updates.
