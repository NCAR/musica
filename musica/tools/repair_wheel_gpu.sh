#!/usr/bin/env bash
set -e

auditwheel repair --exclude libcublas --exclude libcublasLt --exclude libcudart -w "$2" "$1"

for whl in "$2"/*.whl; do
  tmpdir=$(mktemp -d)
  unzip -q "$whl" -d "$tmpdir"
  tree "$tmpdir"
  echo "Before patchelf:"
  readelf -d "$tmpdir"/_musica*.so
  patchelf --remove-rpath "$tmpdir"/_musica*.so
  patchelf --set-rpath "\$ORIGIN:\$ORIGIN/../../nvidia/cublas/lib:\$ORIGIN/../../nvidia/cuda_runtime/lib" --force-rpath "$tmpdir"/_musica*.so
  # Remove bundled CUDA libraries
  rm -f "$tmpdir"/musica.libs/libcudart-*.so*
  rm -f "$tmpdir"/musica.libs/libcublas-*.so*
  rm -f "$tmpdir"/musica.libs/libcublasLt-*.so*
  echo "After patchelf:"
  readelf -d "$tmpdir"/_musica*.so
  # Repack the wheel with correct structure
  (cd "$tmpdir" && zip -qr "${whl%.whl}.patched.whl" .)
  rm -rf "$tmpdir"
  # Replace the original wheel with the patched one
  mv "${whl%.whl}.patched.whl" "$whl"
done
