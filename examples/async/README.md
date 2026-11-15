# Explicit asynchronous tasks in Ama

This example completes one task and cancels another. The scheduler is ordinary
Ama code: each record owns its task, each task owns its native operation, and a
typed FIFO contains checked references to runnable records. No callback executes
Ama and no scheduler runs implicitly.

From the repository root, choose a new output directory:

```sh
make
./ama --emit examples/async .main --out /tmp/ama-async-demo --sdk src --standard-root stdlib/modules --compile
/tmp/ama-async-demo/program
```

Expected output:

```text
completed
cancelled
```

Read [main](main) first. It explicitly enqueues both records and performs one
step of each. Both tasks publish their wait and yield. The producer then posts a
native result for the first record, imports its owned bytes, and requests
cancellation of the second. Two more dispatches commit their different outcomes.
A deliberately late result cannot revive the cancelled record.

The notification in this example is manual so that its order is visible and
repeatable. [The native-host example](../async-host/README.md) uses actual worker
threads and unloads/reloads a compiled component. The same `..task.events`
retention and token validation rules apply to both arrangements.

The `jobs/` modules show the protocol in small pieces: enqueue/pop, one-step
dispatch, checked delivery, cancellation, task and record. The task's phase is
Completed for both terminal cases; its separate `..task.outcome` says whether the
operation completed or was cancelled. Cancellation is observed at a task step
boundary. It never interrupts native code or runs a continuation in a destructor.

The private `..task.enrollment` marker and callback-owning `..task.operation` make
the owning record noncopyable. A different copyable task needs a fresh explicit
enrollment; copying flags or token data grants no scheduling rights. Clients use
the protocol modules instead of editing membership fields or resuming an enrolled
task through an alias.

The small [print](print.c) extension performs synchronous counted-text output.
Its C view is temporary and it retains no text address. See the
[implementation contract](../../docs/implementation/asynchronous-host.md) for
native submission, bounded scheduling, cancellation and copy/failure evidence.
