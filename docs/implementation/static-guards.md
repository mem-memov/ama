# Static dependency and creation guards

<a id="t201-alignment"></a>

## T215 producer owner foundation

Static graph producers have compiler identities but are not Type/Template values
([TPL-12](../../spec/language/static-results.md#tpl-12)). The old nominal-only
owner representation could not identify a real graph producer after T208 correctly
forbade nominal construction from static modules. T215 separates that identity
without changing the ordinary term algebra. T210 owns the remaining migration and
full expansion/cache-path acceptance; this foundation is not whole-program
admission.

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
are neither whole-language conformance nor acceptance of the old remaining
fixtures. T210 retains ancestry/cache/definition fixture revalidation; T211/T212
retain producer equations, selected outputs and result-adapter composition.

## T201 implementation alignment

T201 is accepted on input 7469fd4 under the user's 2026-11-18 instruction to continue
the agreed implementation plan. Authority is the revised TPL-11/TPL-12 and
CONST-03/CONST-05 contracts linked below. T181's unfinished constant sources
and tests remain intact. Its seven draft guard files are backed up, with exact
hashes, under /tmp/ama-t201-input-7469fd4 before replacing the unused
dependency-edge adapter. The ordered record below retains failed runs and their
resolutions; current acceptance evidence follows them.

Ordered steps and expected observations:

1. Replace origin-only ancestry refusal with exact closed-key comparison and
   pending unresolved repeats before environment/definition expansion. Remove
   the unused draft dependency API; classify ordinary nongeneric constant
   namespaces as fixed-Type reuse, preserving explicit output-phase producers.
2. Change the existing iterative mixed graph predicate to reject reachable
   *distinct closed vertices* sharing a template origin. Same-key self edges
   are traversed with visited suppression, never skipped before other outgoing
   branches are examined. Partial scopes retain exact identity and no guessed
   canonical equality. Preserve allocation-free diagnostic witness transfer.
3. Align result/publication consumers with pending guard entry and verify
   same-key nominal return versus directed static-definition recursion.
   Independently checked namespace contexts remain required for constant reuse.
4. Replace obsolete negative expectations with positive same-key and pending
   cases, retain growing/unequal-key negatives, exhaust all four-vertex graphs
   against independent matrix closure, and test reordered/cached/partial routes,
   late closure, malformed handles and every observed allocation refusal.
5. Reconcile purpose/contracts and per-file/header evidence, run strict parent
   compositions, ASan/UBSan/counting, compiler build and documentation gates;
   archive and commit before T181 resumes.

The graph proof uses finite BFS over concrete vertices and exact unresolved
scope classes; only unequal fully closed endpoints can prove a creation
conflict. An unresolved same-origin ancestry prevents further repeated expansion
and returns Pending without a new producer context. Actual equations/metadata
permit retry; allocation counts never justify progress. Existing static-definition
cycle checking remains separate. Detailed result/cost evidence will replace
the obsolete baseline explanation when implementation acceptance completes.

Step 1/2 results: removed only the backed-up unused namespace-edge draft.
Implemented pending repeated ancestry, canonical unequal-key witnesses and
mixed-graph closed-endpoint comparison. The graph leaf passes all 131072
four-vertex/order combinations (/tmp/ama-t201-graph.log, exit 0).
The first ancestry run (/tmp/ama-t201-ancestry.log, exit 2) passed the new pending
and closed-key cases, then exposed extra normalization allocation on an unchanged
repeated source query. Preserve the established hot-query contract: reuse the
exact registered parent/application path and refresh its keys, instead of
preparing its environment again. This is an engineering regression, not a
language ambiguity. No jobs active; next fix and rerun ancestry, then consumers.

Step 2/3 continuation: the hot-query fix passes ancestry. The first full guard
run stopped at an obsolete same-key-negative cache expectation
(/tmp/ama-t201-guards-first.log, exit 2); that case now uses a genuinely unequal
Leaf key, while late equal/unequal closure has separate independent expectations.
Guards and publication parents pass on the updated input
(/tmp/ama-t201-guards-strict.log and /tmp/ama-t201-publication-strict.log, exit 0).
The new publication/recursion leaf passes five cases including every observed
self-result publication allocation refusal
(/tmp/ama-t201-publication-recursion-final.log, exit 0). Its two initial runs
failed because global graph/path counts omitted the independently required
integer Type argument; assertions now identify the actual main-template key,
self edge and absence of a speculative main producer scope. Canonical application
provenance, not observed counts, determines these expectations. An optional
fixture initializer is now inline to support narrow consumers under -Werror.

Collected checks: guards parent ASan/UBSan/-Werror exits 0
(/tmp/ama-t201-guards-sanitizers.log), and publication parent under the same
configuration exits 0 (/tmp/ama-t201-publication-sanitizers.log). The result
parent first stopped at its obsolete blanket-origin refusal case, then its
new unequal-key fixture exposed inconsistent named-input/positional-output
bindings. Corrected that fixture to positional inputs/outputs as TPL-12 requires.
The complete result parent now passes under ASan/UBSan/-Werror
(/tmp/ama-t201-results-sanitizers.log, exit 0), and final uninstrumented
results/evaluate passes (/tmp/ama-t201-evaluate-final.log, exit 0).

The first root build exited 0 but reported a maybe-uninitialized capacity
warning in cache/view.c. Its capacity helper returned an external always-false
error result on refusal; returning literal false after recording the same error
makes its successful output-write contract visible to GCC, without changing
observable behavior. The root rerun (/tmp/ama-t201-build-rerun.log) exits 0
without warnings. Final sanitizer runs additionally pass the counter/allocation
failure leaf (/tmp/ama-t201-failures-sanitizers-final.log) and the recursive
publication leaf with stable unchanged-Pending retry assertions
(/tmp/ama-t201-recursion-sanitizers-final.log), both exit 0.

Final cache review reproduced a genuine cold/warm disagreement: an independently
cached nominal result could reuse a proof without the new unknown ancestor and
incorrectly report READY. The cold/warm regression first failed
(/tmp/ama-t201-cache-ancestry-red.log, exit 2). Cache reads now separate canonical
value equality from readiness certification. Reuse a proof checked in this actual
context, or a foreign proof only when every actual ancestor key is closed;
otherwise retain valid value equations and replay the original RHS in this
context. The refreshed graph still checks every cached descendant and incoming
edge. This sufficient exact certificate preserves semantics without demanding
closure of unrelated unknown outer keys. It costs O(H) time and O(1) scratch for
H ancestors; replay has the ordinary evaluator's finite-work cost.

The regression then passed (/tmp/ama-t201-cache-ancestry-green.log, exit 0).
The final recursion leaf has eight cases: cold/warm equal and unequal anchors,
actual-context proof reuse under unrelated unknown ancestors, and every observed
foreign-proof replay allocation refusal supplement the original five cases
(/tmp/ama-t201-cache-proof-composition.log, strict exit 0). Both full consumers
were rerun after the fix: results and publication pass ASan/UBSan/-Werror
(/tmp/ama-t201-results-sanitizers-cache.log and
/tmp/ama-t201-publication-sanitizers-cache.log, collected exit 0). Root make -j4
also passes without warnings (/tmp/ama-t201-build-cache.log, exit 0).

Current evidence is 32 explicit guard, 32 result and 29 publication cases. The
creation oracle covers all 65536 four-vertex directed graphs including self-loops,
in both key/edge discovery orders (131072 comparisons); the separate definition
oracle retains its 1024 relation/order checks. No compiler execution or complete
specialization admission is claimed. ASan/UBSan uses O1, strict C11/-Werror,
-fno-pie/-no-pie, and ASAN_OPTIONS=detect_leaks=0. LSan's established container
restriction remains explicit; every injected operation still unwinds through
independent counted allocation ownership checks.

Per-file evidence registers the new ancestry operation and recursive publication
C/Make leaf, preserves existing registry ordering, and maps every changed code
input to named executable consumers. Existing standalone header probes remain
required. Complete per-file/standalone-header acceptance passes 2658 ready inputs
with zero debts (/tmp/ama-t201-docs-complete-final.log, exit 0), on a HEAD-plus-T201
isolated tree excluding the unaccepted T181 additions. Markdown audit passes 215
files, 2965 local links, 214 canonical rules and 108 byte-unchanged grammar
productions (/tmp/ama-t201-markdown.log, exit 0). First Doxygen generation failed
because its automatic heading anchor duplicated the explicit t201-alignment
anchor (/tmp/ama-t201-doxygen.log, exit 1). Renamed only the visible heading,
retaining the stable explicit link destination. The rerun passes
(/tmp/ama-t201-doxygen-rerun.log, collected exit 0): 2008 maintained file pages,
7795 interfaces, 427 canonical/rendered destinations and 6346 generated pages.
Editor navigation to templates.md:1514 exits 0
(/tmp/ama-t201-editor-navigation.log); this records command dispatch, not a manual
GUI click. Complete task evidence is archived in [DONE](../../DONE.md#t201).
Final archive-input regeneration uses /tmp/ama-t201-doxygen-archive.log; collect
its outcome before explicit staging and record it in the task commit. No active
functional jobs. T181 is next; its partial sources/tests remain excluded from
T201. Specification bytes have not changed.

T178's original acceptance below remains historical evidence on input 8cfe3f6.
The current contract is aligned by T201 on input 7469fd4; its acceptance and
checks are recorded above. T175 supplies scoped immutable identities,
T176 finite normalization and T177 source-preserving application records.
Authority: [TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[COMP-03](../../spec/implementation/compiler.md#comp-03),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-03](../../spec/implementation/compiler.md#arch-03),
[ARCH-05](../../spec/implementation/compiler.md#arch-05).
T178 did not authorize specification changes. This component guards requests; it does
not evaluate static outputs, inspect runtime state or admit executable types.
T179 owns output equations and T184 owns frontend work scheduling/admission.

## Contract and representation

Keep two disjoint relations. Creation nodes retain full nominal Type identity;
edges retain the lexical owner and original application use. The forbidden
predicate is reachability between distinct complete keys of one source template,
not repetition in a capture DAG, a source-name graph or an ordinary reference.
An ordinary zero-parameter producer is retained as a vertex but is not a template
for this predicate. Definition nodes identify one static result or alias in an
exact scoped environment; their dependency graph must be acyclic.

Retain immutable ancestry frames per occurrence/environment before complete keys
exist. A selected compilation root has explicit source/environment provenance.
An application can enter only from its actual lexical owner: syntactically
nested applications in one caller are siblings, not edges between their results.
Repeated origins require complete normalized key comparison before another
scope is prepared. Equal keys close the nominal graph; unequal keys retain both
canonical values and an error path. Unresolved comparisons return Pending with
no new scope/path output. Unknown origins also stay pending. Partial constructor
metadata, supplied-Type reuse and ordinary nongeneric no-output constant
namespaces add no creation edge;
an explicit output phase does require its producer, including ordinary modules
and already supplied Types. Every written child application is checked separately.

Use complete T176-normalized Type keys for concrete graph/cache identity. Keep
distinct symbolic occurrences and every incoming use. When keys close, merge only
equal complete nominal keys and validate all retained edges under the merged
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
provided full nominal shape. Lexical parent equality prevents a written nested
argument from becoming an edge out of the outer result's definition. Active
key comparison precedes repeated target expansion. An unresolved repeat waits
on independently supplied equations instead of spawning more copies. A closed
growing or unequal permuted key is rejected; an equal key closes the graph.

Complete vertex identity is the exact T175 normal form returned by T176, including
all unused fixed bindings and retained constructor captures. Every incoming frame
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
Once closed, only equal complete owner Type and the same original symbol merge.
The least node ordinal selects a representative, not a type or constructor.
Edges remain on original nodes; traversal unions every representative member's
edges. Therefore no incoming dependency disappears after key closure. A directed
cycle exists exactly when some representative has a nonempty path to itself.
Equal output values, equal spellings in different scopes and anchored equality
cycles supply no additional definition edges. Source loading, runtime references,
eager layout and constant evaluation remain their own relations.

An API operation performs finite scans and delegates finite T176 queries; graph
searches use iterative queues, with no source-depth C recursion. During a refresh,
value normalization may transfer complete independent metadata to a canonical
equivalent. Repeat only when its actual fact count increases, so an earlier key
can observe a later query's new metadata. Registered owner graphs and their unique
normal forms are finite, and each term gets at most one complete metadata fact;
these refinements terminate without a pass budget. The caller's later expansion
work set is separately bounded by TPL-11's finite-simple-path argument,
pending-repeat barrier and canonical body reuse; this guard never expands a
source body. A same-key occurrence still retains distinct incoming provenance.
Existing parent/application pairs reuse their registered frame and refresh
changed keys, avoiding repeated environment/normalization allocation.

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
their separately documented costs. Identity and membership lists are exact linear
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

## T178 step results and acceptance (historical)

- Guard ownership and lexical owner shapes are implemented. Generic owners bind
  every original formal to its existing scoped variable; applied owners preserve
  their original nominal shape. Strict owner leaf passed three cases, including
  all observed allocations, foreign/missing handles and checked counters
  (`/tmp/ama-t178-owner.log`, collected exit 0).
- Immutable symbolic paths retain selected roots, actual lexical parents,
  original application records and complete rejected ancestry. Source template
  repetition is checked before target environment preparation or expansion.
  Strict ancestry leaf passed four cases (`/tmp/ama-t178-ancestry.log`, collected
  exit 0): equal/permuted/growing/direct and mutual recurrence, finite repeated
  argument nesting, sibling reuse, idempotent occurrence queries and rejection
  of an incorrect lexical parent. No canonical key is required for this guard.
- Actual T173–T177 source handoffs distinguish unapplied constructor metadata,
  unknown applied origins, plain Type reuse and explicit output-phase producers.
  Ordinary supplied producers and ordinary self edges remain distinct from
  repeated-template paths. Strict classification leaf passed two cases
  (`/tmp/ama-t178-classification.log`, collected exit 0).
- Complete-key graph refresh and whole cached-descendant checking are implemented.
  A frame keeps its lexical environment while concrete graph identity
  uses only T176-normalized closed keys; retain all incoming frames. Normalize
  until newly derived independent metadata facts stop changing, then record
  edges and check reachability by full vertex keys. Warm no-change checks may
  reuse results only while path/equation/metadata/requirement counts and solver
  dirty state certify no new evidence. These are exact monotone facts, not a
  retry budget or heuristic.
- Strict path parent passed its then-current nine cases after graph-refresh
  composition (`/tmp/ama-t178-path-parent.log`, collected exit 0). Independent
  matrix closure agrees with the creation checker for all 65536 four-vertex
  graphs, in both insertion orders: 131072 graph/order combinations. Equal-source
  unequal Type keys remain distinct; ordinary cycles, self edges, complete search
  witnesses and immediate successful scratch release are included
  (`/tmp/ama-t178-graph.log`, collected exit 0).
- Strict cache leaf passed three real-source compositions
  (`/tmp/ama-t178-cache.log`, collected exit 0): independent A/B paths with a
  shared finite Leaf tail, a new edge exposing recurrence through cached
  descendants, and symbolic scopes becoming a genuine cycle after key closure.
  External solving with dirty=false cannot hide newly added equation identities.
  One hundred unchanged refreshes allocate nothing. These are component checks;
  unexpanded source fields are not claimed as complete program admission.
- Separate definition node/edge/path records, active checks and scoped-key cycle
  checking are implemented. Shared refresh is named `static_guard_refresh`
  because it checks both relations. Combined then-current parent passed thirteen
  cases (`/tmp/ama-t178-combined-parent.log`, collected exit 0).
- Six static-definition cases passed (`/tmp/ama-t178-definitions-final.log`, exit
  0): direct/mutual source results, cached foreign ordinary-producer dependencies,
  actual receiver aliases, anchored equality controls, scoped-to-concrete merging
  and malformed foreign handles/sites. The graph model independently enumerates
  all six topological orders for each of 512 three-definition relations in two
  discovery orders: 1024 graph/order combinations
  (`/tmp/ama-t178-definition-model.log`, collected exit 0).
- Eight public-operation allocation cutpoints and four success/rejection graph
  scratch cutpoints passed every observed refusal. Checked owner/path/vertex/
  edge/definition/definition-edge/definition-path counts, both search products,
  missing outputs and borrowed first-error propagation are covered
  (`/tmp/ama-t178-failures-final.log`, collected exit 0).
- Final decomposition places combined orchestration at the guard parent and
  keeps creation-key normalization, definition-edge changes and definition-path
  publication in cohesive child files. Full strict guard parent passed 24 cases
  after that extraction and final counter refinements
  (`/tmp/ama-t178-final-parent.log`, collected exit 0).
- Added an explicit negative source composition for a captured own constructor
  whose next application would terminate: its different finite full key does
  not exempt the repeated original template. Final cache leaf passed four cases
  (`/tmp/ama-t178-captured-final.log`, collected exit 0). The global dispatcher and
  precise per-file/header registry now include all 25 guard cases.
- Root build passed (`/tmp/ama-t178-build.log`, collected exit 0). Complete
  per-file/header coverage passed 2597 ready inputs and zero debt
  (`/tmp/ama-t178-docs-complete.log`, collected exit 0). Markdown navigation passed
  212 documents, 2914 local links, 214 rules and 108 unchanged grammar productions
  (`/tmp/ama-t178-markdown.log`, collected exit 0).
- All 25 guard cases passed strict AddressSanitizer/UBSan
  (`/tmp/ama-t178-sanitizers.log`, collected exit 0), including both exhaustive
  graph domains and every allocation refusal. LSan is disabled under the prior
  established container ptrace restriction; counted zero-live-allocation teardown
  and immediate successful scratch-release checks remain separate evidence.
- First complete Doxygen generation passed (`/tmp/ama-t178-doxygen.log`, collected
  exit 0): 2597 ready inputs/zero debt, 1959 maintained file pages, 7554 documented
  interfaces, 424 actual destinations and 6182 generated pages. Actual editor
  navigation `code --goto /home/u/repos/ama/spec/language/templates.md:1510`
  passed (`/tmp/ama-t178-editor-navigation.log`, collected exit 0); this records
  command dispatch, not an observed manual GUI click.
- Full accepted record is archived in DONE. Final archive-input reference refresh
  uses `/tmp/ama-t178-doxygen-archive.log`; collect its exit status before explicit
  staging and the per-task commit. No functional jobs remain. Next ready task:
  T179, after that commit. No specification change or semantic ambiguity.
- Initial archive refresh passed with the same counts
  (`/tmp/ama-t178-doxygen-archive-initial.log`, collected exit 0). Staged-diff
  review then found extra empty EOF lines in the three extracted child files;
  removed those whitespace-only lines. C tokens and preceding source positions
  are unchanged. Incremental root rebuild and final reference regeneration use
  `/tmp/ama-t178-build-final.log` and `/tmp/ama-t178-doxygen-archive.log` before
  commit; repeating behavioral tests for that token-identical cleanup is unnecessary.

## Reproduction

- Strict parent: `make -C tests/unit/semantic/static/guards
  C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
  -D_POSIX_C_SOURCE=200809L'`.
- Final captured-origin delta uses the same flags with
  `make -C tests/unit/semantic/static/guards/cache`.
- Sanitized parent: `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
  UBSAN_OPTIONS=halt_on_error=1 make -C tests/unit/semantic/static/guards
  C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic
  -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined
  -fno-omit-frame-pointer'`.
- Build `make`; per-file/standalone header checks `make docs-complete`; actual
  generated reference/destination checks `make docs`. The recorded Markdown audit
  `/tmp/ama-t168-doc-check.py` supplements those maintained acceptance gates.

All reported jobs had their completion status collected. Global executable
conformance remains scheduled in T196; these guarded compiler services do not
by themselves admit template syntax through the complete frontend or emit code.
