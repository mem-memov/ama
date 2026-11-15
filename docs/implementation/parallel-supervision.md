# Isolated execution, messaging and explicit supervision

T106 composes the implementations of
[PAR-01–PAR-08](../../spec/language/async-and-parallelism.md#par-01).
The [two-actor example](../../examples/parallel/README.md) executes real generated
Ama on two POSIX threads and checks two different policies for the same peer
failure. The [native-worker example](../../examples/async-host/README.md) separately
checks the supported dynamic-library boundary, callback lifetime and unload.
Neither example grants native callbacks access to Ama state.

## Parallel actor experiment

`make -C tests/unit/codegen/parallel-supervision` shares maintained Ama task and
native bridge inputs with [channel waits](channel-waits.md). Only its host policy
and native observations differ. Exact producer model names bind every generated
interface; the test never infers bindings by inspecting generated implementation.

During serialized setup, the host moves distinct native ports into two fresh
contexts, fills capacity, and lets the receiver take one message. The producer
remains suspended at its explicit wait. Each context is then assigned to its own
worker. Two native entry callbacks rendezvous before either proceeds, proving
simultaneous context admission without timing guesses. The receiver may suspend
again; the supervisor rechecks native readiness under its condition-variable
mutex before sleeping. A sender dispatch return signals that same condition.
Readiness and ownership remain separate: only take transfers message bytes.

Three runs verify normal delivery, continuation after receiver failure, and
explicit peer cancellation after that identical failure. The receiver failure
cleanup detaches its operations and aborts its port under the normal contract. The
supervisor then either returns false to the sender's cancellation checkpoint,
which observes ordinary aborted-delivery data, or explicitly requests cancellation.
The surviving context completes normally in both cases. Full, technical failure,
ordinary delivery failure and cancellation have independently checked outcomes.

The host joins both workers before transferring execution ownership back for
cleanup. Every dispatch uses the context's ordinary exclusive admission path.
No helper C activation spans an Ama yield. Native event observers own only
native rights, and atomic allocation counters check all actual release credits.
After both contexts and observers disappear, only the declared process-lifetime
event-image identity receipt remains. Concurrent trace ordering is intentionally
unspecified; typed value and ownership checks remain active in release builds.

## Native dependency audit for the example

| Dependency | Sharing, synchronization and lifetime |
| --- | --- |
| Ama pools, constants, selectors and failure | Private context allocations; identical module types share immutable schema/code only. |
| Factory role | Serialized host setup copies a scalar into each native probe. Setup configuration stays immutable while workers run; it never selects the interpreter's current instance. |
| Channel queue and wait slots | Native service gate; one sender and one receiver, complete typed envelope ownership, no borrowed Ama buffers. |
| Event endpoints | Native service synchronization and full token authentication. Observer replacement is confined to the receiving executor; producer observation is immutable during concurrency. |
| Supervisor rendezvous and wake | POSIX mutex/condition over native scalar state; predicate and sleep use the same mutex. Callbacks retain no Ama view. |
| Allocation accounting | C11 atomic counters; both malloc and compiler-folded calloc are counted. |
| Text/integer decoding | Fresh receiving-context ownership; immutable text sharing never crosses contexts. |
| Executor affinity | Permanent workers during execution; setup/teardown transfers only at rest. All dependencies here allow that transfer. |
| Creator code | The channel implementation is resident in this executable. Separate dynamic creator-code credits and unload barriers are exercised by the native-worker example. |

The argument composes disjoint mutable Ama storage with separately synchronized
native services. An operation may access its own admitted context and native
rights only. Whole-message transfer creates independently owned receiver data;
induction over dispatch, enqueue, take, yield and cleanup preserves that partition.
Supervisor decisions contain no reference to another context's fields. Therefore
one peer's terminal transition cannot execute or invalidate another peer except
through the explicit endpoint and host policy transitions.

## Acceptance matrix

The suites below have different responsibilities; passing a thread stress test
alone is not the isolation proof.

| Requirement | Executable evidence |
| --- | --- |
| PAR-01 context-local storage and simultaneous lifecycle | `tests/unit/runtime/isolation`: distinct pools/constants, concurrent native construction/destruction, equal local IDs rejected across origins, reuse without revival. |
| PAR-02 one executor and transfer at rest | `tests/unit/codegen/executor-transfer`: active/inspection-lease rejection, synchronized transfer and retained state, explicit thread-affine payload. |
| PAR-03 independently owned typed messages | `tests/unit/async/messages`, `tests/unit/codegen/message-codecs`: exact schema, malformed data and failed decoding prefixes, receiver-owned fields. |
| PAR-04 bounded FIFO and shutdown | `tests/unit/async/channels`, `tests/unit/async/channel-waits`, `tests/unit/codegen/channel-waits`: count/bytes, cancellation, late notification, sender drain and receiver abort. |
| PAR-05 actual native concurrency | `tests/unit/codegen/parallel-supervision`, `tests/unit/codegen/parallel-contexts`: real overlapping executors; native callbacks only publish records. |
| PAR-06 explicit wider policy | `tests/unit/codegen/parallel-supervision`: one receiver failure with continue-peer and cancel-peer decisions; `tests/unit/library/supervision`: explicit child-report versus parent-failure decisions. |
| PAR-07 explicit generated context | `tests/unit/codegen/parallel-contexts`: same module types, private state, native completion while peer executes, failure isolation and reuse. |
| PAR-08 separately generated namespaces and C boundaries | `tests/unit/library/header`, `tests/unit/library/execution`, `tests/unit/library/nested`, `tests/unit/library/supervision`: independent headers/archives, full-key collision rejection, typed foreign-handle rejection, nested control ownership and normal C returns. |
| Creator code after context teardown | `tests/unit/library/async` and the supported native-worker example: late completion, cancellation/restart, code credits, independent snapshots and actual unload/reload. |

## Separately generated components and nested children

A component namespace does not authorize cross-context field access. Public
transport copies declared data; a native child wrapper owns an opaque checked
foreign-context control resource under EMBED-07, not an ordinary Ama reference.
The [nested adapter](c-library-nested.md) is noncopyable and has sole authority
over its child. Its child yield returns normally through C before a parent may
yield. The [nested supervisor](c-library-supervision.md) checks report ownership,
explicit reset/propagation and cleanup after failures, including code credits.

For a parallel parent containing children, extend the dependency audit recursively
to every reachable child component and native constructor, entry, clone, destructor,
reporter and allocator. The parent's exclusive lease alone does not synchronize
a shared child reactor or make a thread-affine child transferable. A quiescent
transfer is permitted only if all owned native resources allow the new executor;
otherwise keep the complete ownership tree on its original executor. Child code
credits and exported report owners remain independent unload obligations.

The nested acceptance fixtures use shared native observer counters under a
serialized test-host contract. They are not evidence that those particular
observer objects may run concurrently. Their namespace/control/lifetime checks
combine with the explicit native dependency preconditions, not a claim of
unrestricted parallelism for arbitrary native adapters. The two-actor example
uses independently audited synchronized dependencies instead.

## When processes are required

Native providers must document synchronization and affinity for their entire
lifecycle. For unsupported shared mutable dependencies, the host must serialize
all relevant use or place the affected contexts in separate processes. Merely
locking one entry method leaves constructors, cleanup, callbacks and unload
unprotected. No compiler scan or guessed heuristic certifies arbitrary C.

Context ownership does not contain native memory corruption, `abort` or process
termination. If the application requires survival of those failures, choose process
isolation and an explicit interprocess transport/supervisor. The current native
in-process channel SDK is not an interprocess queue. This is a stated operating
boundary, not an automatic fallback or new language syntax.

## Reproduction and checks

Run the public native-worker recipe in a fresh output directory and the two-actor
Make target linked above. The actor leaf supports strict release checking,
ASan/UBSan and TSan; it instruments both the compiler composition driver and the
actual generated executable. Assertions required by the consumer stay effective
under NDEBUG. Shared fixture extraction is checked by rerunning the original
channel-wait suite. [ready.json](documentation/ready.json) maps maintained files
to actual named cases; Doxygen checks source/spec destinations and reproducibility.
