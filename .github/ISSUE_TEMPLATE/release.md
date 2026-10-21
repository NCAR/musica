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
  - [ ] MIAM
  - [ ] MIEM
  - [ ] TUV-x
  - [ ] Mechanism Configuration
- [ ] Update the version in `CMakeLists.txt`
- [ ] Update the version number in `CITATION.cff`
- [ ] Update the version number in `package.json`
- [ ] Check if `configs/` changed since the last release: `git diff --stat vX.X.X main -- configs/`
  - [ ] If it changed, set `version` in `python/musica-data/pyproject.toml` to today's date (`YYYY.M.D`)
  - [ ] Set the `musica-data` lower bound in `pyproject.toml` to the same version
- [ ] On GitHub, merge `main` into `release` — **do NOT squash and merge**
  - Alternatively, merge locally and push: `git checkout release && git merge main && git push`
- [ ] Create a tag and add release notes on GitHub — be sure to select the `release` branch as the target

---

## Python (automatic)

PyPI publishing happens automatically via the release action when a tag and release are created.

- [ ] Verify the PyPI release was published successfully
- [ ] If `musica-data` was updated, verify that the new [musica-data](https://pypi.org/project/musica-data/) version was published

> Note: `pyproject.toml` version is updated automatically from `CMakeLists.txt`

---

## JavaScript (automatic)

npm publishing happens automatically via the release action when a tag and release are created.

- [ ] Verify the npm release was published successfully

---

## C++ / Fortran — Spack (manual)

Spack is the primary delivery mechanism for C++ and Fortran users.

- [ ] Update the spack file in the [spack-packages repository](https://github.com/spack/spack-packages/):
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

- [ ] **After Musica_jll.jl has completed publishing**, update `julia/Project.toml` (do **not** change any `uuid` because UUIDs are permanent package identifiers):
  - [ ] Bump the top-level `version` (Musica.jl's own version). This must be increased past the last registered version or JuliaRegistrator will reject the registration. It is **not** auto-synced from `CMakeLists.txt`.
  - [ ] Update the `[compat]` bound for `Musica_jll` to the newly released version (e.g. `Musica_jll = "0.15"`).
    - [ ] Also bump `version` in `etc/Musica_jll/Project.toml` (the local dev stub) to satisfy this new bound.
  - [ ] Merge these changes into `main` before registering.
- [ ] Go to the most recent commit on `main` and post a **new comment** containing the registration command below. Copy the whole code block, filling in the release notes.

  ```
  @JuliaRegistrator register subdir=julia

  Release notes:

  ## Breaking changes

  - blah
  ```

  This will automatically open a PR into the Julia General Registry
