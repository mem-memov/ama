# Detached waits and native operation ownership

[ASYNC-09](../../spec/language/async-and-parallelism.md#async-09) separates
observable task lifetime from native completion lifetime. Removing a task does
not execute its continuation. Native callbacks may finish later, but cannot
publish a result into a replacement task or keep an Ama address alive.

## The two ownership boundaries

Visible registration is ordinary declared Ama data. The
[registration validator](task-registration.md) checks the live task reference,
enrollment epoch and wait generation before delivery and again before readiness
reconciliation. Explicit retirement clears the visible waiting/enrollment state
before releasing its native operation. Destruction invalidates the owning checked
identity before releasing native resources. Thus a stored registration referring
to a removed task cannot grant execution rights, even if its Boolean fields remain
in another object. A replacement must publish a fresh wait.

`..task.operation` owns one prepared native registration. Its reference parameter
is `source& ..task.events`; its result is an ordinary `..task.token`. Calling it
first retires its previous registration, then prepares a fresh token. Destroying
it unlinks its registration and releases its native endpoint. Its default state
owns nothing. It is deliberately noncopyable: a copied token is not an independent
operation or callback right. Capacity/allocation failure after restart leaves the
old operation retired and the new result unpublished; it is a technical failure,
not rollback of an external operation.

The operation should be an owned private field of the task that needs it. A
suspended task can then be removed without running anything after `%`. Native
provider code may clone its endpoint for asynchronous work; such work owns only
native resources and immutable tokens. This component supplies retirement, not
physical interruption of a worker. The provider still acknowledges its own rights.

## Exact native retirement

`ama_native_event_service_detach` takes the service, the full token and an output
Boolean. Under the existing metadata gate it removes exactly that live cell:

- Prepared cells return to the free list.
- Armed cells also decrement the pending-producer count.
- Completed cells unlink from the FIFO in constant time. Their byte owner is
  released after leaving the gate; survivor order is unchanged.
- Stale, foreign or already closed identities succeed with `retired=false`.

Null arguments fail without changing the output. No callback, allocation or byte
release runs inside the gate. Retirement never clears the notification latch:
an existing or in-flight wake credit still belongs to the sole waiter. This
preserves the [binary notification invariant](scheduling-rounds.md).

For a slot identity `(domain, service, slot, generation)`, detachment removes
membership before reuse. Reuse advances the nonwrapping generation. Therefore an
old callback either wins publication before detachment, in which case retirement
removes its queued bytes, or observes a stale token afterward. Neither ordering
can publish into a newly armed cell. Outstanding callback and byte rights keep the
closed native service and code lease alive until their final acknowledgments.
This proof does not depend on callback timing or allocator address reuse.

## Checked native reads through references

The generated reference read facade now has a typed `with_read` visitor. It
resolves the explicitly supplied checked handle, verifies nominal type, lifetime
and read availability, then supplies a temporary const full view to a synchronous
C callback. A refused access does not call the visitor. The visitor's checked
status is returned unchanged. The private binding scope lives on the C stack;
**the nested view expires when the visitor returns**, even if the enclosing native
entry has not finished. A provider must retain its own native resource credit
before returning, never the view, its payload address or an Ama reference.

This is a generated adapter under
[CABI-03](../../spec/implementation/native-interface.md#cabi-03) and
[CABI-05](../../spec/implementation/native-interface.md#cabi-05), not an Ama
execution callback. It does not expose execution metadata or dispatch Ama code.
The operation constructor uses it to acquire native rights from `source&` without
putting persistent Ama references into its native payload.

## Evidence and source boundaries

The per-file executable map is in
[ready.json](documentation/ready.json). These independently runnable suites cover
the actual boundaries:

- `make -C tests/unit/async/events/detach`: each FIFO position, survivor ordering,
  native bytes, prepared/armed retirement, idempotence, foreign tokens, closed
  services and exact resource counters. Implementation:
  `src/native/events/submission/detach.c`, public header and guarded assembly.
- `make -C tests/unit/async/events/callbacks`: a real worker holds bytes across
  detachment and immediate cell reuse; its late post is stale and cannot complete
  the replacement. Existing close/acknowledgment tests retain resources until
  actual thread join. This harness also runs under ThreadSanitizer.
- `make -C tests/unit/codegen/reference-visitors`: live, empty and retired references
  execute through compiled generated adapters; refusal never enters the visitor.
  The emitter and public facade declarations live in `src/codegen/emit/views/`
  and `src/native/scaffold/emit/types.c`. Existing runtime native-borrow tests cover
  nominal mismatch and busy access.
- `make -C tests/unit/codegen/task-detachment`: one-cell capacity exposes leaked
  registrations across restart, source reconstruction, queued-completion retirement,
  list-slot reuse and removal of a suspended task. No continuation runs during
  destruction; context teardown also closes the source before its retained task
  endpoints. The maintained scenarios are under its `project/` directory.

All view access and owner operations occur on the single Ama executor. Only native
posting/acknowledgment may race. [Cancellation arbitration](task-cancellation.md) and
[copy/failure boundaries](task-copy-and-failure.md) build on this component.
Loadable-host acceptance remains T099.
