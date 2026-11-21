# Executable live traversal and continuation copying

T084 connects the explicit foreach IR to generated labels and copies its retained
control state under [CONT-04](../../spec/language/continuations.md#cont-04).
[Selection](list-traversal-selection.md), [mutation](list-traversal-mutation.md)
and [origin validation](list-traversal-origin.md) describe the underlying model.
No specification change or new native ABI protocol is needed.

## Execute the resolved control graph

[The control emitter](../../src/codegen/control/traversal.c) translates entry,
selection, exit and reset into checked runtime operations followed by their
resolved labels. Selection has two explicit successors. Exhaustion has already
closed the role, so the following leave is deliberately idempotent. Fresh entry
and completion discard scopes; yield retains them. Resume validates retained
scopes before dispatching a saved body label. A missing current occurrence
redirects to that loop's next selection, after discarding its descendants.
All helpers return before an Ama body executes; no unfinished C activation
represents an Ama continuation.

For example, assuming an owned list and compatible values, insertion exactly at
the current frontier is future work:

```text
items[item&]
{
 item& !
 once
 ?
  items[]
   insert !
    < |1
    < & extra
  once
   off
}
```

The compiled tests provide the declarations and expected outputs. They also
insert behind the frontier, where the new occurrence is not visited, and remove
an associated reference-list occurrence before attempting to call through it.
The latter refuses before native effects. Explicitly assigning another handle
detaches the origin and permits ordinary reference access.

## Copy controls only after constructing the ownership boundary

Object types have optional compiler-owned control hooks, separate from native
payload lifecycle protocols. Factory construction attaches these hooks only to
types with foreach scopes. An operation protects the complete source ownership
tree before constructors run. A stackless walk of those protected records checks
active collection identities, boundaries and saved-position consistency. Cached
proof belongs only to this operation and never changes source control cells.

Each clone work record snapshots its continuation mode by value. Recipes may be
short-lived/reused during composed initialization; retaining a pointer to their
mode would incorrectly change earlier work's meaning. Ordinary copies retain
fresh inactive control storage. Continuation copies install controls after the
new lists and their membership identities exist.

[Membership translation](../../src/runtime/program/lists/traversal/copy/membership.c)
first validates the complete source identity. A live occurrence then maps by its
validated position to the corresponding fresh destination occurrence. A missing
or obsolete origin maps to a reserved dead identity, never a coincident record
number or a new occurrence at the same position. Active cursors retain numerical
boundaries. Shared binding cells are assigned as values, without incrementing
role counts or restoring an outer value.

The same exact ownership map used for declared references remaps active local
handles, installed handles and cursor targets. External targets remain unchanged.
A complete list-forest fixup remaps these reference cells again after all sibling
roots exist; it does not recopy cursors or reset their namespace/boundary. Stable
root replacement uses the final root identity rather than a temporary clone slot.
No acquisition, caller link, pending frame, borrowed pointer or native activation
is copied. Failure discards prepared destinations and preserves protected sources.

## Why the algorithms preserve the contract

Within a copy boundary the source-to-destination object map is injective.
Ownership fields determine the same ordered tree, so a fully validated live
membership at position i has exactly one corresponding new membership at i.
An invalid source identity has no such image: assigning generation zero prevents
revival even when source and destination reuse the same record index. Applying
the map to copied reference values is idempotent because fresh destination
identities are not keys in the selected source map. This permits the final forest
phase without double-copying control state.

For traversal let P = n - q be the remaining candidate potential. Each candidate
selection decreases P by one, including skipped missing references. Insertion
before q changes both n and q equally; insertion at/after q increases P by one.
Removal never increases P; swap preserves it. Consequently candidate examinations
are bounded by initial P plus insertions at/after the frontier. Unbounded user
insertion can produce an unbounded traversal, as specified; there is no hidden
iteration cap, snapshot array or visited set.

There is fixed storage per lexical cursor/binding. Copying walks the owned tree
and fixed lexical inventories; exact source-handle map/protection lookup currently
uses linear operation-record scans, so this implementation does not claim linear
overall cloning time. It adds no list-length-dependent traversal snapshot.
Mutation correction retains the fixed per-field catalog cost documented in T082.

## Evidence and file coverage

The per-file executable map is maintained in
[ready.json](documentation/ready.json). These suites exercise the actual components:

- `make -C tests/unit/runtime/traversal-mutation`: exhaustive boundary edits
  and a finite candidate-budget test with past/future insertions and deletion.
- `make -C tests/unit/runtime/traversal-copy`: live namespace/boundary copies,
  removed-record nonrevival, obsolete exited-inner origins, ordinary clearing,
  stable-root replacement, active-collection preflight, internal/external handles,
  every allocation-failure prefix, later native clone failure, yielded owned
  children, mixed initializer modes and cross-sibling controls after separately
  prepared list-forest roots.
- `make -C tests/unit/codegen/traversal`: actual frontend, planning, C emission,
  native compilation and execution for binding reuse, mutation, origin refusal,
  independent continuation resumption and cross-sibling internal references.
- `make -C tests/integration/objects`: the maintained CLI traversal project and
  exact resource destruction trace, together with existing lifecycle scenarios.
- `make -C tests/unit/runtime/objects/clone-preparation` and
  `make -C tests/unit/runtime/list-transfer`: generic clone and forest consumers.

The new source boundaries are control/model emission, protected-tree control
preflight, clone-control fixup and the schema/validation/membership/state/reference
phases under `runtime/program/lists/traversal/copy/`. Their public/internal headers
also have standalone compilation entries in the inventory. The copy and generated
execution suites run with ASan/UBSan; whole-project strict tests and reproducible
Doxygen checks complete the task evidence.
