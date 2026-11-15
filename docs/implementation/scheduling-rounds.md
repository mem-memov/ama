# Bounded rounds and native reactor waits

This component implements the bounded-service and blocking rules of
[ASYNC-07](../../spec/language/async-and-parallelism.md#async-07), building on
[typed FIFO membership](ready-queues.md) and
[wait registration/import](task-registration.md).
The actual Ama round is
[maintained here](../../tests/unit/codegen/scheduler-rounds/project/round).
Its concrete schema has two nominal task domains and one shared native event hub.
It is not an implicit runtime scheduler: applications call the round and decide
when to invoke the separate native wait.

## A finite round

The round takes owned integer task/import budgets, defaulting to two each.
`..task.budget.check` accepts only values in `1..SIZE_MAX`; zero, negative and
unsupported host sizes fail technically before any task step. Comparisons and
increments then use ordinary exact `..integer` operations.

Each invocation performs these operations:

1. Validate both budgets, queue owners and the native hub.
2. Make at most the task budget's number of queue decisions. A persistent cursor
   alternates the preferred typed queue; if it is empty, the other queue gets the
   opportunity. Stop immediately when both queues are empty. A decision may discard
   an obsolete entry without executing a task, but still spends its decision credit.
   Every actual step increments the separate step metric.
3. Take and route at most the import budget's number of native packets. Every
   consumed packet counts, including an obsolete/unmatched completion. An empty
   native queue ends this phase immediately. Matching checks the full token,
   live checked task, active enrollment, run epoch and wait generation. Delivery
   uses normal Ama calls to an explicitly owned mailbox; it does not execute tasks.
4. Observe physical queue backlog and native readiness without consuming another
   packet. Remaining work returns `ready=true`, so the next call gets fresh budgets.
5. Before an idle boundary, audit every enrolled record. A started task must be
   Completed, with no Ready/member/wait flags, or Awaiting a fully valid live wait.
   A missing Ready entry, a broken stamp/liveness relation or an unstarted forgotten
   task is a technical protocol error, not quiescence. Aggregate live waits from
   that same audit rather than trusting a stale Boolean alone.
6. Return `waiting=true` only when declared live waits also have a native producer.
   With no queues, native backlog or live waits, return `quiescent=true`. The
   one-hub schema rejects orphaned armed native work instead of sleeping forever.

The three published state flags are mutually exclusive. Metrics satisfy
`0 <= steps <= decisions <= task_budget` and
`0 <= imports <= import_budget`. Task and import credits reset every invocation;
the queue arbitration cursor persists. A nonempty typed queue receives an
opportunity within two decisions, and continuously Ready work cannot prevent the
following import phase. No live foreach is used as a scheduling budget.

This is a bound on logical service opportunities, not elapsed wall-clock time.
Ordinary list removal can shift membership metadata. Matching/auditing visits
finite registry snapshots. Their bodies do not structurally change those
registries, and native callbacks cannot execute Ama to replenish them.

`..task.require` turns a failed explicit protocol predicate into a technical
failure. It does not manufacture a task outcome or permit a failed context to
continue. [Detachment](task-detachment.md) and
[cooperative cancellation](task-cancellation.md) extend this round boundary.

## Observe first, then wait explicitly

`..task.events` now exposes actual `poll` and `wait` method instances in addition
to prepare/arm/ready/post/take. Their state holds native resource leases only.
`poll` returns two ordinary conditions: `ready` means retained native data, and
`pending` means at least one armed native producer. Both can be true. Poll does
not consume a packet or spend an import credit.

When a round reports Waiting, its caller explicitly calls the hub's `wait` method.
That method retains the existing native service through its activation, checks
queued data again, and blocks only when armed work remains. It returns a Ready
condition without consuming the completion. The caller then starts another
bounded round, which imports the data before stepping the awakened task.

An arrival after the round's poll therefore remains safe. If the import quota
was exhausted but native backlog remains, the round returns Ready and another
round imports the next bounded portion. It never drains an indefinitely
replenished source merely to reach a blocking point.

The native service also reports `ready=false` immediately when neither queued data
nor armed producers remain. Prepared-only reservations cannot complete until the
owner arms them, so they are not a reason to sleep. In the Ama protocol, Waiting
with no such producer is an invariant error. Quiescence does not call the wait
primitive or burn a maximum-sized quota on empty queues.

## A retained binary wake credit

The current blocking backend uses a process-private POSIX semaphore, matching the
supported native host arrangement. The semaphore is initialized before publishing
service identity and is destroyed only after the final native lease. The sole
consumer serializes wait with other owner operations, including disposal; this is
not permission to destroy a running Ama context or cancel its executor thread.

Native metadata has an exact armed-cell count and a `notified` latch. Arm adds
one cell to the armed partition; the one accepted completion removes it. Duplicate,
stale and unarmed posts do not change the count. Closing clears that partition.
The queue remains the authoritative retained data store.

A successful post reserves a wake only on `notified=false -> true` under the
metadata gate. It releases the gate before `sem_post`. The native callback's
owning endpoint keeps the semaphore and code/resource dependency alive throughout
that operation. The sole waiter clears `notified` only after consuming a credit.
Consequently there is at most one credit, either reserved, pending, or consumed
but not yet cleared. A valid live semaphore cannot overflow under this invariant.
Repeated polling/taking without waiting coalesces notifications instead of
accumulating an unbounded number of stale credits.

The consumer checks the retained queue before `sem_wait` and again after consuming
a credit. A callback before the OS wait leaves a credit; a callback during the
wait wakes it; a callback coalescing with a credit just consumed still leaves its
record in the queue for the next metadata check. No edge can disappear between
an empty check and sleeping. At most one stale credit is drained before an actual
block. `EINTR` retries internally; other wait errors preserve the output and report
a technical native status. No borrowed Ama state enters the callback.

## Executable evidence and scope

The [native wake suite](../../tests/unit/async/events/wake/main.c) uses real POSIX
semaphores and producer threads. Exact wrappers expose the pre-kernel-wait boundary;
there are no elapsed-time decisions. Tests cover initialization failure before
identity publication, prepared/closed owners, queued/armed snapshots, a post in
the check-to-sleep gap, interruption retry, finite stale-credit drainage, wait
errors and final resource retirement. Existing concurrent callback/close tests
also exercise the new signal path.

The [generated round suite](../../tests/unit/codegen/scheduler-rounds/main.c)
executes the actual Ama schema and maintained native provider. It verifies a
perpetually replenished Ready task, two typed queue domains, native import backlog,
positive budgets and empty maximum budgets, quiescence, and invalid live-task
states. A test-only native gate releases the worker precisely when generated
code reaches its OS wait. The worker owns a native endpoint and native stop flag;
it never retains an Ama view or calls Ama. Failure teardown releases that gate
through native cancellation and joins before freeing the job.

The provider is a standalone test host whose executable code remains loaded.
Loadable-provider code pins and the final published asynchronous host API belong
to T099. This task does not claim complete cancellation/detachment or parallel
execution contexts. All callbacks still queue native data for one executor.

Run `make -C tests/unit/async/events` and
`make -C tests/unit/codegen/scheduler-rounds`. `AMA_SCHEDULER_CASE` selects
`quiescent`, `fairness`, `backlog`, `blocking`, `budgets` or `protocol`; the default
runs every contract. Production and generated paths are checked with strict
`NDEBUG`, ASan/UBSan, and the native producer path with ThreadSanitizer.
