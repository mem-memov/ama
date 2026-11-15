# Supported asynchronous library and host

[ASYNC-12](../../spec/language/async-and-parallelism.md#async-12) permits an Ama
scheduler that invokes native reactor operations, or a C host that explicitly
services native work between yielded-root calls. The implementation supports
both without adding an implicit scheduler or allowing callbacks to execute Ama.

Start with the working [typed Ama example](../../examples/async/README.md) and
[SDK-free native host example](../../examples/async-host/README.md). The
[standard-library guide](../../stdlib/README.md) lists the actual modules and
links the detailed phase, registration, queue, round, detachment, cancellation
and copy/failure contracts.

## The concrete boundaries

`..task.events` owns a fixed-capacity native completion service and returns actual
method instances: prepare, arm, ready, post, take, poll and wait. Native providers
obtain independently owned endpoint rights before returning from submission.
Callbacks queue native records only; the executor later validates a declared wait
and imports its bytes into ordinary Ama storage.

`..task.operation` ties one reservation to task ownership. Its explicit detach
method retires delivery; lifecycle destruction remains safe without running code
after `%`. `..task.phase` distinguishes Ready/Awaiting/Completed scheduling state.
`..task.outcome` distinguishes Pending/Cancelled/Completed operation results at the
operation's documented commit point. `..task.enrollment` prevents copying an
owning scheduler record into a second active enrollment.

The queue/record modules in the Ama example are nominally typed source templates,
not universal native containers. Their membership flag and epoch live in declared
Ama fields. Each pop clears membership before one task step; repeated readiness
coalesces. Their supported callers follow ASYNC-04's scheduler ownership rule.
[Bounded rounds](scheduling-rounds.md) place exact task/import credits around these
same operations and define poll/explicit-wait behavior. The small example uses
manual notification to make the order readable; the native example uses real
workers.

## Code retention is independent of context retention

[EMBED-06](../../spec/implementation/c-library.md#embed-06) forbids unloading code
still needed by a detached callback, buffer or release operation. A context pin
alone cannot satisfy this: its wait can be safely detached while native code
still needs to acknowledge an independent resource.

The native SDK now exposes a native-only code provider in the operation-scoped
`AmaNativeServices.code` value:

```c
AmaNativeCodeLease lease = {0};
AmaStatus status = ama_native_code_acquire(&services->code, &lease);
if (status.code != AMA_OK) {
    return;
}
/* Transfer the owned credit into a native resource before the entry returns. */
```

The services address and typed Ama views remain temporary. The acquired lease
contains native code-lifetime bookkeeping and an infallible native release, not a
context, world, pool, checked Ama handle or execution capability. Call
`ama_native_code_drop` to consume a local credit; empty release is harmless. Lease
bits are move-only ownership: copying them does not retain another credit. A
missing zero-valued provider explicitly denotes process-resident code.

Compiled C-library lifecycle setup binds the provider to its private component
code counter before creating contexts. Context preparation copies the immutable
provider into the program. Native entry emission copies it separately from the
caller-refusal snapshot, so host-entry roots and nested Ama calls both receive the
correct component provider. Separately compiled child components receive their
own providers through their own context definitions.

`..task.events` acquires one credit before service creation and transfers it into
the service's existing native keepalive. Constructor failure releases the
unconsumed credit. On success, all method/endpoint/buffer rights retain the native
service, which retains that one code credit until final retirement. This avoids
an additional code-counter acquisition per callback or byte buffer.

For one service, let `R` be its owner, method, endpoint and buffer reference count.
The service owns one creator-code credit exactly while its storage is alive.
Closing clears visible producer/queue state and drops owner resources, but cannot
free storage while `R > 0`. Therefore every remaining callback/release operation
has its creator image retained. The final reference consumes the keepalive only
after native cleanup. Counter acquisition is checked against `SIZE_MAX`; failure
preserves the empty destination. The native metadata gate invokes no user/Ama
code and no allocator.

This retains the service creator's compiled image. Independently loaded native
providers also retain their own code as their explicit host contract requires.
The sample backend is linked to the host and remains resident across component
unload. No opaque payload conceals an Ama relationship.

## A count does not replace a loader barrier

The existing public `code_can_unload` query observes open contexts and all code
credits, including snapshots, diagnostics, explicit pins and native service
credits. The supported host first excludes new API/native work, waits for active
creator-code calls to return, and then tests quiescence before system unload.
Final release can reduce the count while its C function is still returning;
joining or an equivalent exact call barrier remains necessary.

The host example does not take an extra manual pin that could mask a missing
service credit. It disposes a yielded context while a worker still owns binary
bytes and an endpoint, requires Busy, releases/joins the worker and then requires
Ready. Other cases retain an exported text snapshot after context disposal and
require Busy until that value is independently released. Actual unload/reload
also proves fresh native image domains instead of relying on virtual address
reuse.

## Acceptance evidence

The per-file executable map is in [ready.json](documentation/ready.json).

- `make -C tests/unit/runtime/library`: native-only credits, independent moves,
  malformed providers, initialized-empty output and exact counter exhaustion.
- `make -C tests/unit/library/async`: real separately linked backend, compiled
  component with `NDEBUG`, SDK-free consumer, native-before-submit-return and
  delayed completion, cancellation before/after arrival, restart followed by an
  old event, pending context disposal, exported-value retention, constructor
  failure and physical `dlclose`/`dlopen`. Workers acknowledge on their own native
  thread; the host joins before unload.
- `make -C tests/integration/async-examples`: builds/runs both public examples
  through product commands, rejects missing arguments/duplicate ABI names, and
  proves an existing output directory is preserved.
- [Registration races](task-registration.md), [ready membership](ready-queues.md),
  [bounded service/wakeup](scheduling-rounds.md),
  [detachment](task-detachment.md), [cancellation](task-cancellation.md) and
  [copy/failure boundaries](task-copy-and-failure.md) supply the remaining
  controlled completion/recheck/yield, duplicate, stale epoch, slot reuse and
  continuously replenished queue acceptance cases.

Native Address/UndefinedBehaviorSanitizer and ThreadSanitizer checks use the
actual endpoint/worker/code-counter handoffs. The ThreadSanitizer worker harness
uses POSIX threads, matching the supported host; it does not substitute a mock
callback for a real worker.
