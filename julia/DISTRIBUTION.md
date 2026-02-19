# MUSICA Julia Package Distribution Guide

This document describes the process for publishing Musica.jl to the Julia ecosystem.

> [!NOTE] 
> Julia 1.7 **must** be used to run `BinaryBuilder.jl`, regardless of the version of Julia you are building for.

## Overview

The distribution process involves two repositories:
1. **Yggdrasil** - Creates pre-compiled binaries (Musica_jll)
2. **General Registry** - Registers the Musica.jl package

## Step 1: Submit to Yggdrasil

[Yggdrasil](https://github.com/JuliaPackaging/Yggdrasil) is Julia's community build tree that uses BinaryBuilder.jl to create cross-platform binaries.

### Prerequisites

```bash
# Install BinaryBuilder (optional, for local testing)
julia -e 'using Pkg; Pkg.add("BinaryBuilder")'
```

### Local Testing (Optional)

Test the build locally before submitting:

```bash
cd julia/yggdrasil

# Test for a specific platform
julia --project=@BinaryBuilder build_tarballs.jl x86_64-linux-gnu-cxx11 --verbose

# Interactive wizard mode (useful for debugging)
julia --project=@BinaryBuilder build_tarballs.jl --debug
```

### Submit PR to Yggdrasil

1. Fork [JuliaPackaging/Yggdrasil](https://github.com/JuliaPackaging/Yggdrasil)

2. Create the package directory structure:
   ```bash
   mkdir -p M/Musica
   cp julia/yggdrasil/build_tarballs.jl M/Musica/
   ```

3. Submit a PR with the build_tarballs.jl file

4. Wait for CI to build binaries for all platforms

5. Once merged, Yggdrasil automatically:
   - Creates `Musica_jll` package
   - Registers it in the General registry
   - Builds binaries for all supported platforms

### Yggdrasil PR Checklist

- [ ] Build succeeds on all target platforms
- [ ] Library products are correctly exported
- [ ] Dependencies (libcxxwrap_julia_jll) are correctly specified
- [ ] Version matches MUSICA version (0.14.4)

## Step 2: Update Musica.jl for JLL

After Musica_jll is created, update the package to use it:

### Update Project.toml

Add the JLL dependency (get the UUID from the generated package):

```toml
[deps]
CxxWrap = "1f15a43c-97ca-5a2a-ae31-89f07a497df4"
Libdl = "8f399da3-3557-5675-b5ff-fb832c97cbdb"
Musica_jll = "<uuid-from-yggdrasil>"
Preferences = "21216c6a-2e73-6563-6e65-726566657250"

[compat]
CxxWrap = "^0.16"
Musica_jll = "0.14"
Preferences = "1"
julia = "1.10 - 1.11"
```

### Update src/Musica.jl

Uncomment the JLL loading code:

```julia
# In get_library_path(), uncomment:
try
    using Musica_jll
    return Musica_jll.libmusica_julia
catch
end
```

## Step 3: Register in General Registry

After Musica_jll is available, register Musica.jl:

### Using Registrator Bot

1. Ensure the package is in a public GitHub repository
2. Comment on any commit or issue: `@JuliaRegistrator register`
3. The bot will open a PR to the General registry
4. Wait for automated checks and maintainer approval

### Registration Requirements

- [ ] Package has a valid UUID in Project.toml
- [ ] Version follows semantic versioning
- [ ] Package loads without errors
- [ ] Tests pass
- [ ] No duplicate package name conflicts

### First Registration Checklist

For the initial registration:
- [ ] Package name is unique and descriptive
- [ ] Authors and contact info are correct
- [ ] License file exists
- [ ] README provides usage examples

## Version Updates

When releasing new versions:

1. **Update MUSICA** - Make changes to the C++ code
2. **Update Yggdrasil** - Submit PR with new version and git hash
3. **Update Musica.jl** - Bump version in Project.toml
4. **Register new version** - Use `@JuliaRegistrator register`

### Version Synchronization

Keep versions synchronized across:
- `CMakeLists.txt` (MUSICA version)
- `julia/Project.toml` (Musica.jl version)
- `julia/yggdrasil/build_tarballs.jl` (Musica_jll version)

## Troubleshooting

### BinaryBuilder Issues

**Library not found:**
- Verify `CMAKE_INSTALL_PREFIX` is set correctly
- Check that install targets are defined in CMakeLists.txt

**CxxWrap ABI mismatch:**
- Ensure `libcxxwrap_julia_jll` version compatibility is correct
- Use `expand_cxxstring_abis()` for platform definitions

**Cross-compilation failures:**
- Avoid Fortran dependencies (TUVX, CARMA) for initial builds
- Use `CMAKE_TOOLCHAIN_FILE` for cross-compilation

### Registration Issues

**UUID conflict:**
- Generate a new UUID: `using UUIDs; uuid4()`

**Package load failure:**
- Test locally: `julia --project=. -e 'using Musica'`
- Check for missing dependencies

## Resources

- [BinaryBuilder.jl Documentation](https://docs.binarybuilder.org/)
- [Yggdrasil Repository](https://github.com/JuliaPackaging/Yggdrasil)
- [Julia Package Registration](https://pkgdocs.julialang.org/v1/registries/)
- [General Registry](https://github.com/JuliaRegistries/General)
- [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl)
