# Asynchronous work and parallel execution

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Start with the existing continuation rules](#start-with-the-existing-continuation-rules)
- [One executor, several tasks](#one-executor-several-tasks)
- [A task reports what should happen next](#a-task-reports-what-should-happen-next)
- [Keep the scheduling relationships visible](#keep-the-scheduling-relationships-visible)
- [Bridge native events without leaking Ama references](#bridge-native-events-without-leaking-ama-references)
- [Queue discipline and progress](#queue-discipline-and-progress)
- [Cancellation, completion, and resource lifetime](#cancellation-completion-and-resource-lifetime)
- [Continuation copying and failures](#continuation-copying-and-failures)
- [Implementing an asynchronous host](#implementing-an-asynchronous-host)
- [Parallelism uses isolated execution contexts](#parallelism-uses-isolated-execution-contexts)
- [Messages cross the boundary; references do not](#messages-cross-the-boundary-references-do-not)
- [Native concurrency and failure boundaries](#native-concurrency-and-failure-boundaries)
- [Implementing and verifying parallel contexts](#implementing-and-verifying-parallel-contexts)

<!-- /chapter-contents -->

An Ama module already contains the state needed to stop and continue a
calculation. Asynchronous work adds a reason to stop: a socket is not ready, a
timer has not expired, or another task has not produced its result. Parallel
execution adds a different capability: independent calculations can execute at
the same time on different processors.

These capabilities have different safety boundaries. A scheduler can interleave
tasks that belong to one execution context. Parallel executors use separate
contexts and exchange messages; they do not concurrently enter the same Ama
instance.

This chapter defines the contracts for those facilities. It is a specification
of required behavior, not a claim that a particular compiler or library already
implements them. An ordinary Ama program still starts and executes synchronously.
It uses a scheduler only when the application or its explicitly selected host
provides one. No new punctuation, implicit task creation, or additional `.abi`
record is introduced here.

## Start with the existing continuation rules

<a id="async-01"></a>

**ASYNC-01.** **Suspension has an immediate caller.** A standalone `%` saves the
next Ama statement in the current instance and returns its results to the
immediate caller. It does not suspend the caller's remaining computation, the
whole invocation tree, or the execution context. The distinction is the same
whether that caller is another Ama module or the host at the root boundary.

For example, suppose `prepare`, `process`, and `finish` are previously declared
callable instances. This is an Ama body fragment:

```text
prepare !
%
process !
%
finish !
```

An entry-start call runs `prepare` and returns at the first `%`. The first resume
runs `process` and returns at the next `%`. The following resume runs `finish`
and completes. If an enclosing module calls this instance, that enclosing
module continues immediately after each returned call unless it explicitly
yields too.

A fresh `!` restarts the body and resets its interface under STATE-02 and CALL-07; `%` resumes
without a reset and accepts no input bindings under CONT-03. Neither operation
implicitly schedules another instance. A saved continuation has no live C stack
activation.

For clarity, this chapter calls a module instance a *task* when an application
has enrolled it in a scheduler. Being a task is a role with a library contract,
not a new Ama type category. Creating or copying an instance does not enroll it.

## One executor, several tasks

<a id="async-02"></a>

**ASYNC-02.** **Scheduling is cooperative.** An execution context has at most one
active Ama executor. That executor may run an explicitly selected scheduler,
which chooses an available task and performs one step. The first step uses `!`;
a subsequent step uses `%` only if that same instance has a saved position.

A step lasts until the task completes, yields, or fails technically. The scheduler
cannot interrupt a running Ama body or native entry. A task that never returns
prevents every other task in that context from progressing. A scheduling budget
limits the number of completed steps per round; it cannot limit the duration of
a step. Applications requiring responsiveness must divide long calculations
into finite steps and keep native operations appropriately short or nonblocking.

The runtime state named `Waiting` means that an instance is still busy waiting
for its nested synchronous invocation. It is not an asynchronous wait state.
A task waiting for an event must first return by `%` and become available in
runtime state `Yielded`. Its scheduler then records that it is waiting for an
event. No acquisition token or busy state is held throughout the event wait.

This also explains why adding another thread is unnecessary for ordinary
nonblocking I/O: while one task is yielded, the same executor can run another.
It does not make a blocking native operation nonblocking.

## A task reports what should happen next

<a id="async-03"></a>

**ASYNC-03.** **Step results are explicit.** A scheduled task has a documented,
ordinary typed result describing exactly one of these phases:

| Phase | Meaning | Scheduler action |
|---|---|---|
| Ready | The task yielded and can make another step without an event | Put it at the tail of the ready queue |
| Awaiting | The task yielded and has registered a particular wait | Leave it out of the ready queue until that wait becomes ready |
| Completed | The task completed its body | Retain or dispose of its results; do not resume it |

The task protocol also specifies the ordinary typed result of completion. That
result can represent success, a cancelled operation, or an expected operational
failure. Expected failure uses [ERROR-01](../implementation/diagnostics.md#error-01)'s
single condition-capable failure value with its contained description, not a
separate success flag and reason. Cancellation remains an explicit terminal
application outcome; its documented code/context may belong to that failure
value when the operation classifies cancellation as a failure. The concrete
module names, result names, and representation of the three
phases belong to the declared library interface. The table does not establish
new reserved words or an implicit enumeration type.

The phase and failure value have separate roles. A false failure value does not
mean the task has completed: `Ready` or `Awaiting` still describes an unfinished
step. The interface states when completion data are valid; only a published
completion may be consumed as the terminal outcome. A technical `failed` host
outcome is not a true application failure value and remains nonresumable.

The phase must be assigned on every return from a step and must agree with the
body's actual completion or yield. A task must publish an awaiting registration
before returning the corresponding phase. Expected failures such as a refused
connection or an absent file are ordinary results defined by that operation;
they are not technical execution failures.

For example, assume `task` is a previously declared task instance whose first
public result is its step phase. These are two separate Ama call fragments:

```text
task !
 > phase
```

```text
task %
 > phase
```

The result receiver follows the normal declaration and result-copying rules.
The second fragment is used only for a resumable task. It supplies no new inputs.
The example deliberately does not choose a standard-library path for a task or
phase type: this chapter defines their behavior, not an undeclared interface.

An Ama scheduler obtains the phase through that declared result. It does not
need an extra language operation exposing runtime metadata. The host-facing
execution boundary still distinguishes `completed`, `yielded`, and `failed`
under RT-14; it must not mistake the task protocol's `Awaiting` phase for a new
host outcome.

An application that needs to send data to a yielded task uses an explicitly
owned mailbox, cancellation record, or operation object. A reference to that
object is supplied through the task's declared interface and retained under the
normal ownership rules. Producers update it through its declared methods
between task steps. Resumption does not inject values into suspended inputs.

## Keep the scheduling relationships visible

<a id="async-04"></a>

**ASYNC-04.** **The scheduler owns its protocol state.** Tasks, wait registrations,
ready-queue membership, and relationships between them are represented by
declared typed Ama fields and compiler-visible control storage under CABI-05.
A registration identifies its task through an ordinary checked reference. It
also identifies the task's current scheduling enrollment and current wait.
Keeping a task alive requires explicit ownership; a checked reference alone is
not ownership.

While a task is enrolled, only its scheduler protocol may enter, resume, restart,
or remove that task. Clients use its documented control ports and independently
owned operation/mailbox objects rather than calling the raw task through an
alias. A scheduler restart invalidates old waits and publishes a fresh run epoch
before invoking `!`. This is a precondition of the task library contract, not a
hidden hook added to every Ama call: ordinary `!` or `%` does not discover a
scheduler, update its queues, or implicitly repair a bypassed enrollment.

Each enrollment has a fresh run epoch. Each replacement of a wait has a fresh
registration generation. These identifiers never wrap into an earlier valid
identity. Reserve their values before publishing a change; exhaustion is a
technical failure and must not publish a partially replaced registration.
A completion is accepted only if the registration is still live and its run
epoch, generation, and native operation identity all match. Instance
slot/generation checks remain required as well. Restarting, removing, or
reenrolling a task invalidates its former waits before they can be delivered.
The visible registration associates those values with the submission token
defined in ASYNC-05. Looking up a reused operating-system descriptor or request
address is not enough to identify the original wait.

Lists remain nominally typed. A scheduler can own one task type in a typed
queue, maintain separate queues for several task types with explicit dispatch,
or use an application-declared common representation. There is no implicit
universal module type. Opaque C payloads, integer casts, and callbacks must not
be used to evade this restriction by hiding Ama references.

## Bridge native events without leaking Ama references

<a id="async-05"></a>

**ASYNC-05.** **A callback records an event; the Ama executor delivers it.** Native
code may start a nonblocking operation or native-owned worker job during a
normal native entry. Persistent native operation records may contain native
buffers, operating-system resources, and operation identifiers. They must not
contain borrowed Ama views, checked Ama handles, addresses of Ama storage,
saved positions, or encodings of those values.

A retained callback receives only native-owned state and an endpoint for
posting native completion records. Such an endpoint must not expose Ama
storage. A completion record contains the operation identifier and native-owned
result data. Every submission has an immutable native submission token, carried
unchanged by its completion record. The token includes a service identity and
a nonwrapping submission generation and must never identify another submission
while an old completion can still be delivered. Issuing a fresh service identity
must likewise not revive a token from a disposed service. Exhaustion is an
explicit technical failure before submission. These are native correlation
identifiers, not encoded Ama handles. A reused file descriptor or native record
address does not replace the token. The visible registration stores its token
alongside the epoch/generation, so a late completion cannot borrow a newer
registration's identity. The executor later imports completions through a normal generated
access boundary, matches them against the visible registration, and updates
that registration or its explicitly owned result object under the usual
acquisition rules. Only the executor enters Ama code.

The callback must never resume a task itself, including when the underlying
native library invokes the callback synchronously during operation submission.
The record is queued for delivery in both cases. This prevents callback-driven
reentry while inputs, output actions, or native views are still protected.
The restriction includes entering any other Ama library context: the synchronous
native-host facility in [EMBED-08](../implementation/c-library.md#embed-08) is
not available from a retained completion callback. The owned child-context
resource allowed by [CABI-05](../implementation/native-interface.md#cabi-05)
belongs to an explicit wrapper operation, not a callback record or completion
endpoint.

<a id="async-06"></a>

**ASYNC-06.** **Registering a wait cannot lose a wakeup.** The operation protocol
must establish this sequence:

1. Create and publish a live registration with its final operation identity,
   run epoch, and registration generation.
2. Submit or arm the native operation using a completion endpoint that retains
   completed events until the executor consumes them.
3. Recheck operation completion before committing the task to the awaiting
   phase. If it is already complete, record readiness instead.
4. When the task returns, reconcile that phase with any readiness already
   recorded. An already-ready registration must put the task in the ready queue.

Submission and completion publication must have a documented linearization
order provided by the native operation contract. They may be implemented with a
synchronized queue or another exact protocol with the same invariant: after a
successful registration, either a completion remains observable or the task is
already marked ready. An edge notification that can disappear between a check
and registration does not satisfy this contract.

The recheck and the awaiting transition are performed by the same Ama executor.
A native callback may concurrently append a record, but cannot edit those Ama
states. If it appends after the recheck, the retained record is imported during
a later event-service round. There is no need to make callbacks acquire Ama
instances.

## Queue discipline and progress

<a id="async-07"></a>

**ASYNC-07.** **Ready membership is unique and ordered.** Each enrolled task has
at most one entry in its ready queue. The scheduler changes a visible queued
flag together with inserting or removing that entry. Repeated readiness for an
already queued task is coalesced; it does not create duplicate steps. Removing
a task for execution clears that flag before its step starts.

Accepted tasks are appended to the queue tail. A task that voluntarily yields
as Ready goes to the tail after its current step. Multiple events imported in
one round are considered in the completion stream's observed acceptance order;
events observed in one unordered readiness batch use a documented stable
registration order. FIFO refers to this ordering inside one context, not to
physical completion time or a global order of concurrent native events.
Different operating-system observations may therefore produce different valid
execution orders. Once the observed stream and external inputs are fixed,
dispatch is determined by these rules and the selected queue arbitration policy.

The scheduler specifies a fixed positive finite task-step budget and a fixed
positive finite completion-import budget per round. After at most that many
task steps, it services events and cancellation requests, even if tasks remain
ready. Event collection returns a finite batch; importing an indefinitely
replenished completion stream must not prevent the next task round. When both
sources remain nonempty, each round services both within their budgets. With
several typed queues, their arbitration also has a documented finite service
bound for each nonempty queue. It must not indefinitely exhaust one queue
before checking the others.

When no tasks are ready, the scheduler imports pending completions before
blocking. It blocks only if live waits remain and its wait primitive atomically
accounts for queued or concurrently arriving notifications. With neither ready
tasks nor live waits, it reports quiescence according to its declared lifecycle
contract; it must not spin. A live task that is neither ready, awaiting a live
wait, nor completed violates the task protocol.

A live foreach traversal is not a scheduling round boundary. If a body keeps
appending tasks, that traversal can continue indefinitely under FLOW-05. A
scheduler must enforce its explicit budget rather than rely on eventually
reaching the end of a mutable ready list.

## Cancellation, completion, and resource lifetime

<a id="async-08"></a>

**ASYNC-08.** **Cancellation is a cooperative request.** A cancellation request is
recorded in an explicitly owned protocol object and wakes an awaiting task.
The task observes it at a documented step boundary, asks the operation to stop
where appropriate, releases resources through normal methods and lifecycle
cleanup, and returns an ordinary cancelled result. Cancellation does not
interrupt Ama instructions or a native entry.

The operation contract defines the single linearization point that commits its
terminal outcome. Cancellation wins only if it commits before completion. A
later cancellation cannot overwrite an already committed completion; a later
completion cannot overwrite a committed cancellation. The terminal outcome is
published exactly once. A native result that arrives after cancellation may
still need native resource cleanup, but it must not cause a second result or
wake an obsolete task enrollment.

A request to cancel a task is distinct from destroying its available instance.
Destroying a yielded instance runs ownership destruction; it does not run the
remaining continuation, a cancellation branch, or arbitrary code after `%`.
Therefore required safety cleanup must belong to owned resources and their
lifecycle hooks, rather than depend on eventual continuation execution. A
library may require cooperative cancellation before normal task removal, but
its destruction path must still safely detach every registration.

<a id="async-09"></a>

**ASYNC-09.** **Detachment does not imply a callback has stopped.** Removing a wait
or destroying its owner invalidates the visible registration before releasing
its resources. The native operation is then cancelled or detached according to
its contract. Native callback records, buffers, and completion endpoints remain
alive until every callback that can use them has finished or been acknowledged
as impossible. Their lifetime is independent of the former Ama slot's lifetime.
They contain no Ama references and may outlive that slot safely.

A late event for a detached registration is discarded after native cleanup.
Slot reuse, record reuse, restart, or reenrollment must not make it eligible for
a new wait. Context shutdown follows the same rule: stop new enrollment, detach
all registrations, settle the lifetime of native operations and endpoints, and
then release context-owned state. A host may transfer detached native cleanup
to its own explicitly owned reaper, provided that reaper has no Ama state and
cannot deliver into a destroyed context.

## Continuation copying and failures

<a id="async-10"></a>

**ASYNC-10.** **Copying a position does not duplicate an event registration.**
CONT-04 continues to define `%` copying. A copy receives allowed data and saved
positions, not an active invocation or host callback. Copying a task does not
insert it into an existing ready queue or establish a scheduler enrollment.
Ordinary fields, including stored identifiers or queue data inside a larger
copied ownership tree, still follow the normal copy rules; their presence alone
does not authorize a new native subscription. A copied task or scheduler needs
explicit fresh enrollment before dispatch. External references remain external
under COPY-04; they may therefore still name the original operation or
cancellation object. Such a copy is not automatically an independently
scheduled task.

A native resource type that can own an in-flight operation must either be
noncopyable or define a genuine independent clone operation. It must not copy a
callback pointer, completion endpoint, or registration token and call that an
independent operation. A supported independent clone prepares its own native
resources under the ordinary lifecycle protocol. Enrolling the copied task
then creates a fresh scheduling identity and fresh waits; the old registration
cannot wake the copy.

If a copied task intentionally shares an externally referenced operation, that
operation's declared contract must state whether its result is single-consumer
or can be observed by several subscribers. Each subscriber needs its own live
wait registration. Sharing an external reference does not grant an additional
consumption right or silently duplicate a pending notification. Unsupported
copying follows the existing static and lifecycle failure rules.

<a id="async-11"></a>

**ASYNC-11.** **Technical failures end the context.** An allocation failure, invalid
runtime state, failed lifecycle operation, or other technical execution failure
follows ERROR-01 through ERROR-06. It ends the current execution context's Ama
computation; the scheduler must not catch it as an ordinary task result and run
the remaining tasks. Supported cleanup detaches their waits and releases owned
state before the host receives `failed`.

Expected operation failures use typed results and may be handled by an
application. An explicit supervisor outside a failed context may create a fresh
context or report failure; it cannot resume the failed context as though it had
yielded. No language-level exception or automatic task-restart policy is added.

## Implementing an asynchronous host

<a id="async-12"></a>

**ASYNC-12.** **Keep the implementation boundaries explicit.** An implementation
may use either of these arrangements:

- An Ama scheduler invokes native reactor operations that import completions,
  poll without blocking, and wait when no task is ready. Its scheduling and
  ownership graph remains in visible Ama storage.
- A custom host receives a yielded root under RT-14, services a native reactor,
  and later resumes that same root. The host owns the execution boundary;
  task and wait relationships inside Ama still obey ASYNC-04 and CABI-05.

Both arrangements use the existing nonyielding native-entry contract. A native
entry submits, queries, imports, or waits for events and returns; it does not
retain its C activation across `%`. A blocking reactor wait is permitted only
when the scheduling policy has established that no ready Ama task is being
withheld. A native worker doing independent work may run concurrently, but it
receives only native-owned buffers and resources and posts native completion
records. It has no authority to access Ama storage.

Required acceptance evidence includes completion before submission returns,
completion between recheck and yield, duplicate readiness, cancellation racing
completion, restart followed by a late event, slot reuse, destruction while a
callback remains possible, and continuously replenished ready queues. Tests
must establish unique ready membership, no lost wakeups, one terminal result,
and no delivery to an obsolete registration. A finite-step scheduling test must
also show that every continuously ready queue is serviced within its declared
bound. A task that never returns is outside that cooperative progress guarantee.

## Parallelism uses isolated execution contexts

<a id="par-01"></a>

**PAR-01.** **A context is the unit of isolation.** An execution context contains
its own explicit root instances, module instance pools, shared constants,
initialization state,
checked identities, current-instance selector, acquisition state, diagnostic
state, and executor control state. Constants are shared between instances of a
module within that context; their Ama storage is not shared between contexts.
Compiled code and immutable source or literal bytes may be shared.

Different contexts may execute in parallel. Within each context there is still
one Ama executor and the ordinary synchronous invocation rules. Ama does not
permit two executors to enter one instance, or two contexts to share an owned
instance, list, reference cell, or saved continuation. Busy checks enforce the
single-context invocation contract; they are not interthread locks.

The host identifies the context at every execution and typed-access boundary.
An identity valid in one context must be rejected in another, even if module,
slot, and generation numbers coincide. Internal Ama handles may be compact and
context-local, but no host API may interpret one in a different context without
checking its originating context. Context identities, like instance generations,
must not be reused in a way that revives obsolete host access.

<a id="par-02"></a>

**PAR-02.** **Executor ownership can move only at rest.** A context can be assigned
to another host thread only while no executor is active, no native entry or
borrowed Ama view remains live, and no host access conflicts with the transfer.
The host establishes a synchronization boundary publishing all context state to
the new owner. Every live native resource and reactor used by the context must
permit that transfer under its declared thread-affinity contract. Otherwise
the context remains pinned to its required owner; a mutex does not remove a
native resource's thread affinity. Pending native work may remain only if it obeys ASYNC-05 and
cannot access Ama storage. The two threads never execute or inspect mutable
context storage simultaneously. Transfer changes the executor's owner, not Ama
instance identities or saved positions.

This permits a host to schedule contexts on a worker pool without turning each
Ama reference into a shared-memory reference. A yielded context can later
resume on another thread after the required ownership transfer.

## Messages cross the boundary; references do not

<a id="par-03"></a>

**PAR-03.** **A message is independently owned data.** Asynchronous communication
between contexts uses an explicitly declared channel and a self-contained typed
message schema. A sender serializes an application value into native-owned bytes; a
receiver validates the schema and constructs independent Ama values in its own
context. Message data must not contain checked Ama handles, instance or field
addresses, list-buffer addresses, saved code selectors, continuation records,
or native resource pointers. Encoding those values as integers or bytes does
not make their transfer valid.

The schema specifies versioning, field types, length limits, and validation
before publishing a received value. Constructing a received value uses ordinary
native decoding and lifecycle cleanup; it cannot publish a partially initialized
Ama graph. A malformed or unsupported message is an expected channel result.
An allocation or lifecycle failure remains a technical failure of the receiving
context. Transfer of a resource needs a separately declared resource-transfer
protocol; serializing the pointer or assuming that two contexts own the same
resource is forbidden.

Serialization can deliberately send the data used by a computation so that a
receiver starts a new computation. It does not transfer the sender's executable
continuation. The receiver chooses its own declared entry module and state.

A synchronous C host call into a separately compiled Ama library is a distinct
explicit boundary, not an asynchronous channel operation. Its generated typed
native-value transport or serialized schema follows
[EMBED-04](../implementation/c-library.md#embed-04), constructs independent values,
and exports no live Ama storage or checked Ama references. This permits a C
caller or native adapter to pass a text value directly through the published
library API without inventing a queued message. It does not authorize arbitrary
cross-context field access, shared mutable instances, or continuation transfer.
Channel operations still obey all channel-schema and FIFO requirements above.

<a id="par-04"></a>

**PAR-04.** **Channels have bounded, explicit delivery rules.** A channel has one
logical sender endpoint and one logical receiver endpoint, a fixed positive
message-count limit, and a fixed positive byte limit selected when it is
created. Several producers require separate channels or explicit arbitration.
A send publishes one complete message atomically. It either accepts that whole
message or returns an ordinary result explaining that the channel is full,
closed, or rejects the schema or size. An unaccepted message remains the
sender's responsibility. There is no partial acceptance or silent dropping.

Accepted messages arrive in send order for that channel. A receive either
transfers the next complete message to a receiver-owned decoding operation,
reports that none is ready, or reports that the channel is closed and drained.
There is no global ordering guarantee across channels. Capacity is released
when the receiver takes ownership of the message bytes, not when its application
finishes processing the decoded value.

Normal send-side close forbids further sends and preserves accepted messages
until they are received. Disposing of the sender endpoint performs that close,
including during sender-context failure cleanup; the native channel service
retains the queue independently of the sender context. After the queue drains,
the receiver observes closure. Disposing of the receiver endpoint aborts delivery,
including during receiver-context failure cleanup. An explicit abort has the same
effect: queued messages are released, blocked or awaiting peers are notified
that delivery was aborted, and accepted-message delivery is no longer promised. Successful
send means enqueue acceptance, not application-level processing; an application
requiring confirmation uses a declared acknowledgement message. Abrupt context
failure therefore cannot be mistaken for successful processing.

A full channel applies backpressure through an ordinary result or a registered
asynchronous wait for capacity. Neither endpoint blocks an Ama executor while
other ready work should run. Channel close, abort, readiness, cancellation, and
shutdown use the same no-lost-wakeup and stale-registration rules as other waits.

## Native concurrency and failure boundaries

<a id="par-05"></a>

**PAR-05.** **Shared native services have their own synchronization.** Different
contexts may use a host-owned message transport or another explicitly declared
native service with synchronized, native-owned state. Such a service must not
contain hidden Ama references or substitute shared mutable globals for
context-owned state. Native hooks and their dependencies must be safe for
simultaneous calls from different contexts, or be serialized by the service's
documented boundary. Instance lifecycle, constant initialization, and teardown
are included in that audit, not only entry functions.

If a native dependency cannot satisfy that contract in one process, those
contexts must run in separate processes or the host must prevent their concurrent
use. Process isolation is also required when protection from native memory
corruption or process-wide native termination is part of the application's
failure requirements. Context isolation is a language/runtime ownership boundary;
it is not memory protection against arbitrary C behavior.

<a id="par-06"></a>

**PAR-06.** **One failed context does not implicitly cancel the others.** A normal
technical failure returns `failed` for its context, detaches its operations,
and closes or aborts its endpoints under their contracts. Other contexts retain
their state and can continue. A host supervisor may explicitly choose a wider
shutdown policy; that policy is not an implicit consequence of `%`, busy refusal,
or channel closure. Expected message or I/O errors remain ordinary results in
the context that handles them.

## Implementing and verifying parallel contexts

<a id="par-07"></a>

**PAR-07.** **Generated execution takes an explicit context.** Generated code uses
an explicit context parameter or an equivalent checked, exclusively owned
execution object for all pools, constants, identity resolution, diagnostics,
and dispatch state. Mutable process globals, per-type working copies, and
unsynchronized thread-local assumptions must not supply that state. Labels may
be shared code; the selected instance and return selectors remain in the
appropriate context. Ordinary calls still execute directly in stable pool slots
and do not acquire a C activation per Ama invocation.

The context object owns its runtime state through initialization, execution,
yield, resume, failure, and disposal. A host may own and retain that execution
object at the root boundary. An `@C` wrapper may likewise own a separate library
context and its checked instance-control resources only through
[EMBED-07](../implementation/c-library.md#embed-07)'s narrow native-resource
contract and [CABI-05](../implementation/native-interface.md#cabi-05). These
control capabilities are not ordinary Ama references and do not let the parent
resolve or share the child's fields. Its concrete C type names and entry
functions are ABI artifacts, not new Ama syntax or fixed `.abi` records.

<a id="par-08"></a>

**PAR-08.** **Verification covers isolation and communication.** Acceptance tests
must execute two contexts concurrently using the same compiled module types,
including separate constant initialization and pool-slot reuse. They must show
that a handle from one context cannot access the other, that executor transfer
preserves state, and that no context is entered concurrently or through a native
callback.

Channel tests cover FIFO acceptance, full capacity, byte limits, malformed
messages, close with queued data, receiver abort, sender shutdown, cancellation,
and failure during decoding. Native event tests include completions arriving
while another context runs and after the original context is disposed. General
invariants are: one executor per context; no context resolves another context's
Ama identity or receives its checked reference as a value; accepted messages
have one owner at each stage; callbacks have no Ama
access; and every accepted wakeup is either pending, delivered, or invalidated
by an explicit lifecycle event.
