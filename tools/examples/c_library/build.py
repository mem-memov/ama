# @file
# @brief Build standalone and nested C-library examples through the actual product CLI.
# Contract: ../../../docs/implementation/c-library-examples.md.
"""Build orchestration with exact argv, explicit inputs, and recorded commands."""

import json
import subprocess
from pathlib import Path

import model


class Build:
    """Own one fresh example directory and a replayable ordered tool-command record."""

    def __init__(self, compiler, cc, output, sanitize):
        """Reserve a fresh output tree; reject unrepresentable paths before emitting includes."""
        self.root = Path(__file__).resolve().parents[3]
        self.compiler = str(Path(compiler).resolve())
        self.cc = cc
        self.output = Path(output).resolve()
        for path in (self.root, self.output):
            if any(ord(char) < 32 or char in '"\\' for char in str(path)):
                raise ValueError("Example paths must be representable as native C include paths")
        self.output.mkdir(parents=True, exist_ok=False)
        self.aliases = self.output / "aliases"
        self.aliases.mkdir()
        self.sanitize = sanitize
        self.commands = []
        self.sources = self.root / "tests/unit/library/supervision"

    def run(self, arguments, capture=False):
        """Execute exact arguments without a shell; record success/failure for reproduction."""
        arguments = [str(value) for value in arguments]
        self.commands.append({"directory": str(self.root), "arguments": arguments})
        (self.output / "commands.json").write_text(json.dumps(self.commands, indent=2) + "\n", encoding="utf-8")
        result = subprocess.run(arguments, cwd=self.root, check=True, text=True,
                                stdout=subprocess.PIPE if capture else None)
        return result.stdout

    def author(self, root, module, directory):
        """Bootstrap authoring names without compiling or executing a native body."""
        output = self.output / directory
        self.run([self.compiler, "--native", root, module, "--out", output,
                  "--sdk", self.root / "src", "--standard-root", self.root / "stdlib/modules"])
        return model.native(output / "native-model.json")

    def flags(self):
        """Use the supported C11 consumer profile and instrument generated code on request."""
        flags = ["-std=c11", "-O1", "-g", "-Wall", "-Wextra", "-Werror", "-pedantic-errors"]
        if self.sanitize:
            flags.extend(["-fsanitize=address,undefined", "-fno-omit-frame-pointer"])
        return flags

    def component(self, name, root, records):
        """Record every ABI-affecting flag/dependency before invoking verified packaging."""
        settings = [f"c-flag | -I{self.aliases}"]
        if self.sanitize:
            settings.extend(["c-flag | -fsanitize=address,undefined",
                             "c-flag | -fno-omit-frame-pointer", "link-flag | -fsanitize=address,undefined"])
        configuration = self.output / f"{name}.exports"
        configuration.write_text("\n".join([
            "ama-library | 1", f"component | example.{name}", "abi | ama-c-1",
            "target | x86_64-linux-gnu", "calling-convention | c", *records, *settings, ""
        ]), encoding="utf-8")
        output = self.output / name
        document = json.loads(self.run([self.compiler, "--library-build", root, configuration,
                                       "--out", output, "--sdk", self.root / "src",
                                       "--standard-root", self.root / "stdlib/modules", "--cc", self.cc], capture=True))
        return document

    def libraries(self):
        """Build the child first, then a parent whose only foreign graph access is C transport."""
        child_root = self.sources / "child/project"
        worker = self.author(child_root, "/test/fixture/v1.0.0/src/worker", "sdk-worker")
        probe = self.author(child_root, "/test/fixture/v1.0.0/src/probe", "sdk-probe")
        parent = self.author(self.sources / "project", "/test/fixture/v1.0.0/src/adapter", "sdk-adapter")
        model.authoring(worker["/test/fixture/v1.0.0/src/worker"], "AMA_WORKER", self.aliases / "worker-api.h",
                        [("AMA_WORKER_TEXT", worker["//text"])])
        model.authoring(probe["/test/fixture/v1.0.0/src/probe"], "AMA_PROBE", self.aliases / "probe-api.h")
        model.authoring(parent["/test/fixture/v1.0.0/src/adapter"], "AMA_EMBED", self.aliases / "adapter-api.h",
                        [("AMA_EMBED_TEXT", parent["//text"]), ("AMA_EMBED_FAILURE", parent["//failure"])])
        probe_object = self.output / "probe.o"
        self.run([self.cc, *self.flags(), "-fPIC", "-c", self.sources / "probe.c", "-o", probe_object])
        child = self.component("child", child_root, [
            "schema | text | text | //text", "schema | condition | condition | //std/condition",
            "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read", "input | run | request | text",
            "output | run | response | text", "output | run | previous | condition",
            f"link-file | {probe_object}", f"dependency-file | {self.aliases / 'worker-api.h'}",
            f"dependency-file | {self.aliases / 'probe-api.h'}", f"dependency-file | {self.sources / 'probe.h'}"])
        actual = model.native(self.output / "child/native-model.json")
        model.same_bindings(worker, actual)
        model.same_bindings(probe, actual)
        model.public(child, "CHILD", str(self.output / "child/public.h"), self.aliases / "child-api.h")
        with (self.aliases / "adapter-api.h").open("a", encoding="utf-8") as stream:
            stream.write('#include "child-api.h"\n')
        parent_document = self.component("parent", self.sources / "project", [
            "schema | text | text | //text", "schema | condition | condition | //std/condition",
            "schema | failure | failure | //failure", "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read",
            "input | run | message | text", "input | run | reset | condition", "input | run | propagate | condition",
            "output | run | response | text", "output | run | previous | condition",
            "output | run | waiting | condition", "output | run | failure | failure",
            f"link-file | {self.output / 'child/library.a'}", f"link-file | {probe_object}",
            f"dependency-file | {self.aliases / 'adapter-api.h'}", f"dependency-file | {self.aliases / 'child-api.h'}"])
        model.same_bindings(parent, model.native(self.output / "parent/native-model.json"))
        model.public(parent_document, "PARENT", str(self.output / "parent/public.h"), self.aliases / "parent-api.h")

    def hosts(self):
        """Compile each independent C source without SDK includes, then link static/shared variants."""
        sources = self.root / "examples/c-library/host"
        for source in ("observer.c", "standalone.c", "nested.c"):
            self.run([self.cc, *self.flags(), "-I", self.aliases, "-I", self.sources,
                      "-c", sources / source, "-o", self.output / source.replace(".c", ".o")])
        for scenario in ("standalone", "nested"):
            for linkage in ("static", "shared"):
                arguments = [self.cc, *self.flags()]
                if self.sanitize:
                    arguments.append("-no-pie")
                arguments.extend([self.output / f"{scenario}.o", self.output / "observer.o"])
                if linkage == "static":
                    if scenario == "nested":
                        arguments.append(self.output / "parent/library.a")
                    arguments.extend([self.output / "child/library.a", self.output / "probe.o"])
                else:
                    arguments.append(self.output / ("parent/library.so" if scenario == "nested" else "child/library.so"))
                program = self.output / f"{scenario}-{linkage}"
                arguments.extend(["-o", program])
                self.run(arguments)
                self.run([program])
