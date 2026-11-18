# Guarded static result equations

**T201 alignment:** [Creation guards](static-guards.md#t201-alignment) records
the revised [TPL-11](../../spec/language/templates.md#tpl-11) handoff and current
evidence. An otherwise known application propagates Pending when its repeated
ancestry cannot yet compare complete keys. Closed equal keys can produce nominal
results without recursively evaluating those results; directed definition cycles
still fail independently. T179's dated evidence below retains its original scope.

T179 is accepted on input e48fc3e. Authority:
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-11](../../spec/language/templates.md#tpl-11),
[TPL-07](../../spec/language/templates.md#tpl-07),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
T179 did not authorize specification changes. T180 owns final producer closure/alias
publication, T181 runtime constant namespaces, T182 typed runtime connections,
T184 whole-project closure, T185 ordinary validation and T189 final diagnostic
rendering. Corrected the T178 note's future-renderer reference from T183 to T189;
T183 actually owns specialization identity/registry work.

## Contract and source boundary

Operate on immutable collected AST static expressions and accepted scoped
identities. Borrow healthy guards and, through them, applications/solver/terms/
binding plan/inventory/source. Own expression/definition/projection work and
first-error context separately. No module body, initializer, default, native hook,
runtime constant value, method flag or list content is evaluated.

Bind a concrete head through its existing exact occurrence origin, and a
placeholder through its original scoped symbol. Result definitions use their
original `AST_STATIC_RESULT` expression. Alias definitions use the exact original
receiver binding stored on the symbol, not a later same-spelling receiver or a
text search. Original owner environments prevent capture between templates.
Source aliases are classified by T172; this component adds no source parameter.

Each static expression has its own internal result variable. Nested input
expressions evaluate in the same lexical owner, not the outer application's
result owner. T173/T175 retain actual requests and prepared RHS identities;
T177 determines actual captures, residuals and omission semantics. The expression
value remains its producer Type when it has output actions; selected outputs
have separate derived result/receiver terms. A skip supplies no receiver equation.

Resolve output fields only after independently knowing the actual producer
origin. Re-normalize against that actual interface, with static input/output
positions independent and runtime fields excluded. Unknown origins remain
Pending while further evidence may arrive; no output name, expected category or
receiver selects an origin. Known missing/repeated/over-arity output fields fail.

Known output-producing applications enter T178's creation guard before any
producer definition is followed. Expand every declared static result, including
unselected and skipped outputs, in the producer's environment. Original result
or alias value reads enter the distinct static-definition guard. Static output
receivers impose equations; do not add a definition dependency merely because
an existing name appears as a receiver. An existing result receiver must not
create a fictitious self-definition cycle. Alias value definitions retain the
dependency on their original producer outputs, never a mutable new assignment.

Generate equations for result definitions and exact receiver values, retaining
source uses even when value identities are shared. The same original occurrence/
environment uses the same inference/result variables across pending refinements.
Outer producer inputs may remain unknown here; T180 must require all of them
before publishing any selected value. Returned constructors may retain their
own local residuals. Generated equations do not invert general output functions.

## Evaluation and provenance obligations

Keep actual evaluation frames separate from canonical Type values. Each frame
retains lexical environment, original expression, creation ancestry and current
definition ancestry. Walk nested expressions and definition dependencies with an
explicit stack/work list, never C recursion proportional to source depth.
Before following a new dependency, perform both required guards. Captured alias
or result reads retain their original source environment rather than importing a
matching name into the current module.

A semantic value may be reused without redoing equal equations, but its incoming
creation/definition provenance cannot be discarded. Symbolic owners are not a
completed canonical cache: expand/replay their source requirements under each
incoming guarded path. Complete-key caches must validate their outgoing paths
under new ancestry through T178. Adding another unchanged operator/use record is
not semantic progress; defer genuinely unknown obligations until new structural
facts arrive. Final underdetermination/admission is owned by later closure steps.

Independent expected metadata may be refined from its declared expression and
owning environment. Never obtain a pending formal's expected signature by
accepting its first actual argument. Category knowledge alone neither chooses a
nominal origin nor resolves positional slots from an expected signature order.

All outputs remain untouched on refusal. Partially prepared owned records and
valid equations/canonical child values may survive internally; there is no full
rollback promise. Failed owners are destructible only. Preserve original producer,
definition, receiver and application spans for local diagnostics and later T189
chain rendering. Destroy this plan before the guards and all borrowed owners.

## Ordered steps and independent acceptance

1. Bind exact source expressions and original result/alias definitions; establish
   scoped terms and occurrence/provenance records. Test source-order alias
   identity, foreign pointers, different owners and all preparation refusals.
2. Add iterative static expression preparation and actual-origin application
   handoffs, including nested partial/complete constructors and output-phase
   omission. Test expression-value/selected-output separation and actual positions.
3. Expand result/alias value reads through T178, preserving symbolic scopes and
   creation paths. Generate definition and receiver equations; test direct,
   mutual/cached definition and repeated-creation errors before expansion.
4. Validate every required producer output, not just selected ones. Compose
   concrete/symbolic Types, returned partial families, nested projections,
   existing result/parameter/alias receivers and typed expected-result evidence.
   Test Pending origins, wrong names/kinds/runtime fields, skips and noninvertible
   outputs; verify no native/default/body effect.
5. State finite progress and cost invariants for actual work records, refuse all
   observed allocations and arithmetic/publication boundaries, register precise
   per-file/header evidence, run strict component/composition tests, sanitizers,
   build/docs, archive acceptance and commit before T180.

## Evaluator work design

Contexts name exact checked creation frames. An expression use is keyed by its
source descriptor, context and current definition path. A definition use is keyed
by its source definition, context and returned guarded definition path. Repeated
requests reuse these records. Once an application's origin is known, retain its
T177 record and recheck its constraints instead of manufacturing another target
creation frame on every unchanged retry. Unknown-origin records may be replaced
after actual new origin evidence permits re-normalization.

Use an explicit operation stack with expression and definition frames. Expression
phases bind the head, request named result/alias value definitions, visit input
expressions in source order, prepare/apply through T173–T177, enter creation guards,
compute all output definitions for an explicit output phase, add receiver
equations, and complete. Definition phases enter T178 first, evaluate the original
RHS, then equate its derived value; an alias takes its original selected output,
never the producer expression Type. Named definition cycles fail through T178.
An already-active expression variable instead remains a logical pending term;
do not turn a plain equality cycle into a definition dependency.

Captured derived names retain their original environment. Registered contexts
are the authoritative provenance set; process actual matching contexts rather
than choosing a same-named module or inventing an implicit root. A missing
required environment remains pending until the closure scheduler supplies it.
Definition/value terms can be shared, but incoming guards cannot be skipped.

Cache a definition by complete owner Type and original symbol only after its
RHS static requirements are checked and closed and its value has a successful
selected T176 normal form. Check local prerequisites, not unrelated global root
closure. A constant selected result cannot hide an unresolved unused producer
input: the producer expression's full Type must also close. Returned Template
residuals are local and may remain open. Cache hits still enter both applicable
guards, and equate the new scoped derived variable with the cached canonical
value. This is static value reuse, not executable/alias publication.

A cached value and its readiness proof serve different purposes. The value is
valid equality evidence for the same complete owner/symbol. A proof from another
creation context certifies the new read only when every actual ancestor key is
closed: refreshed graph reachability can then check those keys against all cached
descendants. Otherwise evaluate the original RHS in the actual context while
retaining the valid value equation. Prefer an already checked inactive RHS for
this actual context, so unchanged retries do not replay successful local work.
This preserves cold/warm behavior without demanding that unrelated outer keys
close when the actual RHS has no repeated-template comparison. It neither guesses
an unknown key nor suppresses a dependency edge.

This distinction also prevents a valid ordinary self-producer alias from creating
endless fresh source contexts: after its literal output is checked, an equivalent
complete owner/symbol read reuses the guarded value rather than re-expanding the
same ordinary body through another identical context. Symbolic owners remain
uncached and preserve each actual incoming path. Progress is new structural or
checked-validation evidence for stable work identities, never mere snapshot count.

## Invariants, finite work and cost

1. A source key is `(original environment, original expression)`. Its APP value
   is never replaced by an output receiver. Original symbol identities and
   exact binding pointers determine every definition and receiver equation.
2. Each nested operand belongs to the enclosing expression's lexical context.
   A producer context exists only after successful T178 creation entry. Output
   lookup uses that context's original module and original output ordinals.
3. A frame advances monotonically through head dependencies, original inputs,
   actual application, all producer outputs and receiver equations. An explicit
   definition frame enters T178 before allocation/cache lookup, visits its one
   original RHS, then adds its result equation and attempts checked caching.
4. Receiver constraints are equality edges, not definition reads. Derived head
   reads are definition edges even if a value was previously anchored. Missing
   actual provenance remains Pending. Neither names nor expected signatures
   choose an origin.
5. Cache publication requires READY inactive RHS work, independently closed
   owner, full producer expression and selected value, and closed transitive RHS
   requirements. A complete Template value may still have local residual binders.
   Cache hits retain the incoming creation and definition guards. Foreign proof
   reuse additionally requires all actual ancestor keys closed; unknown ancestry
   requires actual-context RHS checking, not merely a cached value.
6. Temporary stack frames belong to one attempt and are destroyed on every
   outcome. Failed semantic owners are destructible only; callers cannot reuse
   partially active work. Public output parameters remain untouched on failure.

Let `W` be the number of visited expression/definition occurrences in one attempt,
`A` the total original actions visited, `U` the registered source/use/context
records, and `Q` the work performed by the independently tested T173–T178 kernels.
The orchestration traverses each frame's finite phases/actions once. Linear
membership/interning and context scans give O((W + A)(U + W) + Q) time; operation
scratch is O(W), and persistent storage is proportional to genuinely prepared
source/use/definition/application and cache records. Scratch allocations remain
owned until attempt completion, so this is not a claim of O(active depth) memory.
The solver and mixed creation/definition graph costs are additional kernel costs,
not hidden constant-time operations. Checking a foreign proof adds O(H) time
and O(1) scratch for H actual ancestors. An inconclusive certificate replays
ordinary finite RHS work in that context; its existing checked proof is retained
for subsequent reads. This is a sufficient exact optimization certificate, not a
semantic rule requiring every unrelated ancestor to close.

Original syntax is finite and nested inputs do not recurse through the host
stack. Definition cycles fail through the separate definition guard. T201 checks
complete repeated keys before preparing another scope: unequal keys fail, unknown
comparisons wait without expanding another copy, and equal keys close the nominal
graph. Ordinary zero-parameter owners have one complete nominal
identity, so repeated definition dependencies are checked on that identity;
independently completed values can be reused without endlessly manufacturing
ordinary contexts. Stable source uses retain known-origin application records.
An unchanged Pending retry does not add applications or creation paths. The
attempt returns Pending to the future closure scheduler instead of polling for
facts. The scheduler and final unresolved/admission checks belong to T180/T184.

The component's call boundary contains only inventory, binding, immutable term,
constraint and guard operations. There is no module execution/native/default
entry API in this component. Compile-time result computation therefore cannot
run a method or initialize runtime storage.

## Acceptance scope

Thirty-one named component cases cover exact source/alias identities, independent
applied environments, persistent guarded uses, closed-value caching, original
input/output skips, origin refinement, whole-expression recovery, nested output
composition and returned partial constructors. They also cover existing result
receivers, absent/runtime/over-arity fields, skipped invalid output definitions,
creation repeats, Pending origin identity, foreign handles and observed allocation
refusals. The source, context, cache, prepare and evaluate leaves each have explicit
`run()` dispatch and counted teardown. The full result parent passed with strict
C11 warnings as errors (`/tmp/ama-t179-results-strict.log`, collected exit 0).
The prerequisite guard repair separately passed all 27 guard cases under strict
and ASan/UBSan builds, including exhaustive finite graph oracles.

## Step record

T178 commit e48fc3e has a clean working tree before this task. Its final archived
Doxygen refresh completed with exit 0; the commit message records that final
verification.

- Added result-plan ownership, first-error propagation, exact expression-head
  binding and original result/receiver-alias definition binding. Head paths use
  their accepted occurrence origin; placeholder heads use their original scoped
  symbol. First alias definitions use `symbol->receiver` identity, so a later
  same-name equality receiver cannot replace the defining expression.
- Strict source leaf passed four cases (`/tmp/ama-t179-source.log`, collected
  exit 0): producer/selected-value distinction, independent generic/applied
  scopes, Type/Template captures, foreign inputs, checked counters and every
  observed path/formal/alias/applied preparation allocation. Counted teardown
  reaches zero; preparation adds no equation, owner parameter or creation path.
- After source preparation, planned iterative expression/definition evaluation.
  Reuse a stable application record after its origin is known; resubmitting an
  unchanged source must not manufacture new provenance contexts as progress.
  Reuse completed definition values only after the owner key, value and static
  RHS requirements are independently closed and checked. New incoming paths still
  enter T178 before cache use. Scoped symbolic definitions remain replayable.
- A Type application without an output phase contributes its required creation
  path, but does not read another module's static output values. Whole required
  specialization validation belongs to T184. An explicit output phase validates
  all outputs under its current static-definition dependency context, including
  skipped/unselected ones. Existing output receivers add equality, not a fictitious
  dependency on their own value definition.
- At the source-preparation checkpoint, registration and evaluator integration
  remained unfinished; subsequent steps below completed both. No specification
  change or semantic ambiguity was introduced.
- Persistent guarded contexts and expression/definition uses passed four cases
  (`/tmp/ama-t179-context.log`, collected exit 0): repeated identities, distinct
  scopes/definition ancestry, guard-before-existing-use lookup, foreign inputs,
  counters and every observed preparation allocation. Together with source
  preparation, eight T179 cases pass; evaluator equations are not implemented yet.
- Cache integration review found a T178 prerequisite defect: a new ancestor's
  original template is known before its full key, but the old graph omitted its
  nonconcrete incoming edge. A complete cached B->A->Leaf suffix could therefore
  bypass the known A->B->A source-repeat check. The exact source regression failed
  as expected (`/tmp/ama-t179-symbolic-cache-before.log`, collected exit 2).
- Repaired validation with a temporary view joining complete keys and exact
  unresolved scoped owners, preserving every partial-endpoint occurrence edge.
  It never publishes a symbolic concrete cache key or merges unresolved owners
  by source name. The first regression passes
  (`/tmp/ama-t179-symbolic-cache-after.log`, collected exit 0). Added the dual
  case with both symbolic ancestor and cached symbolic descendant; its witness
  retains the original closing path even without a concrete edge.
- Strict affected guard parent passed all 27 cases, including the mixed-view
  regressions (`/tmp/ama-t179-symbolic-guard-parent.log`, collected exit 0).
  Sanitized guards passed all 27 cases
  (`/tmp/ama-t179-symbolic-guard-sanitizers.log`, collected exit 0), and the
  strict source/context parent passed eight cases
  (`/tmp/ama-t179-source-context-parent.log`, collected exit 0).
- Definition cache helpers passed five strict cases, including the independently
  anchored constant output whose unused producer input remains open
  (`/tmp/ama-t179-unused-producer-cache.log`, successful leaf cleanup confirms
  completion). Cache publication checks the owner, full RHS producer and selected
  value separately; no known receiver value excuses unknown producer inputs.
- Added shallow source operand preparation, original skip/input/output retention,
  same-context child uses, actual-origin application handoff and guarded producer
  registration. Stable pending snapshots are replaced only after independent
  origin evidence. Whole-expression nominal evidence uses T177 exact recovery;
  selected alias values never substitute for the producer Type.
- Added an operation-local explicit continuation stack for expression/definition
  evaluation. Phases visit derived heads in registered original environments,
  nested lexical inputs, actual applications and every declared static output
  of explicit output phases. Receiver equalities do not introduce definition
  edges. Definition results and alias selected values remain separate.
- Strict preparation/recovery leaf passed five cases
  (`/tmp/ama-t179-prepare-recovery.log`, collected exit 0). Initial evaluator
  composition passed nine cases (`/tmp/ama-t179-evaluate.log`, collected exit 0),
  including returned partial constructors, scoped captures, skipped invalid
  outputs, constant results of incomplete producers, creation guards and every
  observed real-evaluation allocation refusal. Expanded evaluator acceptance is
  running at `/tmp/ama-t179-evaluate-expanded.log`; collect before functional edits.
  At that checkpoint, registration, full affected suites, sanitizers, proofs and
  documentation gates remained; later acceptance below closes those steps.

- Expanded evaluator leaf passed 11 cases
  (`/tmp/ama-t179-evaluate-expanded.log`, collected exit 0). Added public invalid
  receiver/context checks and unchanged-Pending retry identity, giving 12 evaluator
  cases and 30 total result cases. Full strict parent passed all 30
  (`/tmp/ama-t179-results-strict.log`, collected exit 0).
- All maintained result files and the repaired mixed guard view now have explicit
  named per-file test evidence and standalone header probes in `ready.json`.
  The global static dispatcher includes the new component. Current acceptance
  jobs then were the ASan/UBSan result parent and root build; both completed
  successfully as recorded below. No specification change was needed.

## Final acceptance

- All 30 initial result cases passed strict C11 warnings-as-errors and ASan/UBSan
  (`/tmp/ama-t179-results-strict.log`, `/tmp/ama-t179-results-sanitizers.log`,
  collected exits 0). A final independent finite-nesting regression verifies two
  applications of the same original template are sibling creation edges of the
  common lexical owner; the selected value is the inner constructed Type.
  The final evaluator leaf passes all 13 cases under strict and ASan/UBSan builds
  (`/tmp/ama-t179-evaluate-final-strict.log`,
  `/tmp/ama-t179-evaluate-final-sanitizers.log`, collected exits 0). Total: 31 cases.
- All observed allocation refusals unwind through counted fixtures with zero
  remaining allocations. ASan uses `detect_leaks=0` because of the established
  container LSan restriction; counted ownership checks remain independent evidence.
- Root `make` passed (`/tmp/ama-t179-build.log`, collected exit 0). This is component
  implementation, not yet whole-project template admission; T184–T196 retain
  integration, ordinary validation, diagnostics and executable conformance owners.
- Initial documentation acceptance passed with 2633 ready files, zero debts,
  standalone headers, 213 Markdown files/2922 links, 214 rules and 108 unchanged
  EBNF productions. Doxygen 1.11.0 validated 1989 maintained file pages,
  7693 interfaces, 425 canonical destinations and 6281 pages
  (`/tmp/ama-t179-docs-complete.log`, `/tmp/ama-t179-markdown.log`,
  `/tmp/ama-t179-doxygen.log`, collected exits 0).
- The final test delta and archive/status inputs receive a final documentation
  refresh before commit. No semantic source changes followed sanitized acceptance.
  T180 is the next ready task; its owner requires complete producers before
  publishing immutable static results and aliases.

## Live operand preparation after T214

[The operand contract](static-operands.md) owns private immutable preparation
shared by actual expression uses with the same original source/environment.
Collection and cache publication are separate operations. Each actual use still
retains its own lexical child uses, application state and guarded definition/
creation/output traversal; shared preparation grants no readiness proof.

The unused interface-preview application operation and its sole-purpose leaf
are removed. No public preview API or cached preview application remains.
Current source inventory and graph semantics are adapted by T206–T212; this
preparation cleanup does not establish whole-program static-module support.
