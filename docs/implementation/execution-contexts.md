# Independent context ownership and checked host identity

T006 supplies an internal runtime foundation under
[RT-01](../../spec/implementation/runtime.md#rt-01),
[PAR-01](../../spec/language/async-and-parallelism.md#par-01) and
[REF-02](../../spec/language/values-and-references.md#ref-02). It does not connect
the retained old-source generated host yet: T013 defines the host lifecycle
composition and T061 connects the current planned/generated code. The checked
compiled-component export/transport ABI remains T123 onward.

## Domain and explicit owners

The host owns a fresh C registry object and supplies malloc/free-compatible
allocation authority that remains alive through its contexts and callbacks.
Registry object lifetime initialization/destruction requires the ordinary C
owner's exclusive lifetime authority. Normal context operations and disposal
may overlap across host threads. Native resource affinity and allocator/report
callback concurrency remain explicit host obligations under PAR-02; a gate cannot
make a thread-affine resource transferable.

A resolved immutable schema supplies exact unique executable type IDs, truthful
C layouts, declared fields and initializer arrays, native lifecycle/conversion
hooks and source locations. Compiler graph/type/default validity is a precondition;
this foundation checks cheap count/null bounds and exact nominal membership, not
a second source language or untrusted C-pointer validator. The outer definition
and type/constant records are read during factory construction. Referenced
field/declaration/initializer/source arrays and report userdata outlive all their
consumers; code and literal bytes may be shared immutably.

Every context owns a new world, program, runtime-control record, first-failure
status, type descriptors/pools, program descriptors, constant declarations and
constant storage. Arena scratch owns descriptor arrays; pools and constants own
their separate payload/resource allocations. Field/default source arrays remain
borrowed. A type's resolver callback points at this context's program, never a
shared mutable program. Shared constants initialize once before publication and
remain outside instance pools. Creating a context constructs no implicit root,
no pool slot and no Ama body. Internal explicit object creation can later create
multiple owned roots; host creation/entry adapters are separate work.

## Identity without freed-address validation

A copyable host reference carries a context identity and the local nominal
(type, slot, generation) triple. Numeric context zero is empty. A host registry
first finds the exact live context record under membership protection; an absent
identity is stale, without dereferencing an old context address. Only a valid
linear lease can expose runtime owners. Resolving an instance reference compares
its origin context before any local type/pool access. Equal local triples in two
contexts therefore select neither each other's payload nor error/history state.
Forming a wrapper from a trusted local published handle checks identity without
acquiring its referent, including when that referent is Running.

The runtime image owns one atomic identity-allocation frontier outside Ama data.
It stores no world, pool, constant, selector, error or context address. This is
host name-allocation bookkeeping, like allocation authority, not shared mutable
Ama execution state. All registries in that runtime image consume nonzero serials
from the same frontier, including failed factory attempts. Registries never reset
it. Separately compiled images additionally require component identity at their
future checked export boundary; these internal tokens are not that portable ABI.

For a fixed positive limit L, issuance succeeds only by changing frontier n<L to
n+1 with strong compare/exchange. A failed comparison observes a strictly newer
frontier; there is no guessed retry count or spurious-failure fallback. At L the
allocator refuses permanently rather than wrapping through zero. Thus no two
successful issuances in the image share an identity, and registry/context address
reuse cannot revive old access. The retained frontier uses O(1) storage; destroyed
contexts need no per-identity tombstone allocation. Tests use the same primitive
with limit three and the exact UINT64_MAX boundary.

## Admission, publication and retirement

A short atomic registry gate protects only membership and pending-lifecycle
credits. It never spans allocation, native hooks, reporting or Ama execution.
Contenders yield to the host between acquisition attempts; progress assumes host
thread scheduling and finite metadata critical sections, not an Ama scheduler
retry policy. Exact context lookup is a finite O(live contexts) list search.

Each context has a separate atomic exclusive gate. Entering finds a live record
and acquires this gate before reading mutable world/program state. A concurrent
or reentrant entry returns Busy without changing Ama values, saved positions,
refusal history or diagnostic state. Different contexts may hold leases at once.
Acquire/release ordering publishes state between nonoverlapping executors. A
lease is a linear internal C owner: it starts empty, is never shallow-copied and
is consumed on leave. Borrowed views expire before its release; forged/copied
lease pointers are C ownership misuse, not untrusted numeric token validation.

Factory admission increments a checked pending credit before unlocking metadata.
This pins registry lifetime through private allocation and native construction.
All cleanup-visible arrays and constant-storage pointers are installed before
fallible type setup and hooks. A failed prefix unwinds completely with no token
publication. A successful record joins live membership only after constants
succeed, then its identity is published. No rollback can recycle a consumed name.

Single-context disposal acquires its gate and unlinks its identity before cleanup.
A pending credit pins the registry while native destructors/reporters run outside
metadata protection. A callback sees the retired identity as stale, cannot destroy
the pending registry, and may inspect an independent available context. Registry
closure first acquires every context gate; a busy record releases the earlier
temporary gates and leaves membership/resources untouched. Accepted closure
retires all records before any hook and closes new admission until its complete
cleanup finishes. Repeated closure returns the retained first cleanup outcome.

## Complete cleanup and failure isolation

Public create/clone append their completely published independent root to a
world-owned doubly linked chain of stable cells. Private construction, owned
children and adopted list elements never register. Each entry stores its nominal
key; links are runtime bookkeeping, not Ama references. Failed creation adds no
entry. Actual release unlinks a root before its slot can be reused; replacement
retaining the root identity preserves its original links across payload-bank
changes, including terminal cleanup after a destructive replacement failure.

Exclusive terminal cleanup repeatedly destroys the newest remaining root under
[LIFE-05](../../spec/implementation/native-interface.md#life-05). The invariant is
that this chain is exactly the surviving successful-publication sequence. Append
adds its last member, early disposal removes precisely its member, and unchanged
identity replacement leaves the sequence unchanged. Therefore consuming the tail
produces reverse successful-creation order independently of nominal type, pool
layout, generation reuse or reference cycles. Each destruction consumes all owned
descendants; reference edges never own or prolong a target. After all roots,
reverse constant cleanup runs, then pool blocks and descriptor arenas are released.
Root registration/removal needs no allocation or sequence counter; cleanup needs
no emergency allocation or recursive C stack. Live payload addresses never move.
Cleanup preserves the first error while completing later destruction, including
after distinct destructor failures.

Synchronous reporting records the first technical failure and terminal control
phase in that context before calling its reporter. Another context's flags,
constants and instance payloads remain unchanged. Failed contexts retain gated
diagnostic inspection but reject ordinary checked-reference operations; disposal
still releases their resources. Context admission errors do not call that reporter
or overwrite an Ama last-refusal record. Native status validation remains T007.

Private descriptor memory is O(T+K) for T executable types and K shared declarations,
plus their ordinary fields/pools/resources. Schema membership checks are exact
O(T squared + F*T + K*T), with F declared field edges; they infer no identity.
Root links occupy fixed cell metadata; append/removal are O(1). Context closure
visits surviving roots and actual ownership resources using existing lifecycle traversal,
without scanning historical slot extent to choose root order. No claim of constant lookup or history-free pool
reclamation is made beyond the separate pool contract.

## Evidence and next consumers

`make -C tests/unit/runtime/contexts` runs eighteen assertion cases with counted,
address-sensitive resources and the real runtime. They cover distinct constant/
pool/control ownership with zero implicit roots, colliding local triples and
origin-first rejection, reverse root order across mixed types, slot reuse, early
disposal, public clones, stable-root replacement, destructive replacement failure and failed creation/clone prefixes,
distinct destructor errors, stale identities after registry reuse,
reentry and busy-close rollback, nonwrapping issuance, every measured factory
allocation failure, malformed schema and native constructor/secondary-cleanup
failures, local terminal diagnostics, callback lifetime pins, 64 simultaneously
created empty contexts, and a concurrent refused/independent state-access pair.

The recursive runtime parent includes this leaf. The per-file ready registry maps
all new production/header/fixture inputs to these named cases; headers also compile
independently. Generated old-source hosts still use their audited global descriptor
layout and single-owner guard until T061. Their acceptance is not claimed as
current independent-context execution. No asynchronous reactor, public marshalling
schema, resource-affinity transfer or message transport is added by this foundation.

Acceptance checks pass with `make -j4 test` using `-DNDEBUG -Werror` and the
focused context leaf under ASan/UBSan (`detect_leaks=0`; no leak-sanitizer claim).
`make docs-check` covers all 694 maintained inputs with zero debt.
`make docs-reproducible` renders 530 maintained file pages and 2207 documented
interfaces, resolves all 285 canonical/page destinations, emits no warnings and
produces byte-identical output across two complete Doxygen generations.
