# Ama implementation work queue

The whole [spec/](spec/README.md) directory is authoritative. The
[requirement index](spec/requirements.md) locates all 214 current requirements;
[spec/grammar.md](spec/grammar.md) defines source forms. This file orders the
engineering work needed to implement those contracts. It neither changes the
language nor grants permission to edit the specification.

## How to use this queue

Start with the first unfinished task whose prerequisites are satisfied. Plan
small verifiable steps, record the actual results in the task and its owning
contract, and commit the completed task before beginning the next one. Follow
prerequisites and presentation order rather than numeric ID order.

[Completed tasks and acceptance evidence](DONE.md) belong in DONE.md. Keep
unfinished work here. Once all required checks pass, move the complete task
record to DONE.md in the same commit as its implementation, tests, documentation
and queue update. Do not discard decisions, failed-run facts or limitations.
Allocate IDs across both files; preserve old IDs and never reuse them.

Use the [development standard](docs/development/README.md) and its
[task record template](docs/development/task-template.md). A task needs an
explicit deliverable, prerequisites and acceptance conditions. The common
completion criteria below apply to each task. Historical completion is useful
only within its recorded input/configuration boundary; verify the evidence
required by a new consumer instead of assuming a checkbox establishes it.

If a genuine ambiguity or contradiction prevents an exact implementation, stop
work on the affected task, describe the issue and competing interpretations in
[AMBIGUITIES.md](AMBIGUITIES.md), and ask the user for a decision. Do not guess,
add a heuristic, silently pick a language rule, or edit `spec/` to fit the code.
An implementation task, including one in this queue, is not permission to amend
`spec/`. Follow [AGENTS.md](AGENTS.md) and
[CONTRIBUTORS.md](CONTRIBUTORS.md) for the remaining project workflow.

### Completion criteria for every task

1. Implement only the stated contract in small components under nested directories
   that mirror responsibilities. Parent orchestration calls narrow child operations;
   do not collect unrelated behavior in a long function or artificial shared context.
2. State the domain, ownership, pre/postconditions, invariants, failure effects,
   termination and complexity. Keep the general correctness argument with the
   component contract under ARCH-05/ARCH-06; use exact algorithms throughout.
3. Add or update Doxygen explanations of every changed code file's purpose and
   the meaningful types, interfaces and algorithms it contains. Explain why the
   component exists, what it owns and how its failure/lifetime boundary works.
   Link compiler explanations to canonical `spec/` files and stable rule anchors
   under ARCH-06; verify both source navigation and generated HTML destinations.
   Generate documentation and check cross-references for changed interfaces.
4. Map every added or changed code file, including headers and handwritten native
   companions, to meaningful tests in the component evidence registry. Header-only
   declarations need independent compile checks and tests of their consumers;
   generated code is tested through its generator and compiled outputs. Cover the
   actual behavior and failure boundary, not just execution of lines. Existing
   uncovered files become explicit audit tasks, never silent exceptions.
5. Run focused leaf tests and the affected real parent composition. Add boundary,
   invalid-input and failure-injection cases where the contract has those paths.
   Use the existing C/recursive-Make harness. Execution and ABI claims require
   compiling and running generated C; use relevant sanitizers and the supported
   toolchain checks. Documentation-only tasks use document checks instead.
6. Record the requirement IDs, changed components, test commands/results and any
   actual limitations. Mark the checkbox only after its acceptance checks pass.
   Commit the completed task with its evidence and a descriptive task-ID message;
   do not include unrelated user changes. Leave blocked or failing tasks unchecked.

## Sprint: static module templates

This sprint implements the approved [Stage 11](spec/implementation/plan.md#stage-11-static-templates-with-explicit-design-boundaries).
The [T167 planning record](DONE.md#t167) preserves the original sprint.
[T202](DONE.md#t202) defines the new static-module source model;
[T205](DONE.md#t205) resolves its subsequent lifecycle clarification.
[T204](DONE.md#t204) records the consistency review and replaces the remaining
implementation plan.
The user authorized sequential implementation on 2026-11-18, including focused
tests, code documentation, exact algorithms and a separate commit per completed
task. Specification changes still require their own explicit authorization.

The deliverable is compile-time specialization with exact inference, composition,
partial constructors, separate named `@...` graphs, independently concrete
export selectors and concrete native/source/C-library consumers. Keep the existing
intrinsic list foundation. Relative paths remain explicitly deferred. The
[AMB-006](AMBIGUITIES.md#amb-006) rewrite, new raw-storage API, generic native ABI,
binary-only open-template format, static natural-number parameters and dependent
types are outside this sprint. No runtime template objects or specialization
generation are introduced.

### Dependency order and checkpoints

| Phase | Tasks | Assessable result |
|---|---|---|
| Actual baseline | T203 | Evidence-based reuse/adapt/remove map, including unfinished T182 ownership |
| Cleanup, source and binding | T214, T206, T207, T213 | Remove unused preview API; establish exclusive module kinds, immutable AST, scopes and exact source closure |
| Finite structural foundation | T208, T209, T210, T183 | Actual metadata, open requirements, nonexpanding equations, guards and canonical candidate registry |
| Static graphs and ordinary bridge | T211, T212 | Named producer equations, complete inputs and independent concrete export selection |
| Ordinary closure and admission | T182, T184, T185, T186 | Runtime typed connections, checked concrete groups and constant lifecycle plans |
| Execution | T187–T190 | Resolved storage/control plans, compiled execution, diagnostics and concrete native composition |
| Product consumers | T191–T195 | Source delivery, editor/relocation, C facades and useful list algorithms |
| Acceptance and delivery | T196–T198 | Current evidence, clean release and generated reference |

The presentation is a topological order, not a promise that all tasks are one
linear algorithm. Explicit prerequisites below govern readiness. Components may
exchange symbolic terms and deferred obligations; completing an early component
does not claim whole-program closure. Every task is accepted and committed on
its own. Do not batch completed tasks into a phase commit.

This is a specification-led plan, not a source audit. T168 and T169–T181/T201
remain completed historical prerequisites for their recorded contracts. Their
implementation is neither discarded nor accepted automatically for the changed
model. T203 must inspect the actual code, tests, contracts and dirty T182 inputs,
identify reusable parts and assign every replacement to its current task.
Exact source/test paths and narrower implementation steps follow that evidence;
this plan does not invent existing support or mandatory empty directory levels.

T183 deliberately precedes static graph evaluation. It accepts normalized closed
candidate keys and pending validation state without requiring ordinary body
checking. T211 consumes that guarded registry; T212 provides independent Type
identities and constant evidence; T182 then generates ordinary typed connections.
T184/T185 finish closure and ordinary admission. No dependency from the early
registry back to T182 is allowed, and no earlier component promises executable
admission. This avoids both a task cycle and premature validation of mutually
referencing nominal groups.

Four ordering constraints are essential:

- Distinguish ordinary and static module kinds before collecting interfaces.
  Ordinary input/result Type lines introduce Type formals; static graphs collect
  every public binder before free inputs and derive requirements from all uses.
  Reject generic native definitions before preparing native authoring headers.
- Classify runtime receivers and foreach locals before solving their types.
  Later evidence refines terms and cannot change bindings.
- Equality, normalization and pattern components initially operate on finite
  terms without unfolding definitions. Guard active static-definition and
  creation ancestry before any consumer expands a definition, even before a
  concrete specialization key exists and on every relevant cache path.
- Admit executable specializations only after closure and ordinary checks.
  An interned candidate key is not executable admission. Keep term equality,
  source loading, static-output dependencies, creation paths, eager ownership and
  constant-value dependencies as distinct relations.

### Record every implementation step

Use the common completion criteria and the
[task template](docs/development/task-template.md) for every task below.
Before its first edit, record the actual input revision, affected files and
consumers, success/failure partitions, exact contract, invariant/progress/cost
argument, test commands and the ordered steps. The task-specific steps below
are the initial plan; refine engineering details after inspecting that boundary.

After each meaningful step, record its purpose, actual findings/changes,
decisions and reasons, commands/configuration, observed result and limitations,
and next action. Keep these in TODO or its linked owning component contract;
link that record here instead of creating a second backlog. Each task includes
implementation, tests, Doxygen purpose/contract comments with clickable canonical
rule links, and per-file evidence for production, headers, fixtures and support.
Do not postpone those obligations to T198.

During long work, retain the current task/step, active jobs/session handles and
logs, blockers, next action and pending checks. Preserve failed runs and their
resolution. Before continuing, collect active results and check input freshness.
On acceptance, move the full record to DONE and commit that task's complete
change. A newly discovered semantic ambiguity follows the stop/record/ask
procedure above; this queue grants no permission to change spec.

Specification clarifications [T200](DONE.md#t200), [T202](DONE.md#t202) and
[T205](DONE.md#t205) are accepted. The complete bounded review is recorded in
[the consistency/planning note](docs/implementation/static-plan-review.md).
[T203](DONE.md#t203) records the actual source/test ownership audit;
[T214](DONE.md#t214) removes the unused preview service and retains only live
operand preparation. [T206](DONE.md#t206) establishes the current source/AST
boundary; T207 is next. T182 remains unfinished with its partial
files and failed-run evidence. Follow the recorded ownership/disposition map.
Historical
[constant analysis](docs/implementation/static-constants.md) is revalidated in
T212, not treated as current acceptance of removed inline projections.

## Current tasks

### Phase 1. Actual implementation baseline

[T203](DONE.md#t203) completed the source/test audit. Its
[exact dispositions](docs/implementation/audit/static-module-alignment.md) govern
reuse, removal and replacement below; they do not establish current-spec support.

### Phase 2. Establish current source structure and bindings

<a id="t207"></a>

- [ ] **T207 — Collect ordinary Type formals and whole-graph static scopes.**
  Prerequisites: T206.
  Rules: [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-12](spec/language/static-results.md#tpl-12),
  [TPL-13](spec/language/templates.md#tpl-13),
  [NAME-06](spec/language/declarations.md#name-06).
  Boundary: immutable source-owned parameter/result identities and label roles;
  runtime receiver/foreach classification remains T182's responsibility.
  Steps: collect ordinary input/result Type introductions; collect all static
  pair and receiver binders before free inputs; classify unique definitions,
  first defining receipts and later equality receipts independently of solver
  order. Reject own free native Type formals before authoring-header generation.
  Accept: forward result uses are not accidental inputs; every graph result is
  public; pair definitions take their declared role regardless of textual order;
  repeated receipts constrain one immutable name. Foreign labels and equal names
  in independent modules never introduce or capture local formals. Reject duplicate
  pairs, new body/shared parameters and standalone ordinary static declarations.
  Include result-only/unused native parameters and concrete export selectors that
  introduce no parameter. Test the inventory and real parser/native consumers,
  scope/source spans and all refusals; replace obsolete signature/position tables.
  Meet the common per-file tests, Doxygen links and same-task commit criteria.
  Fixtures: Migrate the retained scope/inventory fixtures to whole-graph public names; the
  earlier source-only formal/pattern cases were removed in T206.

<a id="t213"></a>

- [ ] **T213 — Resolve static graph dependencies and source module kinds.**
  Prerequisites: T206.
  Rules: [SRC-01](spec/language/source-and-names.md#src-01),
  [SRC-02](spec/language/source-and-names.md#src-02),
  [DEP-01](spec/implementation/compiler.md#dep-01),
  [DEP-02](spec/implementation/compiler.md#dep-02),
  [DEP-03](spec/implementation/compiler.md#dep-03).
  Boundary: the common source registry, logical origin resolution, required
  source closure and input invalidation; T206 only extracts syntactic spans.
  Steps: traverse every new AST path position; load exact roots and resolve
  ordinary constructor versus static producer roles; retain all source/native
  dependencies and source-located missing/invalid-kind diagnostics.
  Accept: pair RHS paths, nested arguments, producer heads, ordinary selector
  producer paths and constant namespaces enter the actual closure. Static names,
  selectors and foreign labels are not filesystem paths. Preserve both root
  kinds, exact duplicate/alias rejection, name+/ mapping, draft rules and native
  companions. Source-loading cycles do not recursively reload or imply successful
  validation. Cover shared dependencies, reordered roots, missing/invalid sources,
  stale input invalidation and allocation cleanup through real parser/registry
  compositions. No implicit import, new package format or relative path is added.
  Meet the common per-file tests, Doxygen links and same-task commit criteria.

### Phase 3. Build finite structural analysis and guarded identities

<a id="t208"></a>

- [ ] **T208 — Model actual static metadata and minimum usage requirements.**
  Prerequisites: T207, T213.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08).
  Boundary: immutable Type/Template/producer descriptions, formal identities,
  fixed captures, full actual residual signatures and symbolic use requirements.
  Steps: retain complete actual metadata; collect compatible requirements from
  all applications/forwarding uses; check supplied actuals without closing the
  requirement set to the first signature encountered. This component expands
  neither producer definitions nor recursive runtime bodies.
  Accept: additional actual residuals survive; a forwarded-only input retains
  its supplied category, origin and captures. All uses constrain an input and
  each partial result has its own residual map. Reject wrong categories, missing
  named members and incompatible requirements; no first-actual contract, capability
  search or implicit Type reopening. Distinguish static producers from ordinary
  constructors. Test exact identity/category laws, symbolic-to-actual refinement,
  independent scopes and allocation/size boundaries with real inventory/resolver
  consumers. Meet the common tests, Doxygen links and same-task commit criteria.
  Fixtures: Rebuild term/metadata fixtures from current source and actual metadata; do not
  restore explicit expected-signature trees or the removed root-subset mode.

<a id="t209"></a>

- [ ] **T209 — Apply named constructors and solve nonexpanding equations.**
  Prerequisites: T208.
  Rules: [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-10](spec/language/templates.md#tpl-10).
  Boundary: structural terms, named input application, partial normalization,
  equality/congruence/occurs checks and exact nominal constructor recovery.
  Steps: normalize known applications; propagate minimum requirements; solve
  finite structural equalities and uniquely justified B-minus-P recovery while
  retaining unanchored obligations. Do not unfold definitions at this stage.
  Accept: direct/staged/reordered inputs agree and preserve unused captures;
  independent partial applications do not mutate or combine the original family.
  Reject duplicate/unknown/fixed labels, category conflicts, unequal nominal
  origins and infinite terms. Unknown-to-unknown equalities, literals and
  noninjective outputs invent no origin or missing binding. Preserve extra
  residuals and require actual named compatibility rather than exact expected
  signature equality. Derive finite solver progress/cost and recovery uniqueness;
  exercise metadata/constraint consumers, permutations and allocation refusal.
  Remove obsolete positional/exact-signature branches. Meet the common per-file
  tests, Doxygen links and same-task commit criteria.
  Fixtures: Migrate the remaining binding/application fixtures from old mixed interfaces;
  delete static positional/skip-only cases while retaining runtime positions.

<a id="t210"></a>

- [ ] **T210 — Guard definition expansion and every specialization path.**
  Prerequisites: T207, T209.
  Rules: [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-11](spec/language/templates.md#tpl-11),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: separate exact static-definition and specialization-creation graphs,
  original application provenance and pending repeated-key obligations.
  Steps: revalidate the existing guards against the new graph roles; attribute
  applications to lexical owners; expose checks that run before any expansion
  and on every cache path. T183 composes these algorithms with the registry.
  Accept: finite nested applications and identical closed-key cycles pass;
  reachable unequal keys of one origin fail even through a closed cycle or late
  edge. Unknown comparison keys suspend expansion without guessed equality or
  speculative copies. Supplied Types/fixed namespaces add no invented creation
  edge, while actual producer uses do. Pair/first-receipt definitions differ
  from equality checks; direct/mutual static-definition cycles remain errors.
  Check exact finite graph oracles, root/constraint order, cached dependencies,
  pending-to-closed transitions, diagnostics and every refusal cleanup. Preserve
  occurs/eager/value graphs separately. Meet the common per-file tests, Doxygen
  links and same-task commit criteria.
  Fixtures: Migrate the live guard fixtures to current source/metadata without weakening
  creation ancestry or cache-path checks.

<a id="t183"></a>

- [ ] **T183 — Register exact specialization identities and guarded cache reuse.**
  Prerequisites: T208, T209, T210, T213.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-11](spec/language/templates.md#tpl-11),
  [NAME-01](spec/implementation/compiler.md#name-01),
  [NAME-02](spec/implementation/compiler.md#name-02).
  Boundary: the early compile-time candidate registry, full-key encoding and cache
  dependency/provenance records, separate from instance allocation.
  Steps: (1) key complete normalized bindings by original identity/formal order;
  (2) intern structurally valid closed candidate keys with retained dependencies
  and explicit pending-check state; (3) check each incoming ancestry and cached
  descendants under alternate demand paths.
  Accept: direct/partial/alias-equivalent bindings share identity; different
  roots/origins/unused bindings/captures/residual signatures do not merge.
  Interning is not executable admission; T185 supplies ordinary validation.
  This task does not depend on T182 or resolved ordinary body members: compose
  actual metadata/term/guard components over finite candidate graphs first.
  Keep candidate, pending, checked and failed states explicit; T211 may retain
  validation obligations while naming an already closed nominal identity.
  Cold/warm caches and reordered roots agree, including unequal-key paths
  hidden in cached grandchildren or outgoing branches of same-key cycles;
  exact-key closure retains every outgoing check. Unrelated paths do not form false
  source-only cycles. Inject key collisions and allocation failure explicitly.

### Phase 4. Resolve static graphs and independent ordinary exports

<a id="t211"></a>

- [ ] **T211 — Resolve named graph results and complete producer inputs.**
  Prerequisites: T207, T209, T210, T183, T213.
  Rules: [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: result-pair values, explicit named producer outputs, scoped
  application equations and producer closure through the guarded registry.
  Steps: derive structural definitions and receiver equations; retain deterministic
  omitted-input variables; solve/refine guarded demands and validate all graph
  definitions. Apply guards before following each dependency, including unknown
  keys and cache hits, rather than expanding first and checking afterwards.
  Accept: multiple outputs, public intermediate names, forwarding and partial
  returned families preserve exact metadata. Existing receivers add equality;
  fresh names supply no anchor. Every producer input must close, including one
  unused by selected results; every public output retains validation obligations.
  Reject unknown/repeated output labels, implicit first results, producer-as-value
  uses, static-definition cycles and unresolved/noninvertible input requirements.
  Register closed identities with outstanding ordinary checks, never claim final
  executable admission or require T185 prematurely. Test real parser/scope/solver/
  guard/registry compositions, definition versus equality cycles, warm/cold order
  and all preparation/publication failures. Meet the common per-file tests,
  Doxygen links and same-task commit criteria.
  Fixtures: Migrate result/publication fixtures to named graph producers and public binders;
  discard old ordinary-body static result scenarios.

<a id="t212"></a>

- [ ] **T212 — Select independent concrete exports and align constant analysis.**
  Prerequisites: T211, T213.
  Rules: [TYPE-01](spec/language/declarations.md#type-01),
  [CONST-03](spec/language/declarations.md#const-03),
  [CONST-05](spec/language/declarations.md#const-05),
  [CONST-06](spec/language/declarations.md#const-06),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: ordinary declaration/constant-namespace export selection and
  specialization-dependent constant declared-Type evidence. Runtime constant
  initialization remains T186; ordinary call/assignment/foreach inference is T182.
  Steps: resolve one exact selected export; require producer closure independently
  of the consuming context; align shared declarations and both constant-source
  forms with current scopes, known Types and graph dependencies.
  Accept: one named selector precedes ordinary initializers and introduces no
  local placeholder. Reject missing/multiple/unknown selectors, residual Template
  storage and attempts to fill producer inputs from runtime receivers/calls.
  Preserve field inference in permitted ordinary contexts, result-introduced Type
  parameters and parameter-dependent shared lists. Explicit constant namespaces
  use independent concrete Types; ordinary named/positional `>>` is unchanged.
  Direct qualified constants keep their final-member split; member names and
  literal bytes do not choose Types. Known nominal identities may participate
  while whole-group validation remains pending, with no early successful admission.
  Revalidate retained T181 metadata through actual declaration/graph/constant
  consumers, cycles/provenance, unavailable members and resource failures.
  Meet the common per-file tests, Doxygen links and same-task commit criteria.

  Fixtures: Migrate retained constant fixtures to current graph exports and simple ordinary
  namespaces; T181 history alone is not current source acceptance.

### Phase 5. Close and admit ordinary specializations

<a id="t182"></a>

- [ ] **T182 — Generate ordinary typed-connection and nested-field constraints.**
  Status: retained unfinished work; use T203's completed audit and wait for the
  new foundations above.
  AMB-013 is resolved by excluding computed ordinary interfaces; do not implement
  the obsolete preview exception or treat old mixed-interface probes as support.
  Partial foundations pass 24 lexical/typed-scope and five congruence sanitizer
  cases; full step/failure/checkpoint evidence is in the owning note. O2 root
  build has a recorded return-contract warning; T182 is not accepted or committed.
  No jobs active; follow T203's file ownership and the explicitly authorized
  T214 unused-preview removal before resuming the remaining work.
  Its local continuation note remains with the unfinished working-tree files,
  outside T202; the specification task establishes no implementation acceptance.
  Prerequisites: T207, T209, T212.
  Rules: [TYPE-06](spec/language/declarations.md#type-06),
  [NAME-06](spec/language/declarations.md#name-06),
  [TPL-03](spec/language/templates.md#tpl-03),
  [TPL-04](spec/language/templates.md#tpl-04).
  Boundary: runtime declaration/source-order inference, foreach locals and
  semantic input/result/default/assignment/reference connections.
  Steps: (1) resolve the recorded O2 return-contract warning under its actual
  contract, without inventing a fallback; classify lexical receivers and locals
  before solving, keeping static
  names separate; (2) generate equations from typed connections and deferred
  constant evidence; (3) compose two/three-level owner/field applications.
  Accept: later anchors refine existing terms without rebinding; independent
  fields can differ while one field/concrete owner agrees across all uses.
  Check empty typed references/lists, self-reference and unused typed defaults,
  local lifetime, later explicit/inferred names and suffix mismatches.
  Literals, runtime contents, first execution and unknown-to-unknown copying
  provide no invented anchor; unresolved/inconsistent required types fail.
  Use current ordinary name/type pairs and ready export evidence. Test real
  new-parser/scope/constant/solver compositions, allocation failures and later
  concrete anchors without contextual static-producer closure or old interface
  previews. Reuse partial code only through T203's reviewed file ownership.
  Fixtures: Migrate the retained connection fixtures to ordinary Type pairs and current
  export selection; obsolete mixed-interface probes are not acceptance.

<a id="t184"></a>

- [ ] **T184 — Orchestrate the complete guarded specialization closure.**
  Prerequisites: T182, T183, T211, T212.
  Rules: [TPL-06](spec/language/templates.md#tpl-06),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-11](spec/language/templates.md#tpl-11),
  [COMP-02](spec/implementation/compiler.md#comp-02).
  Boundary: reusable frontend semantic pipeline and required-specialization
  worklist, with explicit child operations and publication/cleanup ownership.
  Steps: (1) compose source loading, lexical classification, constraints,
  separate graph equations, independent export references and guarded demands;
  (2) reach exact closure before exposing closed type candidates for ordinary
  validation; (3) test real check-only and multi-root frontend stage handoffs.
  Accept: all required runtime types and producer inputs close; residual returned
  constructors remain compile-time values only. No expansion bypasses the T201
  guard contracts revalidated in T210, including unresolved repeated-key
  obligations, and no cache bypasses path validation. Reordered independent
  constraints/roots preserve acceptance; malformed definitions and skipped
  branches remain checked.
  Check every allocation/publication failure and safe frontend teardown; this
  closure result still requires the ordinary checks in T185 before lowering.

<a id="t185"></a>

- [ ] **T185 — Apply ordinary operation, ownership and lifecycle checks.**
  Prerequisites: T184.
  Rules: [TPL-06](spec/language/templates.md#tpl-06),
  [TYPE-05](spec/language/declarations.md#type-05),
  [COPY-06](spec/language/values-and-references.md#copy-06),
  [PROTO-07](spec/implementation/native-interface.md#proto-07).
  Boundary: resolved semantic capability/access/storage checks and separate
  eager-construction/copy relations over concrete specializations.
  Steps: (1) route substituted fields, defaults and operations through ordinary
  checkers; (2) compute ownership/clone/default requirements on concrete types;
  (3) admit successful candidates and compose generic calls, refs, lists and
  native protocol declarations. Failed candidates cannot reach lowering.
  Accept: literal/default/copy/condition/index/lifecycle requirements pass only
  when supported; supplied arguments and unreachable branches do not hide bad
  defaults/operations. Distinguish valid nominal reference/list cycles from
  eager cycles and owned noncopyable paths; check storage exactness, no implicit
  conversion/private-field access and no expansion of the deferred list scope.

<a id="t186"></a>

- [ ] **T186 — Plan and execute specialization-scoped constant lifecycles.**
  Prerequisites: T183, T185, T212.
  Rules: [CONST-02](spec/language/declarations.md#const-02),
  [CONST-06](spec/language/declarations.md#const-06),
  [LIFE-03](spec/implementation/native-interface.md#life-03).
  Boundary: concrete constant dependency graphs, startup plans and context-local
  constant storage/cleanup; no mutable state in the specialization cache.
  Steps: (1) key vertices by full specialization/member ordinal and order ready
  vertices exactly; (2) integrate initialization and successful-prefix unwind;
  (3) compose real runtime constant/list/native lifecycle consumers.
  Accept: equivalent namespaces share one table per context; different bindings
  and contexts remain independent. Reject value cycles while accepting harmless
  self-typed constants; no body execution supplies values. Failure before root
  startup cleans exactly the successful creations in reverse order, including
  partial native construction; copies preserve immutable shared originals.
  Generated whole-project startup evidence is completed with T188.

### Phase 6. Lower and execute concrete programs

<a id="t187"></a>

- [ ] **T187 — Build concrete resolved storage, lifecycle and control plans.**
  Prerequisites: T185, T186.
  Rules: [COMP-02](spec/implementation/compiler.md#comp-02),
  [GEN-03](spec/implementation/compiler.md#gen-03),
  [GEN-04](spec/implementation/compiler.md#gen-04),
  [RT-02](spec/implementation/runtime.md#rt-02).
  Boundary: semantic-to-planning handoff, concrete layout descriptors and
  explicit typed transfer/control/lifecycle plans.
  Steps: (1) map validated specialization identities and members to resolved
  plans; (2) exclude complete static graphs and their metadata from runtime
  storage/control while preserving ordinary runtime interface ordinals;
  (3) test actual semantic/planning/codegen-model handoffs.
  Accept: every runtime type/layout is closed; plans retain source provenance,
  full nominal distinctions, correct refs/lists/constants and stable pool roles.
  No emitter-side inference, implicit default instances or state save/restore.
  Check arithmetic/layout/allocation failures, unused static results and
  separate static/runtime graphs; retain static metadata only where needed
  for compilation/debug descriptions, not as runtime template objects.

<a id="t188"></a>

- [ ] **T188 — Emit and run specialized stackless programs.**
  Prerequisites: T187.
  Rules: [GEN-01](spec/implementation/compiler.md#gen-01),
  [GEN-02](spec/implementation/compiler.md#gen-02),
  [STACK-04](spec/implementation/compiler.md#stack-04),
  [TPL-05](spec/language/templates.md#tpl-05).
  Boundary: project assembly, per-specialization code/layout emission and
  existing runtime/native-entry dispatch.
  Steps: (1) emit validated specialization plans through narrow codegen children;
  (2) compile/run nested/partial/static-result projects; (3) exercise runtime
  identity, state, startup and continuation consumers under relevant sanitizers.
  Accept: one type/code identity for equivalent specializations with independent
  instances, results, busy state, owned state, references and continuations.
  Constants obey per-context sharing and cleanup; ordinary and explicit `%`
  copies preserve their distinct rules. Verify labels/one assembly, native entry
  inlining and no Ama C-call recursion; test malformed closure rejection and
  allocation/native lifecycle failures through compiled execution.

<a id="t189"></a>

- [ ] **T189 — Accept end-to-end specialization diagnostics and source maps.**
  Prerequisites: T184, T185, T188.
  Rules: [COMP-04](spec/implementation/compiler.md#comp-04),
  [NAME-03](spec/implementation/compiler.md#name-03),
  [DEBUG-01](spec/implementation/diagnostics.md#debug-01).
  Boundary: structured semantic/expansion diagnostics, rendering, generated C
  mappings and execution traces; each earlier task already reports local errors.
  Steps: (1) reconcile original definition/application/related-site chains;
  (2) compose check-only, generated-C errors and runtime traces;
  (3) verify stable outcomes across cache/order changes and low-memory failures.
  Accept: malformed names, kind/signature conflicts, occurs failures, unknown
  evidence, incomplete producers and each distinct cycle identify responsible
  original sites. Include pair definitions versus equality receipts, preserved
  extra residuals and independent-bridge closure errors. Unicode/multiword/hyphen
  paths round-trip; virtual identities
  do not pretend to be source files, and diagnostics remain valid through cleanup.
  A failed compilation must not leave partial output masquerading as a usable
  build or overwrite handwritten native companions. Static evaluation neither
  executes runtime code nor produces fictitious runtime stack entries.

<a id="t190"></a>

- [ ] **T190 — Compile concrete native/template compositions.**
  Prerequisites: T207, T212, T185, T187, T188.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [CABI-01](spec/implementation/native-interface.md#cabi-01),
  [CABI-05](spec/implementation/native-interface.md#cabi-05),
  [CABI-06](spec/implementation/native-interface.md#cabi-06).
  Boundary: native semantic views, scaffold/header models, layout/toolchain
  verification and handwritten concrete native companions.
  Steps: (1) instantiate Ama templates with concrete native types; (2) integrate
  concrete static-export field/dependency Types inside ordinary `@C` modules;
  (3) independently compile generated headers and run actual native compositions.
  Accept: required native default/copy/literal/condition/index capabilities and
  lifecycle/layout contracts are checked after substitution; unsupported ones
  fail. Bound aliases/static outputs do not add owner formals or ABI slots.
  Declared Ama refs remain visible and checked, never opaque native payloads.
  Cover concrete owned/ref/list fields and rejection of open dependencies;
  introduce no generic native hooks or specialization-specific authoring API.
  Revalidate native constructor/destructor behavior with the current selected
  Types; own-child embedding cleanup receives full host acceptance in T194.

### Phase 7. Connect product consumers and algorithm examples

<a id="t191"></a>

- [ ] **T191 — Deliver open templates as source and invalidate consumers exactly.**
  Prerequisites: T213, T183, T188, T190.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [DEP-03](spec/implementation/compiler.md#dep-03),
  [COMP-05](spec/implementation/compiler.md#comp-05).
  Boundary: existing configured roots, project/native source closure, incremental
  input fingerprints and actual independent source-distribution consumers.
  Steps: (1) load an open template library and required companions in a separate
  consumer project with separate static-assembly and ordinary-template sources;
  (2) build/run that consumer's own specializations;
  (3) test repeat/reordered imports, changed inputs and missing dependencies.
  Accept: full root/origin/formal/capture identity survives reuse; compatible
  imports share type/code without instance state. Reject name conflicts, absent
  Ama/native/external C dependencies and unresolved bindings exactly.
  Changed graph definitions/outputs, Type uses and native inputs invalidate
  affected results; no-op builds remain current. Use existing source delivery,
  not a new package manager, file format, virtual filename or binary-only
  template interface.

<a id="t192"></a>

- [ ] **T192 — Integrate static bindings with editor analysis and rename.**
  Prerequisites: T206, T207, T208, T211, T212, T184, T189.
  Rules: [COMP-01](spec/implementation/compiler.md#comp-01),
  [NAME-06](spec/language/declarations.md#name-06),
  [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: existing editor snapshots/index/query/rename consumers of the common
  frontend; preserve their documented transaction and failure contracts.
  Steps: (1) index graph pairs, defining/equality receipts, free inputs, foreign
  labels and ordinary export selectors;
  (2) route navigation/diagnostics/rename through resolved source identities;
  (3) exercise actual workspace queries and checked rename transactions.
  Accept: same-spelled foreign parameters and actual signature labels do not
  collapse; navigation/rename respect whole-graph visibility, ordinary source-
  ordered receiver scope, full atomic names and retained captures. Preserve the
  existing checked transaction contract when labels have multiple actual origins;
  do not invent a rename interpretation for an unspecified case. No generated
  specialization source file or duplicated parser.
  Cover incomplete/error snapshots, Unicode/hyphens, stale versions and capture
  rejection; failed transactions preserve workspace bytes.

<a id="t193"></a>

- [ ] **T193 — Preserve template meaning during source directory relocation.**
  Prerequisites: T213, T184, T189, T191, T192.
  Rules: [TOOL-01](spec/implementation/tools-and-conformance.md#tool-01),
  [TOOL-03](spec/implementation/tools-and-conformance.md#tool-03),
  [TOOL-04](spec/implementation/tools-and-conformance.md#tool-04),
  [TPL-01](spec/language/templates.md#tpl-01).
  Boundary: existing resolved-reference collection, relocation plans, publication
  and recovery; no change to the directory-move product contract.
  Steps: (1) classify concrete module-path spans inside static syntax separately
  from atomic placeholder/formal/result/alias names; (2) extend checked identity
  mapping/preflight; (3) relocate, rebuild/run and exercise refusal/recovery cases.
  Accept: rewrite only real resolved paths, preserve other bytes and static
  binding relationships, and recompute specialization origins from new sources.
  Include moving only a static producer versus its returned constructor: the
  producer identity changes while an unchanged returned ordinary Type preserves
  its original identity. Never rewrite export selectors or descriptive static
  names as paths. Include nested applications/namespaces, both root kinds,
  hyphens/name+/, original-name conflicts/new-root files, interruption and
  rollback/replay.
  No text replacement or treating explanatory placeholder components as files.

<a id="t194"></a>

- [ ] **T194 — Export concrete template facades through the checked C boundary.**
  Prerequisites: T188, T190, T191, T205.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [EMBED-01](spec/implementation/c-library.md#embed-01),
  [EMBED-04](spec/implementation/c-library.md#embed-04),
  [EMBED-08](spec/implementation/c-library.md#embed-08).
  Boundary: existing export configuration, library closure/header/transport,
  artifacts and independent C-host/native-wrapper consumers.
  Steps: (1) export an ordinary concrete Ama facade with closed internal template
  workers and retain already concrete native roots; (2) build static and
  target-supported shared artifacts with their manifests, compile the public
  header first in two independent C units and run a larger C host;
  (3) compose a concrete native wrapper with nested child completion/yield.
  Accept: full specialization/native closure, supported typed transport,
  independent contexts/host objects, ownership and cleanup all hold. Include
  two concrete facades with different substitutions and inspect their ABI identities.
  Reject open export roots, static-module export roots including closed graphs,
  virtual specialization selectors/static argument maps, missing closure and
  unsupported transport. Static results add no runtime C slots; no shared
  mutable Ama refs or yield across an unfinished native call.
  Exercise T205's `A -> owned W -> child B` with independent peer C: normal and
  outer-failure cleanup dispose yielded/failed B exactly once, preserve C and
  respect detached native activity/code lifetimes without Ama execution/reentry.

<a id="t195"></a>

- [ ] **T195 — Exercise useful generic algorithms over existing intrinsic lists.**
  Prerequisites: T185, T186, T188, T190, T191, T211, T212.
  Rules: [TPL-09](spec/language/templates.md#tpl-09),
  [TPL-04](spec/language/templates.md#tpl-04),
  [FLOW-05](spec/language/lists.md#flow-05),
  [LIST-10](spec/language/lists.md#list-10).
  Boundary: maintained executable Ama examples and integration cases, using
  existing list methods and ordinary modules for state/methods.
  Steps: (1) document algorithm inputs, preconditions, invariants and expected
  effects independently of outputs; (2) implement examples of typed list
  processing and nested composition, with a separate static assembly returning
  a partial constructor;
  (3) compile/run several concrete types and negative applications.
  Accept: empty explicitly typed lists need no sample; owned/ref forms, live
  traversal insertion/deletion, busy/refusal and copy behavior remain unchanged.
  Include equal specialization instances with independent state, multiple named
  static outputs and wrong-operation/category cases. Ordinary algorithm bodies
  contain no complex static applications or runtime Template values. Examples
  teach useful composition; no base-list provider, new allocator/storage API or
  privileged generic code.

### Phase 8. Review evidence and deliver

<a id="t196"></a>

- [ ] **T196 — Review current requirement and acceptance evidence.**
  Prerequisites: T189, T190, T191, T192, T193, T194, T195.
  Rules: [TEST-07](spec/implementation/tools-and-conformance.md#test-07),
  [TEST-08](spec/implementation/tools-and-conformance.md#test-08),
  [ARCH-06](spec/implementation/compiler.md#arch-06).
  Boundary: component arguments, per-file registry and reviewed conformance
  matrix; use the actual current canonical registry rather than frozen counts.
  Steps: (1) reconcile all current rules/acceptance rows with named component and
  distinct real-consumer cases; (2) run the recorded affected behavior and
  evidence-gate checks; (3) refresh reviewed mappings/fingerprints only after
  inspecting the corresponding sources, arguments and results.
  Accept: enumerate every current canonical requirement and acceptance row
  from the specification (baseline: 214 rules, 150 rows, 37 template areas),
  including the new model and lifecycle clarification. All have accurate
  evidence or an explicit applicable deferred boundary. The list rewrite is
  excluded, not falsely implemented. No stale baseline hash refresh substitutes
  for tests; unresolved mandatory coverage or documentation debt blocks completion.
  Record actual counts/revision/commands and pass make conformance-check plus
  its focused unit suite before claiming current conformance.

<a id="t197"></a>

- [ ] **T197 — Accept a clean sprint release and its resource/toolchain boundary.**
  Prerequisites: T196.
  Rules: [TEST-05](spec/implementation/tools-and-conformance.md#test-05),
  [STACK-04](spec/implementation/compiler.md#stack-04),
  [COMP-05](spec/implementation/compiler.md#comp-05),
  [ARCH-06](spec/implementation/compiler.md#arch-06).
  Boundary: complete compiler/test/toolchain and release consumers under the
  existing [release contract](docs/implementation/conformance/release.md).
  Steps: (1) state clean-build configurations and resource/failure partitions;
  (2) run isolated clean builds/full tests, relevant sanitizer/toolchain probes
  and real examples/library/tool workflows; (3) review reproducibility and
  specialization growth/state-storage evidence with exact bounds and versions.
  Accept: make and make test pass from the recorded clean state; finite nesting,
  growing rejection, cache/order invariance, native inlining and instance/context
  isolation have actual results. Check invalidation, allocation cleanup and
  publication failures. Do not overlap jobs owning the same outputs; record tool
  failures honestly. No arbitrary semantic specialization/depth budget is added.

<a id="t198"></a>

- [ ] **T198 — Close the sprint with generated technical documentation.**
  Prerequisites: T197.
  Rules: [ARCH-06](spec/implementation/compiler.md#arch-06);
  [documentation delivery](docs/documentation.md).
  Boundary: final maintained comments, component/usage/evidence documents and
  Doxygen generation inputs; generated pages remain unversioned.
  Steps: (1) reconcile current documentation and completed-task evidence,
  leaving no unfinished task/debt hidden in DONE; (2) finish maintained inputs,
  including archival/queue changes, re-review evidence affected since T196 and
  refresh only its justified fingerprints, then rerun make conformance-check;
  run make docs-complete, make docs and make docs-reproducible serially;
  (3) verify the final manifest, source-rule destinations and reproducible output.
  Accept: every maintained code file has its required purpose/contract/testing
  evidence, and clickable source/generated rule links resolve. The generated
  reference describes final TODO/DONE and all delivered code, not an earlier
  checkpoint. Record exact tool versions/counts and delivery command; regenerate
  if any maintained input changes after generation. Commit final inputs/evidence,
  and report supported behavior and remaining out-of-sprint limits accurately.

## Acceptance routing

This is navigation to task owners, not a second definition of language rules
or completed evidence. Exact obligations remain in the
[canonical acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix).
T203 audits the current source/test mapping; T168 remains historical evidence.
T196 reviews the actual full evidence.

| Static module template acceptance area | Current task owner |
|---|---|
| Placeholder spelling | [T196](#t196) |
| Placeholder scope | [T207](#t207) |
| Ordinary-argument inference | [T182](#t182) |
| Nested field specialization | [T182](#t182) |
| Specialization identity | [T183](#t183) |
| Dependent constant declarations | [T212](#t212) |
| Explicit constant namespaces | [T212](#t212) |
| Specialized constant lifecycle | [T186](#t186) |
| Concrete operation checks | [T185](#t185) |
| Template parameter signatures | [T208](#t208) |
| Partial composition | [T209](#t209) |
| Explicit static input categories | [T207](#t207) |
| Defined static results | [T211](#t211) |
| Static output composition | [T211](#t211) |
| Static output lookup | [T211](#t211) |
| Named-only static interfaces | [T209](#t209), [T211](#t211) |
| Static named arity and rejected skips | [T209](#t209), [T211](#t211) |
| Static binding modes and order | [T209](#t209) |
| Static result alias introduction | [T207](#t207) |
| Static alias scope and capture | [T207](#t207) |
| Static output inference and closure | [T211](#t211) |
| Complete producer closure | [T211](#t211) |
| Ordinary static-export boundary | [T212](#t212) |
| Static evaluation boundary and cycles | [T210](#t210) |
| Returned constructor creation provenance | [T210](#t210) |
| List implementation boundary | [T195](#t195) |
| Specialization analysis and diagnostics | [T189](#t189) |
| Finite repeated composition | [T210](#t210) |
| Definition-generated recursion | [T210](#t210) |
| Creation-graph provenance | [T210](#t210) |
| Constructor origins and captures | [T208](#t208) |
| Cache-independent path checks | [T183](#t183) |
| Native template rejection | [T207](#t207) |
| Concrete native template arguments | [T190](#t190) |
| Concrete native composition | [T190](#t190) |
| Template source delivery | [T191](#t191) |
| Source distribution resolution | [T191](#t191) |

[T206](DONE.md#t206) records source integration of the unchanged Internal hyphen
words, Hyphenated name identity and Hyphen context boundaries; T196 reviews
their final conformance alongside spelling. T213 owns new AST source lookup and
dependency closure; T190 owns the native authoring/header consumers. T187/T188
own compiled storage, label dispatch, state and continuation evidence. T194 owns
Concrete facade exports, Template export exclusion and the owned-child/independent-
peer lifecycle acceptance clarified by T205. T192/T193 own editor/relocation
consumers of the same resolved identities. T196 reviews every other canonical
row and existing product regression, and T197/T198 complete release/reference
acceptance. No historical hash refresh or routing label establishes support.

The next unused task ID is **T215**.

## Useful verification commands

Use focused suites for the changed component, then its real parent composition.
The normal project entry points remain `make`, `make test`,
`make -C tests/unit/driver/request` and `make test-clean`; exact new leaf paths
belong in the evidence for their owning tasks. Successful leaves clean outputs;
failed leaves preserve them for diagnosis. Run broad checks when required by the
changed contracts or the release phase, rather than repeatedly rebuilding
unrelated components after a documentation-only edit.

For completed-stage requirement routing, see the
[archived coverage map](DONE.md#requirement-coverage-map). Current conformance
remains the separate [reviewed evidence matrix](docs/implementation/conformance/README.md).
