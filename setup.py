from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "micm",
        ["src/micm/wrapper.cpp"],
        include_dirs=["include", "lib/micm/include"],
        language="c++",
        extra_compile_args=["-std=c++20"],
    ),
]

setup(
    name="musica",
    version="0.1.0",
    author="NCAR ACOM",
    author_email="musica@ucar.edu",
    description="Chemistry interface with python",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
