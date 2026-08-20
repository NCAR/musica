# MUSICA JavaScript Interface

MUSICA provides a JavaScript interface using WebAssembly (WASM) for portable, cross-platform atmospheric chemistry modeling. The WASM backend works in both Node.js and browser environments.

## Installation

```bash
npm install @ncar/musica
```

### Prerequisites

- [Node.js](https://nodejs.org/) (version 22 or later recommended)
- [Emscripten SDK >=4.0.2](https://emscripten.org/docs/getting_started/downloads.html) for compiling to WebAssembly
- CMake (>= 3.21)

### Quick Start

```javascript
import * as musica from '@ncar/musica';
const { MICM } = musica;

// Initialize WASM backend
await musica.initModule();

const version = await musica.getVersion();
const micmVersion = await musica.getMicmVersion();

console.log('MUSICA Version:', version);
console.log('MICM Version:', micmVersion);
```

## Examples

Below are some more detailed examples that demonstrates time-stepping through a chemical simulation.

### In-code mechanism

Chemical mechanisms can be defined entirely in code using our mechanism configuration interface.

```javascript
import * as musica from '@ncar/musica';
const { MICM, SolverType, GAS_CONSTANT } = musica;

import { createRequire } from "node:module";
const require = createRequire(import.meta.url);

function musicaConfig(path) {
  return require.resolve(`@ncar/musica/configs/${path}`);
}

// Initialize WASM backend
await musica.initModule();

const A = new Species({ name: 'A' });
const B = new Species({ name: 'B' });
const C = new Species({ name: 'C' });
const gas = new Phase({ name: 'gas', species: [A, B, C] });

// simple chain: A -> B -> C
const reactions = [
  new reactionTypes.UserDefined({ name: 'A_to_B', gas_phase: 'gas', reactants: [new ReactionComponent({ species_name: 'A' })], products: [new ReactionComponent({ species_name: 'B' })] }),
  new reactionTypes.UserDefined({ name: 'B_to_C', gas_phase: 'gas', reactants: [new ReactionComponent({ species_name: 'B' })], products: [new ReactionComponent({ species_name: 'C' })] })
];

const mechanism = new Mechanism({ name: 'A->B->C CRN', version: '1.0.0', species: [A, B, C], phases: [gas], reactions: reactions });

micm = MICM.fromMechanism(mechanism);
state = micm.createState(1);

// Set environmental conditions
const temperature = 272.5;  // K
const pressure = 101253.3;  // Pa
const airDensity = pressure / (GAS_CONSTANT * temperature);

state.setConditions({
  temperatures: [temperature],
  pressures: [pressure],
  air_densities: [airDensity]
});

// Set initial concentrations (mol/m³)
state.setConcentrations({
  'A': [0.75],
  'B': [0.0],
  'C': [0.4],
  'D': [0.8],
  'E': [0.0],
  'F': [0.1]
});

// Rate parameters
state.setUserDefinedRateParameters({ 'USER.A_to_B': 1.0, 'USER.B_to_C': 0.5 });

// Time-stepping loop
const timeStep = 1.0;  // seconds
const simLength = 100;  // seconds
let currTime = 0;

const results = [];
while (currTime < simLength) {
  // Solve chemistry for this time step
  const result = micm.solve(state, timeStep);
  
  // Store results
  const concentrations = state.getConcentrations();
  results.push({
    time: currTime,
    A: concentrations.A[0],
    B: concentrations.B[0],
    C: concentrations.C[0]
  });
  
  currTime += timeStep;
}

console.log('Simulation complete!');
console.log(results);
```

### Using a configuration file

You may also use one of the configurations provided with this package. Note, this will require you to run in node,
not the browser.

```javascript
import * as musica from '@ncar/musica';
const { MICM, SolverType, GAS_CONSTANT } = musica;

import { createRequire } from "node:module";
const require = createRequire(import.meta.url);

function musicaConfig(path) {
  return require.resolve(`@ncar/musica/configs/${path}`);
}

// Initialize WASM backend
await musica.initModule();

// Create MICM instance from configuration
const micm = MICM.fromConfigPath(musicaConfig("v0/analytical"));


// Create a state with 1 grid cell
const state = micm.createState(1);

// Set environmental conditions
const temperature = 272.5;  // K
const pressure = 101253.3;  // Pa
const airDensity = pressure / (GAS_CONSTANT * temperature);

state.setConditions({
  temperatures: [temperature],
  pressures: [pressure],
  air_densities: [airDensity]
});

// Set initial concentrations (mol/m³)
state.setConcentrations({
  'A': [0.75],
  'B': [0.0],
  'C': [0.4],
  'D': [0.8],
  'E': [0.0],
  'F': [0.1]
});

// Set reaction rate parameters
state.setUserDefinedRateParameters({
  'USER.reaction 1': [0.001],
  'USER.reaction 2': [0.002]
});

// Time-stepping loop
const timeStep = 1.0;  // seconds
const simLength = 100;  // seconds
let currTime = 0;

const results = [];
while (currTime < simLength) {
  // Solve chemistry for this time step
  const result = micm.solve(state, timeStep);
  
  // Store results
  const concentrations = state.getConcentrations();
  results.push({
    time: currTime,
    A: concentrations.A[0],
    B: concentrations.B[0],
    C: concentrations.C[0]
  });
  
  currTime += timeStep;
}

console.log('Simulation complete!');
console.log(results);
```

## Development

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/NCAR/musica.git
   cd musica
   ```

2. Install and activate [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   cd ..
   ```

3. Install Node.js dependencies:
   ```bash
   npm install
   ```

4. Build the WASM module:
   ```bash
   npm run build
   ```

The WASM files (`musica.js` and `musica.wasm`) will be automatically placed in the `javascript/wasm/` directory.

### Testing

Run all tests:
```bash
npm run test
```

Run only unit tests:
```bash
npm run test:unit
```

Run only integration tests:
```bash
npm run test:integration
```

Run tests with coverage:
```bash
npm run test:coverage
```

### Browser Example

You can try a demo at [https://ncar.github.io/musica/](https://ncar.github.io/musica/). 

The same demo can be built and run locally. Note that you'd need to update the import state in [index.html](wasm/index.html).

The `wasm` directory contains an [index.html](wasm/index.html) file that demonstrates using MUSICA in a web browser. 
The example displays version numbers and contains some example mechanisms with interactive sliders.

To run the browser example, you need to serve the files through a web server:

**If you're doing local development**
```bash
npm run example
```

**If you installed from npm, you can also run the examples**
```bash
npx musica-example
```

Then open http://localhost:8000/javascript/wasm/index.html in your browser to see MUSICA running in WebAssembly!

### Global Surface Example

The `wasm` directory also contains [global.html](wasm/global.html), a global chemistry transport example.
MICM solves a NOx-O3 mechanism independently in every cell of a lat-lon surface grid. A prescribed wind
field then transports the species between the cells.

Open http://localhost:8000/javascript/wasm/global.html to run it locally, or try the deployed version at
[https://ncar.github.io/musica/global.html](https://ncar.github.io/musica/global.html).

The example shows how to drive many grid cells from JavaScript:

- One MICM grid cell holds each lat-lon box. A 5-degree grid gives 2592 cells.
- The photolysis rate of NO2 comes from the solar zenith angle in each cell, so a day and night
  contrast moves across the map.
- Prescribed emissions of NO occur in the cells that contain 10 cities.
- Transport runs in JavaScript between the chemistry steps. The code reads the concentrations with
  `state.getConcentrations()`, advects them, and writes them back with `state.setConcentrations()`.

The page reports the wall time for each step. On a 5-degree grid, MICM solves all 2592 cells in a few
milliseconds, so the model animates in real time.

#### Flow patterns

Every pattern comes from a streamfunction on the cell corners. The face fluxes are differences of that
streamfunction, so the discrete wind field is divergence free and the transport conserves mass to
machine precision. A selector offers five patterns:

| Pattern | What it shows |
| --- | --- |
| Rossby-Haurwitz wave | An exact solution of the barotropic vorticity equation, and a standard dynamical core test case. The wave keeps its shape and travels at an analytic phase speed. With R = 4 it makes one circuit in 29.5 days, eastward. |
| Zonal jets with a travelling wave | Tropical easterlies with a westerly jet in each middle latitude band, plus a travelling wave. |
| Blocking high over a low | A stationary high over a low reverses the flow between the two centres and splits the westerly jet. |
| Polar vortex | A strong circumpolar jet near 65 N with a wave on its edge. The jet acts as a partial transport barrier. |
| Deformational flow | The non-divergent test case of Nair and Lauritzen (2010). The deformation reverses at half the period, so an exact scheme returns every tracer to its starting point. |

The advection scheme is also selectable. The default is a second-order scheme with a van Leer limiter.
The alternative is first-order upwind, which is much more diffusive. In a solid body rotation test at
5 degrees, the second-order scheme keeps 77 % of the peak of a cosine bell after one full revolution,
against 32 % for first-order upwind. Both schemes conserve mass, keep the field non-negative, and
preserve a uniform field exactly.

### Code Style

JavaScript code in MUSICA follows these conventions:

- Use ES6+ module syntax (`import`/`export`)

## More Information

- [Full Documentation](https://ncar.github.io/musica/index.html)
- [Contributing Guide](https://github.com/NCAR/musica/blob/main/CONTRIBUTING.md)
