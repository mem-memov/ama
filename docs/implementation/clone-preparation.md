# Clone preparation and identity mapping

T142 separates the existing clone preparation implementation into private slot
reservation, child descent, native/service payload copying and reference fixups.
Doxygen generates the file/function reference pages; this document records the
argument and acceptance boundary. It refines the [ownership contract](instance-ownership.md)
and does not replace the remaining feature tasks.

## Admission and temporary ownership

[COPY-07](../../spec/language/values-and-references.md#copy-07) requires complete
source-tree availability. Existing access preflight resolves checked identities,
verifies the finite owned tree and reachable-type copy capabilities, then protects
every owned source before a native hook. The public clone wrapper owns its world
guard and arena. Replacement and forest-transfer callers own the same obligations
in their surrounding operation; `ama_object_clone_prepare` does not repeat admission.
Busy or unavailable sources therefore fail before native side effects. Declared
nonowning external targets are not acquired or recursively copied.

`reserve.c` borrows a protected source and reserves its final private destination
slot. Its work record supplies the source-to-destination association. A nonnull
returned record means a slot has been registered for cleanup even if subsequent
field-buffer initialization failed. Before registration, failures leave the output
work pointer unchanged. `fields.c` allocates field/list shape and copies nonowning
handles, leaving owned positions empty until their children are reserved.
`children.c` records ownership and installs any reserved child before returning:
late field failure must leave a child cursor from which unwind can reach it.

`payload.c` invokes the native clone after all children complete, marks successful
native initialization in the ledger, and projects the saved pre-acquisition source
service state. [COPY-03](../../spec/language/values-and-references.md#copy-03)
clears positions for ordinary values;
[CONT-04](../../spec/language/continuations.md#cont-04) preserves them explicitly.
Neither mode copies transient caller links, acquisitions or refusal state. Failed
native constructors own their failed native acquisitions; successful native copies
are destroyed by unwind if any later preparation fails.

## One complete map per copy boundary

[COPY-04](../../spec/language/values-and-references.md#copy-04) and
[COPY-05](../../spec/language/values-and-references.md#copy-05) define the boundary.
The operation work list is the map's storage, not an implicit global registry.
For boundary b, include precisely records with `clone_root == b`. Source
preflight excludes repeated owned identities within a tree, and private pool
reservation gives each record a fresh identity. The resulting map is injective.
Two independent initializers can copy the same source in one operation: they have
distinct boundaries and must not merge their identities. A forest-copy caller may
explicitly join its roots into one boundary before the final forest fixup.

Only after all identities exist does `mapping.c` apply that map to every declared
reference/reference-list field in the boundary. Internal references become their
mapped targets; empty and external handles remain unchanged, including external
stale handles. Forward, ancestor and cyclic nonowning references require no
recursive traversal. Replacement can substitute its final existing root identity
for the prepared root, so self/ancestor references survive the later bank swap.
Native opaque resources are copied solely by native hooks, never scanned for Ama
references. No publication or destructive replacement occurs during preparation.

## Driver invariant, failures and cost

At each iterative step, complete siblings form an initialized prefix and the
current uncredited child is reachable through the work cursor. A successful
payload permits one credit/ascent; failure cleans the cursor and then ancestors,
with successful siblings reached through their parent's ledger. Thus a failed
constructor is not destroyed and each completed private resource is destroyed
once. The primary status is reported once and survives secondary cleanup errors.
The caller restores the source protections and releases scratch after the driver
returns. Failure leaves the caller's output unchanged and the source data intact.

For a finite owned tree, each edge is descended and credited once; recursion in
C is unnecessary. The current map uses exact linear search, not guessed keys or
probabilistic matching. With N operation work records and R reference elements,
fixup costs O(N + R*N) per boundary; protected-state lookup may add O(N*P) across
N payloads and P protection records. Field enumeration, preflight, allocation and
native hooks add their own costs. Memory is O(N + P) plus cloned field/native
storage. These are bounds on the retained algorithm, not a claim that future
optimized maps or context/embedding support are already implemented.

## Evidence

The independent [phase tests](../../tests/unit/runtime/objects/clone-preparation/main.c)
enumerate every measured reservation allocation failure, including post-slot
field failure; verify child ownership before prefix credit; separate two copies
of one source and final-root redirection; and inject failure at each of a three-node
tree's native clone constructors. Real [clone](../../tests/unit/runtime/objects/clone/main.c)
and [replacement](../../tests/unit/runtime/objects/replacement/main.c) callers cover
busy descendants, absent capabilities, allocation-prefix rollback, cyclic/forward
references, explicit continuation state and nonmoving replacement publication.
Forest and generated-program compositions remain part of the root test suite.
See [TODO](../../TODO.md) for commands actually run and completion evidence.
