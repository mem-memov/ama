# Static dependency and creation guards

## T215 producer owner foundation

Static graph producers have compiler identities but are not Type/Template values
([TPL-12](../../spec/language/static-results.md#tpl-12)). The old nominal-only
owner representation could not identify a real graph producer after T208 correctly
forbade nominal construction from static modules. T215 separates that identity
without changing the ordinary term algebra. T210 revalidates the current guard
consumers below; T183 owns registry/body integration. These components do not
provide whole-program admission.

### Representation and contract

`static_term_producer_environment` accepts an inventory-owned static definition
and an array containing exactly one store-owned term for every original input
formal. Array order and caller list links are irrelevant. It copies bindings in
original declaration order and interns `(definition, full raw input map)`.
Generic and explicitly applied scopes remain distinct even for zero inputs.
Parameters in an applied producer read their supplied terms; derived results
remain variables in that exact scope. No body executes or expands here. Type and
Template categories remain unchanged, and unknown variables are not defaults.

Guard owners retain the original lexical environment and either an ordinary
nominal shape or a complete compiler-only producer map. Normalization asks the
constraint solver about every input, including unused inputs. Only complete
normalized maps become canonical creation/definition keys. Equal original symbol
plus equal complete owner key merges definition nodes; equal values of different
symbols do not. Ordinary result adapters retain their existing ordinary owner
term view pending their separate T211/T212 migration; producer guard checks use
the complete owner environment, never that optional term view.

`static_creation_producer_enter` requires an original producer expression from
its published parent's lexical definition. Source occurrences select the actual
loaded static origin; a caller cannot supply another origin by name resemblance.
The full map is checked before dereference. A temporary candidate map permits
ancestry comparisons before publishing any producer scope. An unresolved repeated
ancestor suspends entry before candidate scope preparation. Every complete
ancestor comparison is checked: identical keys may close a creation cycle;
different keys of the same origin are rejected with both compiler identities.
Every original source use retains a path, including distinct uses of one producer
under a common parent. Cache graph checks use the same normalized keys and retain
all original edges. Definition cycles are checked independently.

### Invariants, bounds and ownership

Membership and cardinality plus uniqueness establish an exact total input map.
Interning compares original formal identities and already interned child term
identities; by induction on term construction, permutations cannot change raw
identity. This does not assert solved equality for unknown variables. Complete
identity is obtained only through solver queries. Maps of different origins or
any unequal input remain distinct, including unused bindings and Template residual
metadata already retained in child terms.

Each scan visits a finite list or an input-count-bounded array. Let P be the
input count, M/S the inventory module/symbol counts, T the term count and E the
scope count. Membership and duplicate validation costs
O(M + P(M + S + T) + P²); searching E scopes costs O(E P²) with array lookup.
Source-order publication additionally costs O(P S + P²). Published scope storage
is O(P). Normalization uses O(P) temporary storage plus the solver queries and
these validation/interning operations;
no source-body recursion occurs. The existing finite reachability checker keeps
its graph invariants and compares canonical environments instead of nominal-only
values. Scope interning is not admission and cannot bypass the definition graph.

Borrowed source and terms outlive guards. Producer scopes and copied maps belong
to the term arena; comparison/normalization scratch is released on every return.
No incomplete environment is linked or counted after map allocation refusal.
Other partial guard records remain owned until unconditional guard teardown.
Receivers are written only on successful publication; pending entry leaves the
path receiver untouched. No rollback of solver metadata is promised.

### T215 acceptance (2026-11-19)

The producer leaf checks raw-map reuse, permutations, unused inputs, invalid
membership/duplicates, zero inputs, pending-to-closed recurrence, equal/unequal
keys, independent definition recursion and each observed allocation refusal.
The ordinary owner leaf and exhaustive four-vertex graph oracle check unchanged
ordinary semantics and canonical graph behavior. Initial fixture mistakes were
corrected: a graph producer must be used as an actual projection, not an ordinary
result Type path; persistent canonical environments are prepared before the graph
oracle's guard-only allocation baseline. A distinct-Type graph case initially
named a nonexistent standard bit module; the fixture now uses the existing text
module. The documentation compile probe also exposed old direct vertex calls in
the failure fixture; they now prepare the new owner-key interface through the
same support helper exercised by the graph oracle. The initial rendered-link gate found TPL-12 only on a struct member page;
its explanation is now also in the file-purpose comment and the rendered check
passes. The final task snapshot is `/tmp/ama-t215-check-5_43gf9k`; its separate
sanitizer tree ends in `-san`. No working T182 changes enter these trees.

Acceptance commands use `make -C tests/unit/semantic/static/guards/producer`,
`make -C tests/unit/semantic/static/guards/owner`,
`make -C tests/unit/semantic/static/guards/graph`,
`make -C tests/unit/semantic/static/terms`, and
`make`. Strict flags are `-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L`. Sanitizer leaves use `-O1`, the same warnings/C11,
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`, and linker flags
`-fsanitize=address,undefined -no-pie`. `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`
and `UBSAN_OPTIONS=halt_on_error=1` retain the established container restriction;
counted allocation teardown independently checks leaks.

All three guard leaves pass in both configurations. The two graph cases check
262144 graph/order/category combinations per configuration against independent
transitive closure. Eight producer cases include all raw two-input maps over
three actual values in both input orders, including a residual Template; its
complete compiler key is also normalized. Allocation sweeps cover map construction,
generic roots and actual source-backed successful and rejected entry, with no
fixed allocation-count assumptions. Final source-backed refusal reruns are
`/tmp/ama-t215-producer-{strict,san}-refusal-final.log`; other guard runs use
`/tmp/ama-t215-*-accepted-*.log`. The immutable-term parent and final compiler
build pass. The complete per-file gate reports 2713 inputs and zero debts.

Rendered documentation uses Doxygen 1.11.0, Python 3.14.4 and markdown-it-py
3.0.0. `make docs-complete` and actual Doxygen generation/link validation pass;
final counts are recorded in the commit. Source-relative canonical links were
resolved by the generator and the independent local-link review. These checks
are not whole-language conformance. T210's revalidation below extends this
foundation; T211/T212 retain producer equations, selected outputs and
result-adapter composition.

## T210 guard revalidation

The source interface has changed: ordinary modules have paired runtime
interfaces; static producers use exclusive `@...` graphs. This task migrates the
remaining guard consumers and tests rather than accepting old mixed interfaces.
An ordinary module cannot receive a Template as a runtime formal or expose static
outputs. Its old captured-own-constructor scenario is therefore removed. The
underlying unequal-key recurrence and full-key identity remain checked by the
ordinary ancestry cases, producer cases and exact graph oracles.

Current tests use real lexical source occurrences. Ordinary fields receive
explicit compiler IR maps or field inference holes, without source-level static
bindings in their declarations. Finite written nested constructors are expressed
inside a static graph. Producer cache examples use literal graph producer paths
and named maps; producers are never passed as Type/Template values. Known tails
and fixed incoming bindings test actual cache reachability independently of
source-name resemblance. These are component checks, not admission of unevaluated
static result definitions or proof of whole-program conformance.

The definition migration exposed a real zero-input-key bug. Such a producer had
no input value query, so queued solver equations were not settled and the dirty
flag prevented idempotent refresh reuse. Owner normalization now settles the
solver before examining any key, including an empty map. Unrelated unknown
obligations do not make an empty owner key unknown; actual queued conflicts still
fail. Regression cases check contradictions before root creation and after a
cached definition read. Repeated sibling reads check no-change reuse.

Source pair definitions and first receipts keep their original symbol identity.
Later receipts add equality constraints without replacing the first definition.
Equal-valued pairs and aliases remain distinct definition nodes. The tests retain
independent source/creation provenance, both graph relations, pending scopes and
all actual allocation refusal partitions.

Older T178/T201 command, failure and input records now live with their tasks in
[DONE](../../DONE.md#t178) and [DONE](../../DONE.md#t201-alignment); they do not
present alternative current APIs here.

Authority: [TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-03](../../spec/implementation/compiler.md#arch-03) and
[ARCH-05](../../spec/implementation/compiler.md#arch-05).
This component checks required expansion paths. It does not evaluate output
equations, inspect runtime state or admit executable types; those consumers are
owned by T183/T211/T212 and the later frontend/publication tasks.

## Contract and representation

Keep two disjoint relations. Creation nodes retain full normalized ordinary or producer owner identity;
edges retain the lexical owner and original application use. The forbidden
predicate is reachability between distinct complete keys of one source template,
not repetition in a capture DAG, a source-name graph or an ordinary reference.
A zero-parameter owner is retained as a vertex but is not a template
for this predicate. Definition nodes identify one static result or alias in an
exact scoped environment; their dependency graph must be acyclic.

Retain immutable ancestry frames per occurrence/environment before complete keys
exist. A selected compilation root has explicit source/environment provenance.
An application can enter only from its actual lexical owner: syntactically
nested applications in one caller are siblings, not edges between their results.
Repeated origins require complete normalized key comparison before another
scope is prepared. Equal keys close the creation graph; unequal keys retain both
canonical values and an error path. Unresolved comparisons return Pending with
no new scope/path output. Unknown origins also stay pending. Partial constructor
metadata, supplied-Type reuse and fixed ordinary constant namespaces add no
creation edge. Actual static producer uses retain compiler-only producer edges;
ordinary modules expose no static output phase. Every written child application
is checked separately, including inline nested applications inside a graph.

Use complete solver-normalized owner keys for concrete graph/cache identity. Keep
distinct symbolic occurrences and every incoming use. When keys close, merge only
equal complete owner keys and validate all retained edges under the merged
graph. On warm-cache use, include the new incoming edge and check all cached
outgoing paths against the new ancestry before allowing expansion. Binding
equality cannot silently drop provenance. Different specializations of the same
source remain distinct vertices; merging solely by source name is forbidden.

Path validation additionally retains unresolved required owners as temporary
exact-scope classes. Frames with the same guard owner use the same original
environment and captured variables; unrelated unresolved environments stay
distinct. A partial-endpoint edge remains a real recorded use before either
endpoint has a concrete key. Follow it together with complete cached edges when
checking reachability between closed endpoints. A symbolic endpoint cannot
prove inequality; it remains an independent closure obligation. This view
publishes no symbolic cache key.

Definition identity uses the original result/alias plus its scoped owner; after
owner normalization equivalent complete keys must preserve all edges. Check a
new definition dependency and the active definition ancestry before following
it. Equality links and ordinary type-reference/loading/layout/constant relations
never become definition edges merely because they mention the same Type.

Borrow healthy terms, solver and applications; own guard records, finite graph
search scratch and bounded first-error diagnostics separately. Every source and
independent descriptor outlives its borrowers. Outputs remain untouched on
failure; valid partial records may already exist internally. Failed owners are
destructible only; no whole-operation rollback is promised. No native/body/default
execution, filesystem lookup or generated-source effect belongs here.

## Invariants and exact checks

An ancestry frame is an original root or an application whose recorded lexical
environment equals its parent's expanded-definition environment. Inductively it
is a real creation path; captured Types and syntactic argument nesting add no
invented edges. Repeated complete keys are compared by exact canonical identity,
including every unused argument/capture. Unknown comparisons suspend entry
without manufacturing another environment. Repeating the same pending request
adds no semantic facts and cannot justify a closure retry.

For the finite mixed validation graph, start BFS at each concrete template
vertex. Reject precisely a reachable different concrete vertex of that same
original template. Traverse ordinary and exact unresolved-scope classes too,
without treating a partial endpoint as a complete specialization. Visited
suppression closes same-key cycles but does not skip any other outgoing edge;
starting from every concrete template vertex detects paths leaving those cycles.
Retain the exact original edges and both closed endpoint values for diagnostics.
The independent matrix oracle enumerates the complete four-vertex graph domain;
source compositions independently test actual paths and pending scope handoffs.
Full frontend scheduling and executable admission remain later tasks.

## Preservation and progress

Generic owner preparation binds every original formal to its existing scoped
variable. It changes no source binder or environment; applied owners retain their
provided full nominal shape or compiler-only producer map. Lexical parent equality prevents a written nested
argument from becoming an edge out of the outer result's definition. Active
key comparison precedes repeated target expansion. An unresolved repeat waits
on independently supplied equations instead of spawning more copies. A closed
growing or unequal permuted key is rejected; an equal key closes the graph.

Complete vertex identity is the canonical environment obtained by exact solver
normalization of all owner inputs, including unused bindings and full residual
constructor metadata. Every incoming frame
survives closure; equivalent full keys share a vertex and all original edges.
For every forbidden path, start the reachability search at its earlier closed
key and reach its different-key endpoint. Conversely, every reported pair has
the same source template and two unequal full canonical identities, connected
by a retained application path. A return to the start vertex is not a conflict.
Visited validation classes preserve reachability while bounding work, including through
ordinary cycles. Syntactic capture nesting never supplies these graph edges.

Before closure, an exact scoped owner represents the same immutable environment
and captured variables at every incoming use. Unioning its recorded outgoing
requirements therefore preserves real paths for every subsequent substitution.
Different unresolved environments are not joined. After closure, their exact
normal form authorizes canonical joining. Refresh then rechecks all incoming
and outgoing paths, so late equality cannot hide different closed keys behind
an earlier symbolic scope or cache hit. Still-unresolved endpoints remain pending,
not errors justified merely by source-name repetition.

Definition identity is an original result/alias plus its lexical environment.
Once closed, only equal complete owner key and the same original symbol merge.
The least node ordinal selects a representative, not a type or constructor.
Edges remain on original nodes; traversal unions every representative member's
edges. Therefore no incoming dependency disappears after key closure. A directed
cycle exists exactly when some representative has a nonempty path to itself.
Equal output values, equal spellings in different scopes and anchored equality
cycles supply no additional definition edges. Source loading, runtime references,
eager layout and constant evaluation remain their own relations.

An API operation performs finite scans and delegates finite solver queries; graph
searches use iterative queues, with no source-depth C recursion. During a refresh,
value normalization may transfer complete independent metadata to a canonical
equivalent. Repeat only when its actual fact count increases, so an earlier key
can observe a later query's new metadata. Registered owner graphs and their unique
normal forms are finite, and each term gets at most one complete metadata fact;
these refinements terminate without a pass budget. The caller's later expansion
work set is separately bounded by TPL-11's finite-simple-path argument,
pending-repeat barrier and canonical body reuse; this guard never expands a
source body. A same-key occurrence still retains distinct incoming provenance.
Existing parent/application pairs or producer source occurrences reuse their
registered frames and refresh changed keys. Producer entry may allocate temporary
comparison scratch; unchanged public refresh itself allocates nothing.

An unchanged refresh reuses its result only when exact path/definition counts,
equation/metadata/requirement identities and solver dirty state show no new facts.
External solving cannot hide new equations by resetting dirty. Source and term
payloads are immutable, and no binding is overwritten; changes to private records
outside these owner APIs violate the input contract. Cache reuse is an exact
invariant, not a hash guess, iteration limit or choice of preferred origin.

## Costs, ownership and failure boundaries

Let O be scoped owners, K their maximum formal count, F creation occurrences,
V concrete vertices, E concrete edges, D scoped definition nodes, H definition
edges and P definition occurrences. Each owner construction scans original role
ordinals and retains all K values; T175 interning and T176 normalization retain
their separately documented costs. Producer map costs are detailed in the T215
foundation above. Identity and membership lists are exact linear
scans. Vertex lookup is O(V), key representative selection O(D²), and recording
newly concrete occurrence edges O(F). No near-linear aggregate claim is made.

Let S be distinct unresolved required owners and N = V + S. The all-concrete
creation checker takes O(F + V(V + E)) time and O(V) temporary search storage.
With partial-endpoint edges, exact scoped-class lookup and occurrence scans take
O(V + F S + N(N + E + N F)) time and O(V + F) scratch in the conservative case.
Only actual unresolved frames contribute extra node capacity; known concrete
adjacency is reused directly. This deliberately includes linear scope lookup
and makes no near-linear mixed-graph claim.
The definition checker scans original scoped nodes for each representative's edge
union, taking O(D³ + D H) time and O(D) temporary storage. It checks all actual
representatives rather than a source-name approximation. A refresh with R genuine
metadata-refining passes adds R rounds of F + D selected T176 queries, plus its
finite key/membership scans and final graph checks. An unchanged public refresh
is O(1) and allocates nothing; entering an occurrence still performs its explicit
membership, application and active-ancestry checks.

Guard-owned retained storage is O(O K + F + V + E + D + H + P), plus the borrowed
components' canonical values, equations and source records. Successful search
scratch is released immediately. On semantic rejection, its already allocated
search array transfers to a dedicated witness arena so complete original edge
chains remain inspectable without allocation during error reporting. Active
rejection retains existing immutable ancestry plus the closing site. Fixed
diagnostic contexts identify original definitions; T189 owns final chain rendering.

Counters/products are checked before the corresponding publication/allocation.
Failures can retain valid owner/key/node/edge preparation inside guard or borrowed
arenas; complete API outputs remain untouched and the failed owner is destructible
only. No full-operation rollback is promised. Diagnostics propagate first errors
without copying borrowed storage. Destroy guards, applications, solver, terms,
binding plan, inventory/project/source and allocator in that order. Guards neither
merge runtime instances nor allocate runtime state or call native/module bodies.

## Reproduction and acceptance boundary

Run `make -C tests/unit/semantic/static/guards` with the strict and sanitizer
flags recorded in the T215 foundation above. This parent includes ordinary owner,
producer, ancestry, classification, creation graph, cache, definitions,
definition graph and failure leaves. Run `make`, `make docs-complete` and `make
docs` for compiler build, per-file/interface coverage and actual rendered-link
validation. Final T210 evidence and collected outcomes are recorded here before
its task commit. Source-level/equation consumers outside the guards remain
scheduled under their own tasks; no full static parent or language-conformance
claim follows from these component tests.

## T210 acceptance (2026-11-19)

The strict and ASan/UBSan guard parents pass all nine leaves and 43 named cases
in `/tmp/ama-t210-check-ojjtfgt_` and its separate `-san` tree. Commands use the
flags/environment documented above; logs are
`/tmp/ama-t210-{strict,san}-parent.log`, both collected exits 0. The zero-input
settlement fix restores no-allocation repeated sibling reads and refuses queued
contradictions before root creation and after cached reads. The first/equality
receipt regression passes without merging equal-valued definition symbols.

Individual strict migration checks also pass for ancestry, classification,
definitions, definition-model, cache and failure partitions. The first definition
migration run failed the idempotent-read assertion and exposed the actual empty-map
settlement defect; the solver-settlement rerun passes. No expected semantic result
was weakened to hide that failure. The final map copy explicitly reads only
formal/value fields, not ignored caller links. Five affected leaves pass strict
and sanitizer reruns after that clarification; the final compiler build also
passes (11 collected exits 0, `/tmp/ama-t210-*-final.log`). The complete
per-file/standalone-interface gate passes with 2713 inputs and zero debts
(`/tmp/ama-t210-doc-complete.log`). Final rendered links and exact staged bytes
are commit gates; their collected results and counts are recorded in the commit.

The finite domain contains 262144 creation graph/order/kind combinations and
1024 definition graph/order combinations per configuration. All eight public
allocation partitions and success/rejection scratch paths retain counted teardown.
No unchanged suites are repeated after passing: the final memberwise-copy change
reruns its five actual consumers in each configuration, plus the root build.
Unfinished T182 source/tests/evidence are excluded from the isolated trees.
The source-kind and full normalized map proofs retain every actual origin/input,
without treating graph producers as first-class Types or Templates.
