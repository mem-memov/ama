# Explicit typed ready queues

This component implements the queue discipline of
[ASYNC-07](../../spec/language/async-and-parallelism.md#async-07), using the
[phase values](task-phases.md), [enrollment identities](task-enrollment.md) and
[retained completion boundary](task-registration.md).
The maintained executable modules are in
[the typed schema](../../tests/unit/codegen/ready-queues/project/kind/queue).
They are ordinary Ama code: their persistent task relationships are declared
references and owned values. No native queue stores an Ama address, handle or
encoding of a reference.

## Ownership and the queue interface

The scheduler owns a registry of records and one or more queues. A record owns
its task in persistent private storage. It publishes a checked task reference,
run epoch, enrollment/Ready/started flags and one `queued` flag. That membership
flag belongs to the enrollment, so two matching queues cannot simultaneously
accept it. Distinct nominal task domains have distinct record, queue, entry and
operation types; a matching layout does not allow substituting one domain for
another.

A queue owns an ordered list of entry objects. Each entry contains a checked
record reference and a copied epoch. It does not own or copy the task. These
are scheduler-owned protocol objects: clients use the operations below and do
not edit membership flags, reorder the registry or copy a live scheduler into
another active scheduler. Ordinary language copying alone creates no enrollment.
[Noncopyable enrollment owners and explicit snapshot adoption](task-copy-and-failure.md)
provide the corresponding copy boundary.
Copy/failure boundaries and complete detachment are later roadmap components.

The schema supplies four explicit operations:

- `enqueue(queue&, record&)` checks liveness, active enrollment, a nonzero epoch,
  Ready state, absence of current membership and no Completed phase in an
  already started current run. It appends a new entry at the
  tail, initializes its identity and sets `queued`. Repeated readiness returns
  `accepted=false` without another entry.
- `pop(queue&)` removes the physical head, then authenticates its checked owner
  and epoch. For a current enrollment it clears `queued` before returning the
  record/task references and `available=true`. An obsolete epoch or dead owner
  is discarded without clearing another enrollment's membership. Non-Ready work
  is not executed. Empty or discarded-head results are unavailable; this alone
  is not a quiescence decision.
- `step(record&)` explicitly enters an unstarted task with its owner argument,
  or resumes the actual started task with `%`. It reads the published phase
  after the call returns and updates declared Ready state.
- `dispatch(queue&)` makes one pop decision, executes at most one task step, and
  calls `enqueue` after the step if that task remains Ready. A voluntary Ready
  yield consequently rejoins behind work already in the queue.

These operations run on the single scheduler executor between task steps.
Membership transitions never suspend; task execution starts only after pop clears
membership. Native callbacks cannot interleave Ama mutations.
Expected acquisition/preflight refusal of the queue returns without using an old
local entry reference. Allocation, identity exhaustion or other technical failure
stops the context; a partially completed transaction is never resumed as a valid
scheduler operation. Bounded rounds, reactor blocking and cancellation are T095
and subsequent tasks, not implicit behavior of this component.

The record initializer obtains its epoch from a private persistent `..task.sequence`.
Each explicit entry advances the retained generation and publishes an independent
snapshot; resetting the initializer's public result fields cannot rewind that
history. It resets `started` for the new run. Eligibility then permits its first
explicit task entry even if the preceding run's retained phase was Completed.
Once the current run has started, Completed overrides a late Ready notification.
The schema's task has no external pending operation; restarting real waits also
requires the detachment protocol of T096.

## Two sources of ordering

For an ordered completion stream, `import` observes one retained native packet,
matches its full token against the typed registry, and immediately enqueues the
matched ready record. A repeated observation of the same packet is coalesced by
membership. The resulting FIFO order is the stream's observed acceptance order.
It does not claim an order of physical wall-clock completion.

For an unordered readiness batch, `observe` marks readiness for each matched
packet without enqueueing. After the finite batch has been observed, the
scheduler visits the registry in its stable registration order and calls
`enqueue` for ready members. In this schema that order is append order; removal
preserves remaining relative order and new registrations append. Protocol clients
do not reorder the registry. An unordered batch reported as C, A, B therefore
inserts A, B, C when that is their registration order. Already queued members
retain their existing positions.

Unissued tokens, absent packets and notifications for an already Completed current
run do not mark readiness. Matching compares the
complete native identity; native cell indices alone never identify a task. The
packet and registry relationships remain normal declared Ama values, and
completion handling does not run the matched task inside a native callback.

## Invariants and proof

An enrollment key is the pair of checked record identity and nonwrapping epoch.
A **current entry** matches a live active enrollment with its current epoch.
An obsolete physical entry may await disposal after a changed epoch or owner
removal; it is not membership of the new enrollment. Lifecycle invalidation must
clear the former enrollment's flag before newly enrolling the same record.

At valid operation boundaries, every current queued enrollment has exactly one
current entry and `queued=true`. Every accepted unqueued enrollment has none.
Initially every queue is empty and all flags are false. Enqueue either leaves
this state untouched, or observes false, appends exactly one entry and changes
the flag to true. A second enqueue sees true and cannot append, including through
a second matching queue. Pop of a current entry removes that sole entry and
clears the flag before any task body. Pop of an obsolete entry cannot clear a
new epoch's flag because it compares the epoch first; checked-handle acquisition
rejects a dead owner even when an address and numeric epoch could be reused.
These cases preserve the invariant inductively.

The only insertion is at the tail and the only removal is at the head. Therefore
a fixed accepted stream preserves its order. Dispatch finishes removal before
execution and appends a Ready continuation after existing entries, so consecutive
steps of one task cannot displace work already accepted ahead of its requeue.
Batch reconciliation supplies a deterministic accepted stream from stable registry
order. Explicit caller arbitration chooses among different typed queues; finite
service bounds for that arbitration belong to T095.

The reference queue uses ordinary list operations. Removing index zero may shift
list membership metadata; this component makes no constant-time removal claim.
It prioritizes visible typed ownership and the specified order. A future storage
optimization must preserve the same identities, flags and observed order.

## Executable evidence

`make -C tests/unit/codegen/ready-queues` compiles the actual Ama operations and
compares every task identifier and step count. Native trace instrumentation
rejects any task step that still observes `queued=true`, including with production
`NDEBUG`. A deliberate violation verifies that this checker is active.

The queue tests isolate membership and order with controlled readiness inputs;
full wait publication/mailbox safety is established by T093, and complete reactor
acceptance belongs to T099.

Cases cover duplicate readiness, tail requeue, completed-task rejection, actual
native acceptance order, stable unordered-batch order, coalescing across matching
queues, two distinct nominal task domains, changed epochs, reentry after completion and removed/replaced owners. Empty queue
references cannot reuse a previous local selection or damage real membership.
The test instantiator replaces only listed whole `.kind.<module>` path lines with
an explicitly selected domain; it does not infer types or cast between domains.
`AMA_READY_QUEUE_CASE` selects `coalesce`, `ordered`, `unordered`, `multiple`,
`stale` or `guard` for focused diagnosis; an unset selector runs all cases.
