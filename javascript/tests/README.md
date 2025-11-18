# JavaScript Tests for MUSICA

This directory contains JavaScript tests that mirror the Python test suite for MUSICA.

## Test Framework

All tests use **Node.js built-in test runner** (available in Node.js 18+), which provides a lightweight, zero-dependency testing solution. This approach:
- Requires no additional testing framework dependencies
- Uses familiar `describe()` and `it()` syntax
- Provides built-in assertions via `node:assert`
- Integrates seamlessly with CI/CD pipelines

## Test Structure

```
javascript/tests/
├── integration/          # Integration tests
│   ├── analytical.test.js   # Analytical mechanism tests
│   └── chapman.test.js      # Chapman mechanism tests
└── unit/                 # Unit tests
    └── micm/
        └── state.test.js    # State class unit tests
```

## Running Tests

### Prerequisites

1. Build the native addon first:
   ```bash
   npm run build
   ```

2. Ensure Node.js 18+ is installed:
   ```bash
   node --version  # Should be 18.0.0 or higher
   ```

### Run All Tests

```bash
npm test
```

This runs all test files matching the pattern `javascript/tests/**/*.test.js`.

### Run Specific Test Suites

```bash
# Run only integration tests
npm run test:integration

# Run only unit tests
npm run test:unit

# Run a specific test file
npm run test:analytical   # Analytical mechanism tests
npm run test:chapman      # Chapman mechanism tests
npm run test:state        # State unit tests
```

### Run Tests with More Details

For more verbose output:

```bash
node --test --test-reporter=spec javascript/tests/**/*.test.js
```

## Test Coverage

### Integration Tests

#### Analytical Tests (`analytical.test.js`)
Mirrors `python/test/integration/test_analytical.py`

- Single grid cell tests with multiple solvers:
  - Rosenbrock standard order
  - Backward Euler standard order
  - Rosenbrock (vector-ordered)
- Multiple grid cell tests (1-10 cells) with:
  - Random initial conditions
  - Verification against analytical solutions
  - Multiple solver types

#### Chapman Tests (`chapman.test.js`)
Mirrors `python/test/integration/test_chapman.py`

- Tests Chapman mechanism with:
  - v0 config directory format
  - v1 JSON config file format
  - v1 YAML config file format
- Verifies:
  - Photolysis reactions
  - Concentration evolution
  - Rate constant handling

### Unit Tests

#### State Tests (`state.test.js`)
Mirrors `python/test/unit/micm/test_state.py`

- State initialization:
  - Single and multiple grid cells
  - Error handling for invalid inputs
- Concentration management:
  - Setting and getting concentrations
  - Single vs. multiple grid cells
  - Empty update handling
- Environmental conditions:
  - Temperature, pressure, air density
  - Integer value handling
  - Automatic air density calculation
- User-defined rate parameters:
  - Parameter setting and retrieval
  - Multi-grid cell support
- State ordering:
  - Species ordering
  - Rate parameter ordering
- Grid cell operations

## Test Comparison with Python

All JavaScript tests are designed to closely mirror their Python counterparts:

| Python Test | JavaScript Test | Status |
|------------|----------------|--------|
| `test_analytical.py` | `analytical.test.js` | ✅ Implemented |
| `test_chapman.py` | `chapman.test.js` | ✅ Implemented |
| `test_state.py` | `state.test.js` | ✅ Implemented |

## Development

### Adding New Tests

1. Create a new test file following the naming convention `*.test.js`
2. Use the Node.js test runner syntax:
   ```javascript
   const { describe, it } = require('node:test');
   const assert = require('node:assert');
   
   describe('Feature name', () => {
       it('should do something', () => {
           // Test implementation
           assert.strictEqual(actual, expected);
       });
   });
   ```
3. Add a corresponding script to `package.json` if needed
4. Update this README with test descriptions

### Helper Functions

Common helper functions are available in test files:

- `isClose(a, b, atol, rtol)`: Checks if two floating-point numbers are approximately equal (similar to NumPy's `isclose`)

## Continuous Integration

Tests are designed to run in CI/CD environments. The exit code indicates success (0) or failure (non-zero).

Example CI configuration:
```yaml
- name: Run JavaScript tests
  run: |
    npm run build
    npm test
```

## Troubleshooting

### Module Not Found Error

If you see `Cannot find module 'musica-addon'`, make sure you've built the native addon:
```bash
npm run build
```

### Test Runner Not Found

If `node --test` is not recognized, upgrade to Node.js 18 or higher:
```bash
node --version  # Check version
# Install Node.js 18+ if needed
```

### Tests Fail on Vector-Ordered Rosenbrock with >4 Grid Cells

This is expected. The vector-ordered Rosenbrock solver currently supports up to 4 grid cells in JavaScript bindings. The tests are configured to handle this limitation.
