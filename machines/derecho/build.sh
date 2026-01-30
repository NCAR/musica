#!/bin/bash
set -euo pipefail

# --------------------------------------------------------------------------- #
#  Build MUSICA (with MICM, TUV-x, CARMA) on NCAR Derecho
#
#  Usage:  machines/derecho/build.sh <install-prefix> [jobs]
#  Example: machines/derecho/build.sh /glade/work/$USER/packages 32
#
#  Run this script from the MUSICA repository root.
# --------------------------------------------------------------------------- #

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <install-prefix> [jobs (default: 8)]" >&2
  exit 1
fi

INSTALL_PREFIX="$1"
JOBS="${2:-8}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

mkdir -p "$INSTALL_PREFIX"

# ---- Environment ---------------------------------------------------------- #
source /etc/profile.d/z00_modules.sh 2>/dev/null || source /usr/share/lmod/lmod/init/bash
export LMOD_QUIET=1
module --force purge
module load ncarenv/25.10
module load gcc/14.3.0
module load ncarcompilers/1.2.0
module load cray-mpich/8.1.32
module load netcdf/4.9.3
module load parallel-netcdf/1.14.1
module load openblas/0.3.30

echo "==> Modules loaded"
module list

# ---- Configure ------------------------------------------------------------ #
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

echo "==> Configuring with CMake"
cmake "$REPO_ROOT" \
  -DMUSICA_ENABLE_MICM=ON \
  -DMUSICA_ENABLE_TUVX=ON \
  -DMUSICA_ENABLE_CARMA=ON \
  -DMUSICA_BUILD_FORTRAN_INTERFACE=ON \
  -DMUSICA_ENABLE_TESTS=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"

# ---- Build ---------------------------------------------------------------- #
echo "==> Building ($JOBS jobs)"
make -j"$JOBS"

# ---- Test ----------------------------------------------------------------- #
echo "==> Running tests"
ctest --output-on-failure

# ---- Install -------------------------------------------------------------- #
echo "==> Installing to $INSTALL_PREFIX"
make install

echo "==> Done. MUSICA installed to $INSTALL_PREFIX"
