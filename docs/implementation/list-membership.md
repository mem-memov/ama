# Checked list memberships

T075 introduces a metadata-only collection component under
[`src/runtime/lists/membership`](../../src/runtime/lists/membership/membership.h).
It supplies the representation required by
[LIST-03](../../spec/language/lists.md#list-03) and
[LIST-13](../../spec/language/lists.md#list-13). It does not execute elements,
destroy targets, acquire application lists or retain native pointers.

## Identity and storage

The current order is a contiguous `size_t` array of local record indices. Every
record lives in a separately allocated fixed block of 32 records. Growing the
directory moves pointers to blocks, never the records themselves. A record stores
a checked target handle, its current order position, a generation and liveness.
Duplicate, empty and stale target handles are legitimate data; equal targets do
not imply equal memberships.

A saved `AmaMembership` contains the complete owner handle, field selector,
collection incarnation, record index and record generation. Current protected
order selection may reconstruct that identity. Saved identity lookup compares
every namespace component and the recorded generation before publishing a
borrowed record. It never searches for the target or reads a new generation to
repair an old identity. The caller supplies exact exclusive list authority and
checks owner liveness; this component cannot authenticate a context or acquire an
owner through a pointer to metadata.

## Publication and retirement

Insertion prepares order capacity, directory capacity and a free record before
changing live order. It then consumes the prepared record, shifts indices, updates
suffix positions and publishes the new count and identity without callbacks or
allocation. Preparation failure leaves every existing identity, value and order
unchanged; retained spare capacity is permitted.

Removal marks records dead before reuse. If a generation can advance, retirement
increments it and links the slot into the free chain. Otherwise the slot remains
permanently excluded from that chain. Later allocation uses another record;
exhaustion never wraps or revives an identity. Swap exchanges two order indices
and updates their inverse positions without changing membership identities.

Reference replacement with an identical complete handle is an allocation-free
no-op. A different handle prepares a fresh record before retiring the old record,
then replaces one index at the same position. Length and position remain unchanged;
there is no intermediate remove/insert event. Owning-element payload replacement
does not call this primitive because its containing occurrence remains unchanged.

Reset validates the next incarnation before retiring any entry. It advances the
incarnation even for empty contents. At its configured counter limit it fails
without changing the collection. Initialization is restricted to a new owner/field
lifetime; callers must not dispose and initialize metadata to reset the history of
a still-live namespace. Containing-root replacement must preserve and advance the
live field's incarnation when connected in the replacement layer.

## Invariants and argument

For each current position `i`, `order[i]` denotes a live record whose position is
`i`; no other order position names that record. Initial empty state satisfies this
bijection. Insertion consumes a free, therefore absent, record and shifts each
affected position exactly once. Removal retires exactly the removed range and
compacts the remaining suffix exactly once. Swap permutes two distinct indices
(or leaves one unchanged). Replacement substitutes one fresh record at the same
position. Thus each operation preserves the bijection inductively.

A free record's next issued generation is strictly greater than its retired
generation, or the record is never reused. Incarnation is also strictly monotone.
Together with the exact owner/field namespace comparison, this proves that no
saved retired membership can validate after reuse or reset. Bounds checks use
subtraction for ranges, and capacity products and growth are checked before
allocation. No probabilistic identity, target search or mutation heuristic is used.

Selection, saved-identity lookup, swap and prepared record take/retire are O(1).
Insertion/removal update an O(n) suffix; reset visits O(n) live entries. Geometric
order/directory growth copies O(capacity) entries on a growth event. Memory is
O(order capacity + historical record blocks); permanently exhausted slots retain
their exclusion history. Full-width `size_t` indices need no re-encoding step.

## Evidence and boundaries

`make -C tests/unit/runtime/membership` runs six explicit cases. `identity.c`
checks duplicate targets and reused slots, all full-identity components, stable
record addresses across five blocks, exact inverse positions after insert/remove/
swap, same-handle no-op, changed/empty targets and incarnation exhaustion.
`failures.c` checks generation exhaustion across multiple blocks, every initial
allocation failure, live-state replacement failure, unchanged failure outputs,
overflowing ranges and empty end ranges. `common.c` checks the order bijection
independently and counts every allocation/release. `main.c` dispatches the cases;
the Makefile runs and cleans this independently executable leaf.

The ready registry maps all component source/header files to these named cases,
with standalone header compilation. Strict and ASan/UBSan runs cover the same real
component; the whole strict suite checks composition with runtime consumers.
T076 connects this representation through [list value integration](list-values.md).
[List service authority](list-service-frames.md), method result frames and
[live cursor corrections](list-traversal-mutation.md) are separate higher layers.
The exact acquisition-frame address belongs to those layers; membership
edits themselves do not acquire or release a list.
