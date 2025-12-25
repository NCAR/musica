# MUSICA JavaScript Interface

MUSICA provides a JavaScript interface using WebAssembly (WASM) for portable, cross-platform atmospheric chemistry modeling. The WASM backend works in both Node.js and browser environments.

## Quick Start

```javascript
import * as musica from '@ncar/musica';
const { MICM } = musica;

// Initialize WASM backend
await MICM.initWasm();

// Create MICM instance from configuration
const micm = MICM.fromConfigPath('./configs/my_mechanism');
const state = micm.createState(1);

// Solve
const result = micm.solve(state, 60.0);
```

## Version Functions

The version functions use the WASM backend and are asynchronous:

```javascript
import * as musica from '@ncar/musica';

// Asynchronous API (returns Promises)
const version = await musica.getVersion();
const micmVersion = await musica.getMicmVersion();

console.log('MUSICA Version:', version);
console.log('MICM Version:', micmVersion);
```

## MICM Solver Usage

The MICM solver provides a consistent interface for atmospheric chemistry modeling:

```javascript
import * as musica from '@ncar/musica';
const { MICM, SolverType } = musica;

// Initialize WASM backend
await musica.initModule();

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

## Building

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
npm run build
```

The WASM files (`musica.js` and `musica.wasm`) will be automatically placed in the `javascript/wasm/` directory.

## Testing

```bash
npm run test
```

## Browser Example

in [wasm](wasm) there is an [example.html](wasm/example.html) file which will
display the version number from musica and micm as well as solve a very simple
system by making calls into musica with the built library.

To run this, the file must be provided by a web server. A simple way to view this is
to serve this file with python. From this directory ([javascript](./)), run a server

```bash
python3 -m http.server 8000
```

You'll then be able to open http://localhost:8000/wasm/example.html in your browser
and see musica return data from C++ through web assembly!
