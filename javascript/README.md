# MUSICA JavaScript Interface

MUSICA provides a JavaScript interface using WebAssembly (WASM) for portable, cross-platform atmospheric chemistry modeling. The WASM backend works in both Node.js and browser environments.

## Installation

The JavaScript interface is included in the MUSICA repository and requires building from source.

### Prerequisites

- [Node.js](https://nodejs.org/) (version 18 or later recommended)
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) for compiling to WebAssembly
- CMake (>= 3.21)

### Quick Start

```javascript
import * as musica from '@ncar/musica';
const { MICM } = musica;

// Initialize WASM backend
await musica.initModule();

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

## Complete Usage Example

Here's a more detailed example that demonstrates time-stepping through a chemical simulation:

```javascript
import * as musica from '@ncar/musica';
const { MICM, SolverType, GAS_CONSTANT } = musica;

// Initialize WASM backend
await musica.initModule();

// Create MICM instance from configuration
const micm = MICM.fromConfigPath('./configs/analytical');

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

The `wasm` directory contains an [example.html](wasm/example.html) file that demonstrates using MUSICA in a web browser. The example displays version numbers and solves a simple chemical system.

To run the browser example, you need to serve the files through a web server (due to WASM security requirements):

```bash
# From the repository root or javascript directory
python3 -m http.server 8000
```

Then open http://localhost:8000/javascript/wasm/example.html in your browser to see MUSICA running in WebAssembly!

### Code Style

JavaScript code in MUSICA follows these conventions:

- Use ES6+ module syntax (`import`/`export`)
- Use `const` and `let` (avoid `var`)
- Follow async/await patterns for asynchronous operations
- Add JSDoc comments for public APIs
- Use descriptive variable names

### Debugging

For debugging the WASM build, you can build in debug mode:

```bash
npm run build:wasm
```

This creates a debug build with symbols that can help diagnose issues.

## API Reference

### Main Functions

- **`initModule()`**: Initialize the WebAssembly module (must be called before using MICM)
- **`getVersion()`**: Get the MUSICA version string
- **`getMicmVersion()`**: Get the MICM version string

### MICM Class

- **`MICM.fromConfigPath(configPath)`**: Create a MICM solver from a configuration directory
- **`createState(numCells)`**: Create a state object for the specified number of grid cells
- **`solve(state, timeStep)`**: Solve chemistry for the given time step

### State Methods

- **`setConcentrations(concentrations)`**: Set species concentrations
- **`getConcentrations()`**: Get current species concentrations
- **`setConditions(conditions)`**: Set environmental conditions (temperature, pressure, air density)
- **`getConditions()`**: Get current environmental conditions
- **`setUserDefinedRateParameters(params)`**: Set user-defined reaction rate parameters
- **`getUserDefinedRateParameters()`**: Get current user-defined rate parameters

## More Information

- [Full Documentation](https://ncar.github.io/musica/index.html)
- [Contributing Guide](../CONTRIBUTING.md)
- [Main Repository README](../README.md)
