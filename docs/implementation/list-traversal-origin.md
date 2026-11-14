# Checked iteration origins and resume decisions

T083 connects [FLOW-05](../../spec/language/lists.md#flow-05)'s origin rules to
ordinary target acquisition, retained payload reads, structural clone preflight
and saved-scope validation. It builds on [selection](list-traversal-selection.md)
and [mutation correction](list-traversal-mutation.md).
The [complete source adapter and copy integration](list-traversal-execution.md)
connect these checks to executable foreach and continuation operations.

## A binding is more than its copied handle

After selection, a binding's compiler-visible metadata records the collection,
membership and installed target identities. Removing a reference-list entry
retires its membership even when that external target remains alive. Therefore
calling through the associated binding must fail rather than execute that
surviving target as though the entry still existed.

This informative Ama fragment assumes a reference list `targets&[]` and a
compatible declared `saved&`. Its compiled execution is covered by T084:

```text
targets&[target&]
{
 saved&
  target&
 targets&[]
  clear !
 target& !
 :
  saved& !
}
```

The assignment copies only the handle into `saved&`. Clear removes all
occurrences without destroying their external objects. The call through
`target&` refuses because its membership disappeared. The independent `saved&`
can still call the surviving object, subject to ordinary availability checks.

An explicit assignment/output changing the binding's actual handle detaches its
origin and makes it an ordinary reference. Assigning the same full handle is a
no-op and retains the origin, including an already-invalid origin. Neither
operation moves the cursor boundary. Nested reuse replaces the shared binding
and origin without saving/restoring an outer value. The last role's closure
drops association metadata; a persistent reference keeps its ordinary handle.

## Validate before consuming target state

Resolved access descriptors carry an optional exact binding location. Local
access selects that compiler-visible cell; persistent reuse selects its original
declared handle. Views borrow origin metadata for the current operation only.
Ordinary scalar/shared views start without an origin, and reference assignment
copies only the handle, so associations cannot leak through value copying.
Private fresh construction still resolves declared reference initializers in
its prepared storage; lexical locals are never construction-time fields.

Before entry, resume or retained-read acquisition, the runtime validates the
associated membership and target identity before checking target availability,
preparing inputs, resetting interfaces or dispatching a body. Missing membership
or referent is an ordinary acquisition refusal under
[CALL-04](../../spec/language/calls-and-results.md#call-04). Busy targets still
use normal availability refusal; origin validation does not acquire a referent.

Retained payload/index reads check origin before a native condition/index hook
can run. Only explicit callback-free missing/busy checks carry origin-refusal
provenance. A native decoder's arbitrary error does not become a refusal merely
because it has the same numeric status. An invalid associated index operand
therefore fails before decoder effects and before selecting an eventual target.

Checking an origin first checks list busy state and validates the exact
owner/field/incarnation/record/generation identity. These reads have no callback,
conversion or allocation, so the context's single executor cannot interleave a
mutation. An ordinary body receives no structural authority. Only a direct
method helper can borrow its exact existing grant when validating a busy source
collection under [LIST-10](../../spec/language/lists.md#list-10).

Owning append/insert retains a staged origin tuple for an associated source.
After the declared inputs are prepared, preflight validates that origin before
cloning or publishing anything. Plain reference-list insertion copies a handle
without accessing its target state and creates an independent occurrence; it
does not inherit the old origin. This preserves
[REF-07](../../spec/language/values-and-references.md#ref-07)'s distinction between
copying reference storage and consuming the referenced object's state.

## Resume the retained lexical chain

The compiler emits immutable scope descriptors in preorder. Each identifies its
earlier parent scope, checked cursor/binding/value offsets, exact element type,
header diagnostic selector and next-selection selector. Context validation
checks this shape before allocation/construction. Local cells and compatible
persistent scalar references have distinct explicit location contracts; no name
or runtime contents participate in resolution.

For each still-active retained scope, outermost first:

1. Reacquire its source briefly and validate owner/field/incarnation. A replaced
   collection or failed internal acquisition is technical at that header.
2. Validate that scope's current membership and referent, independently of the
   shared binding cell possibly overwritten by a nested role.
3. If the current occurrence/referent is missing, discard active descendants
   innermost first, preserve this loop's already-corrected boundary and return
   its next-selection selector. No saved body statement is selected.
4. Otherwise continue to the next retained scope. If all validate, return zero
   so the controller keeps the saved body dispatch.

A live busy referent is not a missing candidate and does not trigger automatic
skipping. Any subsequent attempted access still uses normal Busy refusal.
Nested exit never restores an earlier shared origin. Consequently an outer
cursor's original membership can remain live while the shared binding retains
a removed inner membership: a later read through that binding still refuses.

The reset helper discards roles in reverse preorder without target destruction
or list reacquisition. It is idempotent for inactive slots and works after
collection replacement. Generated fresh-entry/exit dispatch and continuation mapping use these helpers;
see [execution and copy integration](list-traversal-execution.md).

## Invariants and evidence

An associated binding always names its installed handle and one exact origin;
ordinary copies carry neither the metadata pointer nor the association. A
different explicit assignment clears association before publishing the new
handle; a same-handle write preserves it. Validation has no native side effects,
so refusal precedes all target work. Cursor identities and binding identities
remain separate, which makes nested reuse and outer-first resume deterministic.

Each origin check uses constant-size identities and direct membership lookup,
plus existing O(T) nominal lookup. Scope validation walks the fixed lexical
inventory and, when abandoning an iteration, uses strictly decreasing parent
indices to identify descendants exactly. No snapshot arrays, visited sets,
registration allocation or target search is introduced.

Eleven runtime cases check every target mode, native decoder suppression,
independent references, same/different handle writes for locals and reused
fields, exact busy-list authority, owning-clone versus handle-copy preflight,
outer/inner missing iterations, no restoration, collection replacement at the
correct header, deleted/reused target slots, failed internal acquisition,
valid saved dispatch, busy live targets and scope reset without target cleanup.

The actual C layout probe checks generated scope ancestry, shared offsets and
local/persistent access descriptors, and executes both schema validators.
Runtime/generator ASan/UBSan, the full strict suite, per-file coverage,
standalone headers, links and reproducible Doxygen validate this staged boundary.
