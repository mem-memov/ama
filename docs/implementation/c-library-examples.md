# C hosts and native-owned compiled children

These executable examples implement [EMBED-01–EMBED-08](../../spec/implementation/c-library.md).
They use the product CLI, generated public headers and ordinary independent C
translation units. They do not link a test-only compiler entry point.

## Build and run

From the repository root, with GCC 15.2.0 and GNU binutils installed:

```sh
make
python3 tools/examples/c_library/run.py --out /tmp/ama-c-examples
```

The output directory must not exist. Paths containing spaces are supported;
control characters, quotes and backslashes in include paths are rejected. Nothing
in the source tree is rewritten. Choose another fresh output directory to rebuild.
The command builds and runs these executables:

| Executable | Components | Expected observation |
| --- | --- | --- |
| `standalone-static` | Child archive, explicit C probe object | Two yields; retained text survives context disposal |
| `standalone-shared` | Child shared library | Same behavior through shared linkage |
| `nested-static` | Parent archive, child archive, C probe object | Explicit nested suspension, supervised failure and reset |
| `nested-shared` | Parent shared library containing its child dependency | Same behavior through shared linkage |

The maintained hosts are [standalone.c](../../examples/c-library/host/standalone.c),
[nested.c](../../examples/c-library/host/nested.c) and a separately compiled
[observer.c](../../examples/c-library/host/observer.c). The native parent, child
and explicit C probe reuse the maintained
[supervision fixtures](../../tests/unit/library/supervision/main.c), so the
executable example and failure-injection tests exercise the same implementation.
The source checkout is needed to build the example; consumers of a built component
need its public header and explicit link dependencies, not the Ama compiler SDK.

`child/` and `parent/` each contain `public.h`, `component.json`, `symbols.txt`,
`library.a`, `library.so`, the generated assembly and native declarations.
`child.exports` and `parent.exports` record the complete typed interface and
ABI-affecting flags/dependencies. `aliases/` contains C conveniences generated from
metadata, not a second ABI declaration. Native authoring bootstrap outputs are
under `sdk-worker/`, `sdk-probe/` and `sdk-adapter/`.

`commands.json` records **every executed command**, in order, as exact argument
arrays with their working directory. It includes the native bootstrap, both
`--library-build` requests, each C compilation, each final link and each run.
It is the exact build/link recipe for the selected absolute output path; arguments
are passed directly without shell expansion. Do not replay the whole record over
existing owned output. Use a fresh builder run when changing paths or flags:
those explicit configuration strings participate in component identity.

For example, the final static links have this ordering (replace `$out` with your
chosen absolute directory):

```sh
gcc -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic-errors \
    "$out/standalone.o" "$out/observer.o" \
    "$out/child/library.a" "$out/probe.o" -o "$out/standalone-static"
gcc -std=c11 -O1 -g -Wall -Wextra -Werror -pedantic-errors \
    "$out/nested.o" "$out/observer.o" "$out/parent/library.a" \
    "$out/child/library.a" "$out/probe.o" -o "$out/nested-static"
```

Shared links replace the archive/dependency sequence with the corresponding
absolute `library.so` path. The child shared library contains the probe object;
the parent shared library contains its explicitly linked child archive and probe.
The host observer calls the compatible C probe symbol supplied by that dependency.
A static archive does not absorb its external archives/objects. The builder records
those dependencies in configuration and supplies them again at the final static
host link. It compiles all host C files without `-I src` or generated native SDK
include paths.

## Native authoring without guessing generated names

Both `--native` and project/library generation publish `native-model.json` with
format `ama-native-model-1`. `modules` contains one record per required native
module, in deterministic scaffold order. Each record contains:

- `module`: exact logical Ama path;
- `namespace`, `parameters`, `results`: objects with the registered C `name` and
  complete framed identity encoded as lowercase `key_hex`;
- `interface`, `begin`, `end`: exact generated SDK header basenames.

This is tooling metadata, not a wire structure or new C ABI. Native source still
uses generated declarations. The example first invokes `--native` without
`--create` to obtain names without compiling authored C, then builds the real
components. It compares every bootstrap record with the final model, including
full identity keys. It never reconstructs hashes, searches headers or picks a
name by prefix. A missing, duplicate or changed binding is an explicit error.
The published `symbols.txt` is the exact public implementation symbol inventory
already used by the packaging verifier; it is not an authorization to bypass the
public header's version/key/layout checks.

## What the nested boundary means

An ordinary Ama call stays in its context and dispatches to labels. A compiled
child is different: the native parent exclusively owns a foreign C context/root
resource and calls its checked public API. Values cross as owned typed snapshots;
no parent Ama field reference is hidden in that resource.

The child returns `YIELDED` normally through every C helper before the native
entry returns. Only then does outer Ama execute its own `%`. A child continuation
and a parent continuation are independent. The example's C observer verifies
that no helper activation remains when control returns to the host.

The failure run makes the child fail during execution and also report a secondary
cleanup failure. The primary description is copied into the parent's ordinary
failure result before retiring the child context. A subsequent explicit reset
creates a fresh child. The report remains readable after reset and parent context
disposal, and keeps its originating component's code registry busy until released.
The example observer requires acquisition/retirement counts to balance.

## Target, loaders and sanitizers

The accepted profile is GNU x86-64 Linux ELF LP64 System V, using the currently
supported GCC 15.2.0 backend and GNU `ar`, `objcopy`, `nm`. This example selects
`x86_64-linux-gnu`; the header also checks the supported
`x86_64-pc-linux-gnu` spelling. Other architectures, Windows, macOS and foreign
calling conventions are not claimed by these examples.

The shared examples use normal link-time shared-library loading. The absolute
shared path recorded by the linker is intentional for the fresh output directory;
relocatable installation, SONAME/RPATH policy and a portable plugin loader are not
provided by this example. Real `dlopen`/`dlclose`, stale-reload rejection and
resolved public function calls are separately tested by the
[package suite](../../tests/unit/library/package/main.c).

Before unloading an image, the host must stop new calls using its own loader
barrier, release all snapshots/diagnostics/code pins and query **every component
registry in that image**. A parent shared object here also contains the child
component. `code_can_unload` is an ownership check; it cannot protect against a
host calling `dlclose` while another thread is entering code or holding an
untracked raw function pointer. See [code lifetime](c-library-packaging.md).

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
python3 tools/examples/c_library/run.py --out /tmp/ama-c-examples-sanitized --sanitize
```

This instruments the generated libraries, native C dependency and independent
hosts with ASan/UBSan. LeakSanitizer is unavailable in the current environment;
explicit native-resource counters and ownership/unload assertions still run.
The runtime retains one tiny nonrecycled load receipt per physical load to prevent
stale-token resurrection after unloading; its process-lifetime history is
[documented separately](c-library-packaging.md).

## Conformance evidence

Each linked document explains the independent component checks and composed
cases rather than treating this one example as sufficient proof.

| Rule | Current evidence |
| --- | --- |
| EMBED-01: explicit export contract | [Export checking](c-library-exports.md), `tests/unit/library/configuration` and `tests/unit/library/resolution`; CLI malformed/export-map tests |
| EMBED-02: factories and lifecycle | [Lifecycle](c-library-lifecycle.md), `tests/unit/library/lifecycle`; standalone factory/read/disposal |
| EMBED-03: entry, resume and retained reads | [Execution](c-library-execution.md), `tests/unit/library/execution`; two explicit yields in all four hosts |
| EMBED-04: typed C transport and ownership | [Transport](c-library-transport.md), `tests/unit/library/transport`; retained text/report outlive contexts |
| EMBED-05: diagnostics and failure boundaries | [Execution](c-library-execution.md), [supervision](c-library-supervision.md); primary versus secondary failure, allocation-failure injection |
| EMBED-06: ABI, independent linking and code lifetime | [Header](c-library-header.md), [packaging](c-library-packaging.md), `tests/unit/library/package`; no-SDK multi-file static/shared consumers, actual reload tests |
| EMBED-07: native-owned isolated child | [Nested ownership](c-library-nested.md), `tests/unit/library/nested`; partial initialization, reverse disposal and opaque foreign ownership |
| EMBED-08: nested suspension and supervision | [Supervision](c-library-supervision.md), `tests/unit/library/supervision`; reentry rejection, abandoned continuations, explicit reset and copied failure |

The real command `make -C tests/integration/c-library-examples` runs all four
hosts, validates exact metadata and recorded commands, rejects malformed SDK maps,
and checks that existing outputs and failed tools are handled explicitly. The
native declaration leaf independently compiles an interface using the map's exact
parameter/result names. Per-file test and header evidence is registered in
[the documentation inventory](documentation/ready.json).
