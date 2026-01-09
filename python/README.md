# MUSICA Python Interface

MUSICA provides a Python interface for atmospheric chemistry modeling, including chemical mechanism analysis and visualization capabilities.

## Installation

### Using pip (Recommended)

Install the latest stable release from PyPI:

```bash
pip install musica
```

### GPU Support

For GPU-accelerated simulations, first add the NVIDIA PyPI index, then install with the GPU option:

```bash
pip install --upgrade setuptools pip wheel
pip install nvidia-pyindex
pip install musica[gpu]
```

**Note:** GPU support is only provided on Linux systems with NVIDIA GPUs and CUDA installed.

### Development Installation

To install MUSICA in development mode:

```bash
git clone https://github.com/NCAR/musica.git
cd musica
pip install -e .
```

To enable GPU support in a development build:

```bash
BUILD_GPU=1 pip install -e .
```

### Optional Dependencies

Install with tutorial dependencies for full functionality:

```bash
pip install 'musica[tutorial]'
```

## Usage Example

Here's a complete example of using MUSICA to solve a simple chemical system:

```python
# --- Import Musica ---
import musica
import musica.mechanism_configuration as mc

# --- 1. Define the chemical system of interest ---
A = mc.Species(name="A")
B = mc.Species(name="B")
C = mc.Species(name="C")
species = [A, B, C]
gas = mc.Phase(name="gas", species=species)

# --- 2. Define a mechanism of interest ---
# Through MUSICA, several different mechanisms can be explored to define reaction rates. 
# Here, we use the Arrhenius equation as a simple example.

r1 = mc.Arrhenius(name="A->B", A=4.0e-3, C=50, reactants=[A], products=[B], gas_phase=gas)
r2 = mc.Arrhenius(name="B->C", A=1.2e-4, B=2.5, C=75, D=50, E=0.5, reactants=[B], products=[C], gas_phase=gas)

mechanism = mc.Mechanism(name="musica_example", species=species, phases=[gas], reactions=[r1, r2])

# --- 3. Create MICM solver ---
# A solver must be initialized with either a configuration file or a mechanism:

solver = musica.MICM(mechanism=mechanism, solver_type=musica.SolverType.rosenbrock_standard_order)

# --- 4. Define environmental conditions ---
temperature = 300.0
pressure = 101000.0

# --- 5. Create and initialize State ---
# In the model, conditions represent the starting environment for the reactions 
# and are assigned by modifying the state.

state = solver.create_state()
state.set_concentrations({"A": 1.0, "B": 3.0, "C": 5.0})
state.set_conditions(temperature, pressure)

# --- 6. Time parameters ---
time_step = 4  # stepping
sim_length = 20  # total simulation time

# --- 7. Solve through time loop ---
# The following loop simply solves the model per each time step:

curr_time = time_step
while curr_time <= sim_length:
    solver.solve(state, time_step)
    concentrations = state.get_concentrations()
    curr_time += time_step

# --- 8. Solve and create DataFrame ---
# It is likely more useful to solve at each time step and store the associated data:
import pandas as pd

# Prepare to store output per time step
output_data = []

# Save initial state (t=0) for output visualization
initial_row = {
    "time.s": 0.0, 
    "ENV.temperature.K": temperature, 
    "ENV.pressure.Pa": pressure, 
    "ENV.air number density.mol m-3": state.get_conditions()['air_density'][0]
}
initial_row.update({f"CONC.{k}.mol m-3": v[0] for k, v in state.get_concentrations().items()})
output_data.append(initial_row)

curr_time = time_step
while curr_time <= sim_length:
    solver.solve(state, time_step)
    row = {
        "time.s": curr_time,
        "ENV.temperature.K": state.get_conditions()['temperature'][0],
        "ENV.pressure.Pa": state.get_conditions()['pressure'][0],
        "ENV.air number density.mol m-3": state.get_conditions()['air_density'][0]
    }
    row.update({f"CONC.{k}.mol m-3": v[0] for k, v in state.get_concentrations().items()})
    output_data.append(row)
    curr_time += time_step

df = pd.DataFrame(output_data)
print(df)

# --- 9. Visualize Results ---
import matplotlib.pyplot as plt

df.plot(
    x='time.s', 
    y=['CONC.A.mol m-3', 'CONC.B.mol m-3', 'CONC.C.mol m-3'], 
    title='Concentration over time', 
    ylabel='Concentration (mol m-3)', 
    xlabel='Time (s)'
)
plt.show()
```

## MUSICA CLI

MUSICA provides a command-line interface (`musica-cli`) for working with examples and configuration conversion.

### Basic Usage

Check the installed version:
```bash
musica-cli --version
```

View available options:
```bash
musica-cli -h
```

### Available Options

| Option | Description |
| ------ | ----------- |
| `-h`, `--help` | Show help message and exit |
| `-e`, `--example` | Name of the example to copy out |
| `-o`, `--output` | Path to save the output to |
| `-v`, `--verbose` | Increase logging verbosity. Use `-v` for info, `-vv` for debug |
| `--version` | Show the installed MUSICA version |
| `--convert` | Path to a MUSICA v0 configuration to convert to v1 format |

### Available Examples

| Example Name | Description |
| ------------ | ----------- |
| `CARMA_Aluminum` | A CARMA example for simulating aluminum aerosol particles |
| `CARMA_Sulfate` | A CARMA example for simulating sulfate aerosol particles |
| `Sulfate_Box_Model` | A box model example for simulating sulfate aerosol particles |
| `TS1LatinHyperCube` | A Latin hypercube sampling example for the TS1 mechanism |

### Example Workflow

Copy an example to your current directory:
```bash
musica-cli -e TS1LatinHyperCube
```

Copy an example to a specific directory:
```bash
musica-cli -e TS1LatinHyperCube -o /path/to/output/
```

Convert a MUSICA v0 configuration to v1 format:
```bash
musica-cli --convert /path/to/v0/config.json -o /path/to/output/
```

## Development

### Running Tests

Install test dependencies:
```bash
pip install 'musica[test]'
```

Run the test suite:
```bash
pytest
```

Run tests with coverage:
```bash
pytest --cov=musica
```

### Building Documentation

Install documentation dependencies:
```bash
pip install -r docs/requirements.txt
```

Build the documentation:
```bash
cd docs
make html
```

The built documentation will be available in `docs/build/html/`.

### Code Style

MUSICA Python code follows PEP 8 style guidelines. Before submitting contributions:

1. Ensure your code follows PEP 8 conventions
2. Add tests for new functionality
3. Update documentation as needed

## More Information

- [Full Documentation](https://ncar.github.io/musica/index.html)
- [Tutorials](https://mybinder.org/v2/gh/NCAR/musica/HEAD?filepath=tutorials)
- [Contributing Guide](../CONTRIBUTING.md)
