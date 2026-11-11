# Native value ABI

This guide describes the native value contract derived from
[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md), the source of truth.
The [grammar](EBNF.md) defines source declarations and `.abi` records;
[TODO.md](TODO.md) tracks implementation. This contract does not certify that
all implementation work is complete.

## Type identity and representation

Every value has a module-defined type. Generated C types and fields use
collision-checked hashes of complete qualified identities, including their
category. The compiler retains source identities and emits `#line` mappings.
Matching C layouts do not establish Ama type compatibility.

Handwritten native companions qualify their module-owned C names through
`AMA_NATIVE(local_name)` under NAME-05. ABI records name local identifiers, not
global C functions. The generated context uses the same collision-checked module
prefix for `.h`, `.c` and typed uses. Include guards and external C names retain
ordinary C rules. The [namespace contract and probes](docs/implementation/native-namespaces.md)
cover this boundary independently of the unfinished project backend.

The generated assembly declares the types needed by the project and its transitive
dependencies. Generated typed declarations determine native entry signatures:
inputs are passed by value or by the specified borrowed representation, results
through output parameters, and the entry returns `void`. Technical status uses a
separate service output. Concrete representations and helper signatures must be
checked against the specification before claiming integrated backend support.
The concrete [C signatures and toolchain](docs/implementation/native-abi.md),
[status codes](docs/implementation/native-status.md),
[lifecycle ledger](docs/implementation/native-lifecycle.md) and
[runtime metadata](docs/implementation/runtime-metadata.md) are specified and
validated by section-2 harnesses. They are not yet connected to the frontend.

Instances execute directly in stable pool storage. Native code may borrow typed
views for the current operation; these views must not escape through globals,
payloads, callbacks or asynchronous work. Persistent Ama ownership and references
belong in declared Ama fields. Opaque native payloads contain native resources
only, never hidden Ama handles or encoded instance addresses.

## Generated headers and native authoring

Under CABI-06, explicit scaffolding generation provides a native interface header
and, when requested, a companion `.c` skeleton that directly includes that header.
The header is self-contained: it includes the standard, native representation
and generated dependency headers needed for parameter/result/state types, the
entry signature and ABI services. Types are complete wherever value passing or
permitted field access requires a definition; a permitted opaque/view contract
may use forward declarations. Resource internals are exposed only as permitted
by the native module's declared C representation/view contract.
Legal reference cycles are handled through declaration ordering and guarded
headers, without requiring the author to repair include order or copy typedefs.

The header can be the first project include in an otherwise empty C translation
unit. It does not depend on the final project assembly or a forced include, and
does not include Ama companion `.c` files, native entry/hook bodies or project
runtime/storage definitions. Inline utilities in required standard/external C
headers remain permitted dependencies.

The header establishes and cleans up its own `AMA_NATIVE` context under NAME-05;
the skeleton explicitly establishes the context for its body rather than relying
on leaked header macros. Both contexts use the same module identity during
independent checking and final assembly, and clean up temporary macros before
another module's context begins. Repeated inclusion and inclusion into the final
assembly must preserve identical qualified names without duplicate definitions.

Generation supplies machine-readable C-tooling settings, including the working
directory, include paths, defines and language/toolchain flags; a compilation
database is one supported choice. These let a C language service resolve types
while editing the scaffold before the execution backend is available. Editor
features depend on the editor's C-analysis support.

Acceptance independently syntax-checks the header and untouched C skeleton using
those settings, then checks their composition with the generated project. Include
transitive type dependencies, equal helper basenames in distinct modules, legal
reference cycles, repeated inclusion and naming-context cleanup. Independent
syntax checks do not change the single project translation-unit or mandatory
native-entry inlining contracts.

Generated headers/configuration have designated ownership; handwritten companion
files are preserved unless their creation/replacement is explicitly requested.
These requirements describe the target generator; its implementation remains
tracked in section 6 of [TODO.md](TODO.md).

## Fixed capabilities

A module's `.abi` companion binds exactly these symbols:

| Symbol | Capability | Required |
| --- | --- | --- |
| `+` | Default construction of fresh native storage | Yes |
| `-` | Destruction of native storage | Yes |
| `\|` | Construction from literal UTF-8 bytes and explicit length | When used |
| `=` | Independent cloning of native storage | When used |
| `?` | Conversion to a C `bool` condition | When used |
| `[]` | Conversion to a C `size_t` index | When used |

For example:

```text
+ integer_initialize
- integer_destroy
| integer_from_literal
= integer_clone
? integer_to_condition
[] integer_to_index
```

Each record starts in column one and contains a symbol, ASCII spaces and a C
function identifier. Records may occur in any order. Blank lines and `#` comments
are permitted. Duplicate or unknown symbols, malformed records and absent
mandatory bindings are errors. The grammar specifies exact whitespace and encoding
rules. Capabilities are never inferred from helper names or C source contents.
The compiler emits typed uses; the supported C toolchain verifies signatures.

## Common technical status

All native operations use the same status representation:

```c
typedef struct {
    uint32_t code;
    const char *message;
    size_t message_length;
} AmaStatus;
```

Code zero means success. An optional message is immutable UTF-8 with program
lifetime; a null message requires zero length. Reporting a failure must not require
successful allocation. The ABI must distinguish expected invalid-index refusal
from an unrelated technical conversion failure.

Construction, cloning, conversion, destruction and native execution all report
through this contract. Bound helpers may return status; native entries preserve
their void-return interface and use the generated service output. Application
failures remain ordinary declared results. Native code returns normally before
the runtime handles a technical failure; there are no language exceptions.

## Construction and destruction

Track storage as fresh, constructing, live or finalized. Select exactly one fresh
construction path: default, literal or clone. Do not default-construct and then
overwrite resources with another constructor.

Generated owned fields construct in declaration order before the native payload.
A failed native constructor releases its own partial resources; its destroy hook
is not invoked on failed construction. The generated lifecycle unwinds previously
live owned fields in reverse order.

For a live value, invoke the native destroy hook at most once, while owned children
are still live, then destroy children in reverse order. Native hooks do not own
or destroy generated Ama fields. Destruction failures do not stop remaining
cleanup: preserve the primary failure, report additional failures, and do not
retry or expose partly destroyed storage as live. Resource disposition after a
failed release must be documented by the native type.

## Copying and replacement

A native clone hook constructs genuinely independent native resources. A type
without `=` cannot be cloned where ownership requires it; references to that type
remain copyable. The compiler derives composed copy capability from owned fields.

Generated cloning maps the visible ownership tree to fresh state. Internal
references follow that mapping; external references retain their targets. Native
hooks clone only opaque resources. Ordinary copies clear saved positions
recursively; `%` copies retain them. Neither copies active callers, busy state
or refusal history. Busy descendants and forbidden ownership overlap are rejected
before mutation; ordinary self-assignment is a no-op and `%` self-copy is invalid.

Prepare and validate the complete replacement before destructive commit. A
preparation failure preserves the old destination. Once destruction begins, its
failure is terminal rather than a rollback: finalize remaining old and new
resources exactly once and report the error.

## Execution boundary and verification

Native entry bodies inline into the generated project assembly. Internal C
helpers may use ordinary calls, recursion, allocation and libraries. Native code
cannot dispatch Ama modules through C calls, modify runtime control metadata,
or yield through an unfinished C activation.

Runtime acquisition failures, list-operation refusals, application results and
technical errors keep their separate specified meanings. Capture the active Ama
execution path before cleanup; native C stack traces are not part of that promise.

Specify domains, invariants and failure effects and justify algorithms under
ARCH-05/ARCH-06. Test native contracts and their composition with generated state,
including partial construction, clone remapping, replacement failures, cleanup
errors and retained results. Compile and run generated C; checking emitted text
alone does not establish ABI or mandatory inlining correctness.

## Generated native authoring declarations

The implemented authoring generator uses tagged `struct AMA_NATIVE(State)` for
native payloads, module-qualified read/write views, and generated parameter/result
aggregates. Each C skeleton directly includes its self-contained interface header.
See [the engineering contract](docs/implementation/native-scaffolding.md) for the
exact scoped member macros and declaration/publication boundaries. These views do
not define instance-pool storage or introduce separate native translation units.
