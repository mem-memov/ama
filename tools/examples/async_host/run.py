# @file
# @brief Build and run a fresh native-worker/shared-component example through the product CLI.
# Contract: ../../../docs/implementation/asynchronous-host.md.
"""Verify native completion and unload lifetimes without overwriting an existing directory."""

import argparse
import subprocess
from pathlib import Path

from build import Build


def main():
    """Run explicit supported compiler/linker steps and report an actionable failure."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default=str(Path(__file__).resolve().parents[3] / "ama"))
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--out", required=True)
    parser.add_argument("--sanitize", action="store_true")
    options = parser.parse_args()
    try:
        build = Build(options.compiler, options.cc, options.out, options.sanitize)
        build.backend()
        document = build.component()
        build.consumer(document)
    except (OSError, ValueError, KeyError, TypeError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Asynchronous host example failed: {error}\n")


if __name__ == "__main__":
    main()
