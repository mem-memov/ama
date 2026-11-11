# Stable instances and ownership operations

## Pool identity algebra

A per-executable-type pool issues `(type, slot, generation)` handles. Generation
zero is empty. Reservation is private construction storage; publication requires
a live lifetime ledger. Identity validation and activity checks are separate.
No pool or spare capacity creates an implicit instance.

Blocks contain fixed-stride, max_align_t-aligned metadata/payload slots. Directory
growth copies block pointers, never payloads. Each free nonretired slot occurs once
in the intrusive free chain. Release requires invalidated, fresh/finalized storage,
advances generation without wrapping, and retires an exhausted slot. A configurable
nonzero generation limit permits exhaustive reduced-width tests. The supported
payload layout requires alignment no greater than max_align_t; generated layout
consumers must check their concrete layout against this contract.

Allocation failure publishes no reservation or new block. A successfully enlarged
directory may remain as spare capacity after payload allocation failure. Pool
storage disposal is a low-level storage operation: the ownership layer must first
finalize resources. Handles never survive disposal/reinitialization of a runtime.

For B historical blocks and S slots per block, reserve/release/lookup are O(1)
except amortized directory growth and O(S) block initialization. Directory memory
is O(B); payload memory is proportional to allocated blocks. Arithmetic is checked
before allocation, and SIZE_MAX is reserved as the free-chain sentinel.

[Pool tests](../../tests/unit/runtime/pool/main.c) cover stable addresses over many
growth steps, every issued historical handle with a three-generation limit, empty
and mismatched handles, and both growth allocation failures.

## Reclamation

Reclamation finds the last occupied block, rebuilds the free chain using only
retained blocks, then releases free blocks after that boundary. Empty historical
directory entries remain null permanently. Growth appends new slot identities;
it never fills historical directory holes. This permanently retires the released
identities, including unissued slots, and therefore preserves stale-reference
rejection without allocating a per-slot tombstone table. The small directory
history remains O(B) in lifetime block count. Reclamation is O(B*S), allocates
nothing, and cannot move a live payload or reclaim a partly occupied block.
Tests repeat reclaim/regrow with reduced generation limits and verify all old
handles as well as the address of a retained instance.

## Owned construction and cleanup

Generated object descriptors borrow declaration-ordered scalar/list ownership and
reference field offsets and native hook adapters. Descriptors have disjoint,
aligned handle fields before the native payload and a unique nonzero type ID in
the world's borrowed registry. Recipes are finite construction trees; a missing
field recipe selects the descriptor's defaults. Lists default empty, references
empty. Recipe counts/buffers and actual source pointers are trusted generated C
inputs, not user-provided arbitrary memory. The backend must enforce descriptor
alignment and descriptor/recipe lifetimes. Source-level initializer evaluation
will supply these recipes; this layer never evaluates Ama expressions or bodies.

Construction reserves private slots, constructs owned fields in declaration order,
then selects one native constructor. An explicit work chain replaces recursive C
traversal. Each completed child extends the lifetime ledger's prefix by one;
failed child/payload construction never enters that prefix. No handle is published
until the whole root succeeds. Failed payload hooks must release their own partial
native resources. Cleanup walks parent links and decreasing prefix indices without
allocating memory: invalidate, consume native-live exactly once, destroy that payload,
then reverse children and list buffers. Original status wins; every secondary
cleanup error is reported, and a cleanup failure makes the world terminal for
new operations. Storage disposal follows resource finalization.

Native callbacks here are lifecycle adapters, not Ama module dispatch or entry
execution. Reentry into object operations is refused. These descriptors are an
engineering runtime boundary, not a replacement for the typed plan or final
emitter. Whole-source cloning, replacement and complete-subtree acquisition are
layered on this ledger. Clone construction uses source recipes rather than running
declaration defaults first.

For a finite tree with V instances and E owned/reference elements, traversal is
bounded by V and field/element scans; linear descriptor lookup and repeated
flattened-child scans give a conservative O((V+E)^2 + V*T) bound. Construction
workspace is O(V), list storage O(E); cleanup workspace is constant and follows
runtime parent links. No recursive C walk or unbounded emergency allocation is
required. [Lifecycle tests](../../tests/unit/runtime/objects/lifecycle/main.c)
exercise declaration order, selected literal construction, every failed prefix,
secondary errors, list buffers and all allocation failures over nested trees.
