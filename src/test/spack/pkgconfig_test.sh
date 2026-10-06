#!/usr/bin/env bash
# Builds the same sources as the CMake consumer project, but discovers MUSICA
# through pkg-config instead of find_package. The .pc files must be sufficient
# on their own: no extra -I/-L/-l flags appear below.
#
# --static is what pulls in the Requires.private dependencies (fmt), which the
# default static libraries need at link time. gfortran does not add a C++ runtime,
# so it only links if the .pc files carry one (see cmake/cxx_runtime_packaging.cmake).
#
# Usage:
#   pkgconfig_test.sh [--prefix DIR] [--config FILE] [--fortran] [--no-micm] [--no-miem]
#
# Without --prefix, PKG_CONFIG_PATH is used as-is, e.g. after `spack load musica`.
# CXX and FC pick the compilers; they default to c++ and gfortran.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
prefix=""
config="${here}/../../../configs/v1/chapman/config.json"
fortran=0
micm=1
miem=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --prefix)   prefix="$2"; shift 2 ;;
    --config)   config="$2"; shift 2 ;;
    --fortran)  fortran=1; shift ;;
    --no-micm)  micm=0; shift ;;
    --no-miem)  miem=0; shift ;;
    *) echo "unknown argument: $1" >&2; exit 2 ;;
  esac
done

if [[ -n "${prefix}" ]]; then
  for d in "${prefix}/lib/pkgconfig" "${prefix}/lib64/pkgconfig"; do
    [[ -d "${d}" ]] && PKG_CONFIG_PATH="${d}${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
  done
  export PKG_CONFIG_PATH
fi

CXX="${CXX:-c++}"
FC="${FC:-gfortran}"

work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

require() {
  pkg-config --exists "$1" || {
    echo "pkg-config cannot find $1 (PKG_CONFIG_PATH=${PKG_CONFIG_PATH:-})" >&2
    exit 1
  }
  echo "$1 $(pkg-config --modversion "$1")"
}

# pkg-config reports -L for the linker but never -rpath, and a Spack or Homebrew
# prefix is not on the default loader path, so the binary builds and then fails
# to start. CMake and Spack's compiler wrappers both add rpath on their own; do
# the same here, from the -L entries pkg-config just handed us. This is about
# the loader, not about the .pc contents the test is checking.
# Fills the global RPATH_FLAGS array; `read -a` would only take the first line,
# and command substitution would mangle any path containing a space.
rpath_flags() {
  RPATH_FLAGS=()
  local flag
  for flag in "$@"; do
    case "${flag}" in
      -L*) RPATH_FLAGS+=("-Wl,-rpath,${flag#-L}") ;;
    esac
  done
}

require musica
cxxflags=()
# main.cpp only reaches MIEM, and so -lmiem/-lnetcdf, when this is defined
[[ ${miem} -eq 1 ]] && cxxflags+=(-DMUSICA_TEST_MIEM)

read -r -a musica_libs <<< "$(pkg-config --libs --static musica)"
rpath_flags "${musica_libs[@]}"

# shellcheck disable=SC2046  # pkg-config output is intentionally word-split
${CXX} ${cxxflags[@]+"${cxxflags[@]}"} $(pkg-config --cflags musica) "${here}/main.cpp" \
       -o "${work}/test_cxx" "${musica_libs[@]}" ${RPATH_FLAGS[@]+"${RPATH_FLAGS[@]}"}
"${work}/test_cxx"

if [[ ${fortran} -eq 1 ]]; then
  require musica-fortran

  fflags=(-cpp)
  args=()
  if [[ ${micm} -eq 1 ]]; then
    [[ -f "${config}" ]] || { echo "config not found: ${config}" >&2; exit 1; }
    fflags+=(-DMUSICA_TEST_MICM)
    args+=("${config}")
  fi

  read -r -a mf_libs <<< "$(pkg-config --libs --static musica-fortran)"
  rpath_flags "${mf_libs[@]}"

  # shellcheck disable=SC2046
  ${FC} "${fflags[@]}" $(pkg-config --cflags musica-fortran) "${here}/main.F90" \
        -o "${work}/test_fortran" "${mf_libs[@]}" ${RPATH_FLAGS[@]+"${RPATH_FLAGS[@]}"}
  "${work}/test_fortran" ${args[@]+"${args[@]}"}
fi
