# Native lifecycle bookkeeping and failure model

This validates section 2's lifecycle task under LIFE-01 through LIFE-03 and
PROTO-08 through PROTO-10. `src/runtime/lifecycle/` implements the reusable ledger
and primary-error combination; the independently runnable model composes it with
real native allocations. Generated ownership layouts, replacement commit and
whole-program shutdown remain later integration tasks, not claims made here.

## Domain and state

`AmaLifetime` is runtime-owned metadata, never opaque payload. A fresh ledger is
zero-initialized with no resources. Its declared child capacity is a size_t count;
`live_children` is the initialized declaration-order prefix. A constructor kind
is exactly Default, Literal or Clone. All pointers supplied to the ledger APIs
are valid borrowed runtime storage; native code cannot modify them.

Invariants are `0 <= live_children <= child_capacity`, no live native payload
while Constructing, and `cleanup_active => phase == Finalized`. Finish is legal
only after all children are ready. It records whether a native payload exists;
Ama-only storage has none. Invalid transitions return InvalidState without mutation.
The ledger does not execute callbacks or allocate resources.

Construction selects one kind before acquiring resources. Each successful child
increments the prefix once; the capacity check prevents overflow. A successful
native constructor is followed by finish. On constructor failure, do not finish
or mark its payload live: that hook must release its own partial resources.

## Cleanup transitions and proof

Begin cleanup accepts Live or partially Constructing state and marks it Finalized
before releasing resources. The enclosing pool/access layer must also withdraw
external access before cleanup; that layer is implemented later. Repeating begin
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
three separately declared children, all with real malloc/free resources. It checks:

- All three root constructor paths for child counts zero through three.
- Every child/root failure ordinal for each constructor path, including failure
  after acquiring resources, with no runtime destroy call for the failed payload.
- All 16 combinations of four cleanup-error flags, continuing every release.
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
