# Explicit quiescent executor transfer

[PAR-02](../../spec/language/async-and-parallelism.md#par-02) permits a new host
thread to own an idle context only after all conflicting access ends and every
native resource permits the change. This T102 implementation uses the existing
[context admission](../../src/runtime/context/admission.c) release/acquire gate
and an explicit synchronized host ownership boundary. No Ama payload, pool slot,
root identity or saved position is moved or copied.

## Two distinct obligations

The context gate admits only one executor or retained host access at a time.
Leaving a lease performs a release-store after all context writes; the next
successful entry uses acquire ordering before reading those writes. An active
native entry holds that admission, so a competing entry or inspection cannot
obtain a view. A diagnostic lease also excludes an executor until it ends.
Moving a linear lease between C storage locations is not itself a thread handoff
and does not expire a native borrow.

The host must also publish its own context owner and related native bookkeeping.
A mutex/condition handshake, a joined-worker publication or another exact host
synchronization protocol establishes this boundary. The old executor must no
longer execute or inspect mutable context storage, and the new executor cannot
start before publication. Every ordinary host accessor must obey that ownership
protocol; a one-time availability check followed by uncontrolled concurrent
access does not authorize transfer.

Finally, every live dependency must permit migration. A mutex proves exclusion,
not thread affinity. A provider whose native resource must remain on its creator
thread requires a pinned executor until that resource is retired on the required
thread. The host learns this from the provider's declared resource contract,
not compiler guesses about C source or successful trial calls. See the
[native dependency audit](context-isolation-audit.md).

## Concrete host protocol

The maintained [consumer controller](../../tests/unit/codegen/executor-transfer/consumer/controller.c)
uses two permanent POSIX threads and one stable `AmaProjectHost`. All command,
owner and completion publication uses its mutex/condition variable:

1. While a command is pending or active, a transfer request returns Busy and
   changes no owner, Ama state or saved position.
2. At rest, the [handoff operation](../../tests/unit/codegen/executor-transfer/consumer/handoff.c)
   checks the explicit native policy. A pinned dependency refuses a different
   owner before running any Ama or native operation on that thread.
3. Core admission checks that no retained diagnostic/native borrow owns the
   context. After successful preflight, the controller publishes the new owner
   under its mutex. Its command protocol excludes any competing host access
   throughout that boundary.
4. Only that worker consumes the next command. Completion publication makes all
   writes visible before the controller inspects results or selects another
   owner. An owner change preserves the host object and numeric Ama identities.

This is a reference host composition using actual runtime admission, not a new
Ama syntax or an automatic scheduler. The runtime does not invent affinity
metadata for arbitrary native dependencies. A production host can use its own
synchronization with the same obligations and happens-before relation.

Pending native work can survive transfer when its provider contract permits it:
independently owned endpoint rights have no Ama access. The callback queues a
record, and only the newly admitted executor imports it. A callback cannot make
a pinned native resource transferable, enter Ama or retain a borrowed view.

## Evidence and argument

`make -C tests/unit/codegen/executor-transfer` runs two compiled scenarios:

- `it_transfers_a_quiescent_executor_with_explicit_publication_and_no_live_borrow`
  holds an actual native entry, refuses competing transfer/entry/disposal, then
  yields. A retained inspection lease blocks transfer without changing owner.
  After release, worker zero hands off to worker one; both root identity and
  saved selector stay unchanged. A pending native endpoint delivers after the
  handoff, ordinary Ama takes the packet, and a second yield is later resumed
  back on worker zero. The exact native call sequence is preserved. Disposal
  closes the service; a late post is rejected while its native right still retires.
- `it_keeps_a_thread_affine_native_resource_on_its_required_executor` constructs
  a resource tagged with its actual `pthread_t`. Its entry and destructor check
  the real OS thread. The declared host policy refuses migration after yield;
  resume and final cleanup stay on the constructing worker.

The generated consumers use `NDEBUG` with checks that remain effective. Run both
scenarios under ASan/UBSan and TSan, instrumenting the generated runtime/native
code as well as the compiler-side harness. No elapsed-time oracle or migration
of active C stack frames is used. Named per-file evidence and standalone headers
are registered in [ready.json](documentation/ready.json).

For a successful handoff, let `W` be the completed old executor's context writes.
Its lease release publishes `W`; the host mutex release publishes its owner and
command metadata; the new worker acquires that metadata and then acquires the
context gate. Thus `W` happens before the new executor's reads. Exclusion keeps
both executors from owning the context together. Because only ownership metadata
changes, stable slot addresses, generations and saved selectors are invariant.
Refusal publishes no owner change. Native affinity is a separate precondition,
checked before publication rather than inferred from mutual exclusion.
