# Reproducing the current implementation

Start with [the specification](../../../spec/README.md) for language semantics,
[the library guide](../../../stdlib/README.md) for public module interfaces,
and [the matrix guide](README.md) for requirement evidence. This report describes
verified implementation workflows; it does not introduce language requirements.

## What has been checked

The reviewed matrix connects all **200 requirements** and **108 acceptance rows**
through **41 evidence groups**. Each obligation has its own argument and links
to actual component boundaries, contracts, named component cases and distinct
composition cases. The live registry covers 2335 maintained inputs, 1246 named C
cases in 194 unit and 26 integration case translation units, with 240 distinct
command strings. These are inventory counts, not statement-coverage percentages.

[Toolchain acceptance](../toolchain-acceptance.md) records the actual GCC 15.2.0,
x86_64 Linux, C11/LP64 profile, strict release checks, native-entry probes and
sanitizer results. [Product reproduction](../product-reproducibility.md) checks
clean and reused outputs, transitive changes and diagnostic source locations.
[Resource acceptance](../resource-acceptance.md) records allocation/I/O failure,
interruption and recovery observations rather than treating an aborted process
as a managed failure.

The T121 example check used a fresh detached checkout at implementation commit
94b1c0c. It rebuilt the compiler with the strict release flags below, executed
every command in the following table and regenerated Doxygen twice with identical
bytes. Subsequent T121 changes are this report and navigation prose only.
Generated outputs remain outside maintained source inventory.

## Run the examples

Run from the repository root. Each example script requires a fresh output path;
choose another path for another run. No prebuilt compiler or generated C files
need to be copied into the checkout.

```sh
make C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG'
./ama --emit examples/async .main --out /tmp/ama-async-example --sdk src --standard-root stdlib/modules --compile
/tmp/ama-async-example/program
python3 tools/examples/c_library/run.py --out /tmp/ama-c-example
python3 tools/examples/async_host/run.py --out /tmp/ama-async-host-example
make -C tests/integration/examples
make -C tests/integration/objects
make -C tests/unit/codegen/parallel-supervision
```

| Command family | Observable result and contract |
|---|---|
| Async program | Prints completed, then cancelled; [explicit polling and resumption](../../../examples/async/README.md). |
| C-library script | Builds and runs standalone static/shared consumers and native-parent static/shared compositions; retained text, independent suspension, supervised child failure and explicit reset are checked. [Commands and API ownership](../c-library-examples.md). |
| Async host script | Reports asynchronous component lifetime accepted after delayed/duplicate completion and unload-lifetime checks; [host example](../../../examples/async-host/README.md). |
| Integration examples | Checks state, self-reference, live list mutation, ordinary versus continuation copying and owned file-probe failure; [scenario commands](../examples.md). |
| Object examples | Checks constructor-owned methods, independent state and resource cleanup against the expected trace; [stateful example](../examples.md). |
| Parallel supervision | Checks isolated overlapping execution, peer continuation/cancellation and cleanup; [parallel example](../../../examples/parallel/README.md). |

The scripts intentionally exercise child failure and invalid service construction;
their expected runtime diagnostics are compatible with a successful harness exit.
The parallel harness likewise deliberately reports receiver failures. Require
each command's successful exit and the harness's assertions rather than classifying
acceptance from a search for the word error.

## Choose an embedding boundary

For an ordinary C application, use the generated
[public header](../c-library-header.md), explicit
[context lifecycle](../c-library-lifecycle.md),
[typed transport](../c-library-transport.md), and
[entry/resume outcomes](../c-library-execution.md). The packaged static/shared
[component artifacts](../c-library-packaging.md) isolate symbols and track code
lifetime. Consumers in the example compile without compiler-private SDK headers.

For a native Ama module owning a compiled Ama child, use
[exclusive child-context ownership](../c-library-nested.md) and
[child suspension/supervision](../c-library-supervision.md). A child yield returns
normally to its native parent; it does not unwind a C stack into an outer Ama
continuation. The parallel supervision example exercises a generated C bridge,
not an SDK-free prebuilt-library consumer.

Native Ama companions follow the [fixed value ABI](../../../VALUE_ABI.md),
[scaffolding workflow](../native-scaffolding.md) and
[verified toolchain boundary](../native-toolchain.md). Check-only success cannot
validate arbitrary handwritten native C or replace compile/object verification.

## Tooling and practical limits

[Source commands](../source-tool-commands.md),
[debugger client](../debugger-client.md), and
[relocation commands](../relocation-commands.md) document the implemented
protocols and invocation boundaries. They do not imply an installed IDE plugin.
Paths, proposal transactions and relocation recovery have their own executable
evidence in the matrix.

Asynchronous execution is cooperative. Hosts explicitly poll/resume and retain
native completion endpoints; callbacks cannot retain borrowed Ama state or run
Ama. Parallel contexts have separate mutable instance storage and exchange
explicit messages. Context isolation is not an operating-system security sandbox.

The supported generated backend is the tested GCC/target profile, not a universal
binary ABI or a claim about other platforms. The file example is a small owned
synchronous fopen/close probe, not a complete standard filesystem library.
ASan/UBSan and counted-ownership checks pass in their recorded scopes. LeakSanitizer
is unavailable under this environment's ptrace restriction. The installed TSan
crashes on an isolated C11-thread probe; the checked concurrency harness uses
POSIX launch/join, with positive and intentional-race controls. See the
[exact instrumentation evidence](../toolchain-acceptance.md) before generalizing.

## Rebuild the evidence and reference

```sh
make test
make conformance-check
make docs-complete
make docs-reproducible
```

The conformance gate checks exact obligations, hashes and compiled C interfaces;
it does not execute every referenced behavioral test. Run the test commands too.
Doxygen generates the reference under docs/technical, including per-file pages,
interfaces and resolved canonical-rule links. The
[documentation guide](../../documentation.md) explains coverage manifests,
fingerprints, tools and reproducible generation. Generated reference pages are
local build outputs; this checkpoint does not publish a release externally.
