# Checked host execution and context lifetime

This records T013's internal boundary under
[RT-14](../../spec/implementation/runtime.md#rt-14),
[RT-15](../../spec/implementation/runtime.md#rt-15) and
[EMBED-03](../../spec/implementation/c-library.md#embed-03).
The [context owner](execution-contexts.md) creates independent descriptors and
shared values without an implicit root. The host explicitly constructs its
selected root. `src/runtime/context/execution/` then owns one accepted execution
through a linear context lease. It dispatches no Ama module through a C function;
actual generated context wiring remains T061, and public transport/exported
consumer declarations remain T123 onward.

## Admission is separate from execution outcome

`ama_context_execution_begin` receives an explicit context identity, checked root,
compiler-resolved invocation descriptor and empty `AmaContextExecution`. Its
return status answers whether execution was accepted. The invocation target is
NULL because the checked root already selects it. Only Entry and Resume are
valid; Resume accepts no new input bindings. Compiler descriptors and literal
views are borrowed for the call, not retained by the execution owner.

The function checks request shape, obtains the context gate, then validates root
origin, publication, generation, availability and requested mode. Missing resume
position, stale/foreign/empty identity, Busy context and invalid request reject
admission. They release any temporary lease and leave interface values, results,
refusal history, ownership and control phase unchanged. A rejected request does
not produce an execution outcome and cannot turn a healthy context into Failed.

After those checks, the accepted owner holds the context gate and records the
selected root/current identity. Entry runs ordinary protected preparation:
parameters first, then results, each with its selected initializer/source and
per-field commit. Persistent body fields and native payload are retained. The
root temporarily acts as the host's source context for these resolved bindings;
no implicit Ama caller instance or C body is created. A failure from this point
is an accepted technical failure. Begin still returns success, with the first
preparation status in `execution.status`; generated code skips body dispatch
and must finish that execution. This avoids disguising decoder, constructor or
allocation failures as harmless host admission refusals.

Resume copies the saved nonzero selector into `execution.selector`, consumes the
slot's saved position and establishes the host return sentinel. It does not run
preparation, allocate interface state or accept replacement inputs. Entry uses
selector zero to indicate normal initial dispatch. Both healthy paths mark the
selected receiver Running; generated label code uses `control.current` and keeps
`program.current` synchronized for diagnostics. The owner and its lease must not
be copied or released independently.

## Finish, yield and failure

The generated execution kernel returns/yields through the ordinary explicit
return-link helpers. They detach the host connection and make the selected root
Ready or Yielded. `ama_context_execution_finish` validates that state before
publishing Completed or Yielded: current must be the selected root, there must be
no pending outgoing call, and availability/resume metadata must agree with the
requested outcome. A completed root has no resume position. A yielded root has
one and preserves its retained interface and persistent state.

All native borrows must already be closed. Finishing with an active world
operation or an invalid output/owner argument rejects that internal operation
without consuming its execution owner; the caller corrects the mistake and
finishes normally. This API is for trusted generated/runtime orchestration,
not a public capability to modify control structs. Native code receives only
its [bounded typed views](native-borrows.md).

A checked technical status, malformed status, impossible requested outcome or
incoherent return metadata produces Failed. The first already-recorded context
failure wins, followed by accepted preparation status and the kernel's returned
status. Runtime operations report primary failures before their own secondary
cleanup; this ordering preserves their original code, immutable message pointer
and length. A zero-status request for Failed becomes InvalidState rather than a
fabricated successful failure. A technical status cannot publish a healthy outcome.

The primary diagnostic is reported while the current instance and its explicit
caller chain are intact. If it was already reported by a runtime operation, finish
does not report it again. Cleanup then destroys every remaining independent root
in reverse successful creation order, constants in reverse initialization order,
and all pool/descriptor allocations. Secondary cleanup errors are still reported
without replacing the first failure. Returned Failed carries an empty checked
root and the primary status.

The failed context record remains registered only for diagnostics and explicit
cleanup. Its control phase is Terminated, root/current are empty, and every
arena-owned descriptor pointer/count has been cleared. Ordinary reference
resolution rejects the failure before consulting those tables. Later explicit
context disposal safely frees the record without destroying resources again.
A disposed context identity is stale. The code image, immutable message storage,
allocator and report callback retain their existing context-lifetime obligations;
owned public diagnostic snapshots and component unloading are later export tasks.

## State and ownership argument

| Boundary | Phase and gate | Remaining owners |
|---|---|---|
| Context constructed | Idle; gate available | Constants/descriptors, no implicit root |
| Request rejected | Same phase/state as before | Same owners |
| Request accepted | Executing; execution lease held | Same context plus prepared interface changes |
| Preparation reports failure | Terminated; execution lease still held | Failed prefix awaiting finish cleanup |
| Completed/Yielded returned | Idle; gate available | Retained healthy roots and constants |
| Failed returned | Terminated; gate available | Diagnostic record only |
| Explicitly disposed | Identity removed | No context resource |

A successful gate acquisition pins the context record and excludes any second
executor, alternate root entry, reentrant callback or disposal. Checks before
admission have no Ama mutations. After admission the linear owner follows one
finish path, which either preserves healthy storage or consumes every resource
before releasing that gate. Clearing freed descriptor views makes later record
cleanup idempotent. These invariants compose with the initialized-prefix ledger,
root-order list and first-error accumulator; none needs rollback after destructive
cleanup, an emergency heap allocation or retained C activations.

Two distinct contexts have different gates and owners. A failure of one does not
change another's current identity, root, values or saved selector. Context admission
uses the existing registry/context gates; this component introduces no mutable
Ama globals, implicit scheduler or universal default instance.

The registry search is O(C) for C live contexts and ordinary type resolution
O(T) for T context types. Begin/end's added control checks use O(1) storage;
entry preparation has its separately documented transfer/allocation costs.
Resume preparation performs no allocation or interface reconstruction. Failure
reporting/cleanup has the existing finite caller/ownership traversal costs and
uses no runtime allocation. Caller native hooks must terminate for cleanup to
terminate; the runtime does not interrupt them.

## Executable evidence

`tests/unit/runtime/host-lifecycle/` explicitly exercises eleven cases:

- Entry defaults and supplied bindings reset parameters/results in order while
  native persistent state survives; yield/resume preserves results and consumes
  the selector; completion refuses another resume.
- Empty/stale/foreign/invalid-mode and resume-with-input requests preserve values,
  service history and phase, without allocation.
- Every measured entry-preparation allocation fails after admission as Failed;
  only the diagnostic record remains, and explicit disposal frees it.
- Failed execution preserves exact primary message bytes despite a different
  destructor failure, cannot resume, rejects ordinary resolution and disposes
  without repeated hooks or diagnostics.
- Empty, completed, yielded and failed disposal paths release their owners;
  yielded disposal neither constructs nor dispatches a saved continuation.
- Incoherent return/outcome/status cases cannot publish healthy execution;
  active native views and invalid finish arguments retain their linear owner.
- Two sequential contexts hold independent execution leases; failure in one
  leaves the other's yielded value/selector intact. This case runs before thread
  probing. A worker and alternate-root entry then both refuse the active context
  without touching counted allocation/native work; report callbacks also refuse
  entry/disposal until failure cleanup finishes.
- A real acquire/prepare/connect chain reports the callee then caller before
  cleanup, with the next runtime allocation forced to fail. Every root, constant
  and descriptor is released and the same diagnostic is not rebuilt on disposal.

The existing `tests/unit/runtime/contexts/` separately covers startup prefix
failure, shared-value ordering, reverse root order, registry closure and concurrent
independent leases. Generated public headers, encoding/snapshot transport, launcher
policy integration and component-qualified C ABI remain their named roadmap tasks.

T013 acceptance passes the full strict `-DNDEBUG -Werror` test tree, focused
host/context ASan/UBSan leaves (leak detection disabled), complete coverage of
757 maintained inputs and all Markdown links. Warning-free Doxygen checks 585
maintained file pages, 2419 interfaces and all 288 canonical/page destinations;
two complete generations produce byte-identical output.
