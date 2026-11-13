# Retained runtime component contract

This is implementation evidence for T149, not an additional language definition.
The generated file/function reference comes from `make docs` and Doxygen. The
[requirement index](../../spec/requirements.md) supplies canonical rules; the
[roadmap](../../TODO.md) records missing behavior.

## Domain and ownership

The runtime accepts compiler-validated, uniquely registered nominal descriptors,
truthful C field offsets/sizes/alignment and finite acyclic ownership trees.
Reference cycles are permitted because references do not own their targets.
Allocators satisfy the SourceAllocator contract; pointers and array extents are
valid borrowed C storage. These internal interfaces are not a validator for
arbitrary foreign pointers or corrupted descriptor memory.

A world borrows its type registry and report callback. Each type owns a pool;
each occupied cell owns its payload and declared owned descendants. List buffers
own handle arrays, not the payload addresses they name. Native resources may be
opaque, but persistent Ama references occupy declared fields. Native hooks are
synchronous and cannot reenter Ama. Operation arenas own temporary work and
protection records; restore protections before freeing their arena. Immutable
constants own separate arrays and aligned allocations outside instance pools.
Their generated declarations and source views remain borrowed through shutdown.

## Local identities and stable storage

[REF-02](../../spec/language/values-and-references.md#ref-02),
[REF-05](../../spec/language/values-and-references.md#ref-05) and
[REF-06](../../spec/language/values-and-references.md#ref-06) require exact local
identity and no resurrection. The retained handle is the exact nominal type,
slot and generation triple. Generation zero is empty; slot zero is real. Public
lookup requires occupied storage, matching generation and published live identity
before returning a cell. Reserving initializes private storage; lifecycle
completion and publication are distinct operations.

A released generation strictly increases or retires at its configured limit.
Reclamation removes every entirely free payload block, rebuilding free links
before release. Historical directory entries remain null permanently and new
blocks append fresh slot IDs. Thus neither generation exhaustion nor physical
block reclamation can reissue an old triple. Growing the directory copies only
block records, never live payloads. Extended C alignment is explicit and every
layout multiplication/addition is checked before allocation.

The second payload bank stages replacement. It is not an execution working copy:
ordinary generated labels access the active stable bank directly under
[RT-05](../../spec/implementation/runtime.md#rt-05). Preparation builds native
resources at their final bank address, avoiding C relocation of address-sensitive
resources. The old value survives preparation failure. Only complete preparation
permits old destruction and a bank flip under the unchanged root identity.
Destructive failure makes the world terminal and cleanup continues.

Pool lookup/reserve/release are constant-time apart from growth and registry
lookup. Directory growth is linear in historical blocks; geometric allocation
amortizes its copying. Reclamation scans materialized cells and the historical block directory.
Memory includes historical directory extent as well as allocated payload blocks;
no claim of history-free constant memory is made.

## Lifecycle, access and copy

[LIFE-01](../../spec/implementation/native-interface.md#life-01) through
[LIFE-05](../../spec/implementation/native-interface.md#life-05) are implemented
with an explicit resource ledger. Completed children form a declaration-order
prefix. Native construction follows child construction. Cleanup consumes the
native-live bit before invoking its destructor, then consumes children in reverse
order. Failed native constructors never receive a destructor credit and must
release their own partial resources. All later cleanup errors are reported while
the first failure remains the returned primary status. Terminal disposal clears
active links without allocating an emergency traversal stack.

Copy capability uses the declared ownership graph, including element types of
empty owning lists. A monotone reachable-type bitmap adds each type at most once,
so fixed-point iteration terminates after at most the number of types adding
rounds. With T types and E owned field edges, linear registry lookup gives the
conservative bound O(T * (T + E * T)); scratch is O(T). This is separate from
checking availability of the actual initialized source tree. Exact owner chains
use the actual occupied population as a cycle bound, not an arbitrary depth.

Protection records capture prior service state before marking each owned source
Acquired. Only ownership edges are traversed. Same-epoch repeated ownership is
invalid; a distinct later copy boundary can reuse authorized read protection.
Protection restoration checks occupied identity/generation and restoration
ownership, preventing resurrection after replacement. Partial protection failure
is always unwound by the enclosing operation. Current exact linear record/map
searches can make whole-tree preflight/fixup quadratic; they are not hash or
sampling approximations. The construction, clone and whole-list phase contracts
state the prepared-prefix and remapping proofs in more detail:

- [Construction](object-construction.md)
- [Clone preparation](clone-preparation.md)
- [Selection and list transfer](value-selection-and-list-transfer.md)
- [Ownership and replacement](instance-ownership.md)

## Program transitions and diagnostics

[CALL-08](../../spec/language/calls-and-results.md#call-08) separates target
selection/acquisition, input selection, protection, declaration-order interface
installation, body dispatch, output transfers and release. Source selection
occurs once; target-refusal provenance is explicit. A technical index protocol
failure cannot become an alternative branch merely because its code resembles a
bounds refusal. Reference stores copy handles without acquiring their targets.
Condition reads invoke the declared native condition protocol; absent protocols
or technical errors do not invent truth values.

A normal connection records one immediate caller and return selector. Running
callee/Waiting caller transition back to Running caller/Acquired callee on return,
so outputs remain protected until explicit release. Yield additionally requires a
saved nonzero resume selector. It returns to the immediate caller, not through
all callers, and keeps saved position distinct from the transient connection.
Ordinary service copies zero caller links, history and saved position; explicit
continuation copies preserve only permitted saved control state. See
[continuations](continuations.md) and [entry preparation](entry-preparation.md).

Constants initialize before root construction. Their parent/cursor records permit
iterative partial cleanup and reverse declaration/element destruction. Startup
failure therefore leaves no partially published root. Shutdown destroys the root,
constants and pool storage in that order, preserving the primary failure.

Runtime reporting uses exact generated operation IDs and walks the intact caller
chain before terminal cleanup removes it. Subsequent cleanup errors are secondary
and do not attempt to reconstruct a destroyed chain. A location scan is linear in
the generated table; a trace is linear in caller depth times that table size.
Host-result validation checks structural encoding and message views only.

## Explicit remaining work

Local handles are not context-authenticated C-library tokens. Current generated
assembly still has the context-ownership debt recorded by the audit. The
[independent context foundation](execution-contexts.md) provides explicit runtime
ownership, registries and origin validation; T061 connects generated hosts and the
embedding tasks provide their public component boundary. The existing storage layer has no per-membership IDs, live-boundary
foreach cursor or typed intrinsic method frames; T075 onward provide those.
Current continuation records therefore do not certify foreach/async cursor copies.
Construction-time final self identity remains T050. Neither metadata declarations
nor passing synchronous tests imply asynchronous scheduling, parallel execution
or thread-safe access to one mutable world.

## Evidence

The per-file registry maps all 98 runtime C/header inputs, including the
assembly-only implementation sources, to named behavioral tests. Every header is
also compiled independently under strict C11. Direct pool, alignment, lifecycle,
construction, clone, access, replacement, list, selection, transfer, program and
call suites exercise identity boundaries, failure prefixes and ownership cleanup.
The metadata/control suite checks local identity, eligibility, return connections,
refusal snapshots and host outcomes. Generated execution/continuation suites
compile and run actual project C, including retained outputs, active traces,
allocation-free runtime failure paths and host restart/resume/disposal.

T149 changes comments and this evidence only. Compiler-lexed C comparison against
the preceding commit verifies all 58 changed runtime inputs contain identical C
outside comments. Focused runtime/native metadata parents and generated execution,
continuation and assembly consumers are rerun; `make docs` checks originating file
pages, canonical rule anchors and borrowed-source stability.
