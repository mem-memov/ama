# Context-local state and native dependency audit

T100 audits [PAR-01](../../spec/language/async-and-parallelism.md#par-01),
[PAR-05](../../spec/language/async-and-parallelism.md#par-05) and
[PAR-07](../../spec/language/async-and-parallelism.md#par-07).
The [generated-context contract](generated-contexts.md) explains execution;
the [runtime contract](execution-contexts.md) explains admission and cleanup.

## State ownership

| State | Actual owner and boundary | Evidence |
| --- | --- | --- |
| Descriptors, pools, resolver context | [Context preparation](../../src/runtime/context/storage/prepare.c) allocates private arrays and binds each resolver to its context's program. Schema arrays remain immutable. | Context/isolation tests compare owners and equal local handles. |
| Shared declarations and native constant resources | Each context owns a fresh constant ledger, payloads and construction credits before publication. Constants occupy no executable pool slots. | Concurrent constructor/destructor barriers; nested shared-storage cleanup tests. |
| Root/current selectors, return/resume positions, foreach bindings, acquisition | Context control, stable cells and owned fields. The [dispatcher](../../src/codegen/project/dispatch/begin.c) takes an admitted explicit view; C scratch is operation-local. | Generated-context execution barrier and runtime identity tests. |
| Failure, trace selection, refusal history, reporting | Context/world/program/cell records. Reporter and userdata are explicit native dependencies. | Context diagnostic/failure tests preserve peer state. |
| Registry membership and lease | [Metadata gate](../../src/runtime/context/registry/access.c), pending lifecycle credits and each context's exclusive admission. No allocator, hook or Ama body runs under the registry gate. | Simultaneous factories, native lifecycle barriers and callback tests. |
| Context names | [Atomic monotone frontier](../../src/runtime/context/identity/issue.c), independent of Ama state and never recycled. | Exhaustion and concurrent identity issuance. |
| Component setup and administrative mappings | Generated `call_once` publishes setup/status/definition. The [library gate](../../src/runtime/library/access.c) protects membership and explicit pin bookkeeping; a context lease protects that context's root list. | Library admission/identity/lifetime and component tests. |
| Creator-code credits and exported owners | [Code counter](../../src/runtime/library/code.c) has its own gate; the [native provider](../../src/runtime/library/native.c) grants independent credits without Ama access. | Runtime library and real worker/unload tests. |
| Event image domain and sequence | [Receipt publication](../../src/native/events/identity/source.c) uses atomic compare/exchange; a separate gate protects the nonwrapping sequence. | Event identity and physical reload tests. |
| Event queue, cells, wake state, endpoint/buffer counts | Each [native service](../../src/native/events/internal.h) has a gate; independent native rights keep storage alive after close. | Callback, wake, detach and real host worker sanitizer tests. |

Registry/service initialization and end of C lifetime require exclusive ownership.
Ordinary admitted operations may overlap for different contexts. The external
loader excludes new calls and waits for active calls before unload; a zero count
alone cannot prove that a final release has returned. See the
[asynchronous host](asynchronous-host.md).

## Standard-library native hooks

The maintained standard library has no mutable process-wide working value or
current-context variable. Native scalar state belongs to the selected payload.
`integer` owns its digit allocation and uses local arithmetic scratch.
`std.condition`, `std.index`, task flags/phases/outcomes/generations/tokens and
enrollment markers store scalar state and use immutable error bytes. Native
hooks do not select their instance through a process global or thread-local value.

`text` and `bytes` use immutable backing with a checked, non-atomic share count.
Clones inside one exclusively executed context may share that backing. Fresh
constant initialization and C-boundary decoding allocate independently in each
context. Consequently the share count stays within one executor. Cross-context
payload transfer or raw native shallow-copy is forbidden; these counts do not
support sharing a backing owner between independently running contexts. Exported
C values are independent transport snapshots, not pointers into that backing.

Task services and operations own native endpoints or service leases. Ama
references remain declared fields, accessed through synchronous generated
visitors. Event bytes have native synchronized ownership. A token is a name,
not an operation owner or an Ama access capability. Constant capability checks
exclude open external-resource identities. Their hooks still run separately in
every context and participate in the concurrency audit.

## Native dependencies and operating constraints

Shipped modules use the supported C/POSIX allocator, byte operations, integer
formatting, threads and event wake resources. Allocations are independent and
shared event resources have explicit synchronization/lifetime. No third-party
mutable singleton is required. The public asynchronous example's backend is a
single-host serialized FIFO: submission and `finish` must not overlap between
host executors. Its workers own separate jobs without accessing the FIFO. This
T099 example is not a reusable concurrent reactor for arbitrary contexts.

Before allowing a new native dependency to serve simultaneous contexts:

1. Identify each shared allocator, reporter, constructor/literal/clone/destroy
   hook, entry, reactor and external global, including failure cleanup. Startup
   and destruction need the same review as entry operations.
2. For every shared mutable resource state the exact lock/atomic owner or host
   boundary that serializes all access. Busy Ama flags do not synchronize native
   state. User allocator/reporter userdata is not made thread-safe by an Ama gate.
3. Declare executor affinity. A mutex does not make a thread-affine handle
   transferable. Exclude or synchronize process-global locale/environment changes
   and library shutdown with the routines that depend on them.
4. If these obligations cannot be met, serialize the dependency's complete use or
   execute affected contexts in separate processes. Native corruption, `abort`
   and process termination also need process isolation when containment matters.

These are provider preconditions under PAR-02/PAR-05, not new syntax or an inferred
classification of arbitrary C. Searching for globals cannot prove a user library
thread-safe. There is no fallback silently authorizing unsafe parallel execution.

## Argument and executable evidence

Let `M(c)` be mutable Ama storage reachable from context `c` and its owned
resources. Construction creates fresh descriptor/constant allocations; owned
children extend only that context's pools. Supported host input never imports
another context's Ama identity as a value. Therefore `M(a)` and `M(b)` are disjoint
for different contexts. Immutable schema/code may be common. Shared native
administration is a separate synchronized set granting no access to `M(c)`.

Origin checking precedes local lookup, so equal local triples cannot violate
this invariant. A lease admits one executor and publishes writes on release.
Hooks on disjoint payloads and synchronized native services may thus overlap
without racing on Ama state. This argument assumes truthful native providers and
explicit host lifetime authority; finite thread tests alone do not prove it.

`make -C tests/unit/runtime/isolation` runs
`it_isolates_context_storage_during_concurrent_native_constant_lifecycle`.
Both factories reach their own constant hook before either returns, and both
disposals similarly enter their constant destructor. The test checks distinct
resources/resolver targets, zero implicit instances, equal local IDs, foreign
rejection, private control/diagnostics, cloning, generation reuse without revival,
unchanged peer state and exactly six constructed/destroyed credits. Condition
variables and joins determine overlap; no sleep or guessed timing is used.

Run this leaf with strict `NDEBUG` (test assertions stay enabled), ASan/UBSan and
TSan using actual POSIX threads. Runtime `contexts`, `shared-storage`, `library`,
native event and generated `codegen/contexts` suites provide the other mapped
evidence. [ready.json](documentation/ready.json) records per-file commands.
T101 extends generated parallel execution acceptance; T102 addresses explicit
executor transfer. This audit claims neither message transport nor arbitrary
native-affinity support.
