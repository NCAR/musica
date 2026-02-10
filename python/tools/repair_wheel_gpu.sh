#!/usr/bin/env bash
set -e
set -x

# Exclude CUDA libraries from auditwheel - users should install via nvidia-* pip packages
auditwheel repair --exclude libcublas --exclude libcublasLt --exclude libcudart --exclude libmvec -w "$2" "$1"

pip install wheel

for whl in "$2"/*.whl; do
  tmpdir=$(mktemp -d)
  unzip -q "$whl" -d "$tmpdir"
  tree "$tmpdir"

  # declare an array of libraries to patch
  plugins=("$tmpdir/musica/libmusica_cuda.so" "$tmpdir/musica/libmicm_cuda.so")


  for cuda_plugin in "${plugins[@]}"; do
    if [[ -f "$cuda_plugin" ]]; then
      echo "Found CUDA plugin: $cuda_plugin"
      echo "Before patchelf:"
      readelf -d "$cuda_plugin"

      # Use patchelf to fix the rpath and library dependencies
      patchelf --remove-rpath "$cuda_plugin"
      patchelf --set-rpath "\$ORIGIN:\$ORIGIN/../musica.libs:\$ORIGIN/../nvidia/cublas/lib:\$ORIGIN/../nvidia/cuda_runtime/lib" --force-rpath "$cuda_plugin"

      # Replace versioned CUDA library names with generic ones (these may need periodic updates)
      patchelf --replace-needed libcudart-c3a75b33.so.12.8.90 libcudart.so.12 "$cuda_plugin" 2>/dev/null || true
      patchelf --replace-needed libcublas-031ce6c2.so.12.8.4.1 libcublas.so.12 "$cuda_plugin" 2>/dev/null || true
      patchelf --replace-needed libcublasLt-10b5e663.so.12.8.4.1 libcublasLt.so.12 "$cuda_plugin" 2>/dev/null || true

      # Remove bundled CUDA libraries - users should use nvidia-* pip packages
      rm -f "$tmpdir"/musica.libs/libcudart-*.so*
      rm -f "$tmpdir"/musica.libs/libcublas-*.so*
      rm -f "$tmpdir"/musica.libs/libcublasLt-*.so*

      echo "After patchelf:"
      readelf -d "$cuda_plugin"
    else
      echo "No CUDA plugin found, skipping patchelf steps"
    fi
  done

  # Repack the wheel with correct structure
  wheel pack "$tmpdir" --dest-dir "$(dirname "$whl")"
done