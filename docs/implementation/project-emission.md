# One project C assembly

`ama --emit ROOT .module --out DIR --sdk /path/to/ama/src` uses the same strict
frontend and typed plan as `--check`. It generates C without executing constructors
or invoking the C compiler. Add `--compile` to compile, verify and publish
`DIR/program`. Generated files and the executable share the existing scoped
manifest, content hashes, lock, staged replacement and rollback protocol. Native
companions remain handwritten inputs; an unowned/edited output needs explicit
`--overwrite`.

The launcher includes `project.h` and `project.c` at file scope. `project.c`
defines one execution function containing all Ama labels and an explicit host
boundary for entry, resume and disposal. The header includes native declaration files, concrete heap layouts,
shared-value storage, native companion bodies, typed lifecycle adapters, runtime
and source/operation tables. Native declarations are exactly those supplied to
isolated authoring headers. Each native entry is redeclared through its exact
function typedef before its definition, including its void return type.
The author establishes and cleans the body's generated naming context. No Ama
module becomes a C function.

`offsetof`, `sizeof` and `_Alignof` provide physical layouts and runtime descriptor
checks. Hash-qualified helper/type/member identities derive from complete logical
paths; compiler helpers and authored native prefixes occupy disjoint categories.
Shared values have no pool slot or service state. The SDK `runtime/assembly.h`
includes runtime definitions once; qualified private helper names avoid collisions.
See [startup](program-startup.md), [entry preparation](entry-preparation.md) and
[execution](stackless-execution.md) for construction and transfer invariants.

## Native compilation and verification

The compiler materializes the in-memory assembly in a private temporary directory,
then invokes tools with argument arrays, never shell text. The supported GCC target
is enforced by the SDK. User `--c-flag` arguments precede mandatory C11/O2 and
non-LTO/non-whole-program compilation settings. Required native libraries link
after the project object. `nm --format=posix` examines the unstripped object before
linking. Verification requires `main` and rejects any surviving registered native
entry or its compiler-generated suffix family. Always-inline entry bodies must
therefore disappear; ordinary C helpers/lifecycle callbacks may remain.

Only after successful compilation, entry verification and linking does the
executable join the artifact set. Publication sets its execute permission. Failure
leaves existing published outputs intact and prevents publishing a failed build.
Temporary tool outputs are cleaned separately from owned public outputs. Compiler
and linker diagnostics retain their native details. Build flags that request
additional compiler side files remain the toolchain user's responsibility.

The compilation database describes the final launcher translation unit with the
same include roots and mandatory language/optimization settings. Every argument
is separately JSON encoded. Native modules are never independently compiled or
linked as Ama modules.

## Correctness and evidence

Every planned field has one descriptor at its declaration slot; shared declarations
never enter instance layouts. Concrete C types provide the sizes/alignments and
check native hook/entry signatures. Qualified identities and unique control
selectors prevent accidental cross-module wiring. Emission is finite traversal
with linear output size and explicit registry lookups; allocation failure retains
the first diagnostic and publishes no incomplete artifact. Tool success and symbol
verification are necessary before executable publication; failures cannot silently
fall back to ordinary C calls for Ama/native entry dispatch.

[Assembly tests](../../tests/unit/codegen/assembly/main.c) compile shared isolated
and project declarations, include equal native local names, and inject every
emission allocation failure. [Execution tests](../../tests/unit/codegen/execution/main.c)
run real compiled projects through the production verifier, including a negative
non-inlined entry. [CLI tests](../../tests/integration/execution/main.c) execute the
published binary and protect previous/unowned files on failure. [Check-only tests](../../tests/integration/check/main.c)
prove that `--check` and emission without `--compile` never invoke native C.

Suspension/resume execution and list action dispatch remain sections 9 and 10;
this stage supports their existing acquisition checks and rejects unsupported
execution explicitly. Whole-list value copying and indexed result routing already
use the typed lifecycle runtime.
