# Correcting live boundaries during list edits

T082 integrates [FLOW-05](../../spec/language/lists.md#flow-05)'s numerical
boundary rules with every committed insertion and range removal, including
clear. It chooses [LIST-13](../../spec/language/lists.md#list-13)'s **fixed scan**
strategy. Selection and binding storage come from
[T081](list-traversal-selection.md); [missing-origin/resume checks](list-traversal-origin.md) are T083, and complete
controller/continuation integration is documented in
[T084](list-traversal-execution.md).

## The boundary marks a gap

A cursor's boundary `q` separates the current sequence into past positions
`[0, q)` and future positions `[q, n)`. It is not attached to the selected
element. For example, after selecting `A` from `[A, B, C]`, the gap is
`[A | B, C]`. Inserting at that gap puts the new occurrence in the future;
inserting before `A` leaves the new occurrence behind the gap. Appending is
future work while the cursor is active. Each nested scan has its own gap.

Removing the selected occurrence moves the gap left when necessary. Reinserting
at the resulting gap is future work. Clearing the list moves every matching gap
to zero; appending before the still-running body finishes can provide another
candidate. An already closed cursor remains closed.

Swap changes order but leaves the numerical gap unchanged. Moving a past
membership into the future can visit it again, while moving a future membership
into the past can exclude it. This is positional scanning, without a hidden
visited set or copied initial sequence.

## Immutable catalogs, separate mutable cursors

The compiler emits one immutable cursor-offset array for each declared list
field with lexical foreach sites. The containing table has one entry per
declared field, allowing direct field lookup; fields without sites have empty
entries. Offsets use actual `offsetof(GeneratedStorage, cursor_member)` in
lexical order. The same fixed cursor type is shared by the structural mutation
layer and the program-level foreach helpers.

The checked context schema validates bounded/aligned, strictly increasing,
nonoverlapping cursor extents before constructing objects. Non-list fields
cannot own nonempty catalogs. Each context borrows the immutable catalog through
its own object descriptors; actual cursor slots remain inside its independent
stable pools. No native payload owns a cursor pointer, and no registration
allocation or active-list link is required.

During a mutation, a temporary view borrows the current owner's stable payload
and the selected field's catalog. It verifies that the physical list is that
owner's exact declared field and that its namespace is valid. It validates each
catalog entry before pointer arithmetic. Only active cursors with the same full
owner identity, field ordinal and collection incarnation participate. The active
bit also covers suspended cursors. An old incarnation is not corrected or
reactivated merely because the same field now holds another collection.

## Preflight before effects, correction within publication

Before construction/destruction, the mutation checks every matching cursor's
`q <= old_length`. Invalid metadata fails without structural or native effects.
The ordinary bounds, capability, allocation and complete owning-subtree checks
retain their existing ordering and refusal provenance.

Insertion reserves membership metadata and prepares a new owned object/reference
before publication. After publishing the membership, it increments every
matching `q` for which `position < q`. Insertion at the gap leaves `q` unchanged.
The successful insertion's checked new length bounds every increment, so cursor
arithmetic cannot wrap.

Range removal at `i` of `c` occurrences subtracts
`min(c, max(q - i, 0))` from each matching boundary. The implementation performs
the subtraction only when `i < q`; neither `i + c` nor another unchecked sum is
needed. Clear is ordinary removal of `[0, n)`, which sets matching boundaries to
zero. Empty removal changes nothing. Swap, atomic reference-slot replacement,
capacity changes and owned payload updates leave boundaries unchanged.

Membership edits, ownership ledger updates and the complete cursor scan occur
under the same world's mutation guard. No application callback runs between
publishing the new membership order and correcting the cursors. Stable pool
storage keeps the borrowed cursor addresses valid even if native construction
grows another block of the owner's type.

Preparation/refusal failures publish neither membership edits nor corrections.
An owning destructor failure is different: cleanup is irreversible and terminal,
so all selected credits are consumed, the complete removal is published and all
matching boundaries are corrected even when the primary status is failure.
Cursor correction does not retry native cleanup or turn that technical failure
into a refusal handler.

## Invariants and costs

For insertion before the gap, both sequence positions and the boundary shift by
one, preserving which old occurrences are future. Otherwise the gap stays put
and the inserted occurrence joins the future. For removal, the number of old
past positions that survive is exactly
`q - |[i, i+c) intersect [0, q)|`; this is the implemented correction. Consequently
`0 <= q' <= n'` is preserved for every matching cursor. Each correction depends
only on its previous boundary and the committed edit, so visiting cursors in
lexical order gives the same numerical result as simultaneous correction.

Let `K` be this field's fixed number of lexical sites. Validation, preflight and
publication each take O(K), including inactive/old-incarnation checks. No scan of
another field's sites is required. Extra instance memory remains the fixed T081
cursor/binding slots, independent of list length; catalogs use O(F + K_total)
immutable shared metadata per type. Context validation visits the whole catalog
once. Generation uses exact field/site scans, without a probabilistic registry.
List order compaction, native lifecycle work and existing type lookup retain
their separate costs.

## Evidence

Ten runtime cases use actual declared list storage, stable pools and the real
foreach selection helpers. Small actual lists exhaust all insertion positions,
removal ranges and pairs of independent active boundaries. The removal oracle
counts surviving past positions directly rather than duplicating the runtime's
formula. Other cases cover remove/reinsert at the gap, clear/append, closed
cursor nonrevival, cross-gap swap repeats/skips, old incarnations, malformed
catalogs, allocation refusal and busy owning deletion before any effect.

A real yield followed by another caller's checked read/list acquisition verifies
correction of suspended cursors while preserving their saved resume selector.
Native destructor Busy consumes both resources, publishes the full clear and
corrects both cursors without retry. Two independent context factories install
the same immutable catalog but separate mutable slots; malformed duplicate
offsets fail before allocation.

The compiled C layout probe checks every generated per-field cursor count and
offset against actual members, then runs the real schema validator. Runtime and
generator ASan/UBSan, full strict tests, per-file coverage, standalone headers,
links and reproducible Doxygen validate this staged boundary. Complete source
mutation/traversal examples and mapped continuation acceptance are covered by
[T084](list-traversal-execution.md).
