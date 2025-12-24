# MUSICA JavaScript Interfaces

MUSICA provides two JavaScript interfaces:

1. **Node.js Native Addon** - Can include the full suite of musica software since a Fortran compiler can be made available
2. **WebAssembly (WASM)** - Portable interface for browsers and environments without native addon support. This can't—[at least not without some work](https://gws.phd/posts/fortran_wasm/)—provide support for the Fortran parts of musica.

## Unified API (Recommended)

The package now provides a unified API that automatically uses the available backend (Node.js addon or WASM). This is the recommended way to use MUSICA in your applications.

```javascript
const musica = require('@ncar/musica');
const { MICM } = musica.micmSolver;

// Works with Node.js addon (when built)
const micm = MICM.fromConfigPath('./configs/my_mechanism');
const state = micm.createState(1);

// To explicitly use WASM backend (when available)
await MICM.initWasm();  // Initialize WASM
const micm = MICM.fromConfigPath('./configs/my_mechanism');
```

### Version Functions

For Node.js addon:
```javascript
const musica = require('@ncar/musica');

// Synchronous API
const version = musica.getVersion();
const micmVersion = musica.getMicmVersion();

console.log('MUSICA Version:', version);
console.log('MICM Version:', micmVersion);
```

For WASM (async):
```javascript
const wasm = require('@ncar/musica/wasm');

// Asynchronous API (returns Promises)
(async () => {
  await wasm.initModule();
  
  const version = await wasm.getVersion();
  const micmVersion = await wasm.getMicmVersion();
  
  console.log('MUSICA Version:', version);
  console.log('MICM Version:', micmVersion);
})();
```

## MICM Solver Usage

The MICM solver interface is consistent across both backends:

```javascript
const musica = require('@ncar/musica');
const { MICM, SolverType } = musica.micmSolver;

// Create MICM instance from configuration
const micm = MICM.fromConfigPath('./configs/analytical');

// Create a state with 1 grid cell
const state = micm.createState(1);

// Set initial conditions
state.setConcentrations({
  'A': [1.0],
  'B': [0.0],
  'C': [0.0]
});

state.setConditions({
  temperatures: [298.15],
  pressures: [101325.0],
  air_densities: [1.0]
});

// Solve for a time step
const result = micm.solve(state, 60.0);  // 60 second time step

// Get updated concentrations
const concentrations = state.getConcentrations();
console.log('Concentrations:', concentrations);
```

## Backend Selection

The package automatically selects the available backend:

1. **Default behavior**: Uses Node.js addon if available (synchronous)
2. **WASM backend**: Call `MICM.initWasm()` to explicitly use WASM
3. **Check backend**: Use `MICM.getBackendType()` to see which is active

```javascript
const { MICM } = require('@ncar/musica').micmSolver;

// Check which backend is being used
const backendType = MICM.getBackendType();
console.log('Using backend:', backendType);  // 'node' or 'wasm' or null
```

## Building

### Node.js Native Addon

```bash
npm run build:node
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

```bash
npm run build:wasm
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

## Scaffolding Status

The WASM bindings have been scaffolded with embind but are not yet fully implemented:

### Completed
- ✅ MICM class scaffolding with `fromConfigPath`, `fromConfigString`, `createState`, `solve` methods
- ✅ State class scaffolding with all methods (concentrations, conditions, parameters, etc.)
- ✅ Unified JavaScript API that works with both backends
- ✅ CMakeLists.txt updated to build WASM with wrapper files

### In Progress
- ⚠️ Full implementation and testing of WASM bindings
- ⚠️ Validation with both backends built and working

The JavaScript layer is now ready to work with either backend once the implementations are complete.
