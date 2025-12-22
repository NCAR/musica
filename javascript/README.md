# MUSICA JavaScript Interfaces

MUSICA provides two JavaScript interfaces:

1. **Node.js Native Addon** - Can include the full suite of musica software since a Fortra compiler can be made available
2. **WebAssembly (WASM)** - Portable interface for browsers and environments without native addon support. This can't—[at least not without some work](https://gws.phd/posts/fortran_wasm/)—provide support for the Fortran parts of musica.

## Usage Examples

### Node.js Native Addon

```javascript
const musica = require('musica-addon');

// Synchronous API
const version = musica.getVersion();
const micmVersion = musica.getMicmVersion();

console.log('MUSICA Version:', version);
console.log('MICM Version:', micmVersion);
```

### WASM Interface

```javascript
const musica = require('musica-addon/wasm');

// Asynchronous API (returns Promises)
(async () => {
  await musica.initModule();
  
  const version = await musica.getVersion();
  const micmVersion = await musica.getMicmVersion();
  
  console.log('MUSICA Version:', version);
  console.log('MICM Version:', micmVersion);
})();
```

## Building

### Node.js Native Addon

```bash
npm run build
```

### WASM Module

First, install and activate [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):

```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Then build the WASM module:

```bash
# Configure
emcmake cmake -S . -B build-wasm \
  -DCMAKE_BUILD_TYPE=Release \
  -DMUSICA_ENABLE_JAVASCRIPT=ON \
  -DMUSICA_ENABLE_TUVX=OFF \
  -DMUSICA_ENABLE_CARMA=OFF \
  -DMUSICA_ENABLE_TESTS=OFF

# Build
cmake --build build-wasm
```

The WASM files (`musica.js` and `musica.wasm`) will be automatically placed in the `javascript/wasm/` directory.

## Testing

### Test Node.js Native Addon
```bash
npm test
```

### Test WASM Interface
```bash
npm run test:unit:wasm
```

in [wasm](wasm) there is an [example.html](wasm/example.html) file which will
display the version number from musica by making a call into musica with the built library.

To run this, the file must be provided by a web server. A simple way to view this is
to serve this file with python. Navigate to the [wasm](wasm) directory and run this command

```bash
python3 -m http.server 8000
```

You'll then be able to open http://localhost:8000/example.html in your browser
and see musica return data from C++ through web assembly!

## API Compatibility

Both interfaces provide the same functionality, with the main differences being:

| Feature | Native Addon | WASM |
|---------|-------------|------|
| API Style | Synchronous | Asynchronous (Promises) |
| Initialization | Automatic | Manual (`initModule()`) |
| Performance | Faster | Slightly slower |
| Platform | Node.js only | Node.js + Browsers |
| Binary Size | Varies by platform | ~same for all platforms |

## Current Features

Both interfaces currently support:
- ✅ Getting MUSICA version
- ✅ Getting MICM version

Additional features from the native addon will be gradually added to the WASM interface in future updates.
