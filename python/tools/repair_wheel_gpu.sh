#!/usr/bin/env bash
set -euo pipefail

INPUT_WHEEL="$1"
OUTDIR="$2"

mkdir -p "$OUTDIR"

python -m pip install --upgrade wheel

# 1. auditwheel FIRST (only time)
auditwheel repair \
  --exclude libcublas \
  --exclude libcublasLt \
  --exclude libcudart \
  -w "$OUTDIR" "$INPUT_WHEEL"

# 2. Patch the repaired wheel
for whl in "$OUTDIR"/*.whl; do
  tmpdir="$(mktemp -d)"
  unzip -q "$whl" -d "$tmpdir"

  so="$(ls "$tmpdir"/musica/_musica_gpu*.so 2>/dev/null || true)"
  if [[ -n "$so" ]]; then
    patchelf --remove-rpath "$so"
    patchelf --set-rpath \
      '$ORIGIN:$ORIGIN/../nvidia/cublas/lib:$ORIGIN/../nvidia/cuda_runtime/lib' \
      --force-rpath "$so"

    patchelf --replace-needed libcudart-*.so.* libcudart.so.12 "$so" || true
    patchelf --replace-needed libcublas-*.so.* libcublas.so.12 "$so" || true
    patchelf --replace-needed libcublasLt-*.so.* libcublasLt.so.12 "$so" || true

    rm -f "$tmpdir"/musica.libs/libcudart-*.so*
    rm -f "$tmpdir"/musica.libs/libcublas-*.so*
    rm -f "$tmpdir"/musica.libs/libcublasLt-*.so*
  fi

  rm "$whl"
  (cd "$tmpdir" && python -m wheel pack . -d "$OUTDIR")
  rm -rf "$tmpdir"
done
