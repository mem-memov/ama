# Scoped static symbols and finite signatures

T172 boundary: semantic/static inventory, scope and signature components. Input
2562706 includes the explicitly authorized AMB-011 clarification. This component
binds source names and category/signature obligations; it does not specialize
modules, solve concrete type equations, create runtime fields or generate C.
The loaded Project and immutable AST/source bytes outlive every static record.

## Ownership and exact scope operations

A StaticInventory owns one arena and borrows one valid Project plus allocator
authority. Module records refer to exact ProjectModule identities. Names compare
complete SourceSpan byte sequences, including three dots and all components,
without normalization or filesystem lookup. Each static symbol has an explicit
parameter/result/alias role. A parameter or declared result is visible throughout
its module; an alias is visible from its actual receiver offset onward. Runtime
block nesting does not shorten that static scope. Foreign labels/signature entries
are not inserted into the surrounding module's symbol namespace.

Allocation and diagnostic primitives own partial records even on failure. Error
messages use constant storage, primary/related sites borrow the Project, and
reporting performs no allocation. A failed inventory remains destructible and
cannot certify a usable semantic stage. Exact lookup of already inserted records
is a component operation, not a claim that whole-interface collection is complete.
Only a completed valid inventory may feed later static semantic stages.

Static results and aliases may retain explicitly unresolved categories, with their
actual defining/receiving syntax. No unresolved category defaults to Type.
Template parameters retain finite expected named signatures; unknown projected
producer metadata becomes an explicit obligation for T179. Signature names belong
to their own expected interface, never the caller's local parameter table.

## Planned collection and validation steps

1. Implement and independently test lifetime, exact module/name lookup,
   insertion, duplicate diagnostics and position-based alias visibility.
2. Collect all explicit formals/results and structural interface introduction/
   receiver sites. Classify aliases before finalizing genuine free parameters,
   honoring TPL-02/TPL-12 and output-only heads under T199.
3. Build/check finite recursive signatures, including exact name-set equality,
   known category errors and explicit unresolved expression obligations.
4. Bind every local expression head/receiver in source order; reject unknown
   body/shared names and alias use before reception. Keep foreign labels distinct.
5. Run parser/project-to-inventory compositions, deep/failure partitions and
   per-file/documentation acceptance before archiving and committing T172.

The primitive algorithms scan finite owned lists. Exact-name comparison costs
O(name bytes); lookup costs O(symbols times maximum name bytes), without guessed
matches or hash identity. Insertion either extends one tail/count invariant or
reports a duplicate without linking another symbol. Arena destruction frees its
finite allocation chain rather than traversing recursive signature structure.
Whole-collection termination/cost arguments will be completed with those actual
algorithms, not assumed from these smaller primitives.

Initial checkpoint (superseded by the later phase records): step 1: define the owner/model and narrow scope operations, then add
independent unit cases. No jobs are running. T172 is unfinished; no whole-template
inference, category classification or compiler integration is claimed yet.

Step 1 results (2026-11-18): model/public/private interfaces and cohesive owner,
error, scope lookup and insertion components are implemented. The independent
scope leaf passes exact module/name separation, allocation-free queries, alias
visibility at/before/after reception, all nine duplicate-role pairs, every actual
insertion allocation failure and first-error retention
(`/tmp/ama-t172-scope-primitives.log`). Added boundary partitions compare distinct
normalization spellings without folding them, keep ordinals explicitly unassigned,
and inject maximum parameter/result counters to prove checked refusal without
counting aliases. The static parent dispatches this leaf and passes all four cases
(`/tmp/ama-t172-scope-parent.log`); session 43977 completed successfully. All maintained
primitive files have named per-file registry evidence and standalone header probes.

Remaining: AST collection/events, explicit binder/result inventories, source-order
implicit parameter/alias classification, finite signature construction/comparison,
local use binding and actual parser-to-inventory cases. No full collector or
valid inventory publication is implemented or claimed. Do not archive or commit
T172 on these primitive tests alone; pending metadata is explicit, never Type by
default. Next action: collect the parent run, then implement/test structural event
collection using original byte positions, keeping foreign labels out of locals.

Structural occurrence phase: new events components append source heads/receivers,
walk expression and runtime owning edges with explicit arena frames, and stably
merge-sort their original positions. Context records distinguish shared, input,
runtime result, static definition, body and namespace expressions; no table lookup
or category guess occurs here. Foreign labels/signature entries and omitted
receivers never become local occurrences. Head/receiver records retain exact AST
owners for subsequent classification. Parameter/result inventories are still pending.

The strict static parent passed scope plus the initial event cases with
`-Wall -Wextra -Werror -std=c11 -pedantic`
(`/tmp/ama-t172-events-parent.log`). Events cover all six expression contexts,
nested operands/receivers and exact positions. The sorter is checked against an
independent stable insertion oracle for 120 five-record permutations and each
0..5 prefix (720 cases), without allocation or identity loss. Collection failure
injection and maximum-count refusal preserve partial ownership. Expanded owning-
edge/depth cases run in session 19137 (`/tmp/ama-t172-events-expanded.log`);
collect before source edits. Original optional project fixture queries are now
static inline so narrowed fixture consumers compile without unused-function
warnings; their bodies and behavior are unchanged.

Traversal invariant: each pending record owns an unprocessed AST subtree/next
node. Processing advances that cursor and queues only real child owners; command
and output aliases are not queued twice. Expression work similarly consumes a
head and actual input children, emitting receivers at original spans. Every
nonconsuming transition pops a previously created frame. With n visited AST/
expression/binding records and e occurrences, collection costs O(n + e) time and
arena storage. Stable merge passes partition the complete chain into sorted runs,
double their bound without overflow, preserve equal-site order, and cost
O(e log e) time with no sorting allocation. This is lexical extraction only;
subsequent scope/signature binding has its own remaining correctness obligations.

Expanded occurrence checks passed (`/tmp/ama-t172-events-expanded.log`): selected
handlers, root handlers, folded loop commands/handlers, loop and traversal bodies,
both branch arms and 600-deep expression owners are visited exactly once. The
expanded and interface sources each pass every actual collection allocation
refusal; no alias ownership edge is followed twice. Event tests are decomposed
into context, ordering, failure and depth support files with explicit named
coverage. Source collection still binds no symbols.

Explicit inventory phase now scans only top-level @< formals and @> static
results. It retains original declaration ASTs, Type/Template spelling categories
for explicit formals, separate result ordinals and explicitly deferred result
categories. Runtime fields and foreign signature slots contribute no local name.
Semantic signature records are intentionally not built by this earlier phase;
no complete inventory validity is published. The strict parent including the new
explicit leaf is session 83867 (`/tmp/ama-t172-explicit-parent.log`); that run
completed successfully. Four cases check runtime/static interleaving, whole-module
visibility, nested foreign-name separation, explicit binder/result collisions,
unused native formals for T174 and every allocation failure.

All new primitive/event/explicit files now have per-file named evidence; header
commands remain registered. No jobs are running. Next: finite signature building
and comparison, then source-order implicit introduction/alias classification and
local-use binding. T172 remains unfinished and uncommitted.

Finite signature phase: explicit signatures now build owned foreign trees with
iterative work frames, reject same-level duplicate complete names, and publish
only a complete root. Named equality is independent of sibling order; known
category/name mismatches win over unrelated deferred slots. The comparer
memoizes exact ordered signature-pointer pairs as work identities, not semantic
identities. This bounds shared-DAG processing by the finite number of distinct
pairs instead of repeatedly unfolding shared branches. Unit partitions cover
reordered nested sets, missing/renamed slots, category changes, symmetric pending
metadata, every real build/compare allocation refusal, 900-deep trees and a
100-level binary-sharing DAG. The initial signature leaf passed
(`/tmp/ama-t172-signature.log`); its expanded memoization case is in the parent
run below.

Classification and binding phase: sorted candidate/receiver positions determine
roles after explicit formals/results are collected. Only genuine implicit
parameters participate in whole-interface Template collection. Earlier bare uses
of a collected Template are forward uses; its actual pattern introduction owns
its formal ordinal. Explicit formals keep their own declaration positions.
Foreign labels do not participate. The classifier leaf passed
(`/tmp/ama-t172-classify.log`), including output-only heads/skips and explicit
controls, result-only Types, receiver precedence, forward Template use and
original ordinals, excluded body/result-applied heads and every allocation
refusal. Scope binding subsequently resolves exact local identities at each
original position or the exact retained ProjectDependency request; it cannot
repair an earlier alias use or create a body parameter.

Whole collection now has one public attempt boundary. It collects every loaded
module before resolving cross-module signatures; prepares explicit, inferred
pattern and original-order constructor skeletons; propagates independently known
result/slot metadata monotonically; detects actual known signature cycles with
an explicit DFS and exact pointer memoization; and checks repeated/explicit
patterns against complete named signatures. A known Type cannot accept static
input bindings. Repeated acyclic subtrees remain legal. Only successful lexical
collection/known checks publish inventory.valid. The flag does not certify
application normalization, static output closure, executable specialization or
frontend integration. Actual AST result definitions and receiver sites remain
available with no runtime field or evaluation.

Pending ownership is explicit: T173/T175/T177 normalize input applications and
residuals; T179 resolves projected producer metadata. Signature entries retain
pending_expression, pattern records retain their whole source occurrence and
pending comparison, and aliases retain their receiver. Deferred never becomes
Type by default or equality success. Later metadata resolution must rerun known
cycle/category/signature checks before executable acceptance. This component's
finite-scope validity is deliberately narrower than whole-program validity.

Correctness/termination: each name's first syntactically eligible candidate or
receiver fixes its role; Template collection then examines the finite interface
rather than actual-value order. Ordinals count genuine preceding introduction
sites, so distinct positions induce a unique dense order. The monotone metadata
pass changes each deferred slot/result at most once; a no-change pass terminates.
Known signature DFS consumes each edge once, with an active record exactly for
an ancestor on the explicit cursor chain. An edge to active therefore witnesses
a cycle; completed sharing is not a cycle. Exact linear identity lookup costs
O(V(V+E)); explicit-frame storage is O(V). Comparison visits at most P distinct
ordered pairs, using O(P) scratch; linear memo lookup costs O(P squared), plus
whole-name membership scans at those pairs. These exact bounds use no hash
collisions, nesting threshold, timeout, first-actual choice or structural guess.

The initial whole-collection composition passed
(`/tmp/ama-t172-composition.log`), covering actual output-only bindings, results,
foreign-name separation, forbidden sources/categories, branch aliases, repeated
signature agreement, direct/mutual known cycles, acyclic sharing, cross-module
constructor metadata, explicit pending obligations and every real whole-stage
allocation failure. The expanded strict six-leaf parent is session 66425
(`/tmp/ama-t172-known-parent.log`); collect its result before further source edits.
T172 remains unfinished until parent/regression/sanitizer/documentation acceptance.

The strict six-leaf parent completed successfully
(`/tmp/ama-t172-known-parent.log`, session 66425). Boundary review then extended
known-constructor coverage: concrete roots also retain/check foreign patterns;
when an output phase requests producer closure, provided root inputs may be a
subset under TPL-12. Without that phase, the complete root input signature must
match under TPL-07. Subset comparison never relaxes a nested Template slot's
expected signature. Added actual compositions cover concrete-root foreign-name
separation and omitted input slots with/without outputs. The updated parent is
session 61300 (`/tmp/ama-t172-final-parent.log`); collect before source edits.
The per-file registry now names every actual component/composition case and
retains independent header compilation commands. No acceptance gate is inferred
from registration alone.

Functional acceptance (2026-11-18): session 61300 completed the strict parent
successfully (`/tmp/ama-t172-final-parent.log`). Added independent provided-root
versus exact-nested comparison checks and a real 700-level parsed explicit
signature exercise the final boundary. All 36 cases across six leaves passed
with strict C11 warnings as errors, AddressSanitizer and UBSan in session 60286
(`/tmp/ama-t172-sanitizers.log`). LeakSanitizer remains disabled for the documented
container limitation; counted allocator teardown independently checks every
partial owner and injected failure. The full project parent passed session 16908
(`/tmp/ama-t172-project.log`); compiler `make` passed session 65583
(`/tmp/ama-t172-build.log`). These runs finished before comment edits.

The first documentation-completeness gate correctly rejected missing canonical
links in several helper contracts (`/tmp/ama-t172-doc-complete.log`); all newly
maintained compiler Doxygen contracts now include their canonical component
rule. No behavior changed in this documentation correction. The local document
audit passed 206 Markdown files, 2884 links, 214 canonical rules, 108 unchanged
EBNF productions and 37 template areas (`/tmp/ama-t172-links.log`). Full generated
documentation acceptance and archival remain pending; no complete sprint or
executable-template support is claimed by this task.

Documentation completeness now passes: 2405 maintained inputs are ready, with
zero explicit debt (`/tmp/ama-t172-doc-complete-final.log`, session 39482).
Canonical links in classification helpers were additionally narrowed to TPL-02,
which actually defines Template collection and formal introduction order.
The correction changes comments only. Generated Doxygen file pages/interfaces
and canonical destinations are the remaining acceptance check.

Final T172 acceptance: generated Doxygen passed in session 45173
(`/tmp/ama-t172-doxygen.log`): 1811 maintained file pages, 6939 documented
interfaces and all 418 referenced canonical destinations; 5711 total pages.
The generated-link validator checked actual HTML targets and source stability.
Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0 were used. Source navigation
also passed: `code --goto /home/u/repos/ama/spec/language/templates.md:110`
returned success at TPL-02's actual source location; this records editor command
acceptance, not an unobserved GUI click. Canonical EBNF/specification content was
not changed by T172.

Additional phase cost bounds: let N be occurrences, S local symbols, L total
expression-binding records, B maximum compared name bytes, and M modules.
Classification uses O(NSB + SL + S squared) time: each new implicit parameter
may scan the finite interface for a Template site, while ordinals count earlier
symbols. Binding uses O(N(S + N)B + L) including exact dependency request scans.
Constructor preparation uses O(S squared) ordinal selection, and explicit
signature duplicate checks cost the sum of O(q squared B) over sibling sets of
size q. If H is the finite number of result/signature slots considered by the
metadata pass, each changes from deferred at most once, yielding at most H+1
passes; each pass costs O(H(N + M + S) + L). These are conservative finite
operation bounds, not arbitrary source limits. Exact-name and pointer memo
searches intentionally prioritize transparent identity/ownership contracts;
future indexing may preserve the same equality without changing semantics.

T172 is accepted and archived with its full task record and evidence. No jobs
remain from functional acceptance. The final archive/documentation input is
regenerated before committing. Remaining sprint work starts at T173; application
normalization, static equations, native admission and executable template
integration remain owned by their existing tasks.

The archive refresh also passed (`/tmp/ama-t172-doxygen-archive.log`, session
62936), with unchanged 1811/6939/418 documentation counts. Staged review found
one extra blank line at the end of metadata.c; it was removed without changing
any C token. The active checkpoint was made independent of transient runner
status. Final staged-tree documentation refresh is logged in
`/tmp/ama-t172-doxygen-staged.log`; collect it before the authorized commit.
Functional tests need no repetition for this whitespace/checkpoint correction.
