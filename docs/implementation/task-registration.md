# Publishing a wait and importing its completion

This implementation connects the [native completion queue](native-events.md) to
ordinary Ama state under [ASYNC-04](../../spec/language/async-and-parallelism.md#async-04),
[ASYNC-05](../../spec/language/async-and-parallelism.md#async-05) and
[ASYNC-06](../../spec/language/async-and-parallelism.md#async-06).
The executable composition lives in
[the maintained fixtures](../../tests/unit/codegen/event-bridge/project/setup).
It is the registration/import component; bounded reactor service, unique ready
membership and cancellation are separate roadmap tasks.

## The native methods have explicit owners

`..task.events !` constructs a positive-capacity native service (default 64).
Its returned `prepare`, `arm`, `post`, `ready`, `take`, `poll` and `wait` modules are actual
instances, invoked with selected-result calls. Neither the constructor nor a
method supports copying. Each method owns a native resource lease, never a
pointer into its parent's Ama payload. A default, unbound method reports a
technical error. Reinitializing the constructor closes its previous service.

For example, with an explicitly declared `service` of type `..task.events`:

```text
service !
 < |8
service
 prepare !
  > operation
```

`operation` is an ordinary `..task.token` scalar. It contains the complete native
identity and owns no callback rights. Its condition means that an identity was
issued, not that an operation is ready. `..task.token.same` compares all fields.
Native locators, code-image addresses and integers never stand in for Ama task
references.

Before `arm`, the task publishes its checked reference, run epoch, wait generation,
token and explicitly owned mailbox reference in a live Ama registration. Only
then may it call:

```text
service
 arm !
  < operation
service
 ready !
  < operation
  > completed
```

[Bounded rounds and native wait](scheduling-rounds.md) describe `poll` and `wait`,
including their single-consumer lifetime contract.

The current `arm` method supports an explicit manual notification source. It marks
the native reservation armed without starting an external operation. `post` accepts
an operation token, a nonnegative `..std.index` code representable in `int64_t`, and
an owned `..bytes` value; it copies the payload before submitting it. Duplicate
and stale notifications are discarded normally. Unarmed operations, malformed
native state, allocation failure and exhausted capacity/identities are technical
failures. This primitive does not yet claim an OS I/O backend or reactor wait API.
The native C submission boundary separately supports synchronous and worker-thread
completion with signed codes.

`take` returns an owned `..task.packet`. Its condition means a completion is present;
zero code and empty data are valid present results. Its declared results are a
`..task.token`, a signed arbitrary-precision `..integer` code and `..bytes` data.
An empty queue returns an absent packet. Preparation of code/data finishes before
presence is published; temporary native completion storage is released on every
success or failure path. Copying an owned packet copies its declared value state.

`..bytes` owns immutable counted binary data, including embedded zero and invalid
UTF-8. Empty values allocate nothing. Clones acquire nonwrapping shares of immutable
backing; replacement prepares independent backing before releasing the old share,
even when the input aliases the old bytes. These are context-local shares, not a
cross-context message transport.

## Readiness is retained across each boundary

The generated composition uses this order:

1. Reserve the native token, publish the live Ama registration, then arm.
2. Recheck native readiness. Record readiness in the owned mailbox if complete;
   otherwise publish the Awaiting phase.
3. Return from the task with `%`. A callback may have posted after the check;
   its native record is still retained.
4. Import native completions through `take` and normal typed Ama calls. Consult
   the actual registration through a checked reference, not an owned snapshot of
   its `live` flag. Verify task liveness, active enrollment, live wait, exact run
   epoch, exact wait generation and the complete native token.
5. Publish matched data in the mailbox and mark it delivered/ready. Revalidate
   the current live registration before enqueueing, even if readiness was recorded
   before revocation. Reconcile
   task phase with mailbox readiness before deciding whether to enqueue the task.
6. A resumed task consumes its mailbox. It need not query a recycled native token
   after `take` has already imported the result.

Revocation and stamp/token mismatches leave the mailbox unchanged. Importing one
record does not execute the task. The task is resumed explicitly from a typed
reference queue. Later FIFO membership work will coalesce repeated readiness.

There are only two ways to remove an accepted native completion: `take` transfers
it to the sole executor, or service closure retires it. Consequently completion
after a negative recheck cannot disappear before reconciliation. Before import,
the native record witnesses readiness; after a valid import, the declared mailbox
witnesses it. A ready flag overrides an earlier Awaiting phase. Invalidated waits
are deliberately excluded from this progress invariant and their native bytes
are still retired. No callback may mutate those declared fields or enter Ama.

## Evidence and limits

The native storage suite exhausts six arrival boundaries, including a callback
inside `arm`, between recheck and Awaiting commit, and after reconciliation. It
uses the actual retained queue while independently modeling executor phase/flags.
The generated Ama suite executes six schedules around the actual recheck,
phase commit, `%`, import and later reconciliation, including early resumption
before mailbox delivery. An early step rechecks readiness and yields again; it
never treats an empty mailbox as completed work. It also rejects revoked
waits, changed epochs/generations, inactive enrollment and mismatched tokens.
Existing enrollment cases cover destroyed owners and checked-handle reuse;
native producer tests cover concurrent callbacks and close/ack races.

Run `make -C tests/unit/async` and `make -C tests/unit/codegen/event-bridge`.
For focused diagnosis, `AMA_EVENT_BRIDGE_CASE` selects `service`, `reconcile`,
`invalid` or `lifecycle`; an unset selector runs every group and unknown selectors
fail the harness.
The binary-value component additionally injects allocation/share exhaustion and
checks aliased replacement and exact allocation retirement. These checks establish
registration/import safety; scheduler fairness and cancellation need their own
subsequent acceptance evidence.
