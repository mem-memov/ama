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
The [T167 planning record](DONE.md#t167) preserves its review and acceptance.
The user authorized sequential implementation on 2026-11-18, including focused
tests, code documentation, exact algorithms and a separate commit per completed
task. Specification changes still require their own explicit authorization.

The deliverable is compile-time specialization with exact inference, composition,
partial constructors, static results and concrete native/source/C-library
consumers. Keep the existing intrinsic list foundation. The deferred
[AMB-006](AMBIGUITIES.md#amb-006) rewrite, new raw-storage API, generic native ABI,
binary-only open-template format, static natural-number parameters and dependent
types are outside this sprint. No runtime template objects or specialization
generation are introduced.

### Dependency order and checkpoints

| Phase | Tasks | Assessable result |
|---|---|---|
| Baseline | T168 | Reviewed gaps, component ownership and executable acceptance map |
| Source and interfaces | T169–T174 | Shared names, immutable syntax, exact origins, scoped interfaces and early native rejection |
| Static analysis components | T175–T180 | Nonexpanding terms/equalities, partial applications, guards and fully closed static projections |
| Complete specialization | T181–T186 | Typed connections, guarded cache/orchestration, ordinary checks and specialization constants |
| Execution | T187–T190 | Resolved plans, compiled execution, source diagnostics and concrete native composition |
| Product consumers | T191–T195 | Source delivery, editor/relocation consumers, C facades and useful list algorithms |
| Acceptance and delivery | T196–T198 | Current evidence, clean release and final generated reference |

The presentation is a topological order, not a promise that all tasks are one
linear algorithm. Explicit prerequisites below govern readiness. Components may
exchange symbolic terms and deferred obligations; completing an early component
does not claim whole-program closure. Every task is accepted and committed on
its own. Do not batch completed tasks into a phase commit.

T168 fixes the exact production/test paths after inspecting maintained sources.
The boundaries below name existing responsibility owners and intended children,
not new mandatory directory spellings or guessed existing support. This planning
review inspected canonical requirements, maintained contracts and source/test
directory topology; it did not inspect compiler function bodies, run functional
tests or establish template support. Some component prose still describes
historical subsets: T168 must reconcile it with actual files and recorded tests.

Four ordering constraints are essential:

- Inventory declared static names and classify lexical aliases before final
  free-formal collection; reject generic native definitions from that complete
  inventory before preparing native authoring headers.
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

Current implementation checkpoint: T175 accepted and archived in DONE.
Input 5c00e82; [static term record](docs/implementation/static-terms.md).
Strict parent (76 cases), term sanitizers (nine cases), root build, standalone
headers and documentation gates passed. Final archive-input refresh uses
/tmp/ama-t175-doxygen-archive.log; collect an outstanding run before the task
commit. Next ready task is T176 after that per-task commit. No specification
change; full executable integration remains T184.

## Current tasks

### Phase 1. Recognize sources and establish interfaces

### Phase 2. Build exact static analysis components

<a id="t176"></a>

- [ ] **T176 — Solve finite structural type constraints exactly.**
  Prerequisites: T175.
  Rules: [TPL-03](spec/language/templates.md#tpl-03),
  [TPL-10](spec/language/templates.md#tpl-10),
  [ARCH-05](spec/implementation/compiler.md#arch-05).
  Boundary: nonexpanding equality/nominal decomposition, recursive signature
  checks, occurs checking and monotone delayed obligations.
  Steps: (1) state the finite-term algebra and fixed-point invariant; (2) implement
  exact refinement/conflict/unresolved outcomes; (3) compose bound application
  terms with independent model/order/failure tests.
  Accept: anchored equality cycles and later anchors solve; contradictory,
  unanchored required and infinite substitution terms fail precisely.
  Permuting equations preserves the result. Nominal reference cycles are not
  substitution cycles. No literal/capability/name search, arbitrary iteration
  budget or default type chooses a solution; definitions are not expanded here.

<a id="t177"></a>

- [ ] **T177 — Apply partial constructors and recover nominal input patterns.**
  Prerequisites: T173, T175, T176.
  Rules: [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08).
  Boundary: structural application/recovery operations over known nominal
  origin metadata, called by later typed-connection/projection analysis.
  Steps: (1) validate binding/category/signature contracts and context-specific
  omission descriptors; (2) apply/recover captures, compute exact residuals and
  normalize direct/staged/reordered source applications into T175 records;
  (3) compose nested patterns with finite equality solving.
  Accept: equivalent direct/staged/reordered applications have equal normalized
  records; forwarding and completing partial constructors preserves captures;
  complete root input patterns and nested partial patterns invert only declared
  formals. Check repeated pattern agreement, residuals already open, unknown
  actual origins deferred, wrong-category/name/signature and fixed rebinding.
  Distinguish static-argument residuals, field inference terms, independent
  constant namespaces and explicit-output producer terms without runtime samples.

<a id="t178"></a>

- [ ] **T178 — Guard static dependencies and specialization creation paths.**
  Prerequisites: T175, T177.
  Rules: [TPL-11](spec/language/templates.md#tpl-11),
  [TPL-12](spec/language/static-results.md#tpl-12),
  [COMP-03](spec/implementation/compiler.md#comp-03).
  Boundary: provenance/ancestry and static-definition dependency operations;
  all later definition expansion must enter through these checked boundaries.
  Steps: (1) model distinct graphs and original owner/application source edges;
  (2) check active symbolic ancestry before following each expansion edge;
  (3) test concrete and unresolved-key paths with the term/application components.
  Accept: finite repeated supplied/nested constructors and independent sibling
  reuse pass; direct, mutual, permuted and growing creation paths fail before
  unbounded expansion. Preserve lexical owners, forwarded/captured origins and
  supplied-Type reuse versus explicit-output producer edges.
  Prove progress without depth/time limits; distinguish output-definition cycles
  from equality, loading, eager ownership and constant-value cycles.

<a id="t179"></a>

- [ ] **T179 — Generate guarded static result projections and equations.**
  Prerequisites: T176, T177, T178.
  Rules: [TPL-12](spec/language/static-results.md#tpl-12),
  [TPL-11](spec/language/templates.md#tpl-11).
  Boundary: static result definition evaluation and projection descriptors,
  using known origins, normalized bindings, finite terms and checked expansion.
  Steps: (1) bind declared output definitions and actual-origin lookup;
  (2) expand required definitions through T178 and generate exact receiver
  equations; (3) compose nested/returned constructors and static namespaces.
  Accept: concrete/symbolic Types and partial Template results, named/positional
  projection and all-skipped output phases preserve provenance; the expression
  still denotes its producer Type. Validate unselected outputs and reject
  unknown origins/members, runtime-field selection and definition cycles.
  Static processing executes no body, default constructor or native effect;
  source-located diagnostics accompany each failure from the start.

<a id="t180"></a>

- [ ] **T180 — Finalize complete producers and immutable result aliases.**
  Prerequisites: T172, T176, T179.
  Rules: [TPL-12](spec/language/static-results.md#tpl-12),
  [TPL-08](spec/language/templates.md#tpl-08).
  Boundary: per-application closure obligations and publication of resolved
  static results/lexically classified aliases.
  Steps: (1) create distinct omitted-input terms for each explicit output phase;
  (2) solve and require unique complete producer bindings before publishing;
  (3) compose alias equality/visibility with partial returned constructors.
  Accept: output equalities can close missing producer inputs; unused inputs,
  unselected outputs and all-skipped phases still require full closure.
  A fresh alias provides no type anchor; existing aliases require exact equality.
  Returned Template residuals may remain bound as their own parameters; enclosing
  producer inputs may not. Compare the same omission with and without outputs
  in argument/field/result/constant contexts and reject noninvertible inference.

### Phase 3. Close and validate executable specializations

<a id="t181"></a>

- [ ] **T181 — Resolve dependent constant types and explicit namespaces.**
  Prerequisites: T174, T177, T180.
  Rules: [CONST-03](spec/language/declarations.md#const-03),
  [CONST-06](spec/language/declarations.md#const-06),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: static declared-type and namespace resolution for shared values and
  constant sources; it supplies typed evidence, not constant runtime execution.
  Steps: (1) bind `@=` types to collected owner formals without new introductions;
  (2) resolve independent concrete namespaces or guarded output-phase equations;
  (3) compose declared-member types with the solver and static projections.
  Accept: result-introduced types, empty typed lists and nested templates work;
  named/positional reads select only declared `@=` members. Keep direct qualified
  constants distinct from atomic placeholders. No constant name or runtime
  receiver can choose an open namespace; output equalities may close it first.
  Register symbolic obligations for later owner substitution without premature
  lookup. Reject open/unknown namespaces, invalid storage/operations and members.

<a id="t182"></a>

- [ ] **T182 — Generate ordinary typed-connection and nested-field constraints.**
  Prerequisites: T176, T177, T180, T181.
  Rules: [TYPE-06](spec/language/declarations.md#type-06),
  [NAME-06](spec/language/declarations.md#name-06),
  [TPL-03](spec/language/templates.md#tpl-03),
  [TPL-04](spec/language/templates.md#tpl-04).
  Boundary: runtime declaration/source-order inference, foreach locals and
  semantic input/result/default/assignment/reference connections.
  Steps: (1) classify lexical receivers and locals before solving, keeping static
  names separate; (2) generate equations from typed connections and deferred
  constant evidence; (3) compose two/three-level owner/field applications.
  Accept: later anchors refine existing terms without rebinding; independent
  fields can differ while one field/concrete owner agrees across all uses.
  Check empty typed references/lists, self-reference and unused typed defaults,
  local lifetime, later explicit/inferred names and suffix mismatches.
  Literals, runtime contents, first execution and unknown-to-unknown copying
  provide no invented anchor; unresolved/inconsistent required types fail.

<a id="t183"></a>

- [ ] **T183 — Register exact specialization identities and guarded cache reuse.**
  Prerequisites: T171, T178, T180, T182.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-11](spec/language/templates.md#tpl-11),
  [NAME-01](spec/implementation/compiler.md#name-01),
  [NAME-02](spec/implementation/compiler.md#name-02).
  Boundary: compile-time specialization registry, full-key encoding and cache
  dependency/provenance records, separate from instance allocation.
  Steps: (1) key complete normalized bindings by original identity/formal order;
  (2) intern structurally valid closed candidate keys with retained dependencies
  and explicit pending-check state; (3) check each incoming ancestry and cached
  descendants under alternate demand paths.
  Accept: direct/partial/alias-equivalent bindings share identity; different
  roots/origins/unused bindings/captures/residual signatures do not merge.
  Interning is not executable admission; T185 supplies ordinary validation.
  Cold/warm caches and reordered roots agree, including repeated-origin errors
  hidden in cached grandchildren; unrelated concrete paths do not form false
  source-only cycles. Inject key collisions and allocation failure explicitly.

<a id="t184"></a>

- [ ] **T184 — Orchestrate the complete guarded specialization closure.**
  Prerequisites: T174, T181, T182, T183.
  Rules: [TPL-06](spec/language/templates.md#tpl-06),
  [TPL-10](spec/language/templates.md#tpl-10),
  [TPL-11](spec/language/templates.md#tpl-11),
  [COMP-02](spec/implementation/compiler.md#comp-02).
  Boundary: reusable frontend semantic pipeline and required-specialization
  worklist, with explicit child operations and publication/cleanup ownership.
  Steps: (1) compose source loading, lexical classification, constraints,
  projections and guarded demands; (2) reach exact closure before exposing
  closed type candidates for ordinary validation; (3) test real check-only and
  multi-root frontend stage handoffs.
  Accept: all required runtime types and producer inputs close; residual returned
  constructors remain compile-time values only. No expansion bypasses T178 and
  no cache bypasses path validation. Reordered independent constraints/roots
  preserve acceptance; malformed definitions and skipped branches remain checked.
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
  Prerequisites: T181, T183, T185.
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

### Phase 4. Lower and execute concrete programs

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
  plans; (2) remove static-only inputs/outputs/aliases from runtime storage and
  keep runtime ordinals; (3) test actual semantic/planning/codegen-model handoffs.
  Accept: every runtime type/layout is closed; plans retain source provenance,
  full nominal distinctions, correct refs/lists/constants and stable pool roles.
  No emitter-side inference, implicit default instances or state save/restore.
  Check arithmetic/layout/allocation failures, unused static results and
  interleaved interfaces; static metadata remains available only where needed
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
  original sites. Unicode/multiword/hyphen paths round-trip, virtual identities
  do not pretend to be source files, and diagnostics remain valid through cleanup.
  No output artifacts are published on failed compilation; static evaluation
  neither executes runtime code nor produces fictitious runtime stack entries.

<a id="t190"></a>

- [ ] **T190 — Compile concrete native/template compositions.**
  Prerequisites: T174, T185, T187, T188.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [CABI-01](spec/implementation/native-interface.md#cabi-01),
  [CABI-05](spec/implementation/native-interface.md#cabi-05),
  [CABI-06](spec/implementation/native-interface.md#cabi-06).
  Boundary: native semantic views, scaffold/header models, layout/toolchain
  verification and handwritten concrete native companions.
  Steps: (1) instantiate Ama templates with concrete native types; (2) integrate
  closed Ama template fields/dependencies inside concrete `@C` modules;
  (3) independently compile generated headers and run actual native compositions.
  Accept: required native default/copy/literal/condition/index capabilities and
  lifecycle/layout contracts are checked after substitution; unsupported ones
  fail. Bound aliases/static outputs do not add owner formals or ABI slots.
  Declared Ama refs remain visible and checked, never opaque native payloads.
  Cover concrete owned/ref/list fields and rejection of open dependencies;
  introduce no generic native hooks or specialization-specific authoring API.

### Phase 5. Connect the existing product consumers

<a id="t191"></a>

- [ ] **T191 — Deliver open templates as source and invalidate consumers exactly.**
  Prerequisites: T171, T183, T188, T190.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [DEP-03](spec/implementation/compiler.md#dep-03),
  [COMP-05](spec/implementation/compiler.md#comp-05).
  Boundary: existing configured roots, project/native source closure, incremental
  input fingerprints and actual independent source-distribution consumers.
  Steps: (1) load an open template library and required companions in a separate
  consumer project; (2) build/run that consumer's own specializations;
  (3) test repeat/reordered imports, changed inputs and missing dependencies.
  Accept: full root/origin/formal/capture identity survives reuse; compatible
  imports share type/code without instance state. Reject name conflicts, absent
  Ama/native/external C dependencies and unresolved bindings exactly.
  Changed template/signature/output/native inputs invalidate affected results;
  no-op builds remain current. Use existing source delivery, not a new package
  manager, file format, virtual filename or binary-only template interface.

<a id="t192"></a>

- [ ] **T192 — Integrate static bindings with editor analysis and rename.**
  Prerequisites: T169, T172, T173, T184, T189.
  Rules: [COMP-01](spec/implementation/compiler.md#comp-01),
  [NAME-06](spec/language/declarations.md#name-06),
  [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: existing editor snapshots/index/query/rename consumers of the common
  frontend; preserve their documented transaction and failure contracts.
  Steps: (1) index distinct static declarations, aliases and external labels;
  (2) route navigation/diagnostics/rename through resolved source identities;
  (3) exercise actual workspace queries and checked rename transactions.
  Accept: same-spelled foreign parameters and actual signature labels do not
  collapse; rename respects lexical alias visibility, full atomic names and
  captures. No generated specialization source file or duplicated parser.
  Cover incomplete/error snapshots, Unicode/hyphens, stale versions and capture
  rejection; failed transactions preserve workspace bytes.

<a id="t193"></a>

- [ ] **T193 — Preserve template meaning during source directory relocation.**
  Prerequisites: T171, T184, T189, T191, T192.
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
  Include nested applications/namespaces, both root kinds, hyphens/name+/,
  original-name conflicts/new-root files, interruption and rollback/replay.
  No text replacement or treating explanatory placeholder components as files.

<a id="t194"></a>

- [ ] **T194 — Export concrete template facades through the checked C boundary.**
  Prerequisites: T188, T190, T191.
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
  Reject open export roots, virtual specialization selectors/static argument
  maps, missing closure and unsupported transport. Static results add no runtime
  C slots; no shared mutable Ama refs or yield across an unfinished native call.

<a id="t195"></a>

- [ ] **T195 — Exercise useful generic algorithms over existing intrinsic lists.**
  Prerequisites: T185, T186, T188, T190, T191.
  Rules: [TPL-09](spec/language/templates.md#tpl-09),
  [TPL-04](spec/language/templates.md#tpl-04),
  [FLOW-05](spec/language/lists.md#flow-05),
  [LIST-10](spec/language/lists.md#list-10).
  Boundary: maintained executable Ama examples and integration cases, using
  existing list methods and ordinary modules for state/methods.
  Steps: (1) document algorithm inputs, preconditions, invariants and expected
  effects independently of outputs; (2) implement examples of typed list
  processing, nested composition and a returned partial constructor;
  (3) compile/run several concrete types and negative applications.
  Accept: empty explicitly typed lists need no sample; owned/ref forms, live
  traversal insertion/deletion, busy/refusal and copy behavior remain unchanged.
  Include equal specialization instances with independent state, explicit static
  outputs and wrong-operation/category cases. Examples teach useful composition;
  no base-list provider, new allocator/storage API or privileged generic code.

### Phase 6. Review evidence and deliver

<a id="t196"></a>

- [ ] **T196 — Review current requirement and acceptance evidence.**
  Prerequisites: T168 through T195.
  Rules: [TEST-07](spec/implementation/tools-and-conformance.md#test-07),
  [TEST-08](spec/implementation/tools-and-conformance.md#test-08),
  [ARCH-06](spec/implementation/compiler.md#arch-06).
  Boundary: component arguments, per-file registry and reviewed conformance
  matrix; use the actual current canonical registry rather than frozen counts.
  Steps: (1) reconcile all current rules/acceptance rows with named component and
  distinct real-consumer cases; (2) run the recorded affected behavior and
  evidence-gate checks; (3) refresh reviewed mappings/fingerprints only after
  inspecting the corresponding sources, arguments and results.
  Accept: all 214 currently indexed rules and 150 current acceptance rows,
  including the 37 template areas and hyphen/facade consumers, have accurate
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
T168 verifies the source/test mapping; T196 reviews the actual full evidence.

| Static module template acceptance area | Primary task |
|---|---|
| Placeholder spelling | T169 |
| Placeholder scope | T172 |
| Ordinary-argument inference | T182 |
| Nested field specialization | T182 |
| Specialization identity | T183 |
| Dependent constant declarations | T181 |
| Explicit constant namespaces | T181 |
| Specialized constant lifecycle | T186 |
| Concrete operation checks | T185 |
| Template parameter signatures | T177 |
| Partial composition | T177 |
| Explicit static input categories | T172 |
| Defined static results | T179 |
| Static output composition | T179 |
| Static output lookup | T179 |
| Static positional interface order | T173 |
| Static skips and arity | T173 |
| Static binding modes and order | T173 |
| Static result alias introduction | T180 |
| Static alias scope and capture | T172 |
| Static output inference and closure | T180 |
| Complete producer closure | T180 |
| Output-phase omission boundary | T180 |
| Static evaluation boundary and cycles | T179 |
| Returned constructor creation provenance | T178 |
| List implementation boundary | T195 |
| Specialization analysis and diagnostics | T189 |
| Finite repeated composition | T178 |
| Definition-generated recursion | T178 |
| Creation-graph provenance | T178 |
| Constructor origins and captures | T178 |
| Cache-independent path checks | T183 |
| Native template rejection | T174 |
| Concrete native template arguments | T190 |
| Concrete native composition | T190 |
| Template source delivery | T191 |
| Source distribution resolution | T191 |

T169 additionally owns Internal hyphen words, Hyphenated name identity and
Hyphen context boundaries, plus affected ordinary name/path regressions.
T187/T188 own compiled storage/dispatch/state evidence;
T194 owns Concrete facade exports and Template export exclusion, with the
existing compiled-library/embedding regressions. T192/T193 own affected editor
and directory-relocation consumers. T196–T198 cover full evidence/release/docs;
these additional obligations are not discharged by the 37-row routing table.

The next unused task ID is **T200**.

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
