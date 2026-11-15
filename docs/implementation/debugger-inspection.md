# Explicit debugger inspection

The runtime debugger API in `src/runtime/debugger/debugger.h` observes a context
at rest. It does not execute Ama, create default instances, invoke conversion
hooks, or reconstruct a call history. User-facing command integration belongs to
T115; this component supplies the checked inspection boundary.

`ama_debug_begin` takes exclusive context admission into an initially empty linear
session. Execution, disposal and another inspection cannot enter that context
until `ama_debug_end`. An executing phase or active lifecycle operation rejects
admission. The session, immutable catalog and registered reader array must remain
alive through all synchronous callbacks. Callbacks may make nested inspection
queries. Ending a session from its supplying callback returns Busy, preserving
all borrowed storage. The C provider must not mutate a session or its borrowed
runtime views, copy its linear lease, or execute Ama through a callback.

The compiler emits `ama_project_debug_catalog` in the retained project assembly.
It maps actual nominal IDs, storage ordinals and lexical scope ordinals to original
source names. UTF-8 names are emitted byte-for-byte using C octal escapes. The
catalog matches the program's exact source-map pointer and count; field operation
IDs and scope extents are checked before publication. C metadata providers must
supply truthful immutable descriptors. Omitting the catalog leaves names absent;
no spelling is guessed from hashes or unrelated source locations.

Instance enumeration walks actual published pool identities, skipping private
reservations, reclaimed directory holes, dead generations and retired cells.
Resolution checks context origin before the local identity. Inspection may observe
a busy instance's metadata without acquiring it. A snapshot reports lifecycle,
activity, ownership, transient immediate-caller connection, outgoing acquisition,
last refusal, recorded operation and saved resume independently. An operation is
mapped only to its exact source row; a missing row remains absent. A recorded
operation is not proof of an outstanding invocation or historical call chain.

Declared parameter, result and persistent fields are selected only through the
validated field descriptors. Scalar fields expose context-qualified identities;
lists expose acquisition, count and incarnation. List enumeration reports current
ordered membership identities and target handles, without dereferencing a target
or advancing execution. Foreach inspection reads compiler-defined cursor and
binding cells: numerical boundary, current occurrence, target, role count and
installed value. Inactive local bindings remain absent. Scheduler/task state
implemented in declared Ama fields is inspected through those same identities and
fields, with the same admission requirements; there is no special decoder of
opaque native scheduler storage.

Generic inspection never reads native State. `ama_debug_native` requires an
explicit type-specific read adapter and ordinary read availability. The trusted
adapter owns its native layout knowledge, thread-affinity and code lifetime; it
must perform a synchronous read without retaining borrows or executing Ama. It
publishes exactly one boolean, canonical sign/magnitude integer, valid UTF-8 text,
or byte value. The core checks transport shape and integer canonicality, while
the adapter guarantees its declared text encoding. A missing adapter or malformed
preview is an inspection error and does not change Ama status or refusal state.
Native resources are never exposed as an arbitrary memory dump.

Terminal cleanup removes pools and descriptors. A failed context therefore
exposes retained status and terminal control only, with no instances or fabricated
history. Borrowed source and status strings expire with their documented owners.
All snapshot references are observations: later use must revalidate their origin
and generation under a fresh admitted session.

The invariants follow directly from the exclusive context gate and exact identity
checks: no execution can change observed state during a session; enumeration is
a finite scan of historical slot capacity; each reported identity is published
and live; every payload access is a declared field/control offset or an explicitly
authorized native reader. Inspection allocates no new instance or language value.

Tests in `tests/unit/runtime/debugger` cover admission rollback, cross-context
identity collisions, busy values, stale handles, malformed catalogs/readers,
terminal cleanup, callback lifetime, actual simultaneous thread admission and
malformed preview transport. A native
payload page is protected with `mprotect(PROT_NONE)` during generic inspection.
`tests/unit/codegen/debugger` compiles an actual Russian-named list traversal,
inspects two successive yielded boundaries, original names, declared fields,
membership identities and exact saved source positions, then resumes to completion. It separately checks that refusal to resume a
completed module preserves its inspectable state, whereas an accepted native
failure disposes the generated host and rejects further inspection.
