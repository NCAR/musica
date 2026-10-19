"""Compile and run the Fortran example in fortran/README.md, and compare its
output with the "Output:" block that follows it.

The example is built against an installed MUSICA through pkg-config, the same
way a user without CMake builds it. Put the install on PKG_CONFIG_PATH first.
FC selects the Fortran compiler (default: gfortran).

    PKG_CONFIG_PATH=<prefix>/lib/pkgconfig pytest fortran/test/readme_example.py
"""
import math
import os
import re
import shlex
import subprocess
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]
README_FILE = REPO_ROOT / "fortran" / "README.md"
NUMBER = re.compile(r"[-+]?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?")


def extract_code(readme_content):
    """Extract the Fortran code block from the README."""
    match = re.search(r"```fortran\n([\s\S]+?)\n```", readme_content)
    assert match, "Could not find a Fortran code block in the README."
    return match.group(1)


def extract_expected_output(readme_content):
    """Extract the lines of the "Output:" block that follows the code block."""
    match = re.search(r"```fortran\n[\s\S]+?\n```\s*Output:\n```\n([\s\S]+?)```", readme_content)
    assert match, "Could not find an Output block after the Fortran example in the README."
    return match.group(1).strip().splitlines()


def parse_numbers(lines):
    """Return the numbers on each line, so that formatting differences do not matter."""
    return [[float(n) for n in NUMBER.findall(line)] for line in lines]


def pkg_config(*args):
    result = subprocess.run(["pkg-config", *args, "musica-fortran"],
                            capture_output=True, text=True, check=True)
    return shlex.split(result.stdout)


@pytest.fixture
def readme_content():
    return README_FILE.read_text()


def test_readme_example(tmp_path, readme_content):
    code = extract_code(readme_content)
    expected = parse_numbers(extract_expected_output(readme_content))

    source = tmp_path / "readme_example.F90"
    executable = tmp_path / "readme_example"
    source.write_text(code)

    compiler = os.environ.get("FC", "gfortran")
    compile_command = [compiler, *pkg_config("--cflags"), str(source),
                       "-o", str(executable), *pkg_config("--libs", "--static")]
    print(" ".join(compile_command))
    subprocess.run(compile_command, check=True)

    # The example uses a config path that is relative to the repository root
    result = subprocess.run([str(executable)], cwd=REPO_ROOT,
                            capture_output=True, text=True)
    print(result.stdout)
    print(result.stderr)
    assert result.returncode == 0, "The README example did not run successfully."

    actual = parse_numbers(result.stdout.strip().splitlines())
    assert len(actual) == len(expected), "The output has a different number of lines."
    for line, (a, e) in enumerate(zip(actual, expected), start=1):
        assert len(a) == len(e), f"Line {line} has a different number of values."
        for a_value, e_value in zip(a, e):
            assert math.isclose(a_value, e_value, rel_tol=1e-3, abs_tol=1e-30), \
                f"Line {line}: got {a_value}, expected {e_value}"
