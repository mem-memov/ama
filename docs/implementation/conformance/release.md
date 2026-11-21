# Reproducing the current implementation

Read [the specification](../../../spec/README.md) for semantics,
[the library guide](../../../stdlib/README.md) for native/ordinary interfaces,
and [the evidence matrix](README.md) for requirement coverage. This guide records
current accepted workflows; it does not define another language contract.

The T197 results below precede the T220
[static-marker rule](../../../spec/language/static-results.md#tpl-14).
Marker implementation is accepted in [T221](../../../DONE.md#t221); the later T225
[declaration-expression](../../../spec/language/templates.md#tpl-15) contract,
retained by [T227](../../../DONE.md#t227), awaits [T226](../../../DONE.md#t226).
The [generation/testing policy](../../../spec/introduction.md#core-06) states
the target design boundary without certifying new implementation behavior.
The complete gate refuses the earlier matrix against the newer target until the required alignment is accepted. These
recorded release results do not establish support for the new contracts.
The later [project/package contracts](../../../spec/project/packages.md) and
[independent project tools](../../../spec/implementation/project-tools.md) also
require the separately planned T231–T237/T224 implementation. Their slash paths,
single-dot static names and ama-project commands are not asserted to work in
the historical command examples below.
The later [CMake/application-input contract](../../../spec/implementation/application-builds.md)
and [source-inclusion mode](../../../spec/implementation/c-library.md#embed-09)
likewise await T239–T242 implementation and new acceptance evidence.

## Clean template sprint acceptance

[T197's execution record](template-release.md) verifies the current implementation
after [T196's conformance review](template-review.md). A fresh `make clean`, strict
release compiler build and one complete `make -k -j4 test` all pass. The full test
run took 3014.517 seconds and preserved all 3054 captured source/specification/
registry/matrix inputs. Assertions remain enabled by the test leaf's `-UNDEBUG`.
No partial successful branch substitutes for that complete parent exit.

The accepted T196 matrix connects **214 requirements**, **150 acceptance rows** and
**56 evidence groups**. The complete per-file registry contains **3028** inputs,
**1860** distinct registered C cases across **336** case-source files, and
**359** command strings. The full run executes 336 outer C test commands.
These are inventory/evidence counts, not statement-coverage percentages.

The run includes static graph parsing and solving, finite full-key creation
checks, ordinary specialization admission and storage, native O0/O2/toolchain
checks, specialized constants, compiled source/native consumers, independent
contexts, source/editor/debugger tools, relocation recovery and explicit
cooperative/parallel execution. Expected failing interface/native/resource
fixtures are checked by their enclosing assertions; diagnostic words alone do
not indicate a failing test run.

The supported observed configuration is GCC 15.2.0, C11/LP64 on x86_64 Linux,
GNU Make 4.4.1 and Python 3.14.4. [Toolchain evidence](../toolchain-acceptance.md)
and [resource evidence](../resource-acceptance.md) describe the actual boundaries.
T197 also runs current public C-library and async-host examples under ASan/UBSan.
LeakSanitizer and ThreadSanitizer acceptance are not claimed; owned resource,
callback, acquisition and code-pin counters provide separate teardown evidence.

## Build and run

Use fresh output directories. From the repository root:

```sh
make clean
make -j4 C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG'
make -k -j4 test C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG'
./ama --emit examples/async .main --out /tmp/ama-async-example --sdk src --standard-root stdlib/modules --compile
/tmp/ama-async-example/program
./ama --emit examples/template-lists .main --out /tmp/ama-template-example --sdk src --standard-root stdlib/modules --compile
/tmp/ama-template-example/program
python3 tools/examples/c_library/run.py --out /tmp/ama-c-example
python3 tools/examples/async_host/run.py --out /tmp/ama-async-host-example
```

| Example | Observed behavior |
|---|---|
| [Async Ama](../../../examples/async/README.md) | Exact stdout `completed`, then `cancelled`; a late result cannot revive cancelled enrollment. |
| [Generic lists](../../../examples/template-lists/README.md) | Executes silently, retaining reversed integer/text outputs assembled through a partial constructor; tests independently observe ownership/order/aliasing. |
| [Public C libraries](../c-library-examples.md) | Static/shared standalone hosts and native-parent compositions verify owned values, independent saved state, child failure policy and explicit reset. |
| [Async native host](../../../examples/async-host/README.md) | Native worker completion, duplicates/cancellation and unload lifetime pass through the checked public boundary. |
| [Objects and language scenarios](../examples.md) | Full integration tests exercise constructor-owned methods, state, references, live list edits, copy modes and resource cleanup. |
| [Parallel supervision](../../../examples/parallel/README.md) | Full tests execute isolated contexts, explicit messages, peer continuation/cancellation and cleanup. |

For the public native examples, repeat with `--sanitize` and separate output
paths. T197 uses `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. Both normal and instrumented example scripts
return zero. Their deliberately failed child/invalid-service cases remain
expected observations inside successful harnesses.

## Types, identity and resource bounds

Static modules, partial constructors and inferred Types exist only during
compilation. Every required executable dependency becomes a concrete Type and
passes ordinary operation/default/capability/lifecycle checks before layout.
Equal full specialization keys share Type/code identity; runtime instances,
results, references, continuations and contexts retain independent state.
Static outputs add no runtime object or C result slots.

[Creation guards](../static-guards.md) accept finite written nesting and closed
same-key cycles while rejecting reachable unequal keys of one constructor origin.
They retain all incoming/outgoing provenance through cold/warm caches and late
constraints. Type equations, static definition cycles, eager ownership and
constant value dependencies are separate checks. No arbitrary semantic depth,
specialization count, cache-order choice or timeout substitutes for those laws.

[The typed storage plan](../specialized-planning.md) and actual compiled consumers
check concrete layout, stable instance slots and saved-position modes. Optional
representation/storage elimination remains subject to its proof contract;
this release does not claim every permitted optimization is implemented.

[Resource tests](../resource-acceptance.md) enumerate measured allocation/I/O
refusal points and real durable interruption states, compare exact originals
and recover repeatedly. T197 observes 529 application interruption checkpoints,
1066 application I/O checkpoints and independent pending/terminal rollback/commit
recovery partitions. Counted ownership and structured cleanup remain necessary
alongside sanitizers. A managed OOM/refusal is distinct from a process abort.

## Choose an embedding boundary

A normal C application uses the self-contained [public header](../c-library-header.md),
explicit [context lifecycle](../c-library-lifecycle.md),
[owned typed transport](../c-library-transport.md) and
[entry/resume outcomes](../c-library-execution.md). Static/shared
[packaging](../c-library-packaging.md) isolates component symbols and preserves
code lifetime. Open templates ship as source; concrete ordinary facades expose
closed generic algorithms under [T194's checked boundary](../static-library-facades.md).

A native Ama adapter may exclusively own a separately compiled child context
under the [foreign resource contract](../c-library-nested.md). A child yield
returns normally through all C helpers before any outer Ama yield. Explicit
[parent policy](../c-library-supervision.md) controls later steps and failure;
independent peers survive disposal of another parent and its owned child.
Native companions use the [value ABI](../../../VALUE_ABI.md) and verified
[authoring workflow](../native-scaffolding.md). Check-only success does not
validate arbitrary handwritten native C or foreign library thread safety.

## Scope and final documentation

Use [source commands](../source-tool-commands.md), the
[debugger client](../debugger-client.md) and checked relocation/recovery tools
against explicit roots. Relocation rewrites actual resolved Ama paths while
preserving atomic static names and all unrelated bytes; handwritten C paths and
external build configuration remain outside that rewrite contract.

Dependent numeric/runtime-driven Types, a new template-based intrinsic list
provider, unsupported target/toolchain profiles and arbitrary foreign native
behavior are not claimed. The existing list foundation and its busy/refusal,
checked reference and live-boundary contracts remain in force.

```sh
make conformance-check
make docs-complete
make docs
make docs-reproducible
```

The generated technical reference comes from maintained Doxygen comments and
validated canonical links. T198 owns final archival reconciliation and identical
repeated generation; the clean compiler/runtime acceptance above remains a
separate executed result. Generated pages are not hand-maintained specifications.
