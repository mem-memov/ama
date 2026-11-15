# Parallel execution of generated contexts

T101 accepts the actual generated dispatcher under
[PAR-01](../../spec/language/async-and-parallelism.md#par-01),
[PAR-06](../../spec/language/async-and-parallelism.md#par-06),
[PAR-07](../../spec/language/async-and-parallelism.md#par-07) and
[PAR-08](../../spec/language/async-and-parallelism.md#par-08).
The [state audit](context-isolation-audit.md) records which storage is private and
which native dependencies need synchronization. The
[generated-context contract](generated-contexts.md) defines the implementation;
no implicit scheduler or extra Ama execution stack is introduced here.

## Execution boundary

The host supplies an explicit execution owner. Its atomic admission gate covers
first context creation, entry/resume and disposal; the actual dispatcher takes
that context's admitted view. Different hosts share code and immutable schemas,
not worlds, constant storage, roots, mutable fields or current selectors. A second
entry/disposal for an active host returns Busy before accessing its mutable Ama
state. The context lease remains held through result validation and publication.

Native callbacks retain independently owned endpoints and bytes, not Ama views,
handles, context pointers or execution authority. Callback publication may occur
while another context executes; it changes native queue state only. The receiving
executor later takes and imports that context's own packet using ordinary Ama
methods and control flow.

A technical failure cleans its own context and returns Failed. It does not cancel
another executor or overwrite another context's result/history. Independent native
callback rights can retire after failure; closed services reject their late posts.
The host must still honor the [native dependency contract](context-isolation-audit.md#native-dependencies-and-operating-constraints).

## Controlled schedule and evidence

`make -C tests/unit/codegen/parallel-contexts` runs
`it_runs_parallel_contexts_with_private_startup_reuse_events_and_failure`.
It compiles one real Ama project with shared integer declarations and task event
services, then executes the same generated code on POSIX threads:

1. Both initial entries reach their native bridge before either can return. Each
   bridge acquires its own native endpoint through a synchronous typed reference
   visitor. Both contexts then yield independently.
2. While both hosts are quiescent, checked leases establish disjoint constant,
   descriptor and control owners, despite equal local root triples. A foreign
   origin is refused. Additional explicit scalar roots reuse a pool slot with a
   fresh generation; stale local access remains invalid.
3. One context restarts and waits inside its native bridge. Competing entry and
   disposal are Busy. Its old service is closed, so its old endpoint is already
   unable to deliver to the new incarnation.
4. During that exact active interval, the host publishes owned binary bytes to
   the other context's native endpoint. That context resumes, takes a packet
   through Ama, verifies its code and exact binary bytes through typed views, and completes while its peer is still held inside the native call.
5. The peer's bridge is released with a technical failure. Its context is cleaned;
   both old and freshly retained endpoints can retire afterward. The healthy
   context can restart, receive another event and complete again before disposal.

Condition variables, joins and explicit ownership moves establish the ordering;
there is no guessed delay or retry count. The bridge never retains its supplied
view. Test thread-local storage identifies the native test controller only; no
production Ama selection depends on it.

The generated consumer, runtime and native services are compiled together under
ASan/UBSan and TSan, using the actual POSIX thread and endpoint handoffs. Test
fixture support selects the requested sanitizer for the generated executable as
well as the compiler-side harness. Runtime isolation, generated context and
native event suites supply complementary factory/failure/callback boundaries.
Per-file commands are in [ready.json](documentation/ready.json).

This acceptance concerns independently executed contexts. Explicit transfer
preconditions and affinity remain T102; typed messages, bounded channels and
supervision remain T103–T106. Passing this test cannot certify arbitrary linked
native code as thread-safe or provide process memory protection.
