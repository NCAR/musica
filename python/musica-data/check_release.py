"""
Check a new musica-data wheel against the musica package and PyPI.

1. The musica-data lower bound in the musica pyproject.toml must be the version
   in python/musica-data/pyproject.toml, so that musica requires the data that
   it was tested with.
2. If PyPI already has this version, its data files must be the same as the
   files in the new wheel. If they are not, configs/ changed, but the version
   did not, and the release would not upload the new data.

The script writes publish=true when PyPI does not have this version, and
publish=false when PyPI has it with the same files. It writes to $GITHUB_OUTPUT
when that variable is set.

Usage: python check_release.py <musica-data wheel>
"""

import hashlib
import io
import json
import os
import re
import sys
import tomllib
import urllib.error
import urllib.request
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
PYPI_URL = "https://pypi.org/pypi/musica-data/{version}/json"


def read_project(path):
    with open(path, "rb") as f:
        return tomllib.load(f)["project"]


def musica_lower_bound():
    for dependency in read_project(os.path.join(ROOT, "pyproject.toml"))["dependencies"]:
        match = re.fullmatch(r"musica-data\s*>=\s*([0-9.]+)", dependency.strip())
        if match:
            return match.group(1)
    sys.exit("The musica pyproject.toml has no 'musica-data>=<version>' dependency")


def data_file_hashes(wheel_bytes):
    """Return the SHA256 of each file in the wheel, without the dist-info files."""
    with zipfile.ZipFile(io.BytesIO(wheel_bytes)) as wheel:
        return {
            name: hashlib.sha256(wheel.read(name)).hexdigest()
            for name in wheel.namelist()
            if ".dist-info/" not in name and not name.endswith("/")
        }


def pypi_wheel(version):
    """Return the bytes of the wheel of this version on PyPI, or None if PyPI does not have it."""
    try:
        with urllib.request.urlopen(PYPI_URL.format(version=version)) as response:
            release = json.load(response)
    except urllib.error.HTTPError as error:
        if error.code == 404:
            return None
        raise
    wheels = [url["url"] for url in release["urls"] if url["filename"].endswith(".whl")]
    if not wheels:
        return None
    with urllib.request.urlopen(wheels[0]) as response:
        return response.read()


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    version = read_project(os.path.join(HERE, "pyproject.toml"))["version"]

    lower_bound = musica_lower_bound()
    if lower_bound != version:
        sys.exit(
            f"musica requires musica-data>={lower_bound}, but python/musica-data/pyproject.toml "
            f"has version {version}. Change the lower bound in the musica pyproject.toml to {version}."
        )

    with open(sys.argv[1], "rb") as f:
        new_files = data_file_hashes(f.read())

    published = pypi_wheel(version)
    if published is None:
        print(f"PyPI does not have musica-data {version}, so the release uploads it")
        publish = "true"
    else:
        old_files = data_file_hashes(published)
        if old_files != new_files:
            changed = sorted(name for name in old_files.keys() | new_files.keys()
                             if old_files.get(name) != new_files.get(name))
            sys.exit(
                f"PyPI has musica-data {version}, but with different files. configs/ changed, so change "
                f"the version in python/musica-data/pyproject.toml and the lower bound in the musica "
                f"pyproject.toml. Changed files:\n  " + "\n  ".join(changed)
            )
        print(f"PyPI has musica-data {version} with the same files, so the release does not upload it")
        publish = "false"

    print(f"publish={publish}")
    if "GITHUB_OUTPUT" in os.environ:
        with open(os.environ["GITHUB_OUTPUT"], "a") as f:
            f.write(f"publish={publish}\n")


if __name__ == "__main__":
    main()
