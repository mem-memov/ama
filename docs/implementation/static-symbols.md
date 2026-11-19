# Scoped static identities and finite metadata

This component owns exact source-backed symbols and ordinary Type-interface
metadata. Its arena borrows one loaded Project; that Project and its source
manager must outlive every symbol, occurrence, signature and diagnostic.
[TPL-02](../../spec/language/templates.md#tpl-02),
[TPL-07](../../spec/language/templates.md#tpl-07) and
[TPL-12](../../spec/language/static-results.md#tpl-12) govern the target semantics.
[T206](static-source.md) records the source-model transition and its current
limits. Earlier T172 acceptance is preserved in [DONE](../../DONE.md#t172),
not presented as acceptance of the changed static-module model.

## Current operations

Primitive insertion/lookup compares complete borrowed name bytes and exact
ProjectModule identities. No Unicode normalization, filesystem search by static
name, compatible-layout matching or fallback namespace supplies an identity.
Symbol roles distinguish parameters, pair definitions and producer receivers;
only parameters contribute specialization inputs. Counters are checked before
allocation/publication. Duplicate diagnostics borrow the actual prior and later
sites and allocate no diagnostic storage. Every failure retains destructible
partial arena state and does not certify a completed stage.

The owning AST traversal records expression heads, nested inputs, producer
receivers and qualified constant sources. Foreign labels and ordinary export
selectors do not become local occurrences. It follows runtime declaration,
branch, selected-handler, folded-loop and traversal ownership edges once, never
control/argument aliases. Static graphs are traversed separately with definition
context. A stable source-position sort makes physical ordering independent of
worklist scheduling. Later type evidence cannot change that source tree.

Ordinary input/output placeholder Type lines introduce one local Type formal on
first eligible occurrence. Ordinary shared/body uses cannot introduce formals.
Their original Type-line order supplies constructor metadata. Pair headers in a
static graph supply public result records with separate order and no runtime
storage. Explicit foreign-signature trees and ordinary root input-pattern
records have been removed because their source productions no longer exist.

This is not yet complete static graph inference. Whole-graph receiver collection,
free Type/Template input roles and requirements from every use belong to T207
and T208. Current concrete frontend admission refuses unresolved static graphs
and selected exports. Inventory validity is not executable-specialization
admission, producer closure, or proof that those later features already work.

## Finite actual signature operations

Actual constructor metadata retains complete original formal names and category
edges. A Type has no nested constructor signature. Deferred metadata remains a
real obligation; it never becomes Type or equality success merely because no
actual argument has arrived. Ordinary constructor skeletons are prepared from
actual local Type formals; no AST signature builder remains.

The comparison primitive matches complete named sets without using source order.
It compares actual immutable metadata, not minimum use requirements or source
patterns. T208 owns minimum requirements in the full solver. A known mismatch
takes precedence over deferred entries in either direction. The caller supplies
and destroys scratch; input metadata and output remain unchanged on allocation
refusal. The old inventory-mutating wrapper and root-subset mode were removed
with their last source-pattern consumer, leaving one operation used by the real
constraint cache. No duplicate public entry point or unused mode remains.

Comparison memoizes exact ordered signature-pointer pairs; a shared DAG is not
unfolded exponentially. Known finite metadata cycle checking uses explicit DFS
records: an active vertex is exactly an ancestor of the current cursor, so an
edge to it proves a cycle. Completed sharing is accepted. Allocation failure
remains distinct from a semantic cycle. Current tests construct finite metadata
directly and verify reordered names, missing/renamed/nested/category partitions,
pending precedence, 900-level depth, shared DAGs, cycles and each
measured comparison/traversal refusal. They do not recreate removed formal ASTs.

## Invariants and costs

A finite occurrence walk consumes one source owner or removes one pending frame
per transition. Each symbol insertion checks one finite existing-name chain;
Type-formal ordinals count genuine earlier introduction positions. For N
occurrences, S local symbols and B maximum compared name bytes, primitive name
lookup/classification is bounded by O(NSB + S squared). Stable occurrence sorting
uses O(N log N) comparisons without allocation. Owned traversal scratch is linear
in represented owners and expressions, with no semantic depth limit.

For a finite metadata graph with V vertices and E edges, explicit cycle traversal
consumes every edge once. Linear exact-pointer memo lookup gives conservative
O(V(V+E)) time and O(V) scratch. Comparison visits at most P distinct ordered
pairs, using O(P) scratch and O(P squared) linear memo lookup plus exact name-set
membership scans. These bounds require no timeout, hash collision assumption,
first-actual choice or guessed cutoff. Deferred information propagates
monotonically; every resolved slot changes from deferred at most once.

[Current source acceptance](static-source.md) records actual commands,
configurations, results and pending migrations. Per-file registration is in
[the documentation registry](documentation/README.md); it checks real named test
interfaces and headers, and does not replace behavioral acceptance.
