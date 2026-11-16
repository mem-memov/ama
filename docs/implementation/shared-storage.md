# Shared scalar values outside instance pools

T052 verifies the shared-storage boundary under
[CONST-02](../../spec/language/declarations.md#const-02),
[CONST-03](../../spec/language/declarations.md#const-03),
[RT-01](../../spec/implementation/runtime.md#rt-01),
[RT-11](../../spec/implementation/runtime.md#rt-11),
[LIFE-05](../../spec/implementation/native-interface.md#life-05) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
The existing [startup runtime](program-startup.md) already provides distinct
constant value storage; this task verifies it through full context construction
with real scalar-owned dependencies, instead of introducing a second constant
system.

## Ordered values and independent contexts

The semantic constant planner produces a deterministic dependency order and
reverse cleanup order. Runtime declaration tables consume that validated order;
they neither infer dependencies from names nor heuristically reorder invalid
input. Each declaration owns a separate scalar native payload and owned child
value arrays. These are `AmaConstantValue` trees, not executable pool slots:
there is no Ama handle, generation, acquisition or resume position for a shared
value. Native resources used internally to represent an immutable value retain
their lifecycle obligations, without becoming mutable external resource identities.

Context construction borrows immutable type/source/declaration schemas, creates
its own runtime descriptor/value arrays, and initializes the complete shared
closure before publishing the context. No implicit executable root is created.
A constants-only namespace contributes declarations, not an executable instance
type/pool. The storage planner's
`it_retains_the_general_fallback_and_keeps_shared_namespaces_out_of_pools` verifies
that distinction; context schema construction and generated layout wiring remain
separate boundaries, with T053 integrating concrete planned descriptors.

Within a constant, fields initialize in order. A later field may copy an earlier
completed owned field. A later declaration may copy a completed earlier constant,
including its whole owned subtree. Native clone hooks copy their own payload;
references to mutable instances and standalone self are not allowed in shared
initialization. Shared selection returns a const value view with no mutable field
location or local identity. Attempting acquisition of that immutable scalar
refuses rather than manufacturing a reference or executing it.

Two contexts may share the same immutable schema while all mutable descriptor,
constant-array, field-array, payload and native-resource ownership is distinct.
Creating an explicit owned root copies needed values into fresh pool slots without
changing either context's shared values. Disposing one context cannot invalidate
or mutate the other's storage.

## Publication and cleanup argument

Induct over the ordered declarations and each declaration's field/child traversal.
At every step, completed predecessors are readable immutable values; the current
allocation and its initialized prefix are registered for cleanup. Completing a
child advances its parent's prefix; completing its native payload creates one
native cleanup credit. Failure cleans the current prefix and prior declarations
without publishing a context or starting an executable root.

At shutdown, successful independent roots are destroyed in reverse publication
order before shared values. Shared declarations then clean in reverse startup
order; within each owned value, native payload precedes reverse-order children.
Cleanup consumes credits before hooks, continues after destructor failure and
retains the primary failure. Repeated disposal cannot invoke consumed hooks.
Context identity is withdrawn before resources are released.

This preserves the existing iterative construction/cleanup algorithms and bounds.
For V constructed values/owned elements, structural traversal is linear in V;
exact source access and registry lookups add their documented costs, including
O(d^2) access-chain steps at index depth d. Native hook work is external to that
bound. Owned storage is O(V) plus native payload/resource sizes and descriptor
arrays. Constants allocate no pool blocks; compiler type/code schema sharing does
not imply shared mutable value storage. No recursive C ownership walk or emergency
cleanup allocation is introduced.

## Executable evidence

Run `make -C tests/unit/runtime/shared-storage`. Its immutable schema contains:

1. A literal leaf value 37.
2. A compound value whose first owned child copies that leaf and whose second
   copies the earlier completed child; its native logical default is zero.
3. A whole compound copy of declaration 2.

The complete closure constructs seven native values using one literal, one
default and five clone operations per context. Two contexts retain equal logical
values but independent arrays, payload addresses and native resources. Every
instance pool remains entirely unmaterialized and all execution/root ledgers are
empty before explicit owned construction. Shared acquisition is rejected. An
explicit root in the first context leaves the second context unchanged; exact
native-address traces verify root-before-shared and reverse-declaration/child
cleanup order, including disposal while the other context remains leased.

Each of the seven native constructors is failed in turn, and every factory
allocation is independently failed. No context identity or root is published;
all allocated prefixes are released and a fresh factory retry succeeds. Every
shared destructor position is failed as well: all seven values finish cleanup
once, the error survives and repeated disposal remains stale.

`make -C tests/unit/planning/storage` checks constants-only namespace exclusion
from executable pools; `make -C tests/unit/semantic/constants` checks real source
ordering, cycles and immutable construction restrictions. Strict runtime parent
composition, address/undefined-behavior sanitizers, header coverage and reproducible
Doxygen supplement these invariants. The
[coverage registry](documentation/ready.json) maps each new/reused fixture and
consumer to named cases. [List copying/membership](list-values.md) and
[generated list constants](library-compositions.md) extend the scalar-owned
constant-tree foundation with their own forest and context-local evidence.
