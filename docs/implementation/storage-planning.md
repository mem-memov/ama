# Context, instance and lexical storage planning

This component implements T042's logical storage boundary under
[RT-01/RT-02](../../spec/implementation/runtime.md#rt-02),
[GEN-03](../../spec/implementation/compiler.md#gen-03),
[NAME-06](../../spec/language/declarations.md#name-06) and
[FLOW-05](../../spec/language/lists.md#flow-05).
The [specification](../../spec/README.md) defines language behavior. This chapter
explains the representation and its current acceptance evidence; it does not
claim that later lowering or runtime support is already complete.

## Three owners, with different lifetimes

[The storage schema](../../src/planning/storage/state.h) separates these domains:

| Domain | Required information | Owner and lifetime |
| --- | --- | --- |
| Context | Pool directory, immutable shared values, current checked instance, executor admission, successful-root cleanup ledger, diagnostics | One independent runtime context |
| Instance data | Parameter, retained-result and permanent body fields, native payload | One final stable typed pool slot |
| Instance control | Constructing identity, ownership state, availability, saved position, return connection, acquisition/refusal state and fixed lexical frames | The same instance, with each control record's specified active interval |

A context descriptor records exact pool and constant counts. Immutable type/code
schemas can be shared; the described mutable pools, constant instances and
runtime controls cannot. A constants-only namespace needs no pool or executable
service state. Its constants still participate in context startup and reverse
cleanup. Shared values are not extra fields in every mutable instance.

The general plan retains final constructing identity before initializer
execution. It does not construct an object, publish an available handle, run a
native hook or create an implicit default instance. Actual reservation and
publication remain the ownership/runtime tasks. The plan specifies the state
those operations must maintain.

## Fixed storage for operations and traversal

[Frame construction](../../src/planning/storage/frames/frames.c) enumerates the
resolved lexical call and foreach statements. A frame gets one ordinal in its
module. Repeating a loop does not allocate another frame. Different instances of
the same module have independent copies of these slots. The same busy instance
cannot recursively acquire another activation of its own body.

An operation frame describes target protection, input protection where needed,
output continuation, and intrinsic result storage where applicable. Entry/resume
of an actual module requires a body return connection; a retained read, list
service or intrinsic method does not create a fictitious Ama body return link.
The `FRAME_BODY_RETURN` requirement identifies that invocation responsibility,
not a second copied pending link. The callee's immediate return connection and
the caller's output continuation remain distinct. Protection can be inherited
from a parent operation; a protection requirement is not an instruction to
reacquire an already protected target.

Every frame records separate operation and traversal parents. The first follows
CALL-09's result-action tree. The second follows the nearest enclosing foreach,
passing over ordinary condition loops. Header handlers retain their resolved
incoming loop context. Both relationships are borrowed typed identities, not
inferences from names, indentation rescanning or runtime execution history.

A traversal frame describes activity, full collection identity, boundary and
current membership. Its associated reference-origin record is shared by every
lexical role updating the same binding. Nested reuse updates one cell and one
origin rather than saving/restoring the old reference. A permanent reused
reference stays in its declared field; its origin metadata does not duplicate
that handle. An introduced local reserves one conditional reference cell under
its exact lexical identity. Sibling locals with the same spelling remain distinct.

For example, nested `items[item&]` roles reuse the outer local cell. A later
sibling `items[item&]` can introduce another local. Each header has its own cursor;
only roles referring to the same resolved reference symbol share a binding and
origin. Static reservation does not activate a cell: an empty list or parsed-empty
body must establish no active cell/cursor under FLOW-05.

The general correction strategy is LIST-13's fixed lexical scan: runtime mutation
can inspect reserved traversal slots in lexical order, selecting only active or
suspended cursors whose full collection identity matches. No intrusive active
registry is required by this schema. Runtime correction and list execution remain
later tasks. Frame/cell reservation is independent of collection length.

Operation protections and pending returns are transient. Traversal cursors,
active scoped cells and their origins belong to the continuation control state
that CONT-04 validates and remaps. Recording both families does not authorize
copying transient grants or retaining a raw list-buffer pointer across yield.

## General representation and layout dependencies

The current policy retains stable slots and the complete general logical state
for every executable type. It performs no physical state elision. Absence of a
textual `%`, an empty body, a nonescaping-looking variable or small numeric payload
is not an exact specialization proof. RT-02/GEN-03 permit later specialization
only after all native/host, identity, lifecycle, failure, alias and control
observations have been accounted for. Falling back to this representation is
conforming; no performance improvement is claimed here.

Fields retain logical ordinals and exact nominal type pointers; they are not C
byte offsets. All nominal records exist before links are installed, so reference
cycles are supported. Eager ownership alone determines layout order. Call trees,
reference cells, cursor records and return links do not add eager ownership
edges. Nothing mirrors declared fields into module-wide working state or saves
and restores an instance payload on call/return. The native compiler still
supplies physical sizes and alignments.

## Boundary, proof and checks

The internal one-shot `plan_storage` stage accepts complete typed semantics,
constructs owned plan records and validates them before setting `storage_ready`.
That flag is separate from the full plan's `valid`: advanced source can have a
complete storage description while execution lowering remains explicitly
unsupported. Tests invoke this independent boundary without weakening frontend
unsupported-feature guards. A full plan is published only after all later
procedures, control graphs and entry links pass their own validation.

[Storage validation](../../src/planning/storage/validate.c) checks the exact entry,
nominal inventory, service domains, shared-value identities and counts, fields,
layout order and lexical storage. [Frame validation](../../src/planning/validation/ir/frames.c)
checks one-to-one correspondence with resolved sites, ordinal completeness,
parents, introduced cells and shared origin identities. It allocates no scratch
memory and does not execute user code or native companions.

Let `O` be the number of lexical operations, `F` foreach headers, `C` introduced
local bindings and `R` distinct references used by those headers. The general
per-instance lexical requirement is `O + F` frame slots, `C` conditional cells
and `R` origin records. Induction over the resolved source inventories proves
these counts: each operation/header appends once; only an introduction appends a
cell; a reference's first role appends its origin. Parent links are patched only
after all frame identities exist. No executable cycle is followed. Layout's
strict dependency ordering follows the already proved acyclic eager graph.
Planning and validation use at most quadratic time and linear owned storage in
the finite input inventories, with constant C call depth with respect to source
nesting. Counted allocation failure leaves every prefix owned by the plan arena;
reverse destruction releases the plan before its borrowed semantic/project owners.

[Independent tests](../../tests/unit/planning/storage/main.c) cover context/shared/
instance separation, constructing identity, explicit interface roles, reused and
sibling locals, permanent references, selected-result parents through ordinary
loops, native intrinsic outputs, 128-deep selected chains and traversals,
independent plan determinism, deliberately damaged layouts and every storage
allocation failure. Existing frontend/IR/execution suites verify that adding this
schema does not bypass guards or alter already supported behavior.
