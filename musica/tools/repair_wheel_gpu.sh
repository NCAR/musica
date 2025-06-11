#!/usr/bin/env bash
set -e

auditwheel repair --exclude libcublas --exclude libcublasLt --exclude libcudart -w "$2" "$1"

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
      patchelf --set-rpath "\$ORIGIN:\$ORIGIN/../nvidia/cublas/lib:\$ORIGIN/../nvidia/cuda_runtime/lib" --force-rpath $so_path
      # these may need to be periodically updated
      patchelf --replace-needed libcudart-b5a066d7.so.12.2.140 libcudart.so.12 $so_path
      patchelf --replace-needed libcublas-e779a79d.so.12.2.5.6 libcublas.so.12 $so_path
      patchelf --replace-needed libcublasLt-fbfbc8a1.so.12.2.5.6 libcublasLt.so.12 $so_path
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
  (cd "$tmpdir" && zip -qr "${whl%.whl}.patched.whl" .)
  rm -rf "$tmpdir"
  # Replace the original wheel with the patched one
  mv "${whl%.whl}.patched.whl" "$whl"
done
