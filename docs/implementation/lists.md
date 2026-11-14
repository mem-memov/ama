# Membership-backed list storage

The [list specification](../../spec/language/lists.md) separates checked order
storage from typed service authority and live traversal. T075 supplies
[exact memberships](list-membership.md); T076 connects them to declared fields,
indexed values, construction, copying and replacement under
[list value integration](list-values.md). T077 adds [service authority](list-service-frames.md)
and T078 adds [append/insert execution](list-insertion.md). T079 adds
[remove/clear execution](list-deletion.md). Remaining methods and
cursor corrections keep their roadmap tasks. Whole-list execution and retained whole-list reads remain
invalid language operations, rather than dispatcher actions.

## Direct storage operations

`src/runtime/lists/list.h` exposes insertion before one position and removal of a
contiguous range. `AmaListStorage` borrows a live owner's exact field, element type
and ownership form. Its caller supplies exclusive authority. The world prevents
lifecycle reentry while an edit runs. These C operations are not user-visible
method frames, action values, completion flags or retained results.

Insertion checks `position <= count`, then prepares order capacity and a free
record before native construction. An owning source is preflighted before an
independent ordinary clone; empty source selects initial construction. Reference
insertion copies the complete handle without acquiring its target. Once native
construction succeeds, publication consumes already prepared metadata, updates
the parent ledger and installs the membership without allocation or callbacks.
Preparation failure preserves existing contents and identities; spare capacity
may have grown.

Removal checks `position <= count` and `range_count <= count - position`, avoiding
overflow. It preflights every affected ownership tree before the first destructor,
then destroys owned roots in reverse order and retires the exact memberships.
Reference removal neither acquires nor destroys targets. Destructor failure is
terminal: cleanup continues and preserves the primary error without rollback.
All operation exits release scratch and preflight protections.

## Storage and complexity

`src/runtime/lists/storage.c` prepares private declared shapes at their final
owner/field/incarnation. The order buffer contains local record indices. The
membership pool supplies stable records with exact generations and inverse
positions. Instances themselves remain in stable typed pool slots. No native
payload is relocated by order growth, insertion or compaction.

Insert/remove shift the affected index suffix and update inverse positions in
O(n), excluding native lifecycle and ownership preflight. Prepared record commit
is O(1). Capacity growth is checked and geometric; saved identities do not retain
an order-buffer pointer. Membership lookup and protected indexed selection are
O(1), excluding user index conversion. The detailed invariants and proof are in
[list memberships](list-membership.md).

## Evidence

`make -C tests/unit/runtime/lists` runs the existing storage/lifecycle laws against
the new representation: geometric order growth and allocation failure, ordered
insertion/removal and stale owned roots, whole-subtree preflight, reference
storage, every default-insertion allocation failure, terminal cleanup, clone
reference remapping, cleared ordinary resume state, and rejected lifecycle
reentry. `make -C tests/unit/runtime/membership` tests identity bookkeeping
independently. The object, transfer and generated-project suites compose these
components through their actual boundaries. No action dispatcher or fallback
handle-vector representation remains.

[Typed swap and length](list-ordering.md) complete the six method adapters in T080.

[Live foreach selection](list-traversal-selection.md) provides fixed cursors and lexical binding cells in T081; source execution remains guarded pending traversal integration.

[Traversal mutation correction](list-traversal-mutation.md) connects exact fixed cursor scans to insert/remove/clear publication in T082.

[Traversal origin and resume checks](list-traversal-origin.md) connect missing-membership guards and exact retained-scope decisions in T083.
