#!/bin/bash
set -e
set -x

# macOS wheel repair script using delocate
# Usage: repair_wheel_macos.sh <input_wheel> <output_directory>

input_wheel="$1"
output_dir="$2"

if [[ -z "$input_wheel" || -z "$output_dir" ]]; then
    echo "Usage: $0 <input_wheel> <output_directory>"
    exit 1
fi

# Ensure output directory exists
mkdir -p "$output_dir"

MUSICA_PREBUILT_DIR="/tmp/musica-prebuilt"

echo "Input wheel: $input_wheel"
echo "Output directory: $output_dir"
echo "Prebuilt directory: $MUSICA_PREBUILT_DIR"
echo "Contents of prebuilt lib:"
ls -la "$MUSICA_PREBUILT_DIR/lib/" || true

# Detect architecture from wheel name or system
if [[ "$input_wheel" == *"arm64"* ]]; then
    ARCH="arm64"
elif [[ "$input_wheel" == *"x86_64"* ]]; then
    ARCH="x86_64"
else
    ARCH=$(uname -m)
fi
echo "Detected architecture: $ARCH"

# Create a temporary directory to work with the wheel
tmpdir=$(mktemp -d)
echo "Working directory: $tmpdir"

# Unzip the wheel
unzip -q "$input_wheel" -d "$tmpdir"

echo "Contents of wheel before repair:"
find "$tmpdir" -name "*.dylib" -o -name "*.so" | head -20

# Check if libraries are missing from the wheel and copy them if needed
wheel_lib_dir="$tmpdir/musica"
if [[ -d "$wheel_lib_dir" ]]; then
    # Copy any missing prebuilt libraries
    for lib in "$MUSICA_PREBUILT_DIR/lib/"*.dylib; do
        if [[ -f "$lib" ]]; then
            libname=$(basename "$lib")
            if [[ ! -f "$wheel_lib_dir/$libname" ]]; then
                echo "Copying missing library: $libname"
                cp "$lib" "$wheel_lib_dir/"
            else
                echo "Library already present: $libname"
            fi
        fi
    done
fi

echo "Contents of wheel lib dir after copying:"
ls -la "$wheel_lib_dir/" || true

# Show what libraries the extension depends on
echo "Dependencies of _musica extension:"
find "$tmpdir" -name "_musica*.so" -exec otool -L {} \; || true

# Repack the wheel with the additional libraries
pip install wheel
repack_dir="$tmpdir/repacked"
mkdir -p "$repack_dir"
wheel pack "$tmpdir" --dest-dir "$repack_dir"

# Find the repacked wheel
repacked_wheel=$(find "$repack_dir" -name "*.whl" | head -1)
echo "Repacked wheel: $repacked_wheel"

if [[ -z "$repacked_wheel" ]]; then
    echo "ERROR: Failed to find repacked wheel"
    exit 1
fi

# Now run delocate on the wheel with all libraries present
delocate-wheel \
    --require-archs "$ARCH" \
    -w "$output_dir" \
    -v \
    "$repacked_wheel"

# Clean up
rm -rf "$tmpdir"

echo "Wheel repair completed successfully"
