# Static dependency and creation guards

T178 is accepted on input 8cfe3f6. T175 supplies scoped immutable identities,
T176 finite normalization and T177 source-preserving application records.
Authority: [TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[COMP-03](../../spec/implementation/compiler.md#comp-03),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-03](../../spec/implementation/compiler.md#arch-03),
[ARCH-05](../../spec/implementation/compiler.md#arch-05).
No specification change is authorized. This component guards requests; it does
not evaluate static outputs, inspect runtime state or admit executable types.
T179 owns output equations and T184 owns frontend work scheduling/admission.

## Contract and representation

Keep two disjoint relations. Creation nodes retain full nominal Type identity;
edges retain the lexical owner and original application use. The forbidden
predicate is two occurrences of one original source template on a creation path,
not repetition in a capture DAG, a source-name graph or an ordinary reference.
An ordinary zero-parameter producer is retained as a vertex but is not a template
for this predicate. Definition nodes identify one static result or alias in an
exact scoped environment; their dependency graph must be acyclic.

Retain immutable ancestry frames per occurrence/environment before complete keys
exist. A selected compilation root has explicit source/environment provenance.
An application can enter only from its actual lexical owner: syntactically
nested applications in one caller are siblings, not edges between their results.
Known actual origins are checked against every ancestor before following their
definitions, even when captures remain symbolic. Unknown origins stay pending.
Partial constructor metadata and plain supplied-Type reuse add no creation edge;
an explicit output phase does require its producer, including ordinary modules
and already supplied Types. Every written child application is checked separately.

Use complete T176-normalized Type keys for concrete graph/cache identity. Keep
distinct symbolic occurrences and every incoming use. When keys close, merge only
equal complete nominal keys and validate all retained edges under the merged
graph. On warm-cache use, include the new incoming edge and check all cached
outgoing paths against the new ancestry before allowing expansion. Binding
equality cannot silently drop provenance. Different specializations of the same
source remain distinct vertices; merging solely by source name is forbidden.

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

## Ordered steps and independent acceptance

1. Add scoped guard ownership, original/canonical key records and source-backed
   diagnostic paths. Validate membership before dereferencing foreign handles.
   Test empty owners, exact key reuse, different environments/values and refusal.
2. Implement symbolic active creation ancestry and source-backed application
   classification. Check direct/mutual/permuted/growing paths before expansion,
   finite nested supplied Types, independent sibling reuse, retained captures,
   partial metadata, ordinary producers and output-phase override.
3. Add concrete key refresh and exact whole-path cache validation. Test warm and
   cold agreement, cached descendants conflicting with a new ancestor, distinct
   specialization paths that would form a false source-name cycle, and key
   closure preserving incoming provenance. Compare finite graphs with an
   independent exhaustive path oracle; use no depth/time/iteration limit.
4. Add separate static-definition identities, dependencies and active guards.
   Test direct/mutual/alias/foreign-result cycles, acyclic siblings and ordinary
   reference/equality controls, including symbolic-to-concrete key handoffs.
5. Compose actual T173–T177 source/term/equality records, refuse every observed
   allocation and checked counter boundary, justify finite progress and costs,
   register precise per-file/header evidence, run strict affected tests,
   sanitizers/build/docs, archive acceptance and commit before starting T179.

## Planned invariant and exact checks

An ancestry frame is an original root or an application whose recorded lexical
environment equals its parent's expanded-definition environment. Inductively it
denotes a real creation path; no captured Type or syntactic argument relation is
inserted. Appending a known template origin is permitted exactly when it is not
already on that path. Frames retain symbolic environments without guessing keys.

For a finite concrete graph, the path predicate is equivalent to absence of a
nonempty directed path from any template vertex to another vertex with the same
original source template. Search each template vertex using a visited set of
full concrete vertices. This checks cached descendants under all incoming paths,
permits ordinary-only reference cycles, terminates on finite vertices and never
combines unequal specialization identities. Keep witness edges for diagnostics.
The implementation and independent graph domains below discharge this algorithm
obligation; full frontend scheduling and executable admission remain later tasks.

## Preservation and progress

Generic owner preparation binds every original formal to its existing scoped
variable. It changes no source binder or environment; applied owners retain their
provided full nominal shape. Lexical parent equality prevents a written nested
argument from becoming an edge out of the outer result's definition. Active
template-origin comparison precedes target expansion and does not need concrete
keys, so growing and permuted keys cannot defer rejection indefinitely.

Complete vertex identity is the exact T175 normal form returned by T176, including
all unused fixed bindings and retained constructor captures. Every incoming frame
survives closure; equivalent full keys share a vertex and all original edges.
For every forbidden path, start the reachability search at its first repeated
template origin and reach its later occurrence. Conversely, every reported
nonempty same-origin reachability path is precisely a forbidden creation path.
Visited full vertices preserve reachability while bounding work, including through
ordinary cycles. Syntactic capture nesting never supplies these graph edges.

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
work set is separately bounded by the source-template ancestry rule and canonical
ordinary-body reuse; this guard component itself never expands a source body.

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

The creation checker takes O(V(V + E)) time and O(V) temporary search storage.
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
diagnostic contexts identify original definitions; T183 owns final chain rendering.

Counters/products are checked before the corresponding publication/allocation.
Failures can retain valid owner/key/node/edge preparation inside guard or borrowed
arenas; complete API outputs remain untouched and the failed owner is destructible
only. No full-operation rollback is promised. Diagnostics propagate first errors
without copying borrowed storage. Destroy guards, applications, solver, terms,
binding plan, inventory/project/source and allocator in that order. Guards neither
merge runtime instances nor allocate runtime state or call native/module bodies.

## Step results and continuation

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
