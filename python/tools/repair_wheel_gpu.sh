#!/usr/bin/env bash
set -e

tmpdir=$(mktemp -d)
unzip -q "$1" -d "$tmpdir"

# Remove nested wheel
rm -f "$tmpdir"/*.whl

# Patch GPU .so files as before
so_files=("$tmpdir"/musica/_musica_gpu*.so)
if [[ -f "${so_files[0]}" ]]; then
    so_path="${so_files[0]}"
    patchelf --remove-rpath $so_path
    patchelf --set-rpath "\$ORIGIN:\$ORIGIN/../nvidia/cublas/lib:\$ORIGIN/../nvidia/cuda_runtime/lib" --force-rpath $so_path
    patchelf --replace-needed libcudart-b5a066d7.so.12.2.140 libcudart.so.12 $so_path
    patchelf --replace-needed libcublas-e779a79d.so.12.2.5.6 libcublas.so.12 $so_path
    patchelf --replace-needed libcublasLt-fbfbc8a1.so.12.2.5.6 libcublasLt.so.12 $so_path
fi

# Remove bundled CUDA libraries
rm -f "$tmpdir"/musica.libs/libcudart-*.so*
rm -f "$tmpdir"/musica.libs/libcublas-*.so*
rm -f "$tmpdir"/musica.libs/libcublasLt-*.so*

python -m pip install --upgrade pip
python -m pip install wheel auditwheel

# Repack wheel (generates correct RECORD)
(cd "$tmpdir" && python -m wheel pack . -d "$tmpdir")
interim_wheel=$(ls "$tmpdir"/*.whl)

# Repair with auditwheel
auditwheel repair --exclude libcublas --exclude libcublasLt --exclude libcudart -w "$2" "$interim_wheel"

rm -rf "$tmpdir"
