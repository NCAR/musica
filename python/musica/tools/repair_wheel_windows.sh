#!/bin/bash
set -e

# Windows wheel repair script for MUSICA using delvewheel
# This script bundles necessary DLLs from MSYS2 into the wheel
# so that users don't need MSYS2 installed to use the package

wheel_file="$1"
output_dir="$2"

echo "=== MUSICA Windows Wheel Repair ==="
echo "Input wheel: $wheel_file"
echo "Output directory: $output_dir"
echo "Current working directory: $(pwd)"

pip install delvewheel

# Show what we're starting with
echo "=== Original wheel contents ==="
python -m zipfile -l "$wheel_file"

# Use delvewheel to repair the wheel and bundle necessary DLLs
echo "=== Running delvewheel repair ==="
echo "Adding search paths: /mingw64/bin, /mingw64/lib"

# Run delvewheel with verbose output
delvewheel repair "$wheel_file" \
    --wheel-dir "$output_dir" \
    --add-path "/mingw64/bin;/mingw64/lib" \
    --verbose

echo "=== Repair completed ==="

# Show what we ended up with
echo "=== Repaired wheel contents ==="
for whl in "$output_dir"/*.whl; do
    if [[ -f "$whl" ]]; then
        echo "Contents of $(basename "$whl"):"
        python -m zipfile -l "$whl"
        break
    fi
done

echo "=== Windows wheel repair finished successfully ==="