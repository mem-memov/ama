# Native lifecycle bookkeeping and failure model

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This records the tested lifecycle component under LIFE-01 through LIFE-03 and
PROTO-08 through PROTO-10. `src/runtime/lifecycle/` implements the reusable ledger
and primary-error combination; the independently runnable model composes it with
real native allocations. The [ownership layer](instance-ownership.md) and [context owner](execution-contexts.md)
compose it with actual pools, replacement and shutdown. Current-language and
public embedding consumers remain separate roadmap work.

## Domain and state

`AmaLifetime` is runtime-owned metadata, never opaque payload. A fresh ledger is
zero-initialized with no resources. Its declared child capacity is a size_t count;
`live_children` is the initialized declaration-order prefix. A constructor kind
is exactly Default, Literal, Clone or the fixed integer Size bridge. All pointers supplied to the ledger APIs
are valid borrowed runtime storage; native code cannot modify them.

Invariants are `0 <= live_children <= child_capacity`, no live native payload
while Constructing, and `cleanup_active => phase == Finalized`. Finish is legal
only after all children are ready. It records whether a native payload exists;
Ama-only storage has none. Invalid transitions return InvalidState without mutation.
The ledger does not execute callbacks or allocate resources.

Construction selects one kind before acquiring resources. Each successful child
increments the prefix once; the capacity check prevents overflow. A successful
native constructor is followed by finish. Raw nonzero native completion does not
finish or mark its payload live: that hook releases its own partial resources.
[T009 checked native completion](native-protocols.md) credits a raw zero-code
payload before rejecting malformed message transport, so private unwind can still
release the completed native resource.

## Cleanup transitions and proof

Begin cleanup accepts Live or partially Constructing state and marks it Finalized
before releasing resources. The enclosing pool/access layer must also withdraw
external access before cleanup; the retained ownership layer does so before hooks. Repeating begin
is invalid. Taking the native cleanup obligation clears its flag before invoking
the hook. Next-child selection is forbidden while that obligation remains; then
it decrements the prefix and returns the removed last index. End requires no
remaining obligation and clears `cleanup_active` without making storage live again.

Each native obligation can be taken at most once. The child rank decreases from
k to zero, yielding k-1 through zero exactly once. Therefore cleanup is native
first, then reverse declaration order, without omission or double destruction.
Hooks may fail, but the caller continues selecting obligations. Induction on a
finite ownership tree extends this order to each node's generated cleanup.
No non-owning reference is traversed. Reusing a pool slot needs a new generation
and fresh ledger; it is not reopening a finalized value.

`ama_cleanup_primary(a,b)` returns a when a is already nonzero, otherwise b.
Induction over reported statuses preserves the first failure. The caller reports
each secondary failure as it arrives; accumulation itself needs no allocation or
unbounded error list. After destructive cleanup there is no rollback, retry or
use of partly destroyed payload. The fixture logs a bounded set solely for tests.

Ledger operations use O(1) time/space. Traversing k children costs O(k) plus native
hook costs; the decreasing prefix proves termination if those hooks terminate.
Generated cleanup of a finite tree visits each owned node once. The current
ledger stores counts only, not payload pointers or a hidden control stack.

## Executable evidence

Run `make -C tests/unit/native/lifecycle/model`. The model uses a root plus up to
three separately declared children, all with real malloc/free resources through
a counted fail-at-N allocator. Every completed scenario asserts zero live
allocations, including the clone path’s temporary source. It checks:

- All four root constructor paths for child counts zero through three.
- Every child/root failure ordinal for each constructor path, including failure
  after acquiring resources, with no runtime destroy call for the failed payload.
- All 16 combinations of four cleanup-error flags for each constructor, continuing every release.
- Reverse cleanup and preservation of construction failure over secondary errors.
- Repeated cleanup rejection, invalid ordering/transitions and Ama-only storage.

This bounded model validates actual ledger/companion composition. The invariant
and rank arguments cover arbitrary representable child counts, conditional on
correct generated child ownership and native hook contracts.

The model and the six-binding resource harness also pass ASan/UBSan with
`ASAN_OPTIONS=detect_leaks=0` and `-fsanitize=address,undefined` (non-PIE binaries in
this environment). LeakSanitizer itself fails under the environment's ptrace
supervision and was not certified; its failure is not presented as a successful
leak check. Address/undefined-behavior instrumentation remained enabled.

## T011 transition and real-pool composition evidence

The ledger requires no additional execution state: Fresh admits one constructor
selection; Constructing credits exactly the completed child prefix; Live admits
cleanup; Finalized never becomes live again. Cleanup-active distinguishes an
in-progress finalization from a completely consumed one. Publication and pool
access are separate state, so a temporarily Live private constructor result is
not automatically externally accessible. The generated owner supplies valid
exclusive ledger access, without allowing native authors to alter this metadata.

T011 extends the finite model to Default, Literal, Clone and Size. The fixture's
bounded size constructor is its own path; the full arbitrary-precision production
integer behavior remains the [T009 protocol contract](native-protocols.md) and
T085. Each path covers every child/root construction failure boundary for zero
through three children, successful cleanup, all 16 destructor-failure subsets for
four payloads and repeated cleanup rejection. Different failing destructor codes
are observed in actual cleanup order; an earlier construction code, immutable
message pointer and explicit length survive them unchanged. Failed constructors
self-clean and receive no runtime destructor credit. Each successful path
asserts its exact allocation count, excluding default-then-overwrite. A separate
case fails every actual allocation for every constructor and child count,
including clone-source creation, and proves no counted allocation remains.

The transition case checks rejected construction, publication, child completion
and cleanup operations in Fresh, Constructing, Live, active Finalized and consumed
Finalized states. Every rejection preserves all logical fields; the test does not
compare unspecified C padding. The SIZE_MAX case instantiates a valid reachable
prefix at SIZE_MAX-1 rather than iterating that many times: one credit reaches
capacity, the next is refused before overflow, and reverse selection returns
SIZE_MAX-1. It owns no actual children. For arbitrary capacity C the invariant
`0 <= k <= C` makes the accepted increment `k+1 <= C <= SIZE_MAX`, proving the
complete numeric domain independently of finite resource tests.

`tests/unit/runtime/objects/lifecycle/` composes the ledger with actual pool slots,
address-sensitive resources and generated-style hooks. A new observer checks
that public identity is invalid and native credit consumed before each destroy
callback. During parent cleanup its declared child payloads remain Live with
resources intact. The world operation gate refuses a nested destroy; callbacks
cannot start duplicate cleanup. Every destructor-failure position still releases
the whole tree in parent/native then reverse-child order; a later destroy sees
a stale reference and invokes no hook. Existing allocation-failure and nested-list
cases exercise all measured allocation ordinals and assert zero counted resources.
These are internal component/composition tests; current source integration and
public C embedding remain later tasks.

T011 acceptance passes the full strict `-DNDEBUG -Werror` tree, focused model and
real-object ASan/UBSan leaves (leak detection disabled), complete documentation
coverage of 741 maintained inputs and all Markdown links. Warning-free Doxygen
checks 571 maintained files, 2365 interfaces and 286 canonical/page destinations;
two complete generations produce byte-identical output.
