# TS1 Configuration Fix

## Problem

The original `ts1.json` file was **incomplete** - it only contained reactions without the required species and phases sections needed by the MUSICA v1 parser.

### Original Issues:
1. Missing `version` field (required: "1.0.0")
2. Missing `species` array defining all chemical species
3. Missing `phases` section defining which species belong to which phase
4. Contains SURFACE reactions requiring aerosol phase and diffusion coefficients
5. Contains non-standard keys (`condensed phase`, `diffusion coefficient [m2 s-1]`)

## Solution

Created a script (`/tmp/fix_ts1_config.js`) that:

1. **Extracted all unique species** from reactions (210 species total)
2. **Combined with species from `initial_conditions.csv`** to ensure completeness
3. **Added required v1 format fields**:
   - `version: "1.0.0"`
   - `species`: Array of all 210 species with proper definitions
   - `phases`: Gas phase containing all species
   - Marked `M` as third body species
4. **Cleaned up reactions**:
   - Removed 13 SURFACE reactions (require aerosol phase support)
   - Removed non-standard keys from remaining reactions
   - Final count: 534 reactions

## Result

The fixed `config.json` now:
- ✓ Passes C++ parser validation
- ✓ Successfully loads in MICM solver
- ✓ Runs simulations without errors
- ✓ Contains 210 species, 1 gas phase, 534 reactions

## Reaction Types Supported

- ✓ ARRHENIUS (361 reactions)
- ✓ USER_DEFINED (142 reactions)
- ✓ TROE (31 reactions)
- ✗ SURFACE (13 reactions removed - require aerosol phase)

## Files

- **Original**: `ts1.json` (reactions only)
- **Fixed**: `config.json` (complete v1 format)
- **Fix Script**: `/tmp/fix_ts1_config.js`
- **Species Data**: `initial_conditions.csv`

## To Regenerate

If you need to regenerate the config:

```bash
node /tmp/fix_ts1_config.js
```

This will create a new `config.json` from the original `ts1.json` and `initial_conditions.csv`.

## Note on SURFACE Reactions

The 13 SURFACE reactions were removed because they require:
- Condensed phase definitions
- Diffusion coefficients for gas-phase species
- Aerosol surface area calculations

These could be added in the future by:
1. Adding a condensed phase to the phases section
2. Adding diffusion coefficient properties to relevant species
3. Providing aerosol surface area parameters

## Testing

Both mechanisms now work correctly:

```bash
# Test both Chapman and TS1
node /tmp/test_both_mechanisms.js
```

Output:
```
✓ Chapman Mechanism Example loaded
  ✓ Simulation SUCCESS - 3 points

✓ TS1 Mechanism Example loaded
  ✓ Simulation SUCCESS - 3 points
```

## Date Fixed

November 8, 2025
