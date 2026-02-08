#!/usr/bin/env bash
set -e
set -x

auditwheel repair --exclude libcublas --exclude libcublasLt --exclude libcudart --exclude libmvec -w "$2" "$1"

pip install wheel

for whl in "$2"/*.whl; do
  tmpdir=$(mktemp -d)
  unzip -q "$whl" -d "$tmpdir"
  tree "$tmpdir"

  so_files=("$tmpdir"/musica/_musica_gpu*.so)

  if [[ -f "${so_files[0]}" ]]; then
      so_path="${so_files[0]}"
      ls $so_path
      echo "Before patchelf:"
      readelf -d $so_path

      # Use patchelf to fix the rpath and library dependencies
      patchelf --remove-rpath $so_path
      patchelf --set-rpath "\$ORIGIN:\$ORIGIN/../musica.libs:\$ORIGIN/../nvidia/cublas/lib:\$ORIGIN/../nvidia/cuda_runtime/lib" --force-rpath $so_path

      # these may need to be periodically updated
      patchelf --replace-needed libcudart-c3a75b33.so.12.8.90 libcudart.so.12 $so_path
      patchelf --replace-needed libcublas-031ce6c2.so.12.8.4.1 libcublas.so.12 $so_path
      patchelf --replace-needed libcublasLt-10b5e663.so.12.8.4.1 libcublasLt.so.12 $so_path

      # Remove bundled CUDA libraries
      rm -f "$tmpdir"/musica.libs/libcudart-*.so*
      rm -f "$tmpdir"/musica.libs/libcublas-*.so*
      rm -f "$tmpdir"/musica.libs/libcublasLt-*.so*
      echo "After patchelf:"
      readelf -d $so_path
  else
    echo "No GPU .so file found, skipping patchelf steps"
  fi

  # Repack the wheel with correct structure
  wheel pack "$tmpdir" --dest-dir "$(dirname "$whl")"
done