#!/bin/bash
set -e

# Windows wheel repair script using delvewheel
# Usage: repair_wheel_windows.sh <input_wheel> <output_directory>

input_wheel="$1"
output_dir="$2"

if [[ -z "$input_wheel" || -z "$output_dir" ]]; then
    echo "Usage: $0 <input_wheel> <output_directory>"
    exit 1
fi

# Ensure output directory exists
mkdir -p "$output_dir"

# Check if we're in MSYS2 environment and find Python executable
if [[ -n "$pythonLocation" ]]; then
    # Convert Windows path to MSYS2 path for GitHub Actions
    PYTHON_CMD=$(cygpath -u "$pythonLocation")/python.exe
else
    # Fallback to system python
    PYTHON_CMD=python
fi

echo "Using Python: $PYTHON_CMD"
echo "Input wheel: $input_wheel"
echo "Output directory: $output_dir"

# Install delvewheel if not already available
"$PYTHON_CMD" -m pip install --upgrade delvewheel

# Repair the wheel using delvewheel
# Add common Windows library paths where dependencies might be found
"$PYTHON_CMD" -m delvewheel repair \
    --add-path /ucrt64/bin \
    --add-path /ucrt64/lib \
    --add-path /mingw64/bin \
    --add-path /mingw64/lib \
    --wheel-dir "$output_dir" \
    "$input_wheel"

echo "Wheel repair completed successfully"
