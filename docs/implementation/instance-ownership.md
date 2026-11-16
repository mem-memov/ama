# Stable instances and ownership operations

## Pool identity algebra

A per-executable-type pool issues `(type, slot, generation)` handles. Generation
zero is empty. Reservation is private construction storage; publication requires
a live lifetime ledger. Identity validation and activity checks are separate.
No pool or spare capacity creates an implicit instance.

Blocks contain fixed-stride, explicitly aligned metadata/payload slots. Directory
growth copies block pointers, never payloads. Each free nonretired slot occurs once
in the intrusive free chain. Release requires invalidated, fresh/finalized storage,
advances generation without wrapping, and retires an exhausted slot. A configurable
nonzero generation limit permits exhaustive reduced-width tests. The basic initializer defaults to max_align_t; the aligned initializer accepts
the concrete generated C alignment, including extended native alignment. Blocks
retain their allocation base and align the usable address inside checked padded
storage. Generated consumers must pass `_Alignof` of their actual layout.

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

## T046: stable typed allocation and publication

The [pool API](../../src/runtime/pool/pool.h) implements the storage boundary of
[RT-02](../../spec/implementation/runtime.md#rt-02) and
[RT-12](../../spec/implementation/runtime.md#rt-12). T046 verifies its existing
implementation with live instances, rather than substituting an additional pool
or execution-state copy. Generation/reclamation invariants are tested below,
and [construction expressions](ordered-construction.md) preserve the reserved
final identity through initialization.

Within a materialized block, `cell = block + offset * stride` is fixed for its
lifetime. Only the directory array can relocate on growth. Therefore induction
over successful reservations preserves every existing cell/payload address:
free-list removal changes only unused metadata, while growth appends disjoint
blocks without changing previous pointers. Publication changes visibility, not
address or identity. No default instance is created by initializing a pool or
by obtaining spare block capacity.

Reservation removes exactly one free identifier before returning it, retaining
its final type/slot/generation. If the free chain is nonempty, reservation performs
no allocation. Release advances the generation and returns only nonretired unused
slots to that chain. Its ordering is private; the test checks membership and
uniqueness, without assuming lowest-address or LIFO semantics. Resetting reused
storage clears both payload banks and transient controls. Other occupied cells
retain their own payloads and saved positions.

Private reservation is not public acquisition authority. Normal lookup refuses
unpublished cells, and instance access refuses a nonlive identity for entry,
resume and read. Publication requires a completed Live lifecycle ledger and
rejects duplicate publication; a live identity cannot be released. The trusted
construction path can use the final reserved identity before publication without
making uninitialized contents externally accessible. Source-level self-reference
initializers and owned-child construction are verified by their later integration
tasks, rather than claimed by these direct pool tests.

Growth allocation failure leaves handle/cell output parameters untouched and
preserves existing identities and payloads. A directory enlargement may remain
after the following payload allocation fails; directory capacity is not a live
instance count or a published block. Retrying is valid without rolling back that
spare capacity. Checked arithmetic and alignment remain covered by the existing
alignment leaf. The bounds above apply to storage operations; no native hooks or
module bodies execute here.

Run `make -C tests/unit/runtime/pool`. New named cases check 33 live same-type
instances at block sizes 1, 2 and 7; reuse of 16 holes with the next allocation
forced to fail; final private identity and lifecycle/publication gates across two
nominal types; and failure of each later directory/payload growth allocation with
live earlier blocks. Existing private-address growth and reduced-generation cases
remain in the same leaf. The runtime parent composes this with real object
construction/publication, aligned native resources and invocation consumers;
address/undefined-behavior sanitizer checks supplement counted allocation cleanup.
The [coverage registry](documentation/ready.json) identifies each touched source,
header, fixture and build file and preserves standalone header checks.

## Reclamation

Reclamation rebuilds the free chain from nonretired unused slots of blocks
that still contain an occupied slot. It then releases every entirely free block,
including blocks before later live or constructing instances. Free links therefore
exclude all reclaimed storage before the first release; live payloads never move.
Absent historical directory entries remain null permanently. Growth appends new
slot identities instead of filling old holes. This permanently retires released
identities, including unissued spare slots, without a per-slot tombstone table.
The directory retains O(B) entries for B lifetime blocks. Reclamation takes
O(B*S) time for S slots per block and constant workspace, with no allocation.

The invariant is that the free chain contains exactly the nonoccupied,
nonretired slots in materialized blocks, once each. Rebuilding it without empty
blocks removes all links to soon-absent storage. Each subsequently freed block
has zero reservations, so it owns no live or constructing cell. Checked lookup
rejects absent blocks before reading a cell. Permanent retirement of absent slot
ranges and strictly increasing generations within retained ranges make every
previously released handle stale after any future growth. Block count is a
historical identity extent; directory capacity, allocated block count, occupied
reservations and published live count remain distinct quantities. Upper layers
retain occupied identities throughout active execution/protection and may not
release their resources while borrowed.

T047 tests a middle and first block disappearing while later live/private cells
retain exact addresses and values. A retained partly occupied block reuses its
free slot before growth. Both directory and payload allocation failures after
sparse reclamation preserve all live objects and allow retry. Repeated reclamation
is idempotent. Existing reduced-generation tests verify permanent retirement and
staleness over repeated reclaim/regrow cycles.

The finite model exhausts all 5^6 length-six schedules for each block size in
1..2 and generation limit in 1..3: 93,750 schedules, with every prefix checked.
The five actions are reserve, publish oldest private, release oldest occupied,
release newest occupied and reclaim. An action with no eligible identity is an
explicit no-op. An independent issued-identity ledger computes occupancy,
publication, historical generations, expected sparse holes and the exact free
set. It checks every previously issued handle and every live address after each
operation. The free-chain walk checks uniqueness and completeness against that
set, without assuming an implementation order. This is exhaustive for the stated
finite domain; the general invariant argument supplies the unbounded reasoning.
Run `make -C tests/unit/runtime/pool`; the runtime parent adds object/lifecycle,
list and alignment consumers, with sanitizer runs and counted allocation cleanup.

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

## Clone boundaries and reference algebra

Before cloning, protect the complete source ownership tree and verify each native
payload has its independent clone capability. Availability follows ownership only;
an external referenced target may be running, stale or empty without affecting
copying its handle. Source protections survive every native callback and unwind
on all errors. Native hooks cannot bypass this by reentering object operations.

Each clone boundary records exact source identities and fresh destination slots.
Reservation uniqueness and exclusive ownership make that map injective. Fixup is
`f(h) = map(h)` for identities in the boundary and `f(h) = h` otherwise. The root
participates, as do all owned list elements; references are not traversed for
cloning. Fixup runs after the entire map exists and before publication, covering
forward/ancestor/cyclic references and reference-list entries. Child construction
uses the source shape and clone hooks, never declaration defaults. A recipe may
select source cloning directly; repeated source initializers get separate clone
boundaries, so each copy's self-references select its own instance.

Ordinary clones clear saved positions throughout the tree. Continuation recipes
and explicit continuation cloning preserve each instance's own saved position but
never copy return links, acquisitions or activity flags. The metadata clone
operation builds fresh controls from the protected source's prior available state.
Failure finalizes only prepared resources and restores source protections; output
handles remain untouched. A conservative O((V+E)^2 + V*T) bound follows from finite
work records, linear identity lookup and reference-map lookup, with O(V+E) temporary
storage. [Clone tests](../../tests/unit/runtime/objects/clone/main.c) cover all forms,
internal/external boundaries, independent native allocations, saved positions,
busy/unsupported sources and every preparation allocation failure.

## Replacement without relocating native objects

Each stable pool slot reserves two data banks with the declared C alignment. One is active;
replacement prepares the new root directly in the destination's inactive bank.
Its private staging slot supplies only identity/lifetime bookkeeping and points
to that bank during preparation. New children occupy independent pool slots.
The clone map uses the final destination identity for the source root from the
start. Old destination data and its saved position remain untouched on any
preparation failure.

After complete preparation, finalize old contents without releasing the root
identity, switch the active bank, transfer the prepared ledger/control state,
reparent direct children and publish. Only bookkeeping and declared handles are
transferred; live native payload bytes are never memcpy-moved. This supports a
native payload containing a private pointer into its own stable representation,
without adding a move hook or per-instance heap allocation. Borrowed views must
end before a conflicting replacement; growth never changes either bank's address.
The policy costs twice the declared data bytes per slot, plus alignment padding.

Commit destruction failure is terminal: finish old cleanup and prepared cleanup,
leave the old identity inaccessible, and keep a finalized bookkeeping tombstone
until disposal. Neither partly destroyed old data nor prepared data is published.
No rollback of destruction is attempted. Disposal of the tombstone performs no
additional native destruction. Complete source/destination protections span the
transaction. [Replacement tests](../../tests/unit/runtime/objects/replacement/main.c)
exercise repeated bank switches, self-address-sensitive payloads, stable root
identity/internal references, every preparation allocation failure, every old
cleanup failure and failed prepared native construction. Complexity and temporary
space are the clone bounds plus two bounded cleanup traversals.

## Availability, overlap and terminal disposal

Normal destruction and replacement protect the complete affected old ownership
subtree before the first destructor runs. Source and old destination protections
remain active through callbacks and commit. Running, waiting or incompatibly
acquired descendants cause refusal even under an available root. Protection
records save/restore prior controls on every non-destructive failure. Repeated
initializer copies may reuse this operation's own compatible source protection;
an exact per-pass epoch distinguishes that reuse from duplicate visits. Epoch
exhaustion is an explicit failure, never wrapping. No reference edge extends a
protection or destruction boundary.

Before a distinct replacement, follow owner links in both directions to reject
ancestor/descendant overlap, including ownership through list elements. Each walk
is bounded by the exact occupied-slot population: exceeding it proves a cycle
by the pigeonhole principle. This is not a guessed maximum depth. Siblings are
disjoint even if their common owner is running; non-owning links between them do
not create overlap. Ordinary runtime-identity self-assignment returns without
allocation, cloning, destruction or resume reset after checking root access. It
does not preflight descendants or require clone capability. Continuation self-copy
is rejected. Copyability also checks the complete descriptor ownership relation,
including empty owned lists. Exact reachability of a native type without a clone
hook rejects the copy; reference types do not propagate that requirement. The
finite monotone traversal handles recursive list types without recursion.

After a terminal failure, host disposal is a distinct recovery use of destroy:
first an allocation-free parent-link traversal clears all affected acquisition,
return-link and resume controls, then cleanup finalizes the ownership tree. It
accepts the finalized private tombstones left by failed nested replacements and
never destroys their payload again. The host must stop Ama execution and report
its diagnostic path before this disposal; it is not asynchronous cancellation or
a way to delete a busy instance during normal execution. New operations remain
forbidden. The world registry/roots are host-owned; dropping an owned subtree
requires its owning location to be detached by the caller. Never dispose a child
separately while leaving it in an owner's live-child prefix.

Owner checks cost O(V*T) plus finite pool-population enumeration; descriptor
capability closure has conservative O(T^2*F) cost using linear type lookup, with
O(T) workspace. Protection and ledger traversal retain the prior tree bounds.
[Access tests](../../tests/unit/runtime/objects/access/main.c) enumerate each node
of nested source/destination trees in running/waiting/acquired states, test sibling
versus ancestor relations, no-op/copy modes, empty-list capabilities, authorized
protection reuse, native reentry, every delete-preflight allocation failure and
failed nested replacement disposal with the next allocation forced to fail.

## Completion boundary

The pool/ownership components are independently tested and connected to the
[project assembly](project-emission.md), which supplies concrete layouts,
descriptors, recipes and explicit roots. Native authoring ABI views are
operation-scoped typed adapters, not aliases of the internal generic schema.
[Context ownership](execution-contexts.md), [source-order inference](source-order-inference.md),
[result calls](direct-result-execution.md) and [live traversal](list-traversal-execution.md)
compose these primitives through their own checked boundaries. Ordered list
storage and typed list methods remain distinct interfaces with distinct tests.

Extended alignment is tested with a 128-byte-aligned native representation,
self-address checks, repeated replacement and reclamation in the
[alignment suite](../../tests/unit/runtime/alignment/main.c). Aligned allocation
uses one checked over-allocation per block, not one allocation per instance.
Non-power-of-two target alignments and size arithmetic outside the size_t domain
are explicit errors for the verified C toolchain.
