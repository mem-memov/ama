# Generated host execution in isolated contexts

T061 connects the existing context factory and execution lifecycle to the C
assembly under [PAR-01](../../spec/language/async-and-parallelism.md#par-01),
[RT-11](../../spec/implementation/runtime.md#rt-11),
[RT-14](../../spec/implementation/runtime.md#rt-14),
[CALL-08](../../spec/language/calls-and-results.md#call-08) and
[DEBUG-02](../../spec/implementation/diagnostics.md#debug-02).

## Immutable program, independent owners

Emission produces actual C storage types, immutable field/declaration/source
arrays, `AmaContextType` schemas, shared declaration schemas and a
`AmaContextDefinition`. Only executable types appear in the context factory's
type table. Nominal IDs still come from exact planned identities and may have
gaps for constants-only namespaces; no placeholder becomes a mutable object or
a pool. Native binders select their pool from the active borrow's world by exact
nominal ID, never from an assembly-global descriptor.

Each `AmaProjectHost` owns its registry and context identity. The existing
factory allocates its world, program, type descriptors, pools, constant storage
and lifecycle ledger. No mutable Ama descriptor, constant or execution record is
shared between hosts. The runtime image retains its monotone atomic context-ID
issuance frontier, as in T006: it is identity bookkeeping, contains no Ama state
and never recycles an old host identity. All generated execution state is private
to a context or the currently admitted operation.

The host's atomic gate covers first factory creation, execution and disposal.
Once created, an accepted execution also holds the existing linear context lease.
Different hosts can execute concurrently; a second operation on one host receives
Busy without running Ama or native code. Result/status values are copied before
the gate's release. Disposal acquires the same gate, consumes the registry and
retains its first status for repeated calls. A live diagnostic lease refuses
disposal without marking the host disposed. Registry destruction consumes its
initialized owner exactly when disposal was admitted: this postcondition also
distinguishes a native destructor returning Busy as a technical error. That
accepted failure consumes the host and must never be retried as refusal. The host C object must outlive all
threads/operations using it and must not be copied.

## First root and later invocations

`ama_context_execution_start` explicitly selects the first root's nominal type
in a rootless idle context. Context constants already exist. Root construction
uses final stable pool storage and initializes declared parameters/results once.
It then attaches the host return sentinel and admits label execution without a
second interface preparation. A failed constructor still belongs to the accepted
execution owner, with an empty local root, and must finish through terminal cleanup.

Subsequent entry/resume uses `ama_context_execution_begin`: entry prepares the
existing root interface, resume consumes only its saved selector. The execution
function's current-instance alias refers directly to context control storage;
its program current identity remains synchronized for diagnostics. Native borrows
close before `ama_context_execution_finish`, which validates healthy return or
reports and cleans an accepted failure while the lease still pins the context.

Rejected admission, including resume without a position, leaves a healthy host
available. It does not become a terminal execution failure. An accepted technical
failure cleans resources, disposes this host's context and returns an empty root.
The temporary `active` view is installed only while the execution owns its lease
and is cleared before finish releases it. It is runtime authority, never an Ama
field or a native-payload resource.

## Failure before context storage exists

A factory operation accepted by the registry can fail while allocating its own
record or descriptor arena. Its definition carries the explicit generated
initialization operation ID. Before cleaning a failed private context, report a
previously unreported primary cause through its normal reporter. Before the
record itself exists, a stack-only diagnostic world/program view borrows the
same immutable source table and reports that operation without allocating or
creating an Ama instance. Validation/admission rejection does not report an
accepted execution. Existing primary/secondary ordering remains intact.

## Invariants and evidence

Each context receives fresh mutable descriptor/constant arrays from its own
arena; immutable schema pointers are the only shared program data. Exact
context-ID checks precede local-handle lookup. Thus equal local triples in two
contexts cannot authorize cross-context access. Acquire/release gates establish
visibility before the next executor can inspect a context, and one live lease
pins all world/program pointers through generated label dispatch. No Ama payload
is copied to working globals during a call or context switch.

`make -C tests/unit/runtime/host-lifecycle` runs
`it_starts_an_explicit_root_once_under_the_execution_lease` and
`it_cleans_every_failed_first_root_construction_without_publishing_a_root`.
They count literal/clone/native constructors, reject competing/repeated startup,
and fail every first-root allocation before accepted cleanup.

`make -C tests/unit/codegen/contexts` runs
`it_interleaves_independent_generated_contexts_with_private_constants_and_pools`
and `it_executes_two_generated_contexts_in_parallel_and_refuses_same_host_reentry`.
The first holds two independent yielded hosts, inspects both through simultaneous
leases, checks unchanged Busy refusal of disposal/execution while a lease is
held, verifies distinct descriptor/constant owners, rejects a foreign checked
reference with an equal local triple, and disposes one while the other resumes.
The second uses two real C threads and a test-only native barrier proving their
execution intervals overlap. Reentry/disposal of each active host is refused.
`it_distinguishes_busy_native_destruction_from_refused_host_disposal` checks
the contrasting accepted Busy destructor failure, terminal host state and
idempotent disposal. The barrier and thread-local host pointer are test-controller synchronization,
not generated Ama state or permission for native payloads to hide Ama references.

Existing compiled cases exercise allocation-free pre-record diagnostics,
constructor/native failure, source chains, secondary cleanup, unhandled root
yield and continuation entry/resume/disposal. Layout probes now hold a context
lease for all descriptor, growth and native-borrow observations. Per-file coverage
maps every changed maintained code/header/test to named component/composition
cases; headers retain standalone compilation checks. The public compiled-library
transport remains the embedding roadmap's separate boundary.
