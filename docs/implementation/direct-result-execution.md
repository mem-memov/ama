# Executing methods retained in results

T066 connects [CALL-09](../../spec/language/calls-and-results.md#call-09)'s typed
operation trees to generated C. A selected owned result executes at its existing
pool identity. A reference result executes its checked referent. Neither path
constructs a caller receiver or clones the selected module.

## Keep the unfinished path in the caller

The existing current-call interface uses the initiating instance's `outgoing`
handle and its previous availability state. This identifies the target being
prepared, executed or read. A child operation needs that same interface while
the enclosing result owner remains acquired.

Each planned lexical operation has a fixed slot in generated instance storage.
A selected acquisition saves the enclosing handle and previous availability in
its slot, then installs the selected target as `outgoing`. The enclosing target
remains Acquired; it is never restored to availability while its descendant runs.
These are service records containing identities, not copies of module state.
Slot offsets are generated with `offsetof` from actual C storage. The runtime
checks caller identity, offset bounds and alignment before borrowing the slot.
Offsets are trusted generated descriptors, not a public foreign ABI.

After a child returns or yields, its ordinary return connection has already been
retired by the existing return helper. The child stays acquired while its output
actions execute. Releasing it restores the enclosing `outgoing` and clears its
slot. The generated successor label then runs the next sibling. A selected yield
therefore returns to the initiating chain normally; only the selected instance's
own saved position persists.

The slots follow native storage in the generated structure, outside declared
field descriptors and the typed native State. Construction/reservation and staged
replacement zero this area. Both ordinary and continuation clones traverse
declared fields, invoke typed native cloning and project permitted service state;
neither copies raw storage bytes containing operation slots. No pending caller
grant becomes saved continuation state. No C automatic activation survives a
generated body transfer.

## Separate the result owner from ordinary operands

The leading selected field belongs to the enclosing target's result interface.
Every nested index operand still belongs to the initiating caller. The iterative
selector accepts these two identities explicitly: it evaluates the finite index
chain in dependency order using the caller, then resolves the outer result base
in the protected parent. Each decoder runs once. Subsequent input sources and
ordinary receivers use the unchanged initiating caller.

Selection and acquisition form one contiguous emitted helper group. The IR's
selection/action markers remain label boundaries but do not repeat selection.
All arguments, interface installation and output transfers retain CALL-08 order.
The parent grant authorizes reading its declared result location, while the
selected instance still passes its own availability check. A reference to an
already acquired ancestor refuses Busy.

## Release and refusal invariants

For an active path of length `d`, `outgoing` names its final target. The `d - 1`
active selected slots name the preceding targets with their exact previous
availability states. All `d` targets remain acquired except while the final target
is executing through its ordinary Running/Waiting return protocol. A successful
push extends this invariant by one; a release restores its preceding prefix.
Acquisition validates selection and target availability before activating a slot,
so refusal preserves the previous prefix without a partial child grant.

On refusal, generated abandonment visits the refused link's frame ancestry in
reverse order, tolerates its unentered slot, releases active children and finally
the root. Only then can its own handler start a new root operation. The original
refusal status remains available for its snapshot; a cleanup protocol failure
stays technical. Fatal preparation/output errors use existing terminal context
cleanup and never enter a refusal handler. T067 supplies
[broader local handler acceptance](local-refusal-handlers.md), including deep
unwind, independent dynamic callers and handler suspension.

For finite source nesting depth `D`, service storage is bounded by the planned
lexical frame count. Each push/pop makes a constant number of exact registry/pool
lookups in addition to selection. With the current linear registry of `T` nominal
types, its service work is `O(T)` and abandonment is `O(d * T)` for the unfinished
source path, excluding index/native-protocol costs. These transitions allocate no
memory and use no recursive C dispatch. Output continuations are explicit IR
labels, so completed subtrees cannot resume a discarded sibling traversal.

## Evidence and limits

`make -C tests/unit/runtime/result-calls` independently checks every retained
ancestor across two pushes and pops, exact identity restoration, cleared frames,
Busy/empty refusal before mutation, invalid offsets, and normal versus abandoned
inactive-slot rules. Allocation failure is armed throughout transitions; their
allocation count stays unchanged.

`make -C tests/unit/codegen/trees` compiles and executes five compositions:
stateful nested siblings with a worker-clone hook that always fails; returned
reference identity and a caller index shadowed by the parent; technical decoder
failure bypassing handlers; owned/reference continuation resume; and a returned
self reference refusing reentry before its handler starts a fresh chain. Native
counter values and output ordering distinguish actual retained identities from
copies or incorrect sibling sequencing. Sanitizer flags instrument the generated
programs as well as the test runner.

T068 separately accepts [ordinary condition headers](condition-loop-execution.md).
Intrinsic list services and foreach cursors remain guarded pending their adapters. This implementation does
not turn static acceptance of those plans into a claim of executable support.
