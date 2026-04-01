---
name: Version Release
about: Create an issue to make a new release
title: 'Release X.X.X'
labels: ''
assignees: ''

---

## Pre-release

- [ ] Launch Binder and run all tutorial examples (use the main branch Binder link)
- [ ] GitHub Actions are passing on `main`
- [ ] On a new branch (do **not** name it `release`), update dependency versions in `cmake/dependencies.cmake` — ensure `GIT_TAG` points to an actual tag (not a commit or branch) for each:
  - [ ] MICM
  - [ ] TUV-x
  - [ ] Mechanism Configuration
- [ ] Update the version in `CMakeLists.txt`
- [ ] Update the version number in `CITATION.cff`
- [ ] Update the version number in `package.json`
- [ ] On GitHub, merge `main` into `release` — **do NOT squash and merge**
  - Alternatively, merge locally and push: `git checkout release && git merge main && git push`
- [ ] Create a tag and add release notes on GitHub — be sure to select the `release` branch as the target

---

## Python (automatic)

PyPI publishing happens automatically via the release action when a tag and release are created.

- [ ] Verify the PyPI release was published successfully

> Note: `pyproject.toml` version is updated automatically from `CMakeLists.txt`

---

## JavaScript (automatic)

npm publishing happens automatically via the release action when a tag and release are created.

- [ ] Verify the npm release was published successfully

---

## C++ / Fortran — Spack (manual)

Spack is the primary delivery mechanism for C++ and Fortran users.

- [ ] Update the spack file in the [spack-packages repository](https://github.com/NCAR/spack-packages):
  - Path: `repos/spack_repo/builtin/packages/musica/package.py`
  - To generate a hash: install spack, then run `spack checksum musica` and follow the prompts
  - Often, you have to tag specific maintainers on the spack PR to get it merged

---

## Julia (manual)

All julia code lives in `musica` but is published under two separate packages. Complete `Musica_jll.jl` before `Musica.jl`.

### Musica_jll.jl (binary package)

Julia provides binaries by packaging them into JLLs via [Yggdrasil](https://github.com/JuliaPackaging/Yggdrasil).

- [ ] Open a PR into Yggdrasil updating the `git tag` in `build_tarballs.jl`
  - See prior PRs [#13280](https://github.com/JuliaPackaging/Yggdrasil/pull/13280) and [#13351](https://github.com/JuliaPackaging/Yggdrasil/pull/13351) for reference
- [ ] Wait for the Yggdrasil PR to merge and `Musica_jll.jl` to appear in the Julia General Registry

### Musica.jl

- [ ] Go to the most recent commit on `main` and comment:
  ```
  @JuliaRegistrator register subdir=julia
  ```
  This will automatically open a PR into the Julia General Registry
