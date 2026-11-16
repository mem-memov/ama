# Ownership services before module dispatch

T054 closes the pre-dispatch ownership composition checkpoint under
[REF-08](../../spec/language/values-and-references.md#ref-08),
[COPY-02](../../spec/language/values-and-references.md#copy-02),
[COPY-03](../../spec/language/values-and-references.md#copy-03),
[COPY-06](../../spec/language/values-and-references.md#copy-06),
[CALL-04](../../spec/language/calls-and-results.md#call-04),
[ARCH-05](../../spec/implementation/compiler.md#arch-05),
[ARCH-06](../../spec/implementation/compiler.md#arch-06) and
[TEST-08](../../spec/implementation/tools-and-conformance.md#test-08).
It verifies these ownership services independently.
[Generated execution](project-emission.md), [structured control](control-contexts.md)
and [live lists](list-traversal-execution.md) provide separate composition
evidence without changing the ownership model's domain.

## Independent model domain

The new `runtime/ownership-model` leaf owns one explicit current caller and two
optional independent scalar trees. Each tree has a root and two owned leaves,
actual native resources and declared self/peer/earlier-child references constructed
by the real program recipe path. The caller may hold one read acquisition on a
tree root or its left owned child through its own declared reference fields.

A model slot records public root/child identities, the root's stable cell, expected
logical native values and one optional opaque saved-position marker. The model
also records the caller's reference values/refusal snapshot, the retained grant
and every retired public generation. It does not inspect implementation clone
work records or infer correct state from actual service flags.

There are eighteen actions: create either tree, clone in either direction into an
empty slot, replace in either direction, ordinary exact self-copy of either root,
acquire either root or left child, release the retained grant, destroy either
root, seed either available root's normalized saved marker, and probe the current
Running caller through its self reference. Ineligible actions are explicit
scheduler no-ops; they do not masquerade as successful runtime operations.

The saved marker models service-copy data only. The test never resumes its
numeric selector or claims generated label membership. Actual entry/return/yield
and selector validation remain their dispatch tasks. This keeps the checkpoint
independent of executing an Ama body while exercising ordinary position clearing.

## Transition and invariant argument

Creation reserves a new independent tree and publishes only complete resources.
Cloning requires the complete source tree available and creates fresh identities,
resources and remapped self references; ordinary copied positions clear. Distinct
replacement requires both complete trees available, preserves the destination
root identity/cell and replaces its children/data after successful preparation.
Exact ordinary self-copy is effect-free for these statically clonable schemas if
the root itself is available; a busy owned descendant does not make a no-op
destructive. A busy root still prevents access. Destruction requires every owned
descendant available and retires the entire old public tree.

Read acquisition protects exactly its selected target and retains the previous
Ready/Yielded activity on the caller. Release restores that target. Whole-tree
clone/replacement/destruction must therefore refuse when either a root or one of
its owned children is acquired. A self-targeted Running caller produces a normal
Busy refusal and updates only its own snapshot. Starting another acquisition
while the caller already retains a grant is a technical invalid state, not a
new target refusal, and leaves the existing grant/snapshot intact.

After each transition the oracle checks complete typed identities, expected
logical values, self/peer/owner links, stable root cells, independent live native
resources, exact activity/resume/refusal/outgoing facts, stale historical handles
and population counts. Summed occupied reservations must equal the public live
population at each API boundary; no failed preparation may leave a hidden private
slot. The caller's own owned data and explicitly updated reference values remain
consistent, so an implementation cannot pass by saving/restoring an unrelated
caller payload. All operation guards and borrowed construction authority must end.

Induction over the action word establishes model refinement: successful operations
perform precisely the documented state transition; refusal/no-op leaves the public
model unchanged except the explicit caller refusal event. Required allocation
failure may stop before a later refusal is discovered, but it leaves public
ownership/value/grant state unchanged and releases its initialized private prefix.
Native diagnostic counters may advance during failed native construction; no
universal rollback of native effects is assumed. This tests low-level helper
rollback and ownership: a language executor still treats an unhandled technical
error as terminal rather than continuing an Ama body. Final teardown releases the
retained grant before owned trees and the explicit caller.

The general algorithms and bounds remain those in
[stable storage](instance-ownership.md), [preflight](acquisition-preflight.md),
[clone maps](clone-preparation.md), [replacement](staged-replacement.md) and
[shared storage](shared-storage.md). This adds verification, not an alternative
ownership engine, heuristic, runtime cutoff or allocation policy.

## Exhaustive traces and failure composition

Run `make -C tests/unit/runtime/ownership-model`. All 18^4 = 104,976 length-four
words are enumerated and every prefix is checked, including empty/ineligible
cases. A longer fixed trace mixes saved markers, acquired descendants, refused
copies/removals, effect-free self-copy, successful cloning/replacement, caller
refusal provenance, release, destruction and generation reuse. Every required
allocation measured in that trace is failed independently. The oracle accepts
out-of-memory only when that exact injected allocator event occurred; it then
checks unchanged public state and eventual zero counted allocations.

This is exhaustive for the stated finite action/length domain, not for every
possible program or control stack. It supplements the general invariant argument
and the earlier bounded lifecycle, exact service/identity, 93,750 pool-history
and 8000 scalar reference-map cases. T048/T049/T051/T052 separately inject each
native constructor/destructor and allocation prefix, including terminal cleanup.
[T053 compiled C probes](c-layout-composition.md) establish actual layout,
lifecycle and borrowed native-view handoffs.

Full strict tests, address/undefined-behavior sanitizers, per-file/header coverage
and reproducible Doxygen generation complete this checkpoint before the next
execution phase. The [coverage registry](documentation/ready.json) maps every
new/reused source and fixture to named cases. Concrete list membership/forest
copying, new execution adapters and independent generated host contexts retain
their own TODO prerequisites; this checkpoint does not silently certify them.
