# Ama implementation work queue

The whole [spec/](spec/README.md) directory is authoritative. The
[requirement index](spec/requirements.md) locates all 200 current requirements;
[spec/grammar.md](spec/grammar.md) defines source forms. This file orders the
engineering work needed to implement those contracts. It neither changes the
language nor grants permission to edit the specification.

## How to use this queue

Start with the first unfinished task whose prerequisites are satisfied. Complete
one independently testable task, record its evidence here and in the relevant
component contract, and commit that task before beginning the next one. Task
identifiers are stable; retain them when adding or splitting future work.
New tasks can therefore appear before lower-numbered tasks when their dependency
position requires it: follow prerequisites and presentation order, not ID order.
A phase is a planning group, not one large implementation commit.

The checkboxes below describe work against the current specification, including
revalidation of reusable implementations. They do not claim that every listed
component is absent. Task T001 decides which existing contracts and evidence can
be reused. T132–T134 remove obsolete implementation, decompose retained code and
complete its documenting comments before further language implementation starts.
When a task is already implemented correctly, complete it by verifying
and recording the current evidence rather than rewriting working code. Do not
mark a feature complete from an old checkbox, a design document or a passing
unrelated suite.

Each task states its prerequisites, deliverable and specific acceptance focus.
The common completion criteria also apply to every task. Tasks in later phases
may begin as soon as their explicit prerequisites pass; no consumer may skip an
unfinished prerequisite. The sequence intentionally reaches a runnable stackless
program before implementing all calls, continuations, collections and tooling.

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

## Reported implementation baseline

This table preserves the previous TODO's completion reports. Its linked evidence
has **not been revalidated in this planning change**, and the implementation was
not inspected. The reports predate several current language contracts. They are
inputs to T001, not a blanket claim of current conformance.

| Previous section | Previously reported completion and evidence | What still needs current verification |
|---|---|---|
| 1. Components and test/build foundation | Component map, nested driver responsibilities, recursive C harnesses, incremental command/dependency manifests and formal contract registry. [Map](docs/implementation/components.md), [build](docs/implementation/incremental-build.md), [harness](docs/implementation/test-harness.md), [contracts](docs/implementation/contracts/README.md). | Every current component, its parent composition, file-level test mapping and Doxygen coverage. |
| 2. Native ABI foundation | Supported toolchain; common status and reserved invalid-index distinction; all six hooks; entry inlining probes; construction/finalization bookkeeping; service metadata and qualified native names. [ABI](docs/implementation/native-abi.md), [status](docs/implementation/native-status.md), [hooks](docs/implementation/native-hooks.md), [toolchain](docs/implementation/native-toolchain.md), [lifecycle](docs/implementation/native-lifecycle.md), [metadata](docs/implementation/runtime-metadata.md), [namespaces](docs/implementation/native-namespaces.md). | Context-local state, current list-service ABI, asynchronous native lifetime and parallel access contracts. |
| 3. Source and identities | Stable UTF-8 spans, Unicode 17.0.0 XID tables/generator, extensionless and `name+/` paths, length-delimited category-separated SHA-256 identities with collision rejection, structural diagnostics and compiled source maps. POSIX catalog reportedly rejects symlinks/inode aliases and has explicit output exclusions. [Storage](docs/implementation/source-storage.md), [Unicode](docs/implementation/unicode.md), [paths](docs/implementation/module-paths.md), [identities](docs/implementation/identities.md), [diagnostics](docs/implementation/diagnostics.md), [rendering](docs/implementation/diagnostic-rendering.md). | Current `.`/`..` roots, spaced path components, exact source spelling, draft activation and complete dependency invalidation. |
| 4. Parsing | Byte-preserving layout/AST, entry literal boundaries, draft/section handling, positional placeholders, fixed ABI records, iterative nesting, precise expected-token diagnostics and allocation-failure tests. [Frontend evidence](docs/implementation/parser-errors.md). | Current strict spaces/comments, inferred-receiver syntax, self-reference, result trees, loop condition chains and foreach forms. |
| 5. Semantic analysis and plans | Transitive closure; one base-name namespace; nominal storage/capability checks; separate dependency relations; greatest-fixed-point copy capability; native companions; constants; typed lifecycle/control/storage plans and source-related errors. [Closure](docs/implementation/project-dependencies.md), [semantics](docs/implementation/semantic-analysis.md), [plans](docs/implementation/typed-plans.md). | Exact source-order inference/no-shadow foreach integration, current method interfaces, context-local constants and updated control metadata. |
| 6. Check-only and native authoring | Shared frontend, draft diagnostics, self-contained generated headers/skeletons, native naming contexts, machine-readable C tooling configuration, handwritten-file preservation and isolated header/skeleton checks. | Current grammar/semantics and agreement with the final project assembly. |
| 7. Runtime ownership | Stable typed block pools; empty/stale/generation handles; reclamation; lifecycle failure cleanup; injective clone maps; staged replacement; copy overlap/availability and busy-descendant preflight. | Context boundaries, reserved self-identity during construction, current self-copy capability checks and membership-aware list copying. |
| 8. Stackless execution | Verified `--emit ... --compile` output; direct stable-slot execution; startup constants/root; literal and ordered entry preparation; retained results, control transfers, refusal snapshots and pre-cleanup source diagnostics. [Entry](docs/implementation/entry-preparation.md), [execution](docs/implementation/stackless-execution.md), [emission](docs/implementation/project-emission.md). | Current source semantics, direct result actions, intrinsic list frames and explicit context parameterization. |
| 9. Continuations | Explicit saved positions, output-only resume, entry restart, host outcomes, recursive ordinary clearing/explicit preservation and generated-program/custom-host tests. [Continuation evidence](docs/implementation/continuations.md). | Foreach cursor/origin state, live mutation correction, copy mappings and asynchronous resource/enrollment boundaries. |
| 10. Lists and library, partial | Contiguous handle storage, checked growth/index conversion, standalone mutation/preflight, scalar and refusal-reader modules, interface-order checks and runtime/generated-library suites were reported. Whole-list execution integration remained incomplete. | LIST-03/LIST-13 membership records, six typed methods, service busy protection, live-boundary traversal and current scalar contracts require explicit verification. |
| 11–12. Exploration and release | Examples, editor/debugger integration and final conformance/reproducibility remained unchecked. | Covered by the actionable tasks below; no completion is inferred. |

The earlier list action dispatcher, action/outcome enumeration API, retained
inspection results and undecided conversion-profile proposal are superseded by
the current typed-method specification. Their old pause is not a blocker for
this queue; do not implement or revive those obsolete contracts. Previously
reported release/Werror and focused ASan/UBSan successes remain historical evidence.
LeakSanitizer was reported disabled under ptrace supervision; leak certification
was not claimed and must not be inferred here.

## Phase 0. Establish the current baseline and evidence workflow

Contracts: [compiler architecture](spec/implementation/compiler.md),
[conformance](spec/implementation/tools-and-conformance.md).

- [ ] **T001 — Audit the implementation delta.** Needs: none. Build a matrix for
  all 200 indexed requirements and acceptance rows against existing components,
  code files and executable evidence. Inventory all maintained implementation,
  native companion, test/support and build/tool code; classify each file and
  mixed-responsibility portion as retained, needing adaptation, needing
  decomposition/documentation, or obsolete, with its consumers and exact reason.
  Validate every reported baseline item; distinguish confirmed support, changed
  contracts, gaps and blockers without resetting or trusting old checkmarks
  wholesale. Record independently testable changes as stable component tasks
  with prerequisites, acceptance evidence and separate commits before the
  relevant T132–T134 completion gate. Stop on contradictory requirements.
- [ ] **T002 — Refresh component contracts and boundaries.** Needs: T001. Update
  the responsibility tree and contract registry; identify narrow reusable APIs,
  required decompositions and exact invariant arguments. Give each retained
  responsibility a target subdirectory, owning component, explicit interface and
  migration/test mapping; distinguish reusable behavior from retired contracts.
  Validate that each planned component and assembled parent has an independently
  runnable test home. This plan drives T132–T134 rather than a wholesale rewrite.
- [ ] **T003 — Verify recursive builds and invalidation.** Needs: T002. Confirm
  nested source discovery, header/toolchain/link-membership dependencies, parallel
  make ordering and clean/failure artifact rules. Test no-op and changed-input
  rebuilds through real parent and independent leaf builds.
- [ ] **T004 — Verify the shared failure/test infrastructure.** Needs: T003.
  Maintain C leaf `main`/`it_*`/`run`, guarded fixtures, assertions under release
  flags and deterministic failure injection. Test harness failures, cleanup and
  sanitizer invocation without introducing a second test framework.
- [ ] **T005 — Establish code-to-tests and generated documentation checks.**
  Needs: T004. Inventory every project code/header file and its test coverage;
  register uncovered files for the owning tasks. Extend the `make docs` Doxygen
  build from docs/Doxyfile with reproducibility checks and checks for
  purpose/contract comments, independent headers and documentation
  links, including source-relative rule links and deterministic read-only Doxygen
  anchor mapping. Test actual rendered destinations and stale-link failures.
  Validate the workflow on existing small components before expanding it.
- [ ] **T132 — Remove the obsolete implementation completely.** Needs: T005.
  Complete the removal tasks identified by T001/T002: delete retired code paths,
  duplicate superseded implementations, obsolete-only tests, build registrations
  and conflicting derived descriptions. Update all retained consumers; leave no
  compatibility dispatcher, dormant alternative implementation or stale source
  registration for a retired contract. Preserve conforming reusable behavior and
  its valid evidence; needing a later feature adaptation alone does not make code
  obsolete. Reconcile every removal against the audit inventory and verify a clean
  build, independent affected suites and their real parent compositions. Keep
  this gate open until every obsolete implementation item has been removed.
- [ ] **T133 — Decompose the retained implementation.** Needs: T132. Complete
  the component-sized refactoring tasks from T002 for all retained code, using
  nested responsibility directories, small single-operation functions and narrow
  explicit interfaces. Separate orchestration, recognition, validation, state
  changes and cleanup wherever their contracts are distinct; state ownership,
  invariants and failure effects at each boundary. Add Doxygen explanations and
  tests with each moved or extracted component. Verify preserved conforming
  behavior before and after each refactoring through independent and composed
  tests, and update include paths, build discovery and the coverage registry.
  Reject arbitrary line-count splitting, empty hierarchy and wrapper-only changes;
  close the gate only when every retained responsibility has its reviewed home.
- [ ] **T134 — Complete documenting comments for all retained code.** Needs:
  T133. Close the initial inventory's documentation and per-file test gaps,
  including unchanged retained files, headers, native companions and test support.
  Add Doxygen `@file`/`@brief` comments to C sources and headers; for other code
  formats use valid local comments and linked Doxygen input documentation under
  CONTRIBUTORS.md. Explain each nontrivial function, type and interface: why it
  exists, its inputs/results, ownership, side effects, invariants and failure
  behavior. Compiler explanations must link to the exact
  canonical `spec/` file and rule anchor and explain their implementation relation.
  Preserve byte-exact malformed fixtures by documenting their purpose externally
  under the fixture rules in CONTRIBUTORS.md. Verify all maintained files have
  meaningful test mappings, generate the complete baseline reference and check
  source links, rendered rule destinations and undocumented-interface diagnostics.
  Do not defer retained-code documentation debt to the release phase.

## Phase 1. Fix execution-context and native boundaries before consumers

Contracts: [runtime](spec/implementation/runtime.md),
[native interface](spec/implementation/native-interface.md),
[diagnostics](spec/implementation/diagnostics.md),
[context isolation](spec/language/async-and-parallelism.md#par-01).

- [ ] **T006 — Establish context ownership and identity.** Needs: T134. Define
  explicit context-local pool/constant/error/control ownership and checked host
  identities. Test lifecycle, nonwrapping context identity, rejection of foreign
  access with colliding local numbers and nonreentrant executor entry.
- [ ] **T007 — Validate status and refusal records.** Needs: T006. Preserve the
  common code/message/length ABI, allocation-free immutable messages and distinct
  invalid-index/refusal/fatal paths. Test zero success, malformed native status,
  nested caller refusal preservation and diagnostics during allocation failure.
- [ ] **T008 — Validate the supported C execution contract.** Needs: T007. Record
  toolchain/target settings and exact checks for required native-entry inlining
  and label dispatch. Compile positive internal-helper/recursion probes and reject
  surviving native-entry calls or Ama function-call dispatch.
- [ ] **T009 — Validate all six native protocol adapters.** Needs: T008. Freeze
  typed signatures, byte views, includes, alignment and void-entry service status
  in the ABI harness. Compile and run every hook, wrong-signature rejection,
  unsupported optional capabilities and invalid-index versus fatal conversion.
  Supply a conforming `..integer` test module with literal/clone/index support
  and the fixed typed `from_size` constructor over the full host-size range;
  compile its signature and test exact round-trips for later list consumers;
  T085 later validates the production scalar library.
- [ ] **T010 — Validate native symbol namespaces.** Needs: T009. Reuse complete
  module identity for qualified author names and clean inclusion contexts. Use
  fixed collision-checked prefix vectors in this isolated harness; connect the
  current identity provider after T017 in T055. Test
  same-basename hooks/types/helpers, macro cleanup and consistent header/body uses
  in one translation unit without parsing or rewriting arbitrary C.
- [ ] **T011 — Validate lifecycle state transitions.** Needs: T009. Establish
  fresh/constructing/live/finalized bookkeeping, one selected constructor,
  reverse cleanup and terminal destroy failures. Inject failure at every
  construction/cleanup boundary and prove at-most-once finalization.
- [ ] **T012 — Validate native borrow and resource boundaries.** Needs: T010,
  T011. Expose narrow generated typed access, separate declared Ama graph state
  from native payloads and bound borrow lifetimes. Exercise native copies and
  access failures without retaining Ama addresses, handles or callback views.
- [ ] **T013 — Validate the host/context lifecycle interface.** Needs: T006,
  T007, T011, T012. Define completed/yielded/failed outcomes and startup/resume/
  disposal ownership. Test concurrent/reentrant-entry refusal, failed nonresume,
  disposal states and independence of two sequential contexts before threading.

## Phase 2. Source storage, Unicode identities and diagnostics

Contracts: [source and names](spec/language/source-and-names.md),
[compiler identity/dependencies](spec/implementation/compiler.md).

- [ ] **T014 — Verify original-byte source storage.** Needs: T134. Preserve spans,
  physical line endings, optional final newline and byte-based diagnostic columns.
  Test EOF, empty files, CRLF and allocation failure without fabricated bytes.
- [ ] **T015 — Verify strict Unicode recognition.** Needs: T014. Pin Unicode XID
  data and strict UTF-8 decoding; preserve byte identity without normalization.
  Test malformed encodings, boundary scalars and multiword international names.
- [ ] **T016 — Implement current physical-to-logical path mapping.** Needs: T015.
  Resolve explicit `.` and `..` roots, extensionless modules and `name+/` topology.
  Test spaced components, root-kind distinctions, conflicting roots/aliases and
  precise missing/ambiguous paths without search-order fallbacks.
- [ ] **T017 — Verify deterministic generated identities.** Needs: T016. Use
  complete category-separated identities, unambiguous encoding and exact hash
  collision rejection. Check stable Unicode vectors, forced collisions and
  distinct native/shared/type/control categories.
- [ ] **T018 — Verify the module registry and source catalog.** Needs:
  T016, T017. Register each complete identity once, preserve physical diagnostic
  paths and exclude owned outputs. Test duplicate physical identities and
  deterministic enumeration. Exact activation/draft classification belongs to
  T023's shared lexical/parser path; required-draft validation follows in T029.
- [ ] **T019 — Verify structured frontend diagnostics.** Needs: T014, T018.
  Preserve primary/cause/related spans and stable expected-construct information.
  Compose filesystem, Unicode and identity failures through user-facing messages.
- [ ] **T020 — Verify rendering and C source mappings.** Needs: T019. Render
  original names/excerpts and correctly escaped `#line` paths. Compile generated
  mapping probes and test allocation-free fallback reporting.
- [ ] **T021 — Compose source loading and dependency invalidation.** Needs: T018,
  T020. Track source/configuration/native/generated inputs without duplicate loads.
  Verify changed transitive files and root settings invalidate exactly dependent
  consumers. Integrate inactive-to-active semantics in T029 after T023 establishes
  the shared activation recognizer; do not introduce a second draft parser here.

## Phase 3. Parse current source forms without semantic guesses

Contracts: [grammar](spec/grammar.md),
[source layout](spec/language/source-and-names.md).

- [ ] **T022 — Implement exact layout and token boundaries.** Needs: T015, T019.
  Enforce one-space nesting and separators, compact paths/reference/list markers,
  comment indentation, tabs/trailing-space rejection and literal byte preservation.
  Test every boundary across LF/CRLF and optional final newline.
- [ ] **T023 — Parse sections, drafts and declarations.** Needs: T022. Build
  source-mapped AST nodes for executable/constants/native sections, explicit
  declarations, all four storage forms and initialization. Check activation,
  constants-only files, malformed sections and deep nested bodies.
- [ ] **T024 — Parse accesses, assignments and inferred receivers.** Needs: T023.
  Recognize indexed/qualified/self-reference sources and uniform assignment nodes
  without name lookup. Test suffix/spacing boundaries, legal source modes and
  forbidden literal assignments with exact diagnostics.
- [ ] **T025 — Parse calls and binding phases.** Needs: T024. Recognize entry,
  resume, retained/constant reads, named/positional bindings, empty placeholders
  and entry literal payloads. Test phase/mode/arity structure and explicit empty
  literal versus omitted input without semantic classification in the parser.
- [ ] **T026 — Parse direct-result trees and refusal handlers.** Needs: T025.
  Retain selector paths, sibling order and each handler's owning operation.
  Test nested modes, colon indentation and structural rejection independently
  of declared names or runtime availability.
- [ ] **T027 — Parse branches, loops and foreach.** Needs: T026. Represent
  positive/negative branches, empty bodies, single-root linear condition loops,
  break/return/yield and reference-binding foreach. Test exact repeat roots,
  comments between markers, deep nesting and malformed near-matches.
- [ ] **T028 — Compose the parser and ABI table reader.** Needs: T009, T021,
  T027. Validate fixed `.abi` records separately from Ama literal classification;
  exhaust malformed/duplicate/missing bindings. Run current documented examples,
  grammar boundary cases, truncations and source-to-AST failure cleanup.

## Phase 4. Resolve exact types, visibility and operation contracts

Contracts: [declarations](spec/language/declarations.md),
[values](spec/language/values-and-references.md),
[calls](spec/language/calls-and-results.md),
[control](spec/language/control-flow.md), [lists](spec/language/lists.md).

- [ ] **T029 — Verify complete project closure.** Needs: T028. Load transitive
  Ama/native dependencies once per identity and retain requester traces. Test
  legal dependency cycles, missing required drafts and companion selection only
  for active native modules, including invalidation when a draft becomes active.
- [ ] **T030 — Resolve explicit declaration inventories.** Needs: T029. Enforce
  one base-name namespace across sections and storage spellings; resolve full
  nominal module types. Provide shared exact source/interface-member lookup and
  declared result-label/position metadata for inference and later call validation.
  Test later visible explicit fields and duplicate-related
  spans without conflating visibility with initialization.
- [ ] **T031 — Implement source-order inference and foreach scope together.**
  Needs: T030. Apply TYPE-06/NAME-06's single ordered algorithm, immediately type
  new receivers and resolve/reuse foreach bindings with no shadowing. Test earlier
  unresolved occurrences, later explicit collisions, inferred list sources,
  disjoint locals, nested reuse and forbidden construction-time local access.
- [ ] **T032 — Verify transfers and self-reference typing.** Needs: T031.
  Enforce the complete storage-form matrix, constant/indexed sources and lexical
  standalone `&`. Test absent/mismatched suffixes, exact root-kind identity and
  storage compatibility without inferring unsupported conversions.
- [ ] **T033 — Derive dependency and capability fixed points.** Needs: T032.
  Provide exact dependency/cycle algorithms; derive the eager-owned-layout and
  clone-capability relations separately from module dependencies. Test finite
  graphs, recursive empty lists and noncopyable owned descendants. Require static
  copy capability before runtime self-assignment no-op handling; do not reject
  benign execution/reference cycles. T034 supplies resolved initializer edges.
- [ ] **T034 — Validate constants and initialization plans.** Needs: T033.
  Resolve qualified members and legal constant sources, then construct actual
  initializer dependencies and reject their forbidden cycles. Derive exact
  startup/reverse cleanup order. Test constants-only modules, constant cycles,
  inferred-field defaults and visibility without forward initializer reads.
- [ ] **T035 — Validate normal call/read bindings.** Needs: T032, T034. Resolve
  current named/positional rules, placeholders, exact input/result types, defaults
  and literal capabilities. Test mode-specific rejection and both declaration
  and binding locations without adding unaccepted mixed-mode behavior.
- [ ] **T036 — Validate direct-result operation trees.** Needs: T035. Resolve
  selectors only in parent result interfaces and operands in the initiating
  caller's environment. Test reference/continuation targets, sequential methods,
  inaccessible private names and per-link handlers.
- [ ] **T037 — Validate intrinsic typed list services.** Needs: T009, T033, T036.
  Model six fixed method interfaces and the required `..integer` capabilities;
  enforce service-parent modes, typed receivers and omitted-source versus explicit
  owning-copy capability. Reject obsolete action-dispatch and retained-state APIs.
- [ ] **T038 — Validate control-flow contexts.** Needs: T031, T036, T037. Check
  owned truth/index capabilities, branch forms, empty-body warnings, exact linear
  loop roots and handler break context. Ensure foreach has distinct binding and
  traversal semantics, including statically checking skipped/empty paths.
- [ ] **T039 — Validate continuations and copy boundaries statically.** Needs:
  T033, T038. Enforce no resume inputs, exact ordinary/continuation copy forms,
  static copy capability and detectable ownership overlap. Test self-copy,
  noncopyable resources and legal external reference transfers.
- [ ] **T040 — Compose semantic analysis with current diagnostics.** Needs:
  T034–T039. Check valid/invalid full projects, preservation of spans/storage forms,
  deterministic classification and allocation cleanup. Confirm every interpretation
  is resolved before lowering or C emission.

## Phase 5. Make resolved behavior explicit in IR and storage plans

Contracts: [compiler](spec/implementation/compiler.md),
[runtime](spec/implementation/runtime.md).

- [ ] **T041 — Define current typed IR boundaries.** Needs: T013, T040. Represent
  resolved type/storage identities, lifecycle, operations, failure edges and source
  maps independently of C text. Validate narrow independently inspectable nodes
  and reject unresolved source-level decisions at the boundary.
- [ ] **T042 — Plan context and instance storage.** Needs: T041. Separate per-context
  constants/runtime from stable-slot fields, interface state and control metadata.
  Include constructing identity, return links, saved positions and fixed lexical
  chain/foreach frames as logical state. Permit compact or implicit physical
  fields only with exact capability/liveness proofs under RT-02/GEN-03; record
  the general fallback and verify deterministic layout dependencies and no mirrors.
- [ ] **T043 — Lower entry, transfer and cleanup order.** Needs: T042. Emit
  explicit acquire/preflight/prepare/commit/result/release actions with distinct
  refusal and fatal edges. Test CALL-08 evaluation order and source-lifetime
  handoffs without inserting snapshots or whole-call rollback.
- [ ] **T044 — Lower structured control and result actions.** Needs: T043.
  Assign fixed loop repeat/break labels and depth-first result continuations.
  Test empty-loop single conversion, per-link handlers, nested exits and bounded
  frame use before C generation.
- [ ] **T045 — Verify complete frontend-to-plan composition.** Needs: T044.
  Preserve nominal types, source locations, ownership/capability facts and all
  control successors. Run real source-to-plan cases and invariant checks, including
  drafts, native contracts and invalid-project cleanup.

## Phase 6. Stable ownership storage before executing user bodies

Contracts: [references and copying](spec/language/values-and-references.md),
[lifecycle](spec/implementation/native-interface.md#complete-lifecycle-and-cleanup-protocol),
[pools](spec/implementation/runtime.md).

- [ ] **T046 — Verify stable typed block pools.** Needs: T006, T011, T042. Reuse
  free slots before growth without moving live payloads; reserve final identities
  while constructing. Test multiple same-type instances and pool growth with
  live addresses, without implicit default instances.
- [ ] **T047 — Verify generations and reclamation history.** Needs: T046. Retire
  exhausted identities and reclaim only entirely free blocks while preserving
  staleness across regrowth. For interior reclamation, validate sparse directory
  holes, retained identity history and free-list removal with live later blocks.
  Exhaust small generation/state models and failure paths; keep capacity, free
  slots and live counts distinct and record the implemented reclamation policy.
- [ ] **T048 — Verify acquisition and destruction preflight.** Needs: T047.
  Check whole affected owned subtrees, current-owner authorization and ordinary
  busy/refusal transitions. Test available roots with busy descendants, foreign
  context handles and protection release on every supported failure.
- [ ] **T049 — Integrate ordered construction and destruction.** Needs: T012,
  T048. Initialize fields once, expose final self-identity only under constructing
  rules and finalize children/native payload in the prescribed order. Inject
  failures and test escaped self-handles never revive after failed construction.
- [ ] **T050 — Build exact clone maps.** Needs: T049. Clone independent owned
  trees with injective final-destination mapping and internal-reference fixups;
  preserve external handles and clear ordinary saved positions. Test self/peer
  links, duplicate external targets and failed preparation cleanup. This task
  establishes scalar-owned trees; T076 integrates membership-aware lists.
- [ ] **T051 — Integrate staged replacement and overlap checks.** Needs: T050.
  Validate capability/identity/ancestry/source availability before replacement;
  preserve destinations on preparation failure. Test ordinary self no-op only
  after static capability validation and terminal cleanup failure after commit.
- [ ] **T052 — Separate shared constants from pools.** Needs: T034, T051.
  Initialize immutable module storage per context in dependency order and clean it
  in reverse order. Test no constants-only instance pool, two independent contexts
  and startup failure before root creation. Establish scalar-owned constant trees
  here; T076 supplies concrete list copying and T087 integrates list constants.
- [ ] **T053 — Compose runtime descriptors with planned C layouts.** Needs:
  T045, T052. Verify concrete sizes/alignment/offsets and nominal identity without
  raw-word universal casts. Compile descriptor checks and real native borrow/
  lifecycle probes against planned generated declarations. For any implemented
  compact/elided layout, compare observable transitions with the general model
  and measure actual size without demanding identical allocation-attempt counts.
- [ ] **T054 — Verify ownership services independently of dispatch.** Needs:
  T053. Exhaust small allocation/acquisition/clone/replacement transition models
  and run sanitizer/failure-injection compositions. Record evidence before using
  these services to execute Ama bodies.

## Phase 7. Deliver the first current-spec stackless vertical slice

Contracts: [generated C](spec/implementation/compiler.md#generated-project-organization),
[host outcomes](spec/implementation/runtime.md#root-execution-and-host-control).

- [ ] **T055 — Emit reusable native declarations and scaffold contexts.** Needs:
  T010, T028, T040. Generate self-contained typed headers, optional untouched
  skeletons and C tooling configuration from validated interfaces. Independently
  check headers as first includes, reference cycles and prefix cleanup; preserve
  handwritten files without an explicit overwrite request.
- [ ] **T056 — Emit the single project assembly.** Needs: T045, T054, T055.
  Order declarations/types/constants/native bodies/runtime/labels under one
  context-parameterized execution function. Compile agreement checks with isolated
  scaffolds; never emit a C function per ordinary Ama module.
- [ ] **T057 — Execute startup, simple calls and returns.** Needs: T056. Create
  one explicit root after constants; dispatch between two module types/multiple
  instances using stable slots and validated return selectors. Begin with calls
  that have no inputs or requested outputs; complete entry bindings in T063–T064.
  Compile and run
  nested same-type calls and early completion without payload save/restore.
- [ ] **T058 — Integrate native entry and lifecycle status.** Needs: T057.
  Enforce entry inlining and direct typed access; return normally from native C
  before status handling. Execute all constructor paths and permitted internal
  helpers; reject unsupported entry implementations with precise diagnostics.
- [ ] **T059 — Integrate runtime refusal and source diagnostics.** Needs: T020,
  T058. Report active source/caller paths before cleanup and preserve per-caller
  refusal snapshots. Run busy/stale/pre-root/native-failure cases, including
  allocation-free reporting and secondary destruction errors.
- [ ] **T060 — Expose check-only and verified compilation commands.** Needs:
  T055, T059. Use one shared frontend and explicit generated-output ownership.
  Test draft reporting, check/compile diagnostic agreement and atomic publication
  of `--emit ... --compile` outputs with the toolchain verifier.
- [ ] **T061 — Expose initial host completion/failure/disposal.** Needs: T060.
  Connect explicit context APIs to generated code and launcher policy. Test
  startup failure, normal completion, prohibited reentry and independent contexts
  with no process-global mutable runtime state.
- [ ] **T062 — Accept the minimal runnable slice.** Needs: T061. Publish a small
  complete transitive Ama/native project with stable-address pool growth, return
  routing and cleanup. Compile/run it under supported flags and relevant sanitizers;
  record exactly which later features remain unsupported.

## Phase 8. Complete calls, result chains and ordinary control flow

Contracts: [calls](spec/language/calls-and-results.md),
[control](spec/language/control-flow.md).

This execution slice covers scalar/reference targets and their control flow.
Concrete whole-list/indexed operands and list-service results complete in Phase 10;
their parsed and typed plans are already available from Phases 3–5.

- [ ] **T063 — Complete entry preparation and defaults.** Needs: T062. Acquire
  target and protect sources before reset; construct supplied/default parameters
  in declaration order and then results. Test self-dependent inputs, omitted
  bindings and failures preserving only the effects promised by CALL-08.
- [ ] **T064 — Complete literal inputs and ordered result receivers.** Needs:
  T063. Select literal construction once, preserve payload bytes and resolve
  output receivers/indices in written order. Execute inferred scalar/reference
  receivers, placeholders and failures before body entry or result publication;
  integrate planned list receivers with their concrete storage in T076.
- [ ] **T065 — Complete retained and constant result reads.** Needs: T064.
  Copy the complete permitted value state without executing/resetting the target;
  preserve ordinary copy clearing. Test qualified constants, inferred receivers,
  partial output progress and busy/stale refusal before receiver changes.
- [ ] **T066 — Execute direct-result operation trees.** Needs: T065. Act on
  actual retained result storage, follow returned references, sequence sibling
  methods depth-first and keep parent protections until all actions finish.
  Test stateful constructor/method examples with no intermediate clone.
- [ ] **T067 — Complete local refusal-handler unwinding.** Needs: T066. Release
  only the initiating operation tree before its failing link's handler, skip
  unfinished siblings and retain prior committed effects. Test empty handlers,
  ancestor boundaries and caller lexical scope throughout nested result actions.
- [ ] **T068 — Complete branches, loops and exact repeat boundaries.** Needs:
  T067. Execute standalone negative branches, empty-body single checks, fixed-root
  linear condition commands and nested break/return. Test conversion errors,
  per-link refusal paths and protection release before loop bodies.
- [ ] **T069 — Accept the synchronous language slice.** Needs: T068. Run
  multi-instance state/method/control examples and source-to-generated-program
  failure compositions. Check constant/default initialization order, no hidden
  retries and no unaccepted syntax introduced by implementation shortcuts.

## Phase 9. Saved execution positions and independent continuation copies

Contracts: [continuations](spec/language/continuations.md),
[return links](spec/implementation/runtime.md#return-links-and-resumption-positions).

- [ ] **T070 — Execute statement-boundary yield/resume.** Needs: T069. Save
  exact structured positions separately from active return links and make yielded
  instances available. Test repeated yields, nested immediate-caller return and
  the distinction between busy Waiting and available Yielded.
- [ ] **T071 — Complete restart and host yield policy.** Needs: T070. Restart
  entry without resetting persistent body fields; resume without inputs/interface
  reset. Test root completed/yielded/failed outcomes, explicit host resume/disposal
  and diagnosed standard-launcher rejection of unhandled root yield.
- [ ] **T072 — Copy continuation control state explicitly.** Needs: T050,
  T071. Preserve allowed owned descendant positions for `%` copies and clear all
  positions for ordinary copies. Test distinct identity requirements, whole-source
  availability and absence of copied busy state, callers or refusal history in
  scalar-owned trees. Membership-aware list/cursor continuation copies complete
  in T084 after their storage and live traversal contracts are implemented.
- [ ] **T073 — Integrate continuation failure and cleanup paths.** Needs: T072.
  Reject native-stack capture, ensure failed contexts cannot resume and destroy
  yielded state without executing its continuation. Exercise native resource
  clone failures and mixed completed/yielded ownership trees.
- [ ] **T074 — Accept resumable generated programs.** Needs: T073. Compile/run
  custom-host and nested same-type continuation examples, including ordinary and
  explicit independent copies. Verify source traces use the new active caller
  rather than historical copied return connections.

## Phase 10. Membership-aware typed lists and live traversal

Contracts: [lists](spec/language/lists.md),
[foreach scope](spec/language/declarations.md#name-06).

- [ ] **T075 — Implement checked list membership records.** Needs: T037, T054.
  Replace or adapt reusable order storage to contiguous membership identifiers
  and stable record pools with current positions. Test duplicate reference
  targets as distinct entries, membership generations and collection incarnations.
  Permit compact local order indices with exact identity reconstruction; saved
  membership/origin references retain every required generation and incarnation.
- [ ] **T076 — Implement indexed selection and complete list copies.** Needs:
  T065, T075. Check conversion/bounds/membership before target effects and preserve
  exact owned/reference transfer rules. Integrate inferred list receivers and
  list-owned clone boundaries deferred by the scalar execution slice. Test
  stale/empty indices, fresh copied memberships, whole-list self no-op and
  replacement invalidation.
- [ ] **T077 — Implement list service acquisition and method frames.** Needs:
  T066, T076. Hold owner/list protection across sibling methods/results, permit
  only exact direct structural helper token borrowing and finalize ephemeral
  result storage once. Test alias/callback reentry refusal and escaped stale views.
- [ ] **T078 — Implement append and insert.** Needs: T077. Support omitted default
  creation versus explicit owning clone or reference-handle copy. Reserve counters
  and prepare before publication; test invalid positions, clone capability, busy
  sources, allocation overflow/failure and unchanged destinations on refusal.
- [ ] **T079 — Implement remove and clear.** Needs: T078. Preflight full affected
  owned subtrees, retire memberships and release resources in specified order.
  Test busy descendants, reference removal without destroying external targets,
  atomic no-partial clear and terminal destruction failure.
- [ ] **T080 — Implement swap and length.** Needs: T079. Preserve membership
  identity through swap and construct length via the fixed `..integer` contract.
  Use PROTO-03's typed `from_size` bridge exactly once, without decimal text or
  double construction; test the full supported host-size conversion boundary.
  Test both method positions, representability, sequential service actions and
  selected-result failures without obsolete retained outcome state.
- [ ] **T081 — Implement live foreach selection and bindings.** Needs: T031,
  T074, T080. Store one fixed lexical cursor/cell per instance, validate source
  incarnation and use short acquisitions. Test empty bodies/lists, absent referents,
  compatible reuse, no-shadow errors and busy live targets not silently skipped.
- [ ] **T082 — Correct every active or suspended boundary on mutation.** Needs:
  T081. Apply exact insert/remove/clear adjustments and unchanged swap boundaries
  in each complete publication. Test nested same-list scans, insert at the gap,
  append, remove/reinsert, clear/append and swap repeats/skips. Record either the
  exact O(K) lexical scan or O(A) active/suspended registration strategy; compare
  corrected states and cover registration cleanup/copy/restart if implemented.
- [ ] **T083 — Enforce missing-membership guards and resume behavior.** Needs:
  T082. Preserve origin associations, detach only as specified and validate active
  loops outermost first after resume. Test removed current entries with live
  external referents, nested reused bindings and collection replacement failure
  before any resumed user statement.
- [ ] **T084 — Copy and accept live traversal state.** Needs: T083. Map full
  collection/membership/origin identities in `%` clones, preserve boundary values
  and discard ordinary-copy control state. Verify removed-origin nonrevival,
  membership/clone failure paths, candidate-potential bounds and complete compiled
  mutation/traversal examples without snapshot arrays or hidden visited sets.

## Phase 11. Coherent library contracts and explanatory examples

Contracts: [library](spec/language/library.md),
[acceptance](spec/implementation/tools-and-conformance.md#acceptance-matrix).

- [ ] **T085 — Validate the scalar and canonical integer library.** Needs:
  T069, T080. Publish actual paths/interfaces and literal/clone/truth/index
  capabilities and the fixed host-size bridge, including the full supported
  length range for `..integer`. Test malformed data and nominal typing without
  automatic numeric assumptions.
- [ ] **T086 — Validate refusal and resource library interfaces.** Needs: T085.
  Provide ERROR-01's one copyable failure-module result: it supports `?` and owns
  a publicly readable description, with false/empty success and true/current
  failure states. Use this value instead of separate success/failure Booleans
  and sibling reason results. Test positive checks, retained description reads,
  independent copies, failed-then-successful reset and description allocation
  failure without changing native technical status or last-refusal semantics.
  Keep refusal reading ordinary/read-only and native resources governed by explicit
  lifecycle/clone contracts. Compile realistic same-name modules and test expected
  application failures separately from context-ending technical failures. For native
  text/byte types using immutable shared backing, verify independent Ama values,
  both destruction orders, replacement isolation, share overflow and rollback.
- [ ] **T087 — Standardize and test library compositions.** Needs: T084, T086.
  Make parameter/result ordering predictable and remove obsolete list action APIs.
  Apply the single condition-capable failure-value convention to operations
  reporting expected failures; keep intrinsic list refusals and ordinary false
  predicates distinct. Test no stale result processing after a refused call.
  Integrate list-valued constant startup/cleanup with T052's context-local scalar
  foundation. Compile real ownership/list/native combinations and document actual
  capability limitations without parser-level domain operation names.
- [ ] **T088 — Publish complete synchronous and continuation examples.** Needs:
  T087. Supply reproducible modules for persistent state, self-reference,
  constructor-owned methods, mutable list traversal and independent simulations.
  Include file-operation failure handling with a returned failure module, a
  positive `?` and a retained read of its description, with no separate flag.
  Execute expected outputs/state changes and explain the governing contracts.
- [ ] **T089 — Review usability through the examples.** Needs: T088. Improve
  diagnostics, source navigation metadata and Doxygen explanations using concrete
  failures and repeated workflows. Record proposed language changes as questions;
  do not alter `spec/` or infer new semantics while improving implementation.

## Phase 12. Compiled Ama libraries and C-module embedding

Contracts: [EMBED-01–EMBED-08](spec/implementation/c-library.md),
[native boundary](spec/implementation/native-interface.md),
[host control](spec/implementation/runtime.md#rt-14).

- [ ] **T123 — Resolve explicit C-library exports and build inputs.** Needs:
  T040, T061, T087. Implement the documented export manifest, explicit component
  identity and target/ABI metadata without scanning module names or exporting all
  internals. Resolve exported entries and supported value schemas through the
  existing typed project. Test missing/duplicate exports, identity collisions,
  unsupported schemas, dependency invalidation and preservation of handwritten
  inputs. Keep the executable launcher and library target separate.
- [ ] **T124 — Generate the public C consumer header.** Needs: T055, T085,
  T086, T123. Emit self-contained namespaced declarations, opaque host identities,
  documented ownership and exact ABI/version checks from one export schema. Keep
  native-authoring headers and private runtime layouts outside this public
  boundary. Compile multiple independent C consumers and incompatible-header
  negatives; cover ERROR-01's single condition-capable failure value through its
  independent C snapshot rather than separate Boolean and reason results.
- [ ] **T125 — Implement exported context and instance lifecycle.** Needs:
  T013, T074, T124. Create independent contexts, initialize their constants and
  explicitly construct exported instances without executing their bodies.
  Implement quiescent release/disposal with checked owner, generation and ABI
  identities. Test partial startup, retained completed/yielded state, cross-context
  access with equal local numbers, nonreentrant execution, failure cleanup and
  exactly-once disposal without automatically running saved continuations.
- [ ] **T126 — Implement typed inputs and independent result snapshots.** Needs:
  T069, T084, T086, T125. Generate checked factories, argument preparation and
  public value reads using the documented supported schemas; publish outputs
  only after successful preparation. No consumer receives internal Ama addresses,
  reference handles or continuation storage. Test byte lengths, empty values,
  malformed/schema-mismatched data, overflow/allocation rollback, repeated retained
  reads and snapshots that remain usable after instance/context disposal. Verify
  a failure snapshot contains both its condition value and owned description.
- [ ] **T127 — Implement exported entry, resume and refusal outcomes.** Needs:
  T067, T074, T126. Preserve the three Ama execution outcomes and report invalid
  C requests or operation refusals through the separate API result. Test restart
  versus resume, output-only resume, busy/absent/stale selections, independent
  application-failure values and context-ending technical failure. Reject foreign
  owners and invalid requests before entering Ama or changing execution state;
  do not introduce a fourth Ama outcome or an implicit scheduling loop.
- [ ] **T128 — Build static and shared components with pinned code lifetime.**
  Needs: T008, T010, T017, T123, T124, T127. Compile the complete Ama closure as
  one C translation unit and package its object as a static or supported shared
  library. Apply the component namespace to every externally linked symbol and
  keep generated internals private. Link two independently generated components
  into a multi-translation-unit C host, including overlapping Ama module names.
  Test target/ABI incompatibility, repeatable build metadata and refusal to unload
  code while contexts, callbacks or component-owned values still require it.
  Exercise lifetime pins with a minimal native probe here; the full asynchronous
  and concurrent component compositions follow in T099 and T106.
- [ ] **T129 — Own a foreign Ama component inside a native Ama module.** Needs:
  T012, T073, T128. Implement an example native adapter that owns an opaque child
  context as a native resource in another compiled component. Configure child
  constants/instances explicitly, transfer only supported independent C values
  and release them on all construction/destruction paths. Omit native clone
  support unless a genuine independent resource clone is supplied; do not copy
  an opaque pointer as value cloning. Test that the parent and child cannot
  exchange checked Ama references or use each other's instance identities.
- [ ] **T130 — Exercise nested suspension and failure supervision.** Needs:
  T074, T086, T129. Execute the exact chain parent Ama entry → native C adapter →
  child component context without reentering either active context. Have child
  yield return normally through C; explicitly return or yield the parent and
  resume each context only after its previous native activation has ended. Test
  retained child state, explicit parent policy for child failure, copied diagnostic
  data surviving child cleanup, independent parent continuation and deliberately
  requested propagation as a parent technical failure. Cover child abandonment/
  disposal without automatically resuming its saved position or retaining borrows.
- [ ] **T131 — Accept standalone and nested C-library consumption.** Needs:
  T089, T128, T130. Publish reproducible static/shared-library host and nested
  Ama/native/Ama examples with generated headers, manifest and exact build/link
  commands. Run independent-header, multi-C-file, two-component, ownership,
  yield/resume, failure, teardown and supported sanitizer tests; map every
  EMBED-01–EMBED-08 rule to current component and composed evidence. Explain the
  difference between same-context Ama calls and a separately compiled child
  component, and document actual supported targets and loader limitations.

## Phase 13. Explicit cooperative tasks and native events

Contracts: [ASYNC-01–ASYNC-12](spec/language/async-and-parallelism.md).

- [ ] **T090 — Define and implement the ordinary task-phase interface.** Needs:
  T074, T087. Use typed Ready/Awaiting/Completed results through existing calls,
  not new syntax or universal task casts. Test phase/yield consistency, first
  entry versus later resume and explicit propagation of nested waiting. Keep
  phase separate from ERROR-01's failure value; test valid terminal description
  data and ensure false failure does not turn an awaiting task into completion.
- [ ] **T091 — Implement visible scheduler ownership and enrollment.** Needs:
  T090. Store task references/owners, run epochs and registration generations in
  declared Ama state. Test restart/removal invalidation, nonwrapping exhaustion
  and rejection of stale enrollments without hidden hooks on ordinary calls.
- [ ] **T092 — Implement native submission and completion storage.** Needs:
  T012, T091. Keep native buffers/resources and service/submission tokens separate
  from Ama identities. Test synchronous completion during submission, token reuse
  and worker/callback paths that queue records without entering Ama.
- [ ] **T093 — Implement registration and readiness reconciliation.** Needs:
  T092. Publish waits before arming, retain completion records, recheck before
  awaiting and match full identities on import. Exhaust small interleavings for
  completion before/after recheck, duplicate/late events and no lost wakeups.
- [ ] **T094 — Implement unique FIFO ready membership.** Needs: T093. Coalesce
  readiness and append newly ready/yielding tasks at the tail. Test observed
  completion order, stable ordering within unordered batches and multiple typed
  queues without hidden references or duplicate task steps.
- [ ] **T095 — Implement bounded scheduling rounds and reactor waits.** Needs:
  T094. Enforce finite positive step/import budgets and queue service bounds;
  import before blocking and report quiescence. Test continuously replenished
  work, wakeup races and event fairness without treating live foreach as a budget.
- [ ] **T096 — Implement registration detachment and native record retirement.**
  Needs: T095. Invalidate visible waits before release and retain native callback
  state until callback completion/acknowledgment. Test task/context disposal,
  restart and slot reuse while late callbacks remain possible.
- [ ] **T097 — Implement cooperative cancellation terminal outcomes.** Needs:
  T096. Wake awaiting tasks and commit exactly one cancellation/completion outcome
  at the declared linearization point. Test races and late-result cleanup through
  safe detachment without interrupting native entries or running continuations
  during destruction.
- [ ] **T098 — Enforce copy and failure boundaries for tasks.** Needs: T072,
  T097. Require fresh enrollment of copied tasks and genuine independent native
  operation clones or noncopyable resources. Test external operation sharing,
  subscriber rights and context-wide technical failure with complete detachment.
- [ ] **T099 — Accept the asynchronous library and supported host.** Needs:
  T089, T098, T131. Publish working event/task examples and declared library/host APIs.
  Exercise reactor integration through the supported arrangement, cancellation,
  finite scheduling bounds and all ASYNC acceptance races under native sanitizers.
  Include exported components with pending native completions: code remains
  pinned until detachment/acknowledgment and owned exported values are released.

## Phase 14. Isolated parallel contexts and explicit messages

Contracts: [PAR-01–PAR-08](spec/language/async-and-parallelism.md#parallelism-uses-isolated-execution-contexts).

- [ ] **T100 — Audit context-local generated and native state.** Needs: T061,
  T099. Verify pools/constants/control/errors are independent and native hooks,
  constant lifecycle and linked dependencies are safe for concurrent contexts.
  Test equal local identities and reject foreign host access; identify dependencies
  requiring serialized access or separate-process execution.
- [ ] **T101 — Execute two contexts concurrently.** Needs: T100. Share generated
  code only, with one exclusive executor per context. Test independent startup,
  slot reuse, failures and event delivery while another context runs; run supported
  data-race tooling in addition to lifetime checks.
- [ ] **T102 — Implement quiescent executor transfer.** Needs: T101. Publish
  state across an explicit synchronization boundary with no live native borrow
  or conflicting access. Test yield/resume transfer, nonreentrant rejection and
  thread-affine resources that require a pinned executor.
- [ ] **T103 — Implement typed message encoding and decoding.** Needs: T086,
  T101. Validate version/schema/size before publishing independent received
  values, excluding Ama identities/control state/native pointers. Test malformed
  data, partial construction cleanup and technical decode failure.
- [ ] **T104 — Implement bounded single-sender/single-receiver channels.** Needs:
  T103. Enforce atomic whole-message acceptance, count/byte limits, FIFO and
  explicit ownership transfer. Test full/rejected sends, capacity release on take
  and no implied cross-channel order or application acknowledgment.
- [ ] **T105 — Integrate channel waits and endpoint shutdown.** Needs: T097,
  T104. Provide backpressure without withholding ready tasks; drain on sender
  close and abort on receiver disposal. Test cancellation, pending events,
  sender/receiver failure and native queue lifetime after context teardown.
- [ ] **T106 — Accept isolated parallel execution and supervision.** Needs:
  T102, T105. Run native-only worker and two-context messaging examples with
  independent failure continuation and explicit supervisor policy. Verify all
  PAR invariants and document process isolation for unsupported native concurrency.
  Include separately generated components and nested child-context resources in
  the namespace, native thread-safety, executor-affinity and code-lifetime checks.

## Phase 15. Reusable exploration, debugging and relocation tools

Contracts: [tools](spec/implementation/tools-and-conformance.md),
[diagnostics](spec/implementation/diagnostics.md).

- [ ] **T107 — Expose reusable editor analysis results.** Needs: T060, T089.
  Reuse original source spans, parser/resolver and structured diagnostics for
  highlighting, completion, definitions and type/signature display. Test incomplete
  drafts and changed files without a second grammar, resolver or type system.
- [ ] **T108 — Implement identity-aware rename.** Needs: T107. Plan edits from
  resolved declarations/references, including qualified constants and scoped
  foreach locals. Test collisions, preserved unrelated bytes and revalidation
  before applying source changes.
- [ ] **T109 — Expose explicit debugger inspection.** Needs: T084, T099, T106.
  Inspect type, persistent/interface state, acquisition, saved positions, cursors,
  refusal and task/context state through authorized metadata. Test no hidden native
  payload access, mutable concurrent inspection or fabricated execution history.
- [ ] **T110 — Build complete relocation discovery and preview.** Needs: T018,
  T040, T107. Enumerate all configured active sources, including external importers
  and unused modules, while keeping drafts opaque. Test exact destination, carried
  roots, cross-root kind changes, collisions and orphan `name+/` preflight.
- [ ] **T111 — Derive relocation identity and reference maps.** Needs: T110.
  Compute complete physical/root/module mappings and resolved path-span edits.
  Test injectivity and graph preservation across cycles, qualified constants,
  unchanged logical identities and importers outside the moved directory.
- [ ] **T112 — Validate staged relocation bytes and configuration.** Needs:
  T111. Preserve all non-path bytes and opaque companion/assets; update only
  documented project settings and invalidate generated outputs. Reparse and
  reresolve staged active files; test line endings, comments/literals and drafts.
- [ ] **T113 — Apply relocation as a recoverable transaction.** Needs: T112.
  Require exclusive access and unchanged original file identities/bytes; stage
  moves/importer/configuration edits across supported filesystems. Inject I/O
  failures and verify exact rollback with originals retained until commit.
- [ ] **T114 — Implement relocation recovery and reporting.** Needs: T113.
  Persist commit/rollback state and recover idempotently after interruption. Test
  incomplete rollback retains recovery data and reports affected paths; never
  report success for a partly applied move or rewrite text twice.
- [ ] **T115 — Accept user-facing tools through real projects.** Needs: T108,
  T109, T114. Run editor/rename/debugger/relocation workflows against complete
  projects and regenerate verified output afterward. Document exact scope,
  preserved opaque files, unsupported editors and actual tool limitations.

## Phase 16. Complete coverage, conformance and release evidence

Contracts: [acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix),
[exact algorithms](spec/implementation/compiler.md#exact-algorithms-and-formal-justification).

- [ ] **T116 — Close every code-file documentation and test gap.** Needs: T005,
  T106, T115, T131. Complete the inventory for compiler/runtime/library/tools,
  exported components, native embedding adapters and headers;
  map every code file to meaningful behavioral or compile tests. Generate complete
  Doxygen output and check every substantial component's contract/proof/evidence.
- [ ] **T117 — Close all requirement and composition gaps.** Needs: T116.
  Update T001's matrix for all 200 requirements and every acceptance row, with
  exact commands and current evidence. Add missing independent leaf, parent and
  generated-program cases; no unsupported feature can be marked complete.
- [ ] **T118 — Verify deterministic and incremental product behavior.** Needs:
  T117. Check repeated output, transitive/configuration/native dependency changes,
  Unicode paths, collisions and source maps across clean/reused builds. Verify
  generated files never overwrite handwritten inputs.
- [ ] **T119 — Verify resource and cleanup boundaries.** Needs: T118. Exercise
  arithmetic overflow, identity exhaustion, allocation/output failures, partial
  construction, destructor errors, stale events and transaction recovery through
  real composed paths. Confirm failures preserve every promised precommit effect.
- [ ] **T120 — Run full supported-toolchain and sanitizer acceptance.** Needs:
  T119. Run build, recursive unit/integration suites, relevant ASan/UBSan and
  parallel race checks; verify machine-level entry inlining/stackless boundaries.
  Record unsupported checks and leak-check limitations precisely.
- [ ] **T121 — Publish reproducible examples and conformance report.** Needs:
  T120. Document supported toolchains, library/native/host APIs, complete usage
  commands and evidence-backed limitations. Rebuild example outputs and generated
  documentation from a clean checkout; request permission for any needed spec change.
- [ ] **T122 — Finish the implementation release checkpoint.** Needs: T121.
  Remove obsolete mechanisms and conflicting derived descriptions, verify links
  and complete clean build/test/documentation reproduction. Reconcile all task
  statuses through this checkpoint and commit the final acceptance evidence
  without claiming unverified behavior or automatically publishing outside the
  repository. All implementation changes must be complete before T135.
- [ ] **T135 — Deliver the final technical documentation folder.** Needs: T122.
  Only after all implementation, cleanup and acceptance work is ready, regenerate
  `docs/technical/` with Doxygen through `make docs`, using the final code and its
  Doxygen comments. Doxygen must generate file/interface descriptions and navigation;
  do not author substitute reference pages manually. Document every
  maintained C source and header, including compiler/runtime components, standard
  library and native companions, C tools, tests and fixtures; reconcile coverage
  with the final file inventory and its documented fixture exceptions. Include
  a readable entry page, architecture/component navigation, per-file and interface
  references, ownership/invariant/failure explanations, clickable source/specification
  links and links to test evidence. Include a coverage manifest with source
  fingerprints and reproducible generation instructions, tool versions and
  configuration. Describe generated C through its generators and verified output
  contracts; temporary build/test outputs are not maintained-source inventory.
  Validate complete maintained-file coverage, all rendered destinations and clean
  regeneration; any later code change requires regeneration and renewed checks.
  Deliver the regenerated folder at that repository path; commit generator inputs,
  configuration and generation evidence. Generated pages remain reproducible
  outputs, not handwritten or versioned source.

## Requirement coverage map

This map routes each indexed requirement family to its implementation tasks.
T001 and T117 maintain the finer requirement-to-file-to-test evidence matrix;
this table is navigation, not a parallel semantic definition.

| Canonical requirements | Main task range |
|---|---|
| CORE-01–CORE-05; ARCH-01–ARCH-06; TEST-01–TEST-08 | T001–T005, T132–T135 and the completion criteria; T116–T122 |
| SRC-01–SRC-05; SYN-01–SYN-06; NAME-01–NAME-03; DEP-01–DEP-03 | T014–T029, T110–T114, T118 |
| NAME-04, NAME-06; TYPE-01–TYPE-06; LIT-01–LIT-04; STATE-01–STATE-03 | T030–T040, T042–T045, T049, T063–T069, T081 |
| CONST-01–CONST-05 | T034, T052, T065, T100 |
| REF-01–REF-08; COPY-01–COPY-07 | T032–T033, T039, T046–T054, T072–T084 |
| CALL-01–CALL-09; FLOW-01–FLOW-04 | T025–T027, T035–T044, T057–T069 |
| FLOW-05; LIST-01–LIST-13 | T031, T037–T044, T075–T087 |
| CONT-01–CONT-06 | T039, T070–T074, T083–T084, T098 |
| NAME-05; CABI-01–CABI-06; PROTO-01–PROTO-11; LIFE-01–LIFE-05 | T008–T013, T028–T055, T058, T072–T073, T085–T086, T097–T105 |
| ERROR-01–ERROR-06; DEBUG-01–DEBUG-04 | T007, T019–T020, T059, T067, T098, T109, T119 |
| RT-01–RT-16; GEN-01–GEN-04; STACK-01–STACK-04; COMP-01–COMP-05 | T006–T013, T041–T074, T100–T102, T118–T120 |
| LIB-01–LIB-02 | T085–T089, T099, T106 |
| ASYNC-01–ASYNC-12 | T090–T099 |
| PAR-01–PAR-08 | T006, T013, T052, T061, T100–T106 |
| EMBED-01–EMBED-08 | T123–T131, T116–T122 |
| TOOL-01–TOOL-04 | T110–T115 |

## Useful verification commands

Use focused suites for the changed component, then its real parent composition.
The normal project entry points remain `make`, `make test`,
`make -C tests/unit/driver/request` and `make test-clean`; exact new leaf paths
belong in the evidence for their owning tasks. Successful leaves clean outputs;
failed leaves preserve them for diagnosis. Run broad checks when required by the
changed contracts or the release phase, rather than repeatedly rebuilding
unrelated components after a documentation-only edit.

The plan is complete when every current requirement has implementation,
independent and composed acceptance evidence, readable generated documentation
in the final `docs/technical/` folder and a recorded successful task commit.
The initial code audit, complete removal of obsolete implementation, retained-code
decomposition and documenting comments must also have passed their explicit gates.
Checkboxes alone are never that evidence.
