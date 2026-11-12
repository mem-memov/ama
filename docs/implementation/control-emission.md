# Resolved control emission

T140 separates control-descriptor composition, exact IR-site queries and the C
writers for individual operations. The [architecture](../../spec/implementation/compiler.md#arch-03)
requires narrow dependencies and independent/composed tests. The
[stackless contract](../../spec/implementation/compiler.md#stack-01) preserves
Ama labels, explicit return/resume selectors and the inline native boundary.
This refactoring adds no current grammar, result-tree or context conformance.

## Component boundaries

- `control/resolution/` scans typed IR by operation kind and exact statement,
  binding or field identity. It owns no memory and receives no frontend/model.
- `control/emission/` writes C fragments from a stream, qualified identifier,
  selector values, source span and explicit Boolean choices. It performs no
  name/type lookup or C-text analysis and constructs no unrelated compiler state.
- `control/data/` composes call, assignment and output descriptors using the
  validated model's type/member views and identity/source-description services.
  This is the model-level adapter, not the scalar writer or IR-query component.
- `control/data.c` traverses the model's control records; `control/emit.c` selects
  the exact operation writer and terminal/successor behavior.
- `control/native.c` receives only its output stream and resolved type/member view.
  It binds scoped native parameters/results, snapshots caller refusal, preserves
  the native reentry guard and emits the qualified entry use for toolchain checking.

The model, source spans, symbols, bindings and qualified names outlive emission.
The scaffold arena owns generated identifiers; writers borrow them only for their
call. Streams belong to the caller, including flush/close checks. Partial bytes
on failure remain in unpublished artifact buffers, not successful public output.
Existing artifact generation/publication composes those failure laws.

## Required operation identity and inactive metadata

For each supplied nonliteral input, lowering creates a `CONTROL_EVALUATE_INPUT`
record. For each supplied literal it creates the corresponding
`CONTROL_INSTALL_PARAMETER` record and no separate evaluation. A field query
requires parameter installation or result initialization according to its exact
resolved role. Query keys contain operation kind plus pointer identities from
one immutable plan; spelling and allocation-list order have no semantic role.

The scan returns FOUND only for one match, MISSING for none and DUPLICATE for at
least two. Both invalid required cases clear the result pointer and abort model
emission with a diagnostic. The generator never substitutes a call/acquisition
selector for a missing or ambiguous required input/interface operation.

The retained runtime consumes installation selectors only during entry preparation
and only for parameter/result fields. Resume and retained reads do not prepare;
persistent fields do not participate in that preparation loop. Those inactive
table entries deliberately retain the invocation-site marker, preserving existing
output and behavior. This is an explicitly unused domain, not a missing-IR
fallback. Future IR with repeated lexical occurrences must supply explicit
occurrence identity before changing this uniqueness contract; choosing the first
matching node is not an implementation strategy.

## Dispatch and preservation argument

The dispatcher handles the closed control-operation set. Ordinary operations
write their fragment then their explicit successor; completion, condition,
failure, yield and resume write their own terminal transfer. Unknown operation
tags fail emission. Interface-evaluation/install/replacement bookkeeping nodes
retain the existing label/provenance behavior while the runtime preparation or
transfer operation performs their concrete work; this refactoring does not
introduce a second execution of those effects.

Each label keeps its selector, physical source span and cell-location check.
The trace selector is updated except for refusal snapshots and yield, preserving
the operation whose status/position those transitions expose. Branch successors,
return links and saved positions come directly from IR. No graph relationship is
inferred from generated C, type names or diagnostic text.

For valid current plans, the old binding scan selected evaluation before
installation for nonliterals because lowering prepends evaluations last; literals
have only installation. The new query selects those exact kinds explicitly and
requires uniqueness. Field construction keys are unchanged. Inactive markers,
qualified names, literals, runtime calls, native scopes and branch/return targets
remain the same. Thus the extraction and exact query refine the existing valid
model translation, while malformed required sites now fail instead of being
silently repaired.

Every successful query traverses a finite node list; finding a second match is a
complete witness that uniqueness fails. It costs O(C) time and O(1) memory for C
control records. Data composition retains the existing O(C(B+F)) site-query cost
for B bindings and F fields, plus source/identity-description costs. Scalar writers
are linear in emitted bytes and use O(1) extra state. A caller's stream operations
must return; no arbitrary depth, timeout or source-shape shortcut selects output.

## Independent and composed evidence

`make -C tests/unit/codegen/control` links scalar writers, native type-view output,
IR queries and source-byte/mapping helpers without constructing a frontend or
project. Five named C/assert cases check unique literal/nonliteral/field sites,
missing/duplicate results independent of node order, actual unwritable streams
for each writer family, physical source positions/preserved trace selection, and
compiled C fragments. The fragments use real runtime declarations with small
outcome stubs; they execute refusal versus technical-failure routing, entry-only
preparation, both branch arms and save/resume/restart transitions. Those stubs
certify emitted control flow, not runtime ownership/lifecycle implementation.

The real assembly suite corrupts missing/duplicate input and missing interface
sites in an otherwise composed plan and requires failure before publication.
The ordinary assembly, execution, scalar-library and continuation suites compile
and run actual projects with real runtime/native code. They check ordered output,
shared reads, native failures/traces, return/yield/resume, and rejection of a
surviving non-inlined native entry. Both scales are necessary; string inspection
alone would not establish generated-program behavior.

The current ready registry maps every extracted file to those named tests and
independently compiles its headers. Doxygen generates file/interface reference
pages and validates actual canonical-rule destinations. Remaining grammar,
context, service-frame and control-model changes stay in their owning TODO tasks.

T140 acceptance passed the strict standalone leaf, independent real assembly,
execution and continuation leaves, and complete `make -j4 test` with 63 registered
leaves (including the actual codegen parent). The negative native-entry inlining
case remained enforced. Standalone headers and Doxygen's actual canonical links
passed; 474 maintained code/include pages were generated. The current registry
records 68 ready inputs and 563 explicit remaining debts. No specification changed.
