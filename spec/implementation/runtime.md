# Runtime storage and control transfers

[Specification index](../README.md)

<!-- chapter-contents -->
In this chapter:

- [Explanation: state stays with its instance](#explanation-state-stays-with-its-instance)
- [Pools, identities, and availability](#pools-identities-and-availability)
- [Execution directly over instance storage](#execution-directly-over-instance-storage)
- [Return links and resumption positions](#return-links-and-resumption-positions)
- [Root execution and host control](#root-execution-and-host-control)

<!-- /chapter-contents -->

## Explanation: state stays with its instance

An Ama call changes which instance is executing. It does not move that
instance's fields into a shared working area, and it does not place an Ama
activation on the C stack. The callee's checked identity and explicit return
link are enough to select its stable storage and later select its caller again.

For example, if one job calls another job of the same type, both payloads stay
in their own slots. The first job is busy while the second runs. When the
second completes or yields, result processing finishes under protection and
the first job continues. A saved position after `%` is separate from that
outstanding return link.

Each execution context has its own pools and current-instance selection. This
chapter describes one context. The [parallelism contract](../language/async-and-parallelism.md)
allows several isolated contexts, each with one Ama executor, rather than
concurrent mutation of these same pools.

## Pools, identities, and availability

### Pools belong to an execution context

<a id="rt-01"></a>

**RT-01.** Each execution context creates a dynamic heap pool for each
participating executable module type. Shared `@=` storage has one representation
per module identity in that context, outside all instance pools. Constants-only
modules have no instance pool. Different contexts do not share mutable pool or
constant-lifecycle metadata; see PAR-01 through PAR-03.
Capacity, live instance count, and the set of available slots are separate values.
Allocating initial capacity does not initialize every slot or require executing
module bodies. Pool existence never implies a default live instance. Apart from
the root described below, live objects are created through explicit ownership,
collection, and copy operations. The exact initial capacity remains an
implementation policy, independent of object creation.

### Create one explicit root

<a id="rt-11"></a>

**RT-11.** After that context's shared constants have initialized successfully,
context startup selects one executable entry module and creates one root
instance of its type, initializes its owned state and interface defaults, and
starts it at entry. This is a host startup rule, not permission to execute a type
path in Ama source. Initialize owned children recursively without executing their
bodies; lists without initializers are empty and references without initializers
are empty handles. Initialization failure must clean up constructed resources
without starting the root body. Returning from the root reaches the runtime root
sentinel. Root completion, yield, and failure return control to the host under
RT-14 through RT-16; startup does not introduce an implicit scheduler.
The initial root entry uses the interface just constructed; it does not reset
or construct that interface a second time before executing its first statement.

### Keep state and control metadata in stable slots

<a id="rt-02"></a>

**RT-02.** A slot stores its permanent data, retained interface values, identity
metadata, acquisition state, saved resume position, and service control fields.
Active foreach cursors and declared scoped reference cells are compiler-visible
control storage in that instance, not opaque native payloads or separate heap
activations. Each lexical foreach needs at most one cursor/cell per active
instance; same-type instances keep independent state.
Pools must preserve logical slot identities across growth. Use independently
allocated blocks with stable slot addresses: growing the block directory must
not relocate live instance payloads. Raw C addresses must not escape as Ama
references; references remain checked slot/generation identities.

### Reuse free slots without reviving identities

<a id="rt-12"></a>

**RT-12.** Free slots form an intrusive free list: pool metadata stores its head,
and an unused slot stores the next free slot identifier. Reuse a free slot before
allocating another block. Free-list order need not follow increasing slot number;
lowest-address reuse is not a language guarantee. Occupied-slot identity and user
list order are independent. Allocation must establish a valid generation and
initialized state before publishing the new instance.

Reserve the new instance's final typed slot and nonzero generation before any
of its declaration initializers run. Keep it constructing and unavailable to
external entry, resume, read, or acquisition until construction succeeds. Its
initialization context may form and copy its identity with standalone `&` without
accessing an uninitialized field or native payload. On construction failure,
clean up the initialized parts and invalidate the reserved identity under
REF-04 through REF-06 before release or reuse; no issued handle may later become
valid for a different object. This reservation does not execute a module body.

### Reclaim only safe trailing storage

<a id="rt-13"></a>

**RT-13.** Pool memory may be reclaimed from fully free trailing blocks only;
no live payload is compacted or moved. A partly occupied block cannot be released
merely because half its slots are free. Remove reclaimed slots from free-list
links before releasing a block. Preserve sufficient generation/identity metadata,
or permanently retire released identities under REF-05 through REF-06, so
reallocation of a trailing block never makes a previously stale handle valid again.
Never return generation-exhausted slots to the free list. Reclamation timing,
block size, and free-list strategy are implementation policies. Allocation,
removal, and reclamation must not silently introduce per-instance heap allocation
for ordinary slot storage.

### Protect storage while it is in use

<a id="rt-03"></a>

**RT-03.** Use explicit acquisition to prevent deletion or conflicting mutation
while storage is accessed. Single-threaded execution still requires protection
against nested operations. Pool growth may allocate blocks and move directory
entries, but must not move existing instances. Resolve checked handles before
access; stable addresses do not make a deleted or reused slot valid. List buffer
addresses have separate lifetimes and must not be mistaken for stable slot addresses.

The following states describe required behavior, not a mandatory C enum layout:

| State | Meaning | External entry/resume/read |
|---|---|---|
| Free | No live instance in slot | Refuse |
| Constructing | Final identity reserved; initialization incomplete | Refuse |
| Ready/completed | Available, no saved position | Entry/read; refuse resume |
| Running | Currently executing | Refuse |
| Waiting | Suspended for a nested invocation, still busy | Refuse |
| Yielded | Available with saved position | Entry/resume/read |
| Acquired for transfer/destruction | Temporarily protected | Refuse conflicting access |

### Availability belongs to each instance

<a id="rt-04"></a>

**RT-04.** Availability belongs to an instance, not its module type. A second
available instance of the same module may execute while the first is waiting.
The same busy instance is refused. The language needs no manual `busy` field
checked by every programmer before invocation.

## Execution directly over instance storage

### Execute directly over instance storage

<a id="rt-05"></a>

**RT-05.** Instance data stays in its heap slot during execution. Generated Ama
code accesses permanent fields, parameters, and retained results directly through
the selected instance. There is no separate working area per module type and no
set of global variables mirroring every field of every module. The runtime needs
pool metadata, a current-instance identity, typed access pointers where useful,
and explicit control metadata, not duplicate copies of instance state.

### Enter a nested invocation

<a id="rt-06"></a>

**RT-06.** Before a nested invocation, acquire the target, record the caller
identity and return selector in the callee's service fields, and mark the caller
waiting/busy. Select the callee and jump to its code. Neither caller nor callee
payload is copied, moved, swapped, saved, or loaded as part of this control
transfer. Input binding and interface reset are separate semantic operations.
The caller's fields remain in its slot while another instance executes, including
when both instances have the same module type.

### Finish a body and process its results

<a id="rt-07"></a>

**RT-07.** On completion or yield, preserve the caller identity and return selector,
select the caller, and deliver requested results from the callee's retained
interface to the caller's declared receivers. Keep the callee acquired until
result transfers, CALL-09 output actions, and required cleanup finish; selecting
the caller does not make the callee available prematurely. Release the completed
body return connection once, update availability after protected processing, and
jump to the caller continuation. Release acquisition on supported error paths
as well. Result value cloning/reference copying follows
the type contract; returning does not restore or relocate a caller payload.
For CALL-09, retire a completed body's return connection before another selected
body executes, retaining its output-phase continuation and protection in the
initiating caller's service frames. A retained-read node creates no body return
link of its own.

Typed access pointers are implementation conveniences, not stored ownership or
Ama references. Reselect or reconstruct them from the current checked identity
at control-transfer boundaries; do not depend on C automatic locals surviving a
nested invocation. Ordinary cloning and explicit `%` continuation copying remain
separate operations and are never implicit consequences of invocation or return.

## Return links and resumption positions

### Record explicit return links

<a id="rt-08"></a>

**RT-08.** Service fields record the caller instance and a compiler-generated
return continuation selector. The resulting explicit return chain is allowed.
It does not use C stack activations and must not be confused with an independent
implicit activation stack. The entry invocation terminates at a root sentinel.

### Separate return links from saved positions

<a id="rt-09"></a>

**RT-09.** A return continuation belongs to an outstanding invocation; a saved
resume position belongs to a yielded computation. Store and validate them
separately. Yield releases the former and keeps the latter. A `%` copy inherits
only the allowed resume information, never a pending return chain.
Foreach bindings and cursor records belong to the latter control state and
follow FLOW-05/CONT-04's scope cleanup, validation, and reference-fixup rules.

### Clean up every exit path

<a id="rt-10"></a>

**RT-10.** Normal completion, early return, yield, acquisition refusal, initialization
failure, and shutdown must leave ownership, busy states, and control links
consistent. A failure path must not leave an instance permanently busy.
Execution failures follow the explicit-status and diagnostic protocol below,
not exception unwinding. Destruction of an available yielded root follows RT-15;
active-instance destruction is forbidden, and context disposal follows LIFE-05.
Task cancellation follows the explicit cooperative protocol in
[asynchrony and parallelism](../language/async-and-parallelism.md); it never
interrupts a running body or preserves a failed execution for resumption.

## Root execution and host control

### Return an explicit outcome to the host

<a id="rt-14"></a>

**RT-14.** The host-facing execution interface distinguishes three outcomes:
completed, yielded, and failed. Reaching `%` in the root stores its next position,
marks it yielded/available, and returns control to the launching C program. It
neither completes the root nor automatically resumes it. No active native C call
or C stack activation is preserved for later resumption; all continuation data
lives in explicit instance/runtime storage. Returning to and reentering the host
interface is a program boundary, not a C call per Ama module invocation.

### Resume or dispose a yielded root

<a id="rt-15"></a>

**RT-15.** After a yielded outcome, the host may explicitly resume the same root
or destroy it and release its owned state. Resume continues after the saved `%`
without resetting interface values or accepting new input bindings. A completed
root has no resume position. A failed execution cannot be resumed as though it
had yielded. The host interface must prevent concurrent or reentrant execution
of the entire context, including through another selected entry, and preserve
that context's pools and identities between resumes. Different contexts may
execute independently under PAR-01 through PAR-03.
Destroying a yielded root performs normal ownership cleanup, not continuation
execution. An explicit host may inspect retained results under the usual access
contract before deciding what to do next. Concrete C API names remain an ABI
implementation detail.

### Keep launcher policy explicit

<a id="rt-16"></a>

**RT-16.** The standard standalone launcher maps completion to successful process
exit and failure to a diagnostic plus unsuccessful exit. A root yield without an
explicitly configured host handler is a diagnosed unsupported host outcome and
also exits unsuccessfully after cleanup; it must not silently succeed, spin,
or resume in a hidden loop. A custom host may handle yielded outcomes explicitly.
Technical failures terminate the current execution context and return failed status
to that host after diagnostic reporting and supported cleanup. This is not a
recoverable Ama exception or permission to continue a failed computation.
