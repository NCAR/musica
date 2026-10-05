# Copyright Spack Project Developers. See COPYRIGHT file for details.
#
# SPDX-License-Identifier: (Apache-2.0 OR MIT)

# NCAR's copy of the MUSICA recipe. It tracks the one in spack-packages so that a
# PR can change the CMake options and the recipe together; .github/workflows/spack.yml
# builds this copy against the PR checkout. Changes here should be mirrored upstream.

import os

from spack_repo.builtin.build_systems.cmake import CMakePackage

from spack.package import *


class Musica(CMakePackage):
    """MUSICA - The multi-scale interface for chemistry and aerosols

    MUSICA is a software package which exposes a flexible
    API for including aerosol and gas-phase chemistry in
    many contexts across languages and platforms. It is designed to
    be used in conjunction with other software packages, such as
    climate models, to provide a comprehensive framework for
    simulating atmospheric chemistry processes.
    """

    homepage = "https://github.com/NCAR/musica"
    url = "https://github.com/NCAR/musica/archive/refs/tags/v0.15.0.tar.gz"
    git = "https://github.com/NCAR/musica.git"

    maintainers("kshores", "boulderdaze")

    license("Apache-2.0", checked_by="kshores")

    # Needed by the stand-alone test, which compiles a consumer project
    test_requires_compiler = True

    # Versions
    version("develop", branch="main")
    version("0.16.0", sha256="aec7be3f46abfd334f1842acf88434c1d3d872703714641fc5a4730892a1d947")
    version("0.15.0", sha256="48a01c082d8db8731add80c691c7794e7069927fa17ec21e093ad1774b8cd30b")
    version("0.14.5", sha256="31d9c75e8a4852f8f053f43a44313f3b392911056b72f63209eb2f091c7e486a")
    version("0.14.4", sha256="8de573c1cf9100087efa6ae4871f0d0bd8b41e75a67e43b62761410e5740d0d6")
    version("0.14.2", sha256="78cdab4e355535e1201bb8a4c997187bb59014984b956525dd8ed04c22669953")
    version("0.14.1", sha256="c776fc224b4d40cbc0371726240fd7f02b142c969ce1418627f63e0e4ec81829")
    version("0.14.0", sha256="f6841780747c522bfa4a27da0ec694373e08aa2488a748cd3dea81be5472db0c")
    version("0.13.0", sha256="fec033c39d48081185fcbbab96effe0a8c0994b91d8660f9b91d12ebad3b29d4")
    version("0.12.0", sha256="e81279fbdd42af8bf6540f18e72857ed34e081421a90333c77f9952a3069363b")
    version("0.10.1", sha256="edefab03a676a449761997734e6c5b654b2c4f92ce8f1cc66ef63b8ae8ccccf1")

    # Options from CMake
    variant("fortran", default=False, description="Build Fortran interface")
    variant("tests", default=False, description="Enable tests")
    variant("mpi", default=False, description="Enable MPI support")
    variant("openmp", default=False, description="Enable OpenMP support")
    variant("micm", default=True, description="Enable MICM support")
    variant("miam", default=True, description="Enable MIAM support")
    variant("miem", default=True, description="Enable MIEM support")
    variant("tuvx", default=True, description="Enable TUV-x support")
    variant("javascript", default=False, description="Build the JavaScript addon")
    variant("julia", default=False, description="Build the Julia wrapper")
    variant("shared", default=False, description="Build MUSICA as shared libraries")
    variant(
        "fmt",
        default=True,
        description="Use the fmt library instead of the standard library for formatting",
    )

    # Dependencies
    depends_on("cmake@3.21:", type="build")
    # "test" as well as "build": the stand-alone test consumes the installed
    # .pc files, and build deps are not in the environment by then.
    depends_on("pkgconfig", type=("build", "test"))
    depends_on("c", type="build")
    depends_on("cxx", type="build")
    depends_on("fortran", type="build")
    depends_on("mpi", when="+mpi")
    # MIEM reads netCDF files, so it needs the C library even without TUV-x.
    depends_on("netcdf-c", when="+miem")
    depends_on("netcdf-fortran", when="+tuvx")
    depends_on("libcxxwrap-julia", when="+julia")
    depends_on("fmt", when="+fmt")

    def cmake_args(self):
        args = [
            self.define_from_variant("MUSICA_BUILD_FORTRAN_INTERFACE", "fortran"),
            self.define("MUSICA_ENABLE_INSTALL", True),
            self.define_from_variant("MUSICA_ENABLE_TESTS", "tests"),
            self.define_from_variant("MUSICA_ENABLE_MPI", "mpi"),
            self.define_from_variant("MUSICA_ENABLE_OPENMP", "openmp"),
            self.define_from_variant("MUSICA_ENABLE_MICM", "micm"),
            self.define_from_variant("MUSICA_ENABLE_MIAM", "miam"),
            self.define_from_variant("MUSICA_ENABLE_MIEM", "miem"),
            self.define_from_variant("MUSICA_ENABLE_TUVX", "tuvx"),
            # CARMA defaults to ON upstream, but always fetches an unreleased,
            # commit-pinned CARMA-ACOM-dev tree at build time. Keep it OFF to avoid
            # network access and unmanaged BLAS/LAPACK/netCDF dependencies.
            self.define("MUSICA_ENABLE_CARMA", False),
            # Always bundle C++ dependencies (MICM, TUV-x, etc.), since Spack builds
            # MUSICA standalone rather than as a subproject.
            self.define("MUSICA_BUNDLE_DEPENDENCIES", True),
            self.define_from_variant("MUSICA_ENABLE_JAVASCRIPT", "javascript"),
            self.define_from_variant("MUSICA_ENABLE_JULIA", "julia"),
            self.define_from_variant("MUSICA_BUILD_SHARED_LIBS", "shared"),
            self.define_from_variant("MUSICA_USE_FMT", "fmt"),
        ]
        return args

    @run_after("install")
    @run_after("install", when="@0.17.0:")
    def setup_standalone_test(self):
        """Keep the consumer project around for `spack test run musica`."""
        # The chapman config comes along so the Fortran test can parse a real
        # mechanism; the cache keeps these relative paths, which is what the
        # consumer project's default TEST_CONFIG assumes.
        if self.spec.satisfies("@develop") or self.spec.satisfies("@0.17:"):
            cache_extra_test_sources(self, ["src/test/spack", "configs/v1/chapman"])
 
    def test_installation(self):
        """build and run a consumer project against the installed prefix"""
        if not self.spec.satisfies("@0.17.0:"):
            raise SkipTest("src/test/spack is not in this release")
        source_dir = join_path(self.test_suite.current_test_cache_dir, "src", "test", "spack")
        build_dir = join_path(source_dir, "build")
        config = join_path(
            self.test_suite.current_test_cache_dir, "configs", "v1", "chapman", "config.json"
        )

        # musicaConfig.cmake resolves its own dependencies when a consumer calls
        # find_package: pkg_check_modules for netcdf, find_dependency for fmt.
        # A Spack *build* environment puts dependencies on CMAKE_PREFIX_PATH and
        # PKG_CONFIG_PATH; a stand-alone test runs without one. Walk the whole
        # spec rather than naming packages, so this does not need revisiting
        # every time musicaConfig.cmake gains a dependency.
        prefixes = [str(self.prefix)]
        pc_dirs = []
        for dep in self.spec.traverse(root=False):
            if str(dep.prefix) not in prefixes:
                prefixes.append(str(dep.prefix))
            for libdir in (dep.prefix.lib, dep.prefix.lib64):
                pc_dir = join_path(libdir, "pkgconfig")
                if os.path.isdir(pc_dir) and pc_dir not in pc_dirs:
                    pc_dirs.append(pc_dir)
        if pc_dirs:
            existing = os.environ.get("PKG_CONFIG_PATH", "")
            if existing:
                pc_dirs.append(existing)
            os.environ["PKG_CONFIG_PATH"] = ":".join(pc_dirs)

        cmake = self.spec["cmake"].command
        ctest = Executable(self.spec["cmake"].prefix.bin.ctest)

        args = [
            "-S",
            source_dir,
            "-B",
            build_dir,
            # ";" is the CMake list separator; this is one argv entry, not shell
            "-DCMAKE_PREFIX_PATH={0}".format(";".join(prefixes)),
            "-DTEST_FORTRAN={0}".format("ON" if self.spec.satisfies("+fortran") else "OFF"),
            # musica_micm/musica_state only exist in the install when MICM is built
            "-DTEST_MICM={0}".format("ON" if self.spec.satisfies("+micm") else "OFF"),
            "-DTEST_CONFIG={0}".format(config),
        ]

        with test_part(self, "test_installation_cmake", purpose="configure consumer project"):
            cmake(*args)

        with test_part(self, "test_installation_build", purpose="build consumer project"):
            cmake("--build", build_dir)

        with test_part(self, "test_installation_ctest", purpose="run consumer project"):
            ctest("--test-dir", build_dir, "--output-on-failure")
