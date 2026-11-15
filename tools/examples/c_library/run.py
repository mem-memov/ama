# @file
# @brief Run explicit reproducible C-library examples in a fresh caller-selected directory.
# Contract: ../../../docs/implementation/c-library-examples.md.
"""No source files or existing directories are overwritten by the example builder."""

import argparse
import subprocess
from pathlib import Path

from build import Build


def main():
    """Build through the product CLI and run four independently linked consumer executables."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default=str(Path(__file__).resolve().parents[3] / "ama"))
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--out", required=True)
    parser.add_argument("--sanitize", action="store_true")
    options = parser.parse_args()
    try:
        build = Build(options.compiler, options.cc, options.out, options.sanitize)
        build.libraries()
        build.hosts()
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"C-library example failed: {error}\n")


if __name__ == "__main__":
    main()
