# Resource and failure-boundary acceptance

T119 reviews failure behavior across real ownership, native value, asynchronous
and filesystem handoffs. A passing normal path is insufficient: each boundary
must preserve exactly its promised old state, retire every unpublished owner and
avoid turning a technical failure into a successful value or resumable state.
The [current conformance matrix](conformance/README.md) links these arguments and
named tests to the canonical requirements.

## The preservation boundary

[COPY-02](../../spec/language/values-and-references.md#copy-02) and
[LIFE-04](../../spec/implementation/native-interface.md#life-04) divide replacement
into private preparation and destructive commit. Before commit, the old value,
source graph and visible identities remain unchanged. Once old destruction
starts, rollback is not promised: the context becomes terminal and remaining
old/prepared resources must be finalized once. Earlier independent field/output
commits and external effects do not become part of a whole-call transaction.

The implementation records completed construction prefixes and explicit owner
edges. Each successful native payload construction contributes one destroy
obligation; a failed constructor releases its own partial native work and adds
none. Reverse-prefix cleanup therefore consumes each obligation once. Checked
identity maps and source/destination preflight prevent partially published clones,
ancestor overlap or destruction of busy descendants from bypassing that boundary.
See [ordered construction](ordered-construction.md),
[clone preparation](clone-preparation.md) and [replacement](staged-replacement.md).

## Concrete verification scales

| Boundary | Independent and composed acceptance |
|---|---|
| Ownership and acquisition | `tests/unit/runtime/ownership-model` checks all 104976 length-four words over 18 declared actions against an independent model after every prefix, then injects every measured allocation failure in a longer mixed construction/clone/replacement/acquisition trace. The complete runtime parent adds private construction, preflight, reconstruction, interface/result, whole-list and context consumers. |
| Nonwrapping identity | Pool, membership, context, collection and native-event tests use reduced identity frontiers, retire exhausted records and retain old tokens across deletion/reuse/reclamation. Exhaustion refuses before publishing a successor; no practical-width assumption stands in for the invariant. |
| Arithmetic and immutable values | `tests/unit/native/abi` checks host-size bounds, malformed literals, independent clone failure, arbitrary-precision carries and fixed-width checked transport. Runtime text tests cover share-count exhaustion, both destruction orders, aliased replacement and rollback after acquiring an earlier immutable share. |
| Generated native cleanup | `tests/unit/codegen/continuation-cleanup` compiles and executes actual native resources and generated continuation transfers. Failed clone prefixes retain previous values; destructive errors finalize failed contexts, preserve the primary error and do not become yield/admission outcomes. |
| Application overflow/failure values | `tests/unit/codegen/library-compositions` runs actual library arithmetic and failure modules, retains old results on refusal, clears previous descriptions on later success and cleans partial shared-list startup before root publication. Expected arithmetic failure remains ordinary data; allocation/lifecycle failure remains technical. |
| Events and channels | The complete `tests/unit/async` parent checks callback-owned native records, full submission identity, synchronous/racing completions, cancellation/close/abort, exact buffer ownership and all measured allocation prefixes. A callback never receives Ama execution authority; late completion retains its required native storage/code credit until retirement. |
| Relocation preparation/publication | The complete `tests/unit/tools/relocation` parent injects measured allocation and filesystem refusals through discovery, staging, exclusive leases, preparation, application and recovery. Real child exits at durable checkpoints abandon executor memory; newly decoded disk authority recovers the operation repeatedly without lost originals or duplicate rewrites. |
| Public command failure observation | `tests/integration/products` executes a controlled native technical failure and verifies original-source traces. Its fourth C case deliberately signal-terminates owned compiler/program substitutes and requires the actual process observer to reject both as crashes, rather than counting any nonzero exit as a managed technical result. |
| Output failure after commit | `tests/unit/driver/relocation` refuses writes to the result stream after a real committed move, requires the failure report to preserve the committed outcome and recovery receipt, and recovers that receipt. It also checks explicitly unknown recovery outcomes and every measured command-preparation allocation refusal. |

Commands are independently runnable with `make -C` followed by the indicated
directory. The live [file/case registry](documentation/ready.json) retains exact
named cases and leaf commands, including supporting headers and fixtures.
The runtime and async parents provide meaningful component and composition
checks; generated-program leaves add the actual target C execution boundary.

## Why the failure enumeration is finite and complete for its scope

Allocation and I/O fault tests first measure a successful finite operation, then
refuse each observed ordinal independently. The expected oracle checks the
public state and remaining ownership, not just an error code. This establishes
every required preparation-prefix failure for that scenario; it does not claim
to enumerate all possible programs or arbitrary external-library behavior.
The bounded ownership model is separately exhaustive over its declared action
alphabet and depth, with invariants checked after each action rather than only
at final cleanup.

For asynchronous work, a visible registration plus immutable service/submission
identity determines delivery rights. Detachment removes those rights before
Ama storage can disappear. Separately owned native callback records remain valid
until acknowledgment, so a late callback cannot acquire a new right merely by
observing a reused address. Duplicate/cancelled/restarted events and independently
retained byte/code owners test this law at actual thread and lifecycle boundaries.
See [native events](native-events.md) and [task detachment](task-detachment.md).

Relocation uses a durable decision boundary rather than process-local cleanup
alone. Before a committed decision, recovery restores original paths and bytes;
after it, recovery completes cleanup of the published state. Interrupted recovery
itself must be repeatable. If required recovery I/O fails, saved authority remains
available and the command cannot report an unqualified successful rollback.
See [transaction publication](relocation-transaction.md) and
[recovery](relocation-recovery.md).

## Instrumentation and limits

The acceptance run uses strict C warnings and assertions. Runtime, native ABI,
async and selected generated-program checks also run with AddressSanitizer and
UndefinedBehaviorSanitizer. Generated cleanup/library fixtures propagate those
flags to their generated executables, not merely the compiler-side harness.
LeakSanitizer is unavailable in this execution environment; `detect_leaks=0`
is explicit. Counted allocation, native-owner, callback, buffer and code-pin
assertions provide the corresponding tested lifetime checks without pretending
that a disabled leak detector ran.

Tests never rely on a crash as an accepted failure, on guessed sleeps for event
ordering or on a large identity width making reuse unlikely. Native destructor
failure still cannot promise that an external operating-system resource was
released: [PROTO-10](../../spec/implementation/native-interface.md#proto-10)
requires reporting its disposition and continuing cleanup without a blind retry.
Arbitrary native undefined behavior and process-wide termination remain outside
managed Ama failure semantics. T120 separately records the full supported
toolchain and sanitizer acceptance.
