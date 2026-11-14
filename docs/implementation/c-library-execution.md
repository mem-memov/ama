# Checked C entry, resume and execution outcomes

Selected C exports now have real entry and resume implementations. They execute
the same resolved Ama labels as the ordinary launcher, preserve instance state
between calls and return completed, yielded or failed without an implicit
scheduler. Inputs and snapshots use the [typed transport](c-library-transport.md);
[factories](c-library-lifecycle.md) still create explicit instances without running
their bodies. Static/shared artifact packaging is the next boundary, T128.

This implements [EMBED-03](../../spec/implementation/c-library.md#embed-03),
[EMBED-05](../../spec/implementation/c-library.md#embed-05) and
[RT-14](../../spec/implementation/runtime.md#rt-14). The consumer header declares
only the modes selected by the export configuration. A read-only export cannot
execute an instance through another export's handle, even when both name the
same nominal Ama type.

## One dispatcher for every explicitly prepared root

The generated project has one private `ama_project_dispatch` function containing
all Ama control labels. Entry selects the exact runtime type's entry label;
resume selects a saved IR selector and verifies its nominal owner type. Ordinary
Ama calls and returns remain jumps inside this dispatcher under
[STACK-01](../../spec/implementation/compiler.md#stack-01). Native entry bodies
remain inlined and their ordinary C helpers can use the C stack. The existing
object-code verification still rejects native entries or Ama selector names that
survive as C-callable body functions.

The ordinary launcher and checked library calls both use this dispatcher. Their
host orchestration differs: standalone startup creates its explicit first root
once without a second interface reset; a C library factory and later entry are
two separate operations. Entry resets parameters/results even on a separately
created root's first call, while persistent Ama/native body state remains intact.

A native method fixture with its own resource and call counter demonstrates this
distinction. Creation initializes the resource but does not run the method. The
first and second entries return different counter-derived values; the root's
resource and address remain stable while its interface resets normally.

## Admission before mutation

The inline public wrapper first checks the complete component ABI key and layout.
The implementation then validates output pointers, initialized-empty distinct
result-owner slots, nonoverlap of the outcome destination with those slots, and
entry input shapes. Bounded region overlap uses checked unsigned address
differences for the supported target rather than overflowing end-address sums.
These checks precede context access or interface reset.

The runtime resolves exact service/context/export/instance identities and acquires
one context executor lease. A busy context or target, stale/foreign control,
wrong export or missing saved position rejects without executing, preparing
inputs, consuming a continuation or publishing outputs. Different roots do not
bypass the context guard. Native callbacks cannot reenter the running context,
including through retained reads, diagnostics, instance creation or disposal.

API rejection is separate from an accepted Ama outcome:

| API result | Outcome destination | Meaning |
|---|---|---|
| Invalid request, busy, refused, or ABI mismatch | Unchanged | Admission did not execute/reset the target. |
| API OK, completed | `COMPLETED` | Root return completed normally; independent requested snapshots are published. |
| API OK, yielded | `YIELDED` | Root `%` returned normally; snapshots and the saved position remain available. |
| API OK, failed | `FAILED` | Accepted preparation, body execution or snapshot construction failed technically; the diagnostic is returned after context cleanup. |

An application `..failure` result is still one ordinary value. Its true condition
and owned description can accompany completed or yielded execution with API OK
and an empty technical diagnostic. It does not terminate the context.

## Preserve one executor owner through publication

`ama_context_lease_move` transfers a linear C owner into initialized-empty storage
without releasing the executor gate. The source becomes empty. This is an
explicit ownership move, not a second independently releasable lease.
`ama_context_execution_begin_leased` prepares entry or consumes resume using that
already-held owner. Entry's argument/installation-site arrays live in checked
arena storage; the arrays and borrowed C recipes are destroyed immediately after
synchronous preparation, before body dispatch.

`ama_context_execution_finish_retained` validates the host return and performs
supported terminal cleanup while retaining the lease for result construction and
host identity bookkeeping. The original consuming begin/finish APIs are wrappers
around these same operations. Existing standalone and embedding paths therefore
share validation, primary-failure handling and native-borrow invariants.

Normal completion/yield becomes idle while the executor remains held. Selected
snapshot owners are built privately and published together under that same
lease. If post-execution snapshot construction fails, the accepted operation
returns failed and leaves every output empty. A result that becomes unavailable
after execution cannot be reported as an effect-free admission refusal; its
requested snapshot failure terminates the accepted context. Callers can skip
unneeded outputs with NULL destinations.

On technical failure, core cleanup consumes roots/constants once. Library root
identity mappings are forgotten before executor access is released. The context
retains only diagnostic/cleanup access. Saved continuations are never resumed
automatically during cleanup. A retained-read busy refusal remains effect-free;
it is a different operation from an already accepted execution's output failure.

The generated runner's internal ownership/finalization assertions state compiler
invariants: empty move destinations, valid retained leases, and closed native
borrows before finalization. Ordinary native/decoder/allocation errors take the
failed-outcome path and do not trip these assertions. Both assertion-enabled and
`NDEBUG` component builds exercise the same technical-failure cases.

## Explicit yield and restart

A C call returns once when its selected root completes, yields or fails. A yield
does not install a timer, resume loop or pending native C activation. A later
explicit resume keeps parameters/results and enters the saved label. A fresh
entry discards the old position, resets its interface and starts at entry.
Resume before entry, after completion or through an unavailable export rejects
without changing the outcome destination or retained values.

Several roots can coexist in one context. One may be yielded while another
explicit export executes. Resuming the first root still uses its own saved
position and nominal type, rather than the context's most recently executed
root. Independent contexts retain separate mutable state and failure records;
a technical failure in one does not restart or poison another.

## Executable evidence

`make -C tests/unit/library/execution` generates real public C entry/resume/read
implementations and a separate public-header-only consumer. It checks:

- actual Ama `%`, explicit resume/completion, missing-resume refusal, restart of
  completed/yielded roots and preserved old snapshots;
- multiple exported root types and a read-only alias, wrong-context/export
  controls, output owner aliases and partial outcome/owner overlap;
- native method state/resource/address persistence and callback rejection through
  entry, same/other-root reads, diagnostics and disposal while a context runs;
- normal yielded application-failure values versus accepted technical failure,
  retained primary cause despite a Busy cleanup error, and another context's
  continued operation;
- every measured entry and resume allocation failure, requiring API OK/FAILED,
  unpublished empty result slots and zero remaining ownership after cleanup;
- zero-schema/zero-parameter/zero-result interfaces without zero-length C arrays,
  selected-mode C compilation failures and both assertion-enabled/`NDEBUG` components.

The generated consumer/component builds also run with ASan/UBSan when the leaf
is built with sanitizers. `make -C tests/unit/runtime/contexts` directly checks
lease moves, already-leased begin, completed/yielded/failed retained finalization
and continued busy exclusion until the final owner releases access. Existing
host-lifecycle and generated-execution suites cover the ordinary launcher,
stackless calls, native borrow closure and source diagnostics through the shared
dispatcher.
