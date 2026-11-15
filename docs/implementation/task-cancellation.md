# Cooperative cancellation and one terminal result

This component implements
[ASYNC-08](../../spec/language/async-and-parallelism.md#async-08). A cancellation
request is ordinary declared data. It makes an awaiting task Ready through the
same typed FIFO used for other wakeups; it does not interrupt an instruction,
enter a native callback, or run a continuation during destruction.

The maintained composition is under
[task-cancellation/project](../../tests/unit/codegen/task-cancellation/project).
It reuses the checked FIFO enqueue/pop/step/dispatch implementation and wait
validator from the preceding components. Queue membership still means one entry
per live record identity and run epoch. Repeated requests coalesce before dispatch.

## Arrival is distinct from accepting a result

A native callback merely queues native data. Import authenticates the token,
checked task identity, enrollment epoch and wait generation, then copies bytes
into an owned mailbox. It marks the task Ready and enqueues it. Import does not
commit completion. A cancellation request sets an explicitly owned flag and
performs the same readiness/enqueue operation.

The sample task observes both inputs at its resumed step boundary. If a request
is already present on first entry, it also observes it before the first Awaiting
yield, so that a cold cancellation cannot be lost. An early resume with neither
input returns to Awaiting without fabricating a result. No task body runs in a
callback or in an owning destructor.

For this operation contract, **the scalar terminal-state assignment inside
`..task.outcome` is the linearization point**. When cancellation is requested and
completion has not yet committed, the task commits Cancelled. Otherwise an
available imported result commits Completed. Thus these schedules have explicit,
different results:

- Request, task step, late native arrival: Cancelled; late bytes are cleaned up.
- Import arrival, request, task step: Cancelled; arrival was not a commit.
- Import arrival, task step, request: Completed; the late request is rejected.

This is a documented operation-level choice under ASYNC-08. It does not assign
wall-clock priority to a native callback or add a language-wide scheduler.

## The terminal value

`..task.outcome` has condition inputs `cancel requested` and `completion available`,
and retained condition results `pending`, `cancelled`, `completed`. Its default
value is Pending. Calling it uses the following total transition on valid inputs:

| Current | Cancel requested | Completion available | Next |
| --- | --- | --- | --- |
| Pending | false | false | Pending |
| Pending | false | true | Completed |
| Pending | true | either | Cancelled |
| Cancelled | either | either | Cancelled |
| Completed | either | either | Completed |

The payload is a resource-free native value, with a normal clone capability.
The complete transition validates state and full-width condition payloads before
writing. Invalid inputs fail technically. The payload assignment precedes the
allocation-free one-hot result publication; the single executor cannot observe a
partially published tuple. Repeated entry calls republish the same terminal
value, rather than resetting it through result initialization.

For terminal states `t`, every valid subsequent transition satisfies `F(t,x)=t`.
Pending can make at most one strict transition. Consequently a run has at most
one terminal commit. Native arrival alone cannot add another transition, and a
late request cannot overwrite a completed result. Starting a genuinely fresh run
requires a fresh owned Pending value; copying an outcome copies data, not
scheduler enrollment or external work.

## Release delivery before releasing ownership

After the terminal commit, the task clears its visible waiting/delivery flags,
then calls `..task.operation.detach` with its explicit `operation&`. This ordinary
native method uses a checked synchronous read visitor to retire exactly the
operation's full token. It returns a `retired` condition. An initialized empty
operation, closed source or already retired operation reports false harmlessly;
an empty/stale supplied Ama reference still refuses through normal checked access.

Detachment frees the native reservation and any queued bytes; it does not
invalidate another callback's native resource rights or physically interrupt its
worker. The owner keeps its own endpoint lease until ordinary task destruction or
operation restart. Callback owners acknowledge their independent leases normally.
This distinction permits safe late cleanup without retaining borrowed Ama state.
The task also clears its owned mailbox through an ordinary mailbox entry call,
then publishes Completed phase with its separate ordinary outcome value.

The completed phase describes scheduling, while Cancelled/Completed describes
the operation's result. A repeated cancellation request after that phase neither
changes the request flag nor queues the task. Normal removal still invokes only
ownership cleanup. The [detachment component](task-detachment.md) proves the
native callback and reused-slot boundary independently.

## Source and executable evidence

The per-file map is in [ready.json](documentation/ready.json).

- `stdlib/modules/task/outcome+/native/selection.h` defines the finite transition.
  `make -C tests/unit/async/outcome` exhausts all valid tuples, both terminal
  orderings, idempotence, aliased output and invalid full-width inputs.
- `stdlib/modules/task/outcome`, `.h`, `.c`, `.abi` expose the typed native value;
  `stdlib/modules/task/operation+/detach` and companions expose explicit cleanup.
  Headers have standalone inclusion checks. The actual generated-code suite
  compiles their SDK signatures and verifies mandatory native entry inlining.
- `make -C tests/unit/codegen/task-cancellation` executes cold cancellation,
  cancellation-first and completion-first schedules, arrival-before-commit,
  premature resume, repeated queue wakeups, idempotent empty/armed cleanup and a
  fresh successful run after cancellation. The task explicitly resets its private
  loop predicate on each entry; private declarations alone do not reset values.
  Ama-side assertions require no remaining wait, membership or mailbox bytes at
  terminal observation; exact output requires a coherent one-hot ordinary result.
  A one-cell source detects reservations that were not reclaimed.

The maintained task/record/mailbox schemas are ordinary Ama programs, not parser
exceptions or a hidden runtime scheduler. Existing native callback tests cover
real concurrent post/detach/close races; this composition tests how the single
executor turns those retained events into one ordinary terminal result.
