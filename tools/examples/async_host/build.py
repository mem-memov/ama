# @file
# @brief Build the independently resident backend and actual Ama component with replayable argument vectors.
# Contract: ../../../docs/implementation/asynchronous-host.md.
"""All native dependency and compiler inputs are explicit and remain reviewable."""

import json
import subprocess
from pathlib import Path

import model


class Build:
    """Own one fresh example tree; record commands before invoking each actual tool."""

    def __init__(self, compiler, cc, output, sanitize):
        """Reserve a fresh destination and reject paths the source/configuration forms cannot represent."""
        self.root = Path(__file__).resolve().parents[3]
        self.compiler = str(Path(compiler).resolve())
        self.cc = cc
        self.output = Path(output).resolve()
        for path in (self.root, self.output):
            if any(ord(char) < 32 or char in '"\\|' for char in str(path)):
                raise ValueError("Example paths must be representable in native includes and configuration records")
        self.output.mkdir(parents=True, exist_ok=False)
        self.generated = self.output / "generated"
        self.generated.mkdir()
        self.sources = self.root / "examples/async-host"
        self.sanitize = sanitize
        self.commands = []

    def run(self, arguments, capture=False):
        """Execute exact arguments without shell evaluation and preserve a reproduction record."""
        arguments = [str(value) for value in arguments]
        self.commands.append({"directory": str(self.output), "arguments": arguments})
        (self.output / "commands.json").write_text(json.dumps(self.commands, indent=2) + "\n", encoding="utf-8")
        result = subprocess.run(arguments, cwd=self.output, check=True, text=True,
                                stdout=subprocess.PIPE if capture else None)
        return result.stdout

    def flags(self):
        """Keep the supported C11 host profile and optionally instrument all participating images."""
        result = ["-std=c11", "-D_POSIX_C_SOURCE=200809L", "-O1", "-g", "-UNDEBUG",
                  "-Wall", "-Wextra", "-Werror", "-pedantic-errors"]
        if self.sanitize:
            result.extend(["-fsanitize=address,undefined", "-fno-omit-frame-pointer"])
        return result

    def backend(self):
        """Compile an independent native image that may retain callbacks after the Ama context closes."""
        self.run([self.cc, *self.flags(), "-fPIC", "-shared", "-pthread", "-I", self.root / "src",
                  self.sources / "backend/backend.c", "-o", self.generated / "backend.so"])

    def component(self):
        """Publish through the product CLI, with the backend declared as an exact native dependency."""
        records = ["ama-library | 1", "component | example.asynchronous.host", "abi | ama-c-1",
                   "target | x86_64-linux-gnu", "calling-convention | c",
                   "schema | text | text | ..text", "schema | flag | condition | ..std.condition",
                   "export | run | .main | entry,resume", "input | run | immediate | flag",
                   "input | run | cancel requested | flag", "output | run | message | text",
                   "export | invalid | .invalid | entry", f"c-flag | -I{self.sources / 'backend'}",
                   "c-flag | -DNDEBUG", f"link-file | {self.generated / 'backend.so'}"]
        if self.sanitize:
            records.extend(["c-flag | -fsanitize=address,undefined", "c-flag | -fno-omit-frame-pointer",
                            "link-flag | -fsanitize=address,undefined"])
        configuration = self.output / "async.exports"
        configuration.write_text("\n".join(records) + "\n", encoding="utf-8")
        document = self.run([self.compiler, "--library-build", self.sources / "project", configuration,
                             "--out", self.generated / "component", "--sdk", self.root / "src",
                             "--standard-root", self.root / "stdlib/modules", "--cc", self.cc], capture=True)
        return json.loads(document)

    def consumer(self, document):
        """Compile without SDK includes, then exercise completion, cancellation, restart and real unload."""
        source = self.generated / "client.c"
        source.write_text(model.consumer(document, self.generated / "component", self.sources), encoding="utf-8")
        flags = self.flags()
        if self.sanitize:
            flags.extend(["-fno-pie", "-no-pie"])
        self.run([self.cc, *flags, "-I", self.sources / "backend", source,
                  self.generated / "backend.so", "-pthread", "-ldl", "-o", self.output / "consumer"])
        self.run([self.output / "consumer"])
