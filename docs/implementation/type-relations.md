# Ownership relations and copy capability

T033 verifies the independent relations required by
[COMP-03](../../spec/implementation/compiler.md#comp-03),
[TYPE-04](../../spec/language/declarations.md#type-04) and
[PROTO-07](../../spec/implementation/native-interface.md#proto-07).
The implementation keeps the existing exact graph algorithms; this task expands
their evidence and makes capability-witness initialization explicit.

## Three different questions

| Relation | Edge or predicate | What a cycle means |
| --- | --- | --- |
| Required module closure | A source/type/namespace requires another module | Permitted; each complete identity is admitted once |
| Eager construction | An owned scalar or initialized owning list needs a child | Invalid: no finite construction can finish the cycle |
| Copy capability | Any owned scalar/list field depends on its element type | Permitted if no owned path reaches a noncopyable payload |

Shared constants and nonowning references do not contribute eager/copy ownership
edges. Inputs, results and persistent fields do. An empty owning list contributes
a copy edge even though it contributes no eager edge. An inferred owned field has
ordinary default construction; an inferred list starts empty. Whether later body
code runs or inserts an element does not change these static predicates.
Actual initializer-value dependencies form another graph, completed by T034;
the eager graph does not authorize reading an uninitialized value.

## Exact eager ordering and cycle evidence

[Eager-edge construction](../../src/semantic/relations/eager.c) runs after typed
initializers exist. Every edge means that at least one child's construction must
finish before its owner. A topological order therefore supplies a finite schedule.
A directed cycle would require an infinite descending construction chain and is
invalid independently of runtime availability.

The [graph service](../../src/semantic/relations/graph/graph.h) orders prerequisites
before owners. Among currently ready vertices it chooses the least complete
module-path/ordinal key. Logical paths include their explicit root prefix; equal
paths in the constant graph are distinguished by declaration ordinal. Inserting
vertices or edges in another order cannot change the resulting least topological
sequence. Distinct values must have distinct stable keys; duplicate edges do not
change the prerequisite relation.

The induction invariant is that the emitted prefix contains only vertices whose
prerequisites occur earlier. If unfinished vertices have no ready member, each
has a blocked outgoing edge. Following such edges in a finite graph must repeat
a vertex. The repeated suffix is a concrete cycle, not a guessed depth limit.
Diagnostics retain the original span of every selected cycle edge. A partial
order from a failing attempt remains invalid and cannot be consumed as a plan.
Empty graphs succeed. Neither ordering nor cycle extraction uses C recursion.

With V vertices and E stored edges, repeated ready scans cost O(V(V+E)) plus full
path comparisons; storage is O(V+E). A cycle report allocates at most O(V) notes.
Every allocation failure is explicit and leaves all partial records owned by the
graph arena for destruction. Repeated edges consume their explicit edge storage;
no hash collision or probabilistic membership test affects behavior.

## Greatest fixed point for copying

Let B be the executable types whose native payload, if any, supports copying.
Let E contain all owned scalar and owned-list element dependencies. Define

`F(S) = { T in B : every owned successor of T belongs to S }`.

[Capability propagation](../../src/semantic/relations/clone.c) starts with B and
removes a type whenever it has an edge to a removed type. This sequence descends,
never invents a new member and terminates after at most V removals. Every fixed
point is contained in every intermediate set, by induction using monotonicity of
F. At termination the remaining set is closed under F, hence it is the greatest
fixed point. Equivalently, the rejected types are exactly those reaching a
noncopyable native seed, including each seed itself.

Each propagated rejection retains its actual field and an already rejected
prerequisite. Following these causes ends at the recorded native seed: removal
order strictly decreases along the chain, so a diagnostic witness cannot cycle.
Seeding clears previous causes rather than leaving a stale witness. Capability
calculation allocates nothing and costs O(V(V+F)) for F declared fields. It runs
after receiver inference, before any owned transfer is accepted. Runtime aliasing
cannot exempt an unsupported transfer from this static proof.

## Evidence

Run `make -C tests/unit/semantic/relations`.

The capability unit enumerates all 4^9 directed three-type storage graphs and all
8 native-seed subsets: 2,097,152 cases. Edge states distinguish absence, owned
scalar, owned list and nonowning access; reference-list flags and interface/state
roles are also represented. An independent transitive-closure oracle checks every
type, and cause chains must terminate at their actual native seed. The existing
512 real source/ABI projects separately test the composed loader/type pipeline.

The order unit checks all 512 directed graphs, all 6 vertex admission orders,
forward/reverse edge admission and two complete-key families: 12,288 cases.
Exhaustively enumerating all candidate permutations supplies the least valid
order independently of the production ready scan. Families include distinct root
prefixes and equal module paths with different ordinals. Rejected graphs must
report an actual closed chain of original edge spans.

All 128 role/form/initializer/inferred/payload partitions check eager classification
separately. Real inferred receiver fixtures check empty recursive lists, references,
owned defaults and noncopyable element rejection. Empty graphs and 1,024-vertex
chains/cycles exercise finite iterative traversal with parallel edges. Each graph
allocation is independently failed in successful and cyclic shapes, including
cycle-note allocation; counted owners must return to zero. Existing tests also
check that noncopyable syntactic self-assignment fails before runtime alias checks.
