# Value selection and whole-list transfers

T144 extracts existing algorithms; it does not implement intrinsic list services,
new context identities or the remaining syntax. The canonical rules are
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[COPY-02](../../spec/language/values-and-references.md#copy-02),
[COPY-04](../../spec/language/values-and-references.md#copy-04) and
[COPY-05](../../spec/language/values-and-references.md#copy-05).
See also [entry preparation](entry-preparation.md), [clone preparation](clone-preparation.md)
and the retained [list storage boundary](lists.md). Doxygen generates the source
reference from comments; this document supplies the algorithm argument.

## Selection domains

`runtime/program/values/selection/` separates exact type lookup, immutable shared
storage, mutable instance fields, readable native payloads, list index narrowing
and the iterative access driver. Public selection/evaluation entry points remain
in `values/source.c`. Registry and descriptor pointers are borrowed and immutable;
selected locations borrow their source storage until the surrounding operation
finishes. The runtime receives resolved, finite, acyclic access chains. It does
not parse names or guess whether a field is shared, owned or a reference.

An instance field lookup validates nominal type, checked identity, private/public
access and field bounds. It selects reference storage without acquiring that
reference's current target. A subsequent payload read separately checks target
availability. Shared scalar/list selection instead borrows constant storage with
no pool identity. Acquiring a shared scalar is rejected. List narrowing preserves
owned/reference form and yields one scalar; an out-of-range index changes no list.

For an indexed chain, the driver visits the innermost index source first. After
a node succeeds, its native index protocol is evaluated once to select the next
outer node. Only the final successful node publishes the caller's result view.
The invariant is that `selected` is the exact decoded index for the next node;
no earlier decoder is rerun. Finite chain depth decreases each iteration. Current
traversal rescans prefixes, costing O(D² + D*T) for depth D and linear registry
lookup over T types, plus native decoder costs. It uses constant traversal memory.

Target bounds refusal is an explicit boolean produced by actual list narrowing.
A well-formed dedicated `AMA_INVALID_INDEX` conversion result also refuses under
ERROR-05. Other decoder failures remain technical even if their numeric code equals
`AMA_INDEX_BOUNDS`; malformed success/refusal becomes InvalidState before the
conversion output is consumed. Busy/invalid index payloads fail before decoding.
The four independent [selection cases](../../tests/unit/runtime/value-selection/main.c)
exercise shared/mutable views, checked shapes, explicit source flags and this
same-code/different-origin distinction. Generated execution remains the actual
consumer of the selection interfaces.

## Transfer phase ownership

`runtime/program/transfers/list/` separates shape/alias preflight, private staging,
complete forest fixups, destructive commit and initialized-prefix cleanup.
`transfers/lists.c` orchestrates those phases. Inputs are validated generated value
views/recipes; the world guard and operation scratch/protections belong to the
caller. All work records in that operation belong to this one transfer. This
precondition prevents accidentally joining unrelated prepared objects into a map.

Validation rejects mismatched nominal/storage forms and absent destination storage.
An ordinary identical-list copy succeeds before allocations, capability checks or
native effects; continuation self-copy is rejected. For owned copies, reachable
copy capability and all source/destination owned trees are checked/protected before
staging. Reference-list copies transfer handles without acquiring their targets.
Current typed list service membership/acquisition rules remain separate TODO work.

Staging allocates checked handle capacity and appends only successful elements.
Each owned root is built privately; a failed current root unwinds inside object
construction, while previous complete roots remain in the staging prefix.
Reference staging copies handles only. Source/constant/default constructors remain
separate paths and select exactly one constructor for each owned payload.
The destination stays unchanged on preparation failure.

For an owned source copy, fixup joins the complete prepared forest into one exact
boundary and rewrites declared references again using all final identities. This
second complete-map pass is necessary for cross-element references, which could
still point outside an individual root's first construction boundary. Independent
default initializers retain their independent copy boundaries. The map is exact;
native opaque payloads are never searched for Ama handles.

Commit first checks the containing identity and parent-ledger arithmetic. It then
destroys the old prefix in reverse order, removes its count from the parent ledger,
publishes the prepared slots and installs their handles. Parent ownership of new
roots and preserved service state are assigned before returning. Success consumes
the staging buffer and zeros its former local owner; this zeroing is the only new
scratch-state effect introduced by extraction. Failure leaves staging with the
caller for discard. Once old destruction starts, rollback is impossible: cleanup
errors mark the world terminal, preserve the primary status and still release both
old and prepared completed resources.

## Argument and evidence

Before staging iteration k, exactly k completed elements belong to its buffer.
Successful construction extends that prefix once; failure unwinds the current
unappended element and discards the previous prefix. Reservation uniqueness and
exclusive source ownership make the completed forest map injective. A whole-map
fixup preserves external handles and maps each internal reference to its fresh
identity, including forward/cyclic references between elements. Successful commit
transfers the buffer once, so no local cleanup can free published storage. Failure
before old destruction preserves the destination; failure during destruction
cleans staging without claiming a whole-operation transaction.

The extraction keeps branch/effect/status order; staging-owner zeroing only makes
its handoff explicit. Phase loops advance finite count/field/work cursors. Memory
is proportional to prepared handles, owned work and protection records; native
resources add their own storage. Exact linear map/protection searches and flattened
owned-child enumeration have their existing costs; this is not a linear-time
claim for complete copying. Allocation or count overflow fails before publishing
staging, and cleanup allocates nothing.

The four independent [transfer cases](../../tests/unit/runtime/list-transfer/main.c)
verify allocation-free alias rejection/no-op, private cross-element fixups,
consumed staging on success, retained staging on failed parent validation,
cleanup of both forests after a destructor error, and busy/stale/empty reference
handles without referent acquisition. Existing [forest compositions](../../tests/unit/runtime/transfers/main.c)
enumerate every measured allocation failure and verify continuation projection.
The common fixture has one maintained implementation in
[transfers/common.c](../../tests/unit/runtime/transfers/common.c).
Actual commands and generated-program evidence are recorded in [TODO](../../TODO.md).
