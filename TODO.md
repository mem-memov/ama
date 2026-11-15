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
| 7. Runtime ownership | Stable typed block pools; empty/stale/generation handles; reclamation; lifecycle failure cleanup; injective clone maps; staged replacement; copy overlap/availability and busy-descendant preflight. | Context boundaries, reserved self-identity during construction, copy checks across current inferred/result forms and membership-aware list copying. |
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

- [x] **T001 — Audit the implementation delta.** Needs: none. Build a matrix for
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
  Evidence: [audit](docs/implementation/audit/README.md), 586 inventoried paths,
  200 requirement rows, 108 acceptance rows and actual preprocessing dependencies
  of 57 registered test leaves. `make test` passed; eleven isolated source probes
  confirmed current-contract gaps. No production code or specification changed.
  Findings route removals/refactoring/documentation through T136–T151; historical
  sanitizer reports and full current-language conformance remain unclaimed.
- [x] **T002 — Refresh component contracts and boundaries.** Needs: T001. Update
  the responsibility tree and contract registry; identify narrow reusable APIs,
  required decompositions and exact invariant arguments. Give each retained
  responsibility a target subdirectory, owning component, explicit interface and
  migration/test mapping; distinguish reusable behavior from retired contracts.
  Validate that each planned component and assembled parent has an independently
  runnable test home. This plan drives T132–T134 rather than a wholesale rewrite.
  Evidence: [current map](docs/implementation/components.md),
  [boundary invariants](docs/implementation/contracts/current-boundaries.md) and
  [registry](docs/implementation/contracts/README.md). Every retained/planned
  responsibility has independent and real-parent test homes, explicit state/API
  ownership and its current acceptance limit. Existing suites passed in T001;
  this contract-only task checked links, source/API paths and dependency order.
- [x] **T003 — Verify recursive builds and invalidation.** Needs: T002. Confirm
  nested source discovery, header/toolchain/link-membership dependencies, parallel
  make ordering and clean/failure artifact rules. Test no-op and changed-input
  rebuilds through real parent and independent leaf builds.
  Evidence: [build contract](docs/implementation/incremental-build.md), existing
  13 tracking scenarios and two new controlled candidate-isolation/I/O scenarios.
  Fixed shared manifest scratch names: each live Make process now owns its own
  candidate. The old code failed the forced interleaving; the fix passed the new
  independent leaf, real rebuild parent, strict `-Werror`/`-DNDEBUG` checks and
  complete `make -j4 test` (58 registered leaves). Generated Doxygen reference and
  local links for all three new C file pages were checked. Sanitizer/failure
  infrastructure verification continues in T004; no language support is inferred.
- [x] **T004 — Verify the shared failure/test infrastructure.** Needs: T003.
  Maintain C leaf `main`/`it_*`/`run`, guarded fixtures, assertions under release
  flags and deterministic failure injection. Test harness failures, cleanup and
  sanitizer invocation without introducing a second test framework.
  Evidence: [harness revalidation](docs/implementation/test-harness.md#t004-revalidation-and-deterministic-failures).
  Nine real harness lifecycle/failure scenarios passed under strict release flags;
  ASan/UBSan passed the pool leaf and all four object-parent leaves. Existing
  allocation/native-failure injection and live-resource balances were reviewed
  and exercised. LeakSanitizer failed with its ptrace diagnostic; the explicit
  `detect_leaks=0` profile retains address/UB checks and claims no leak certification.
  The existing correct harness was reused, not rewritten or replaced.
- [x] **T005 — Establish code-to-tests and generated documentation checks.**
  Needs: T004. Inventory every project code/header file and its test coverage;
  register uncovered files for the owning tasks. Extend the `make docs` Doxygen
  build from docs/Doxyfile with reproducibility checks and checks for
  purpose/contract comments, independent headers and documentation
  links, including source-relative rule links and deterministic read-only Doxygen
  anchor mapping. Test actual rendered destinations and stale-link failures.
  Validate the workflow on existing small components before expanding it.
  Evidence: [checked workflow and invariants](docs/implementation/documentation/README.md),
  current ready/debt registries and the independent documentation C/assert leaf.
  All 200 canonical rules map through read-only views to checked HTML destinations;
  actual originating pages and canonical chapters are verified. Two complete
  Doxygen 1.11.0 generations were byte-identical. Standalone status-header inclusion,
  existing native-status tests, strict documentation tests and complete
  `make -j4 test` passed (59 registered leaves). The bootstrap explicitly checks
  18 ready inputs and assigns 582 remaining input debts to T145–T151; the full
  `docs-complete` gate fails until they close. Source navigation was exercised with
  VS Code's real `--goto`; no GUI-click or complete documentation claim is made.
- [x] **T136 — Retire the old list dispatcher across its consumers.** Needs:
  T005. Remove action/outcome modules, configuration flags, fixed positional
  interfaces, dispatch requests, obsolete list result fields and unsupported
  action IR/emission paths identified by audit F02. Migrate retained consumers
  and keep contract-independent capacity, bounds and ownership operations with
  explicit component tests. Replace obsolete-only expectations; do not claim
  current typed methods are implemented before T075–T084. Check CLI diagnostics,
  library closure, clean build and affected runtime/semantic/emission compositions.
  Evidence: [retained storage contract](docs/implementation/lists.md). Removed
  the dispatcher and its frontend/semantic/IR/emission configuration, both native
  action/outcome module families and retained list result flags, including clone
  and transfer copies. Direct storage operations return status and preserve
  checked capacity/range/lifecycle machinery. Complete `make -j4 test` passed
  (59 leaves); seven storage cases passed strict release and ASan/UBSan profiles
  (`detect_leaks=0`). CLI rejects all four removed mappings; semantic checks
  reject whole-list execution/retained results before IR. Clean compiler build,
  independent header checks and Doxygen's actual rule/file destinations passed.
  Typed services, membership/cursor behavior and length remain T075–T084.
- [x] **T137 — Remove conflicting derived descriptions and proposals.** Needs:
  T136. Close audit F09 after T002's refreshed map: update library/native/build
  descriptions to their actual current boundaries, repair links to authoritative
  spec files, and remove superseded list conversion/dispatcher proposals. Preserve
  useful historical execution evidence with its limits; retain no alternate API
  contract. Check every maintained documentation link and obsolete description
  identified by the audit without editing `spec/`.
  Evidence: corrected library/check/native/build/driver descriptions and explicit
  scope notices on retained component arguments. Removed the superseded conversion
  proposal; no selector/outcome mapping remains a published API. Source truth and
  grammar links point into `spec/`, current emission/continuation compositions
  replace obsolete “no backend” claims, and old phase-number references were
  replaced with boundaries or stable tasks. Checked all 73 maintained Markdown
  files and 997 local prose links/anchors, the 200-rule registry, `make help`,
  `make docs-check` and whitespace consistency. No canonical specification changed;
  no compiler-semantic or runtime test rerun is claimed for this documentation task.
- [x] **T132 — Remove the obsolete implementation completely.** Needs:
  T136, T137.
  Complete the removal tasks identified by T001/T002: delete retired code paths,
  duplicate superseded implementations, obsolete-only tests, build registrations
  and conflicting derived descriptions. Update all retained consumers; leave no
  compatibility dispatcher, dormant alternative implementation or stale source
  registration for a retired contract. Preserve conforming reusable behavior and
  its valid evidence; needing a later feature adaptation alone does not make code
  obsolete. Reconcile every removal against the audit inventory and verify a clean
  build, independent affected suites and their real parent compositions. Keep
  this gate open until every obsolete implementation item has been removed.
  Evidence: [exhaustive removal reconciliation](docs/implementation/audit/removal-closure.md).
  All eleven retire/remove decisions are closed: ten audited paths removed and
  the storage header rewritten. Retired API/IR/result/configuration symbols and
  source registrations are absent; only negative tests retain removed CLI flag
  spellings. T136's clean build, independent affected suites, full 59-leaf parent
  composition and Doxygen/header checks remain valid for the unchanged code.
  T137 closes conflicting prose/proposals. Reused those recorded outcomes and
  rechecked inventory, symbols, links and `docs-check`; no new runtime behavior
  or complete language conformance is claimed by this gate.
- [x] **T138 — Separate driver option recognition and request assembly.** Needs:
  T132. Decompose the phases in `src/driver/options/parse.c`: exact option
  recognition, owned argument storage, request-specific validation and root/output
  configuration. Use narrow child contracts, valid Doxygen comments and rule
  links; preserve borrowed argument lifetimes and allocation-failure cleanup.
  Test invalid/duplicate/missing options and real check/native/emit CLI composition.
  Evidence: [option domain, invariants and refinement argument](docs/implementation/driver-options.md).
  Extracted exact table recognition, scoped array ownership/publication, request
  validation and argument consumption; the public options header no longer
  imports the frontend. Five independent cases cover the full 4×10 recognition
  relation, repeat policies, missing/invalid inputs and every allocation failure.
  Strict release, ASan/UBSan (`detect_leaks=0`), actual driver parent, check/native/
  emit compositions and complete root `make -j4 test` passed (60 leaves).
  Five standalone headers, named test interfaces and Doxygen's actual destinations
  passed. All extracted files have purpose/contract comments and canonical rule
  links; the current registry exposes 35 ready inputs and 567 remaining debts.
- [x] **T139 — Separate native publication transaction phases.** Needs: T132.
  Decompose `src/native/scaffold/output/publish.c` into narrow directory/lock,
  preflight, staging, publication and rollback/cleanup responsibilities. State
  descriptor/path/backup ownership and the exact commit point. Keep orchestration
  small and document the phase invariants with rule links. Exercise real filesystem
  conflicts and staged failures through independent and assembled publication tests.
  Evidence: [publication contracts and prefix argument](docs/implementation/native-publication.md).
  Extracted frontend-free artifact/IO records and directory, exclusive lock,
  preflight-policy, temporary/stream/backup, publication and recovery children.
  The parent alone adapts the real manifest model; child tests link no frontend.
  Five filesystem and five real-transaction cases check exact bytes/mode, policy
  errors, preparation failures, ownership, every selected link/rename failure,
  prefix restoration and retained backups when restore fails. Strict caller-flag
  tests, the actual scaffold parent, ASan/UBSan filesystem checks (`detect_leaks=0`)
  and full `make -j4 test` passed (62 leaves). Standalone headers, all canonical
  links and 462 generated file pages passed; 52 ready inputs/566 debts are explicit.
- [x] **T140 — Separate control and descriptor emission responsibilities.** Needs:
  T132. Decompose `src/codegen/control/emit.c` and `data.c` by resolved control/data
  operation responsibilities. Keep source locations, label identities, native-entry
  boundaries and failure routing explicit; do not infer missing IR from C text.
  Add Doxygen contracts/rule links and compile/run real emitted projects, including
  output errors and native inlining rejection, before accepting the refactoring.
  Evidence: [control translation and exact-site argument](docs/implementation/control-emission.md).
  Split scalar/location/position/call writers, model-level descriptor families and
  unique IR queries; native output no longer receives an unused scaffold owner.
  Required input/interface sites cannot fall back to an acquisition selector;
  missing/duplicate required IR fails before publication. Explicit unused metadata
  preserves read/resume/state behavior. Five standalone cases execute typed C
  fragments, source locations and real stream failures. Corrupted-plan assembly,
  real execution/continuation leaves, negative native inlining and complete root
  `make -j4 test` passed (63 leaves). Header checks and 474 Doxygen file pages passed;
  68 ready inputs and 563 remaining documentation/coverage debts are explicit.
- [x] **T141 — Separate object construction phases.** Needs: T132. Refine
  `src/runtime/objects/construction/build.c` into narrow reservation, constructor,
  field-construction, publication and unwind operations. Document initialized-prefix
  and reserved-identity invariants, ownership and failures with canonical links.
  Verify lifecycle ordering, every allocation/constructor failure boundary and
  real parent creation; refactoring does not substitute for T050's new semantics.

  Evidence: [private construction argument](docs/implementation/object-construction.md).
  Split reservation, field/child preparation, payload construction, publication
  and unwind from the iterative driver. Five independent phase cases and the
  existing seven lifecycle cases cover pre/post-reservation failures, allocation
  prefixes, constructor failures, reverse cleanup and explicit continuation fixups.
  Strict standalone tests, full root `make -j4 test` (64 leaves), and the phase
  leaf under ASan/UBSan passed (`detect_leaks=0`; no LSan claim). Doxygen checked
  483 file pages and canonical links; 79 ready inputs and 562 explicit debts remain.
- [x] **T142 — Separate clone preparation phases.** Needs: T132. Refine
  `src/runtime/objects/clone/prepare.c` around capability/access checks, native
  preparation, child work and map bookkeeping. Keep the single operation-scoped
  injective mapping and rollback ownership explicit. Add linked Doxygen arguments
  and independently test failure transitions plus full clone/replacement composition.

  Evidence: [clone preparation argument](docs/implementation/clone-preparation.md).
  Separated private reservation, child work, native/service copying and complete
  boundary-scoped reference fixups. Existing exact source/capability admission
  remains before native hooks. Four standalone cases cover every measured reserve
  allocation failure, late field failure, uncredited child ownership, two copies
  of one source, replacement root redirection and each native failure in a
  three-node tree. Strict phase tests, ASan/UBSan (`detect_leaks=0`) and full root
  `make -j4 test` passed (65 leaves), including clone/replacement/forest/generated
  consumers. Doxygen checked 489 file pages; 89 ready inputs and 559 debts remain.
- [x] **T143 — Separate invocation input preparation phases.** Needs: T132.
  Decompose `src/runtime/program/calls/prepare.c` into source selection, source
  protection, affected-field preflight and declaration-order installation.
  Document alias/borrow/cleanup contracts and CALL-08 correspondence. Test repeated
  sources, overlap, defaults observing earlier inputs, one selected constructor,
  partial installation failure and actual nested call composition.

  Evidence: [entry preparation phases](docs/implementation/entry-preparation.md).
  Separated once-only text-order source selection, owned-source protection,
  all-field conflict checks and parameter/result declaration-order installation.
  Four standalone cases cover indexed/repeated sources, borrowed authority across
  field commits, empty-list/ancestor overlap, defaults observing installed values,
  diagnostic locations and partial failure. Strict phase tests, ASan/UBSan
  (`detect_leaks=0`) and full root `make -j4 test` passed (66 leaves), including
  allocation-prefix failures and generated nested calls. Doxygen checked 495 file
  pages; 97 ready inputs and 558 explicit documentation/coverage debts remain.
- [x] **T144 — Separate value selection and list transfer phases.** Needs: T132.
  Refine `src/runtime/program/values/source.c` and `transfers/lists.c` into exact
  selection, validation, preparation, fixup, commit and cleanup responsibilities.
  Keep owned/reference/shared selection and lifetime boundaries distinct, with
  linked Doxygen contracts. Test whole-forest reference remapping, alias/failure
  boundaries and generated-program consumers; do not preserve retired list state.

  Evidence: [selection and forest-transfer argument](docs/implementation/value-selection-and-list-transfer.md).
  Split shared/instance/index/payload selection and list validation, staging,
  complete-map fixup, destructive commit and cleanup. Successful commit explicitly
  consumes staging ownership; failure leaves it available for prefix cleanup.
  Three selection and four transfer phase cases cover exact protocol effects,
  numeric-status/refusal separation, private cross-root fixups, self-copy,
  parent/destructor failures and busy/stale/empty reference handles. Strict phase
  leaves, ASan/UBSan (`detect_leaks=0`) and full root `make -j4 test` passed
  (68 leaves), including forest allocation-prefix and generated consumers.
  Doxygen checked 511 file pages; 118 ready inputs and 555 explicit debts remain.
- [x] **T133 — Decompose the retained implementation.** Needs: T138–T144. Complete
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

  Evidence: [decomposition closure](docs/implementation/audit/decomposition-closure.md)
  and [exact maintained-input receipt](docs/implementation/audit/decomposition.csv).
  Reconciled all 673 code/build/fixture inputs (347 under `src/`) with explicit
  responsibility homes, contracts, current hashes and ready/debt ownership.
  T138–T144 independently committed the seven identified phase splits and their
  component/real-parent checks. T144's unchanged-source root run passed 68 leaves;
  this documentation gate does not claim another runtime run. Exact inventory,
  hash, contract/consumer-location and Markdown checks passed. `make docs` checked
  511 file pages and 118 ready records; 555 debts remain assigned to T145–T151.
- [x] **T145 — Document source, identity and diagnostic foundations.** Needs:
  T133. Cover `src/source/`, `src/diagnostics/` and the Unicode generator, including
  headers and checked-in data provenance. Explain purpose, exact algorithms,
  ownership/failure contracts and source/identity rule links. Reconcile each file
  with meaningful independent and parent tests; verify the Doxygen output and links.

  Evidence: linked Doxygen purpose/type/function contracts cover all 45 source,
  identity, diagnostic and generator inputs; all 16 headers compile independently.
  The 44 source/diagnostic C/header/include inputs preprocess identically before
  and after comments; every XID range declaration is unchanged. Three new arena
  cases verify alignment, stable lifetime, checked failure and system authority.
  The Unicode leaf now regenerates exact tables and rejects edition/range/output
  failures. Its readonly-output case failed before the generator checked final
  fflush and passes with that fix. Doxygen's .inc-as-PHP default was caught by
  actual originating-page validation, corrected to inc=C and regression-tested.
  Strict source/diagnostic/composition suites, arena ASan/UBSan (`detect_leaks=0`),
  documentation tests and full root `make -j4 test` passed (69 leaves). `make docs`
  checked 512 file pages and canonical links; 167 ready records and 508 debts remain.
- [x] **T146 — Document lexical, AST and parser components.** Needs: T133.
  Cover `src/lexer/`, `src/ast/` and `src/parser/`, including internal headers.
  Explain byte/span ownership, iterative frame boundaries, syntax recognition and
  diagnostics with exact specification links. Identify current implementation gaps
  honestly; verify file-level tests and rendered documentation without claiming
  unsupported current forms are already accepted.

  Evidence: linked Doxygen contracts cover all 34 lexer/AST/parser inputs and
  their ten independently compiled headers; current lexical/control/result-tree
  gaps remain explicitly assigned to T022–T028. Thirty-one inputs preprocess
  identically apart from the three intentional retirement edits. Removed the
  remaining fixed-positional list parser rule and its expectation key; structural
  named bindings still fail actual whole-list semantic execution. The regression
  failed during parsing before the fix and now reaches the correct semantic
  rejection. Strict lexer/parser/semantic leaves and full root `make -j4 test`
  passed (69 leaves), including actual grammar/native and generated consumers.
  Corrected task references for pending path/root encoding to T016/T017.
  Doxygen checked 512 file pages; 202 ready inputs and 473 explicit debts remain.
- [x] **T147 — Document dependency, semantic and planning components.** Needs:
  T133. Cover `src/project/`, `src/semantic/`, `src/planning/` and `src/lowering/`.
  Explain namespace/graph distinctions, fixed points, typed plans, source ordering,
  storage/control ownership and failure invariants. Link exact rules, map meaningful
  component/composition tests and validate generated reference destinations.

  Evidence: linked Doxygen contracts cover all 58 project/semantic/plan/lowering
  inputs and 13 independently compiled headers. Loading closure, eager ownership,
  shared initialization and greatest-fixed-point copy capability remain distinct;
  current inferred/result/control gaps stay assigned to their feature tasks.
  Fixed COPY-06's discovered same-spelling exemption: every owned copy checks
  capability, no static TRANSFER_NOOP removes IR replacement phases, and runtime
  identity decides ordinary self no-op. Direct continuation self-copy uses exact
  resolved nonindexed declaration identity rather than spelling. Semantic/IR
  regressions failed before the fix; scalar/list/indexed/transitive resource and
  reference cases, generated self copies and decoder failures now pass. Fifty-five
  other production inputs retain identical C after compiler comment removal.
  Strict project/semantic/planning leaves, generated execution and full root
  `make -j4 test` passed (69 leaves). Doxygen checked 512 file pages and canonical
  links; 262 ready records and 413 explicit documentation/coverage debts remain.
- [x] **T148 — Document native contracts, scaffolds and toolchain checks.** Needs:
  T133. Cover `src/native/` and its headers. Explain typed hooks, scoped names,
  borrowed views, artifact ownership, transactional publication and verification
  assumptions with canonical links. Reconcile per-file ABI/compile/runtime tests
  and validate Doxygen pages rather than merely adding comment markers.

  Evidence: native ABI/status/namespace/scaffold/ownership/toolchain contracts now
  cover all 45 retained native inputs and 15 independently compiled headers,
  including T139's already verified publication phases. Corrected a generated ABI
  bug: compatible shared + and - C helpers still need both mandatory records.
  The actual staged-table round-trip regression failed before the fix and now
  passes while C definitions remain deduplicated. Removed the signature writer's
  unused type argument and updated real assembly/execution consumers. Forty-one
  other native inputs retain identical compiler-lexed C. Strict native parent,
  independent toolchain probes and full root `make -j4 test` passed (69 leaves).
  Doxygen checked 512 file pages/canonical links; 295 ready inputs and 380 explicit
  debts remain. Native C borrowing/purity and unsupported context/export work are
  stated as contracts/remaining tasks, not inferred from compatible prototypes.
- [x] **T149 — Document retained runtime components.** Needs: T133. Cover every
  `src/runtime/` C/header file, including assembly-only inputs. Explain local
  identity, pool/lifecycle, ownership, clone/replace, interface/control and diagnostic
  boundaries and their exact invariants. Distinguish existing behavior from pending
  context/membership/foreach work. Map direct tests or actual generated-program
  consumers and validate source/rendered rule links.
  Evidence: [runtime contract](docs/implementation/runtime-components.md) and
  per-file ready records cover all 98 retained runtime inputs, including the
  assembly-only helpers. Added purpose, type and operation contracts to the 58
  remaining inputs; compiler-lexed C is identical to the preceding commit.
  Exact generation retirement, permanent reclamation history, initialized-prefix
  cleanup, protected copies, replacement-bank ownership, immediate-caller return
  and pre-cleanup diagnostics are explained with explicit context/list limitations.
  Strict runtime, native metadata and lifecycle parents passed; actual generated
  assembly/execution/continuation/library consumers passed through the codegen
  parent. Doxygen checked 512 maintained file pages and canonical links;
  353 ready inputs and 322 explicit documentation debts remain.
- [x] **T150 — Document driver, compiler and generated-code components.** Needs:
  T133. Cover `src/main.c`, `src/driver/`, `src/compiler/` and `src/codegen/`.
  Explain explicit stage contracts, option/output ownership, IR correspondence,
  native boundaries and emitted failure paths. Cite canonical rules and record
  actual CLI/generated-project tests, including the executable-only entry file;
  validate all file/interface pages and rendered references.
  Evidence: [frontend/emission contract](docs/implementation/compiler-and-emission.md)
  and ready records cover all 68 inputs, including main through 12 actual-process
  cases. Added purpose/type/operation contracts to the 43 remaining production
  inputs; compiler-lexed C is unchanged. Documented stage borrow/destruction order,
  exact local/qualified identities, IR label correspondence, stream failures,
  object verification and the retained single-owner host limitation. Strict root
  `make -j4 test` passed all 69 leaves after the negative scalar library cases
  gained exact failing-field diagnostic checks (the previously unused shared
  checker had broken `-Werror`). Doxygen checked 512 maintained file pages and
  canonical destinations; 396 ready inputs and 279 explicit T151 debts remain.
- [x] **T151 — Document library, tests and build support.** Needs: T133. Cover
  remaining native library sources/headers, test executables/shared fixtures,
  ABI/module inputs and Make infrastructure. Explain public contracts, fixture
  purpose and build/test responsibilities using syntax valid for each format.
  Keep malformed byte-exact cases unchanged and document their exact paths
  externally. Reconcile direct/native/process consumers and any remaining
  per-file gaps, then generate and validate the complete retained-code reference.
  Evidence: [library/fixture/build contract](docs/implementation/library-tests-and-build.md),
  purpose/type/helper comments and named actual consumers close all 279 remaining
  records. All 132 changed C/header/include inputs retain identical compiler-lexed
  code; 47 declarations/expected observations retain identical bytes and have
  exact path-specific explanations. Headers compile with their documented context;
  the deliberate check-only #error header has independently verified rejection.
  Fixed inventory retention after debt closure and added exact narrative-page
  mapping without weakening mandatory compiler rule links. Strict root tests
  passed all 69 leaves; focused strict documentation regressions cover each new
  checker boundary. Doxygen validates 512 maintained file pages and 224 canonical
  page/rule destinations. All 675 maintained inputs are ready and debt is empty;
  remaining whole-reference diagnostics/quality checks belong to T134.
- [x] **T134 — Complete documenting comments for all retained code.** Needs:
  T145–T151. Close the initial inventory's documentation and per-file test gaps,
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
  Evidence: [strict reference workflow](docs/implementation/documentation/README.md).
  `make docs-complete` checks all 675 maintained records with no debt. Added the
  remaining 77 extracted function/type explanations and completed partial parameter
  contracts. Every extracted XML function/typedef/enum/class/struct/union now needs
  a real description; indexed records cannot disappear through missing/empty XML.
  Doxygen warnings fail generation. Narrative page mapping and CommonMark heading
  parent-tree compaction preserve source/specification bytes and line counts.
  Strict documentation tests reject missing descriptions/indexed files, malformed
  XML, warnings and incorrect mappings, including Unicode LF-offset boundaries.
  `make docs-reproducible` produced two byte-identical complete outputs: 512 file
  pages, 2112 described declaration records, all 284 page/rule destinations and zero
  warnings. Forty-five changed C/header inputs retain identical compiler-lexed code
  to the passing T151 69-leaf baseline. No spec file was changed. T135 only regenerates
  the final reference after the remaining implementation work.

## Phase 1. Fix execution-context and native boundaries before consumers

Contracts: [runtime](spec/implementation/runtime.md),
[native interface](spec/implementation/native-interface.md),
[diagnostics](spec/implementation/diagnostics.md),
[context isolation](spec/language/async-and-parallelism.md#par-01).

- [x] **T006 — Establish context ownership and identity.** Needs: T134. Define
  explicit context-local pool/constant/error/control ownership and checked host
  identities. Test lifecycle, nonwrapping context identity, rejection of foreign
  access with colliding local numbers and nonreentrant executor entry.
  Completed: [context ownership contract](docs/implementation/execution-contexts.md).
  [AMB-001](AMBIGUITIES.md#amb-001) is resolved by the authorized LIFE-05 root-order
  clarification. Eighteen component cases cover isolated resources, exact context
  identity, executor admission, failure prefixes and reverse root cleanup across
  reuse/replacement. The strict full suite (`-DNDEBUG -Werror`) and focused
  ASan/UBSan pass (leak detection disabled). Documentation checks register all
  694 maintained inputs with zero debt; Doxygen documents 530 files and 2207
  interfaces with all 285 canonical/page destinations, zero warnings and identical
  bytes across two complete generations. Generated host wiring remains T061.
- [x] **T007 — Validate status and refusal records.** Needs: T006. Preserve the
  common code/message/length ABI, allocation-free immutable messages and distinct
  invalid-index/refusal/fatal paths. Test zero success, malformed native status,
  nested caller refusal preservation and diagnostics during allocation failure.
  Completed: [checked native statuses](docs/implementation/native-status.md).
  Malformed encoding normalizes to static InvalidState; only valid InvalidIndex
  conversion selects refusal, with failed outputs unused and caller snapshots
  preserved. ABI/metadata/selection/context cases and real heap-failure diagnostics
  pass; four focused ASan/UBSan leaves pass with leak detection disabled. The
  strict full suite (`-DNDEBUG -Werror`) passes. Doxygen checks 695 maintained
  inputs, 531 files, 2216 interfaces and all 285 destinations with zero debt,
  zero warnings and byte-identical complete generations. All-hook native adapters
  remain T009; provider-namespace emission is explicitly covered by T010/T055.
- [x] **T008 — Validate the supported C execution contract.** Needs: T007. Record
  toolchain/target settings and exact checks for required native-entry inlining
  and label dispatch. Compile positive internal-helper/recursion probes and reject
  surviving native-entry calls or Ama function-call dispatch.
  Completed: [toolchain conformance](docs/implementation/native-toolchain.md).
  Real compilation, the project compilation database and O0/O2 probes share one
  required profile. Exact native and resolved IR control-entry symbol checks run
  before linking/publication. Independent JSON execution, weaker user-option
  rejection, unsupported configuration diagnostics and retained C-call dispatch
  negatives pass. Ordinary view binders use permitted C helper calls instead of
  requesting mandatory inlining. The strict full suite and ASan/UBSan symbol/
  generated-execution leaves pass (leak detection disabled). Documentation checks
  cover 700 inputs, 536 files, 2229 interfaces and all 285 destinations with zero
  debt/warnings and byte-identical complete Doxygen generations.
- [x] **T009 — Validate all six native protocol adapters.** Needs: T008. Freeze
  typed signatures, byte views, includes, alignment and void-entry service status
  in the ABI harness. Compile and run every hook, wrong-signature rejection,
  unsupported optional capabilities and invalid-index versus fatal conversion.
  Supply a conforming `..integer` test module with literal/clone/index support
  and the fixed typed `from_size` constructor over the full host-size range;
  compile its signature and test exact round-trips for later list consumers;
  T085 later validates the production scalar library.
  Completed: [checked native protocol composition](docs/implementation/native-protocols.md).
  Raw-success constructor credits survive transport rejection; object/constant
  cleanup consumes each credit once; condition/entry failure stops output use.
  Size recipes construct owned fields then invoke the bridge once. Ten runtime
  cases, six integer ABI cases, eight negative signature probes and generated
  malformed-completion cases pass. Existing semantic cases reject unbound optional
  capabilities. The strict full suite and four focused ASan/UBSan leaves pass
  (leak detection disabled). Paired O0/O2 probes retain an ordinary inline-hinted
  C helper while rejecting uninlinable entry; global Winline enforcement is excluded.
  Complete documentation checks 724 inputs, 555 files, 2292 interfaces and all
  286 destinations with zero debt/warnings and byte-identical Doxygen generations.
  Standard-root qualification and actual generated bridge binding remain T017/T055.
- [x] **T010 — Validate native symbol namespaces.** Needs: T009. Reuse complete
  module identity for qualified author names and clean inclusion contexts. Use
  fixed collision-checked prefix vectors in this isolated harness; connect the
  current identity provider after T017 in T055. Include collision-free native
  error-provider namespace allocation from sorted complete identities, reserved
  core-code separation and explicit namespace exhaustion. Test
  same-basename hooks/types/helpers, macro cleanup and consistent header/body uses
  in one translation unit without parsing or rewriting arbitrary C.
  Evidence: isolated full-key provider planning, nonzero ordinal capacity and
  exact lookup are implemented under `src/native/status/providers/`. Frozen
  root-aware Unicode/multiword prefix vectors pass every discovery order and
  injected hash collision; heap ordering passes every permutation through size
  eight. Unchanged root-distinct same-basename companions compile and execute
  together with reverse header/body order, checked module errors and macro cleanup.
  Missing/duplicated/unclean contexts require precise compiler diagnostics.
  The full strict suite, three focused ASan/UBSan leaves (leak detection disabled),
  739-input complete coverage, Markdown links and warning-free reproducible
  Doxygen pass: 569 maintained file pages, 2350 interfaces and 286 destinations.
  Actual current-source identity/dependency emission remains T017/T055.
- [x] **T011 — Validate lifecycle state transitions.** Needs: T009. Establish
  fresh/constructing/live/finalized bookkeeping, one selected constructor,
  reverse cleanup and terminal destroy failures. Inject failure at every
  construction/cleanup boundary and prove at-most-once finalization.
  Evidence: the counted lifecycle model covers all four constructor paths,
  zero through three children, every constructor failure boundary and actual
  allocation ordinal, all 16 cleanup-error subsets per constructor, distinct
  secondary codes and exact primary message preservation. Illegal transitions
  preserve credits; SIZE_MAX prefix arithmetic is checked without overflow.
  Real pool hooks observe prior invalidation, live owned children, consumed
  native credit and refused reentry; every destructor failure position finishes
  cleanup without retry. Existing core transitions remain sufficient; their
  documented invariants are strengthened. The full strict suite, focused
  ASan/UBSan model/object leaves (leak detection disabled), complete coverage
  of 741 maintained inputs, Markdown links and warning-free byte-reproducible
  Doxygen pass: 571 maintained files, 2365 interfaces and 286 destinations.
- [x] **T012 — Validate native borrow and resource boundaries.** Needs: T010,
  T011. Expose narrow generated typed access, separate declared Ama graph state
  from native payloads and bound borrow lifetimes. Exercise native copies and
  access failures without retaining Ama addresses, handles or callback views.
  Evidence: a linear allocation-free native borrow owner admits only the exact
  current Running receiver and closes before dispatch, reporting or cleanup.
  Generated binders check expected nominal type before C layout access and keep
  failed partial views private. Runtime cases cover empty/stale/foreign/Busy
  references, unchanged outputs, expiry, reentry and independent graph/native
  clones. Independent C clients reject input mutation, equal-layout nominal
  substitution and private reference/list storage access. Real generated host
  probes observe closed gates on success and malformed native completion.
  The full strict suite, three focused ASan/UBSan leaves including instrumented
  generated programs (leak detection disabled), complete 748-input coverage,
  Markdown links and warning-free reproducible Doxygen pass: 577 maintained
  files, 2386 interfaces and 287 destinations. Current-source service families
  remain T055/T058 and the list roadmap; public embedding remains T123 onward.
- [x] **T013 — Validate the host/context lifecycle interface.** Needs: T006,
  T007, T011, T012. Define completed/yielded/failed outcomes and startup/resume/
  disposal ownership. Test concurrent/reentrant-entry refusal, failed nonresume,
  disposal states and independence of two sequential contexts before threading.
  Evidence: checked admission returns separately from accepted preparation/body
  outcomes; entry prepares interface fields, resume consumes only its saved
  selector, and finish validates the host-return metadata. Failed execution
  reports before allocation-free root/constant cleanup, clears descriptor views
  and retains only a diagnostic record until explicit disposal. Eleven host
  cases cover exact primary status, every measured preparation allocation,
  rejected requests, malformed outcomes, native-borrow closure, yielded disposal,
  independent contexts, concurrent/alternate-root/callback refusal and a real
  acquire/prepare/connect trace before cleanup. The full strict suite, focused
  host/context ASan/UBSan leaves (leak detection disabled), complete 757-input
  coverage, Markdown links and warning-free reproducible Doxygen pass: 585
  maintained files, 2419 interfaces and 288 destinations. Generated context
  dispatch remains T061; exported public C transport remains T123 onward.

## Phase 2. Source storage, Unicode identities and diagnostics

Contracts: [source and names](spec/language/source-and-names.md),
[compiler identity/dependencies](spec/implementation/compiler.md).

- [x] **T014 — Verify original-byte source storage.** Needs: T134. Preserve spans,
  physical line endings, optional final newline and byte-based diagnostic columns.
  Test EOF, empty files, CRLF and allocation failure without fabricated bytes.
  Evidence: original byte/path ownership and every position through EOF pass a
  direct physical-line model for all 55,987 inputs through length six over six
  representative byte classes. Empty/unterminated/CRLF/Unicode loads, unchanged
  failure outputs and borrowed-input mutation are covered. Source length SIZE_MAX
  rejects before allocation. The read buffer now separates exact bounded planning
  from alias-safe commit, saturating its last geometric growth instead of entering
  an exact-size tail; proofs cover full-domain bounds and linear total copying.
  Every measured multichunk allocation failure preserves prior published sources.
  The full strict suite, source/diagnostic ASan/UBSan leaves (leak detection
  disabled), complete 763-input coverage, Markdown links and warning-free
  reproducible Doxygen pass: 591 maintained files, 2432 interfaces and 288 destinations.
- [x] **T015 — Verify strict Unicode recognition.** Needs: T014. Pin Unicode XID
  data and strict UTF-8 decoding; preserve byte identity without normalization.
  Test malformed encodings, boundary scalars and multiword international names.
  Evidence: Unicode 17.0.0 remains pinned with an exact source-checksum case,
  whole-domain XID comparison and byte-identical offline table regeneration.
  The new allocation-free complete-name predicate composes exact XID words with
  single ASCII spaces, retaining original spelling and precise error offsets;
  token/path integration remains T016/T022. Cases cover every scalar after a
  word separator, all 65536 two-byte views, all nonminimal widths, every out-of-
  range four-byte payload, invalid continuation classes and unchanged truncation
  outputs, alongside existing canonical-scalar/surrogate enumeration. The full
  strict suite, Unicode ASan/UBSan leaf (leak detection disabled), complete
  767-input coverage, Markdown links and warning-free reproducible Doxygen pass:
  595 maintained files, 2440 interfaces and 288 destinations.
- [x] **T016 — Implement current physical-to-logical path mapping.** Needs: T015.
  Resolve explicit `.` and `..` roots, extensionless modules and `name+/` topology.
  Test spaced components, root-kind distinctions, conflicting roots/aliases and
  precise missing/ambiguous paths without search-order fallbacks.
  Evidence: required Project/import or Standard root tags establish exact `.`/
  `..` prefixes; unknown tags fail before scanning. Complete-name validation now
  applies to every logical/physical component, preserving nested plus directories
  only in physical source/companion paths. Seven extended catalog cases cover
  root-order independence, coexisting shorter names, no cross-family fallback,
  multiword Chinese anchors/companions, same-family duplicates, cross-family
  hardlinks and every measured Standard catalog allocation failure. Existing
  callers explicitly tag ordinary roots and link the shared name predicate;
  CLI tests check those tags. The full strict suite, catalog/options ASan/UBSan
  leaves (leak detection disabled), complete 769-input coverage, Markdown links
  and warning-free reproducible Doxygen pass: 597 maintained files, 2449
  interfaces and 288 destinations. Encoding/token/CLI completion remains T017,
  T022 and T068 respectively.
- [x] **T017 — Verify deterministic generated identities.** Needs: T016. Use
  complete category-separated identities, unambiguous encoding and exact hash
  collision rejection. Check stable Unicode vectors, forced collisions and
  distinct native/shared/type/control categories.
  Completed with root-aware version-two counted encoding, exact full-key
  collision checks, checked measurement and one-allocation display construction.
  Frozen Unicode vectors exercise the public producer; all categories, component
  boundaries, invalid metadata and every allocation failure are covered. Updated
  maintained native companions through exact old/new identity mappings. Full strict
  tests, identity/namespace/integer ASan/UBSan (leak detection disabled), 775-input
  coverage and Markdown links pass. Two warning-free Doxygen generations are
  byte-identical: 603 maintained files, 2462 interfaces, 288 destinations.
- [x] **T018 — Verify the module registry and source catalog.** Needs:
  T016, T017. Register each complete identity once, preserve physical diagnostic
  paths and exclude owned outputs. Test duplicate physical identities and
  deterministic enumeration. Exact activation/draft classification belongs to
  T023's shared lexical/parser path; required-draft validation follows in T029.
  Verified existing exact registry/catalog algorithms with multiword seeds in
  both root families, repeated registration, source pointer/physical-path
  retention, root and file-creation order independence, exact output-subtree
  exclusions, missing-family refusal and every composed allocation failure.
  Focused strict release and ASan/UBSan leaves pass (leak detection disabled),
  as do 777-input coverage and Markdown links. Doxygen is warning-free and
  byte-reproducible: 605 maintained files, 2467 interfaces, 288 destinations.
- [x] **T019 — Verify structured frontend diagnostics.** Needs: T014, T018.
  Preserve primary/cause/related spans and stable expected-construct information.
  Compose filesystem, Unicode and identity failures through user-facing messages.
  Parser adaptation now preserves precise originating expectation text alongside
  its broad machine key. Invalid-path prose covers both root families and complete
  Unicode names. Added exhaustive bounded primary/related interval validation,
  invalid-adapter nonpublication and real catalog/parser/identity compositions.
  Full strict suite and focused ASan/UBSan leaves pass (leak detection disabled);
  780-input coverage and local links pass. Warning-free Doxygen is byte-identical
  across two generations: 608 maintained files, 2474 interfaces, 288 destinations.
- [x] **T020 — Verify rendering and C source mappings.** Needs: T019. Render
  original names/excerpts and correctly escaped `#line` paths. Compile generated
  mapping probes and test allocation-free fallback reporting.
  Added compiled filename-byte round trips with trigraphs enabled and every
  LF/CRLF/EOF coordinate, compiler-allocation-denied fallback/excerpt/mapping
  output, invalid-input nonpublication and immediate I/O failure checks. GCC
  cannot safely expand mapped filenames containing CR; SRC-03 unsupported-path
  checks now reject physical CR roots and standalone mappings without renaming.
  Full strict suite and focused ASan/UBSan pass (leak detection disabled), along
  with 782-input coverage and links. Warning-free Doxygen generations are
  byte-identical: 610 maintained files, 2481 interfaces, 288 destinations.
- [x] **T021 — Compose source loading and dependency invalidation.** Needs: T018,
  T020. Track source/configuration/native/generated inputs without duplicate loads.
  Verify changed transitive files and root settings invalidate exactly dependent
  consumers. Integrate inactive-to-active semantics in T029 after T023 establishes
  the shared activation recognizer; do not introduce a second draft parser here.
  Added exact category/key/payload input graphs, counted root configuration,
  deduplicated edges and allocation-free least-fixed-point invalidation. Closure
  capture borrows retained Ama/ABI bytes and loads each required native file once;
  the frontend owns/seals its snapshot and tracks entry/authoring configuration.
  All 512 three-node graphs with three changed seeds match independent reachability;
  real source/ABI/C/header changes, root settings, unrelated inputs, generated
  consumers and every capture/frontend allocation failure are covered. Full strict
  suite, focused ASan/UBSan (leak detection disabled), 797-input coverage and links
  pass. Warning-free Doxygen is byte-reproducible: 624 maintained files, 2537
  interfaces, 289 destinations. Artifact reuse/complete driver and C-preprocessor
  input wiring remain their later tasks; no stage is skipped by these snapshots.

## Phase 3. Parse current source forms without semantic guesses

Contracts: [grammar](spec/grammar.md),
[source layout](spec/language/source-and-names.md).

- [x] **T022 — Implement exact layout and token boundaries.** Needs: T015, T019.
  Enforce one-space nesting and separators, compact paths/reference/list markers,
  comment indentation, tabs/trailing-space rejection and literal byte preservation.
  Test every boundary across LF/CRLF and optional final newline.
  Added allocation-free region-aware token scanning with maximal complete names,
  longest compound punctuation, compact/separated join checks, exact comment
  indentation, blank/trailing-space rejection and untouched literal views. Global
  UTF-8/TAB/lone-CR preflight now precedes activation and payload classification;
  removed the old post-activation byte-check API. Entry literal labels recognize
  complete multiword names and compact storage suffixes without lookup. Updated
  positive fixtures to legal payload bytes and retained explicit negative cases.
  Full strict suite, focused ASan/UBSan (leak detection disabled), 805-input coverage
  and links pass. Doxygen is warning-free and byte-reproducible: 631 maintained
  files, 2564 interfaces, 290 destinations. AST integration of checked regions and
  tokens proceeds in T023–T028; this step certifies the lexical component.
- [x] **T023 — Parse sections, drafts and declarations.** Needs: T022. Build
  source-mapped AST nodes using T022 checked tokens/regions for executable,
  constants and native sections, explicit declarations, all four storage forms
  and initialization. Check activation,
  constants-only files, malformed sections and deep nested bodies.
  Shared lexical classification now precedes activation, with an atomic reusable
  draft/active predicate. The AST uses the shared maximal-name scanner and complete
  Project/Standard type paths. Explicit region cursors enforce declaration/type/
  initializer comments and prevent reopening after a parent comment. All storage
  forms, section/ending cases, invalid drafts and every allocation failure pass.
  Full frontend composition preserves distinct nominal types across both roots.
  Updated call/copy fixture separators through exact successful-AST positions and
  reviewed explicit negative fragments. Full strict suite, parser/compiler ASan/
  UBSan (leak detection disabled), 812-input coverage and links pass. Warning-free
  Doxygen is byte-reproducible: 637 maintained files, 2577 interfaces, 291 destinations.
- [x] **T024 — Parse accesses, assignments and inferred receivers.** Needs: T023.
  Recognize indexed/qualified/self-reference sources and uniform assignment nodes
  without name lookup. Test suffix/spacing boundaries, legal source modes and
  forbidden literal assignments with exact diagnostics.
  Added a distinct standalone-self source tag without a synthetic access/name;
  split qualified/instance source parsing into cohesive operations. Uniform
  assignment nodes preserve known/first-use receiver syntax and exact child-region
  containment. Sources cover recursive multiword indices, references, both root
  families, lists, explicit continuation copies and forbidden source contexts.
  Literal errors retain exact sites; every composed self-source allocation failure
  unwinds. Until T032, self typing reports appended unsupported-feature instead
  of dereferencing a missing access; existing diagnostic codes retain their IDs.
  Full strict suite, focused ASan/UBSan (leak detection disabled), 817-input
  coverage and links pass. Warning-free Doxygen is byte-reproducible: 641
  maintained files, 2588 interfaces, 292 destinations.
- [x] **T025 — Parse calls and binding phases.** Needs: T024. Recognize entry,
  resume, retained/constant reads, named/positional bindings, empty placeholders
  and entry literal payloads. Test phase/mode/arity structure and explicit empty
  literal versus omitted input without semantic classification in the parser.
  Split label/operand parsing from marker orchestration and removed all separator
  trimming. Added a narrow independently tested binding boundary, complete Unicode
  label/storage cases, exact comments, Standard reads, indexed-reference retained
  reads/resumes and independent direction ordinals. Explicit output-phase state
  is separate from ordinary output counts for the following result-tree task.
  Empty literal and omitted input remain distinct; every named-call allocation
  failure unwinds. The exhaustive 5856-form mode/style/order oracle, full strict
  suite and focused ASan/UBSan pass (leak detection disabled), as do 821-input
  coverage and links. Warning-free Doxygen is byte-reproducible: 644 maintained
  files, 2601 interfaces, 292 destinations. Callee arity/types remain semantic work.
- [x] **T026 — Parse direct-result trees and refusal handlers.** Needs: T025.
  Retain selector paths, sibling order and each handler's owning operation.
  Test nested modes, colon indentation and structural rejection independently
  of declared names or runtime availability.
  Added an ordered output-action tree with explicit enclosing operations and
  independently scoped binding phases. Refusal markers retain exact ownership
  and source spans; calls inside handlers start ordinary caller-context trees.
  Explicit parser frames handle 512-level selection paths without recursive C
  activation. Tests cover sibling order, nested indexed references, mode/style
  boundaries, malformed indentation and every allocation failure. Until T036,
  semantic analysis explicitly rejects selection trees instead of dropping them.
  Full strict suite and focused ASan/UBSan pass (leak detection disabled), as do
  832-input coverage and links. Warning-free Doxygen is byte-reproducible: 654
  maintained files, 2630 interfaces, 293 destinations.
- [x] **T027 — Parse branches, loops and foreach.** Needs: T026. Represent
  positive/negative branches, empty bodies, single-root linear condition loops,
  break/return/yield and reference-binding foreach. Test exact repeat roots,
  comments between markers, deep nesting and malformed near-matches.
  Added distinct traversal binders, negative-only branches, written marker spans
  and one folded loop node with an exact root/terminal owner for linear headers.
  Header handlers use incoming loop context; only body frames permit break.
  Exact comment gaps and closed-region checks reject misleading near-matches.
  Every empty written body records a source-ordered warning, including unreachable
  handlers; the driver renders warnings without failing valid programs and checks
  warning output failures. Parsed-empty owned loops now convert once and exit;
  declaration-only bodies retain their backedge, verified by compiled native effects.
  New header/traversal execution reports unsupported-feature until its typed stages.
  Tests cover 256-level control nesting and every allocation failure. Full strict
  suite and focused ASan/UBSan pass (leak detection disabled), as do 843-input
  coverage and links. Warning-free Doxygen is byte-reproducible: 664 maintained
  files, 2657 interfaces, 294 destinations.
- [x] **T028 — Compose the parser and ABI table reader.** Needs: T009, T021,
  T027. Validate fixed `.abi` records separately from Ama literal classification;
  exhaust malformed/duplicate/missing bindings. Run current documented examples,
  grammar boundary cases, truncations and source-to-AST failure cleanup.
  ABI records now reject repeated separators, spaces-only lines and trailing code
  spaces, independently of Ama raw-literal recognition. The finite six-slot oracle
  exhausts all 1957 ordered subsets; duplicate sites, physical/identifier boundaries,
  every byte truncation and every source/ABI allocation failure are checked.
  Composition reads all 63 canonical Ama/ABI text examples in explicit contexts
  directly from spec, plus 22 composed boundary forms; changed fence counts require
  review. An independent complete AST traversal checks borrowed spans, block ends,
  direction ordinals, selected-parent/binding aliases and lexical loop terminals.
  Every byte prefix and source-to-AST allocation failure of a mixed Unicode program
  retains valid diagnostics and fully unwinds. Full strict suite and focused
  ASan/UBSan pass (leak detection disabled), as do 853-input coverage and links.
  Warning-free Doxygen is byte-reproducible: 673 maintained files, 2681 interfaces,
  295 destinations. Structural acceptance does not claim pending runtime features.

## Phase 4. Resolve exact types, visibility and operation contracts

Contracts: [declarations](spec/language/declarations.md),
[values](spec/language/values-and-references.md),
[calls](spec/language/calls-and-results.md),
[control](spec/language/control-flow.md), [lists](spec/language/lists.md).

- [x] **T029 — Verify complete project closure.** Needs: T028. Load transitive
  Ama/native dependencies once per identity and retain requester traces. Test
  legal dependency cycles, missing required drafts and companion selection only
  for active native modules, including invalidation when a draft becomes active.
  Expanded discovery covers selected actions, every handler, folded condition
  commands and traversal bodies. Requests retain their written spans and are
  stably sorted by exact byte position before registry admission; aliases never
  create duplicate visits or imports. The allocation-free merge sort is checked
  on all 5040 seven-request permutations, with equal-key stability and multiset laws.
  Native companions are selected atomically only after a required source parses as
  `@C`; ordinary, unused and draft sources ignore stale companions. Repeated native
  selection reuses the same views. Missing companions remain available to explicit
  authoring; strict native checks still enforce complete contracts. Tests cover
  expanded requester traces, draft activation, native-to-Ama input invalidation,
  complete successful/error allocation-failure prefixes and real typed ABI compilation.
  Existing exhaustive reachability, cycles, roots and deep traversal cases pass.
  Full strict suite and focused ASan/UBSan pass (leak detection disabled), as do
  862-input coverage and links. Warning-free Doxygen is byte-reproducible: 682
  maintained files, 2708 interfaces, 295 destinations.
- [x] **T030 — Resolve explicit declaration inventories.** Needs: T029. Enforce
  one base-name namespace across sections and storage spellings; resolve full
  nominal module types. Provide shared exact source/interface-member lookup and
  declared result-label/position metadata for inference and later call validation.
  Test later visible explicit fields and duplicate-related
  spans without conflating visibility with initialization.
  Semantic visitation now collects every owning AST subtree and stably sorts node
  events by physical position, including selected handlers, folded headers and
  traversal bodies. Loop/foreach bodies alone add loop depth; header handlers keep
  incoming context. The complete explicit inventory preserves module/role ordinals,
  exact storage flags and distinct Project/Standard nominal identities. Shared
  allocation-free interface lookups by name/position now serve call checking and
  subsequent inference. Wrong-role binding diagnostics retain their declarations.
  Tests exhaust all 256 section/storage duplicate pairs, header-handler ordering,
  Unicode/case distinctions, stable-sort boundaries, lexical depths and every
  expanded inventory allocation failure. Later explicit source/target uses succeed;
  forward creation-time initialization still fails. Full strict suite, updated
  focused cases and ASan/UBSan pass (leak detection disabled), as do 869-input
  coverage and links. Warning-free Doxygen is byte-reproducible: 689 maintained
  files, 2732 interfaces, 296 destinations. Inference/local bindings remain T031.
- [x] **T031 — Implement source-order inference and foreach scope together.**
  Needs: T030. Apply TYPE-06/NAME-06's single ordered algorithm, immediately type
  new receivers and resolve/reuse foreach bindings with no shadowing. Test earlier
  unresolved occurrences, later explicit collisions, inferred list sources,
  disjoint locals, nested reuse and forbidden construction-time local access.
  Added one exact event stream for actual declarations, sources, operation heads
  and interleaved bindings. Encountered names remain separate from the hoisted
  explicit inventory; first receivers are typed immediately and later explicit
  collisions retain both real sites. Typed result paths use interfaces only;
  literals, labels, paths and selector bases never become caller occurrences.
  Foreach records compatible reuse or distinct lexical local cells, rejects
  shadowing/construction-time local reads and preserves earlier bindings when a
  later module field reuses the name. Normal fields carry direct source/type/name
  metadata without synthetic declarations; inferred executable sources never
  become creation-time initializer hints. Field order, eager/copy graphs, lifecycle
  planning, diagnostics and C emission consume the same metadata. Intrinsic result
  shapes add only LIST-04's fixed Standard integer dependency with real request
  sites and no family fallback. Runtime tests cover assignments/results, full
  module-state and list copies, pre-assignment constructor snapshots, and default
  owned/empty-reference values after acquisition refusal. Scope/order/storage,
  implicit dependencies and every inference/closure allocation failure pass.
  Full strict suite and focused ASan/UBSan pass (leak detection disabled), as do
  893-input coverage and links. Warning-free Doxygen is byte-reproducible: 712
  maintained files, 2815 interfaces, 297 destinations. Direct-tree/list-service/
  foreach execution and lexical-self transfer guards remain their later tasks.
- [x] **T032 — Verify transfers and self-reference typing.** Needs: T031.
  Enforce the complete storage-form matrix, constant/indexed sources and lexical
  standalone `&`. Test absent/mismatched suffixes, exact root-kind identity and
  storage compatibility without inferring unsupported conversions.
  Evidence: [exact transfers](docs/implementation/typed-transfers.md) separates
  constant/named/self resolution; lexical self has the containing module's exact
  scalar-reference type without a fabricated access. Complete nominal/form and
  cross-root matrices, caller/default self contexts, suffix errors and every
  semantic allocation failure pass. Indexed shared sources retain their access
  chains in emission; compiled traces prove one index conversion per access.
  Full strict `make -j4 test` and semantic/frontend ASan+UBSan checks passed.
  Static self typing is complete; an explicit pre-storage planning guard retains
  execution for T049–T050's final construction/clone identities. Documentation
  coverage checks 900 maintained inputs; Doxygen reproducibility is verified.
- [x] **T033 — Derive dependency and capability fixed points.** Needs: T032.
  Provide exact dependency/cycle algorithms; derive the eager-owned-layout and
  clone-capability relations separately from module dependencies. Test finite
  graphs, recursive empty lists and noncopyable owned descendants. Require static
  copy capability before runtime self-assignment no-op handling; do not reject
  benign execution/reference cycles. T034 supplies resolved initializer edges.
  Evidence: [type relations](docs/implementation/type-relations.md) proves the
  eager-edge predicate, least full-key topological order, actual finite cycle
  extraction and greatest copy-capability fixed point. Capability seeding clears
  stale cause witnesses. 2,097,152 graph/seed cases match independent reachability;
  12,288 graph/admission/key cases match exhaustive topological permutations and
  retain real cycle spans. All 128 field partitions, inferred defaults, 1024-node
  deep/parallel shapes and each graph allocation failure pass. Strict semantic
  unit/integration suites and ASan+UBSan relation checks passed. Documentation
  covers 904 maintained inputs; final Doxygen generations are byte-identical.
- [x] **T034 — Validate constants and initialization plans.** Needs: T033.
  Resolve qualified members and legal constant sources, then construct actual
  initializer dependencies and reject their forbidden cycles. Derive exact
  startup/reverse cleanup order. Test constants-only modules, constant cycles,
  inferred-field defaults and visibility without forward initializer reads.
  Evidence: [shared initialization](docs/implementation/shared-initialization.md)
  explains exact source/read edges, finite fresh-construction closure and linked
  startup/reverse plans. Nested lexical self now rejects mutable identities in
  immutable construction; named shared reads no longer duplicate their root edge.
  Tests cover defaults versus executable inferred sources, scalar/list literals,
  clones versus fresh descendants, qualified-member/forms/forward errors, exact
  indexed prerequisites, constants-only namespaces and every allocation in nested
  closure, cycle diagnostics and publication. Full strict `make -j4 test`, constant
  ASan+UBSan checks and 906-input documentation coverage passed; final Doxygen
  generations are byte-identical.
- [x] **T035 — Validate normal call/read bindings.** Needs: T032, T034. Resolve
  current named/positional rules, placeholders, exact input/result types, defaults
  and literal capabilities. Test mode-specific rejection and both declaration
  and binding locations without adding unaccepted mixed-mode behavior.
  Evidence: [ordinary bindings](docs/implementation/ordinary-bindings.md)
  documents role/ordinal resolution, caller operands, exact forms and omission
  metadata. Duplicate diagnostics retain the interface declaration and first
  binding beside the later offending site. The new recursive semantic/calls leaf
  exhausts input and entry/resume/retained/shared output matrices, literal bytes,
  noncopyable omissions, independent positions, repeated caller receivers,
  role/suffix/arity errors, five duplicate modes and parser mode/style rejection.
  Every complete-binding semantic allocation unwinds. Strict semantic unit and
  integration suites, ASan+UBSan calls and 912-input documentation checks passed;
  final Doxygen generations are byte-identical.
- [x] **T036 — Validate direct-result operation trees.** Needs: T035. Resolve
  selectors only in parent result interfaces and operands in the initiating
  caller's environment. Test reference/continuation targets, sequential methods,
  inaccessible private names and per-link handlers.
  Evidence: [selected operations](docs/implementation/selected-operations.md)
  records exact parent-result symbols, caller-local index/operand resolution and
  ordered typed action links. Repeated selections retain the declared field;
  selecting a noncopyable method requires no target clone. All 36 parent/child
  mode/form combinations, interleaved binding/selection siblings, caller-variable
  collisions, handler roots, wrong roles/suffixes/indices and 256-level reference
  paths pass. Every complete-tree semantic allocation unwinds. Full strict tests,
  ASan+UBSan selected checks and 922-input coverage passed. Planning explicitly
  guards execution before storage publication until IR/lifetime action lowering;
  intrinsic service semantics remain T037. Final Doxygen is reproducible.
- [x] **T037 — Validate intrinsic typed list services.** Needs: T009, T033, T036.
  Model six fixed method interfaces and the required `..integer` capabilities;
  enforce service-parent modes, typed receivers and omitted-source versus explicit
  owning-copy capability. Reject obsolete action-dispatch and retained-state APIs.
  Evidence: [list interfaces](docs/implementation/list-interfaces.md) shares one
  immutable six-entry descriptor table between inference and full validation.
  Service/method kinds have no fabricated nominal types, declarations or method
  instances; further calls retain explicit ephemeral result identities. Tests
  inspect all interfaces, positions and output forms, owning/reference omission
  versus explicit-source copy requirements, all eight native integer capability
  profiles, exact project/standard identity, invalid retired modes and every
  semantic allocation failure. Full strict `make -j4 test` and list ASan+UBSan
  passed. Frontend checking retains the later typed `from_size` build obligation
  without inspecting C; execution remains guarded before storage publication.
  Documentation checks 935 inputs; final Doxygen generations are byte-identical.
- [x] **T038 — Validate control-flow contexts.** Needs: T031, T036, T037. Check
  owned truth/index capabilities, branch forms, empty-body warnings, exact linear
  loop roots and handler break context. Ensure foreach has distinct binding and
  traversal semantics, including statically checking skipped/empty paths.
  Evidence: [control contexts](docs/implementation/control-contexts.md) carries
  exact incoming loop identities through structural collection/source sorting,
  records full header restart roots and terminal owners, and shares ordinary/
  intrinsic result selection. Deferred predicate checking preserves exact own
  native capabilities, negative polarity and syntactic emptiness; foreach retains
  T031's real lexical collection/cell roles. Tests cover nested/header-handler
  exits, all nine header mode pairs, caller indices, intrinsic predicates, empty
  warnings, indexed-versus-foreach forms, nested/disjoint/reused cells, skipped-path
  errors and every complete-control allocation failure. Full strict tests and
  control/walker ASan+UBSan passed; 948 inputs have explicit coverage. New execution
  stays guarded before storage publication. Final Doxygen is reproducible.
- [x] **T039 — Validate continuations and copy boundaries statically.** Needs:
  T033, T038. Enforce no resume inputs, exact ordinary/continuation copy forms,
  static copy capability and detectable ownership overlap. Test self-copy,
  noncopyable resources and legal external reference transfers.
  Evidence: [continuation boundaries](docs/implementation/continuation-boundaries.md)
  records exact resume/prefix/whole-list restrictions and distinct literal/handle/
  ordinary/continuation transfers. Positive origin analysis follows exact AST
  sibling segments of scalar handle stores, propagates real formation sites and
  rejects proved continuation self-alias/current-root ownership overlap. Calls,
  scopes, suspension and unknown stores retain runtime validation; declarations
  are not runtime effects. Tests cover shapes, individual list elements, ordinary
  outputs, exact origin notes, branch joins/arms, external resource references and
  every success/rejection proof allocation. Full strict tests, final focused cases
  and ASan+UBSan passed; 957 maintained inputs have coverage. Final Doxygen is
  reproducible. Runtime identity/availability/overlap checks remain mandatory.
- [x] **T040 — Compose semantic analysis with current diagnostics.** Needs:
  T034–T039. Check valid/invalid full projects, preservation of spans/storage forms,
  deterministic classification and allocation cleanup. Confirm every interpretation
  is resolved before lowering or C emission.
  Evidence: explicit six-stage composition matches separately owned staged analysis
  for complete projects, exact typed metadata, source spans and all diagnostic
  fields. Failed-stage prefixes and every composition allocation are checked.
  Full strict tests and ASan+UBSan passed; 963 maintained inputs have coverage.
  Doxygen reproducibility is verified before committing this task.

## Phase 5. Make resolved behavior explicit in IR and storage plans

Contracts: [compiler](spec/implementation/compiler.md),
[runtime](spec/implementation/runtime.md).

- [x] **T041 — Define current typed IR boundaries.** Needs: T013, T040. Represent
  resolved type/storage identities, lifecycle, operations, failure edges and source
  maps independently of C text. Validate narrow independently inspectable nodes
  and reject unresolved source-level decisions at the boundary.
  Evidence: existing typed storage/lifecycle/control node families now pass an
  allocation-free publication validator with separate lifecycle allocation
  inventories. Exact nominal/field/shared identities, registered hooks, source
  maps, selectors, endpoints, mandatory operands and local/callee edges are
  checked without following executable cycles. Thirty-five deliberate mutations
  are rejected; valid cyclic calls remain valid without allocations. Full strict
  tests, final focused IR cases and ASan+UBSan validation passed. All 970 maintained
  inputs have coverage; final Doxygen reproducibility is checked before commit.
  T042–T045 retain responsibility for new storage and operation lowering; existing
  unsupported-feature guards remain explicit.
- [x] **T042 — Plan context and instance storage.** Needs: T041. Separate per-context
  constants/runtime from stable-slot fields, interface state and control metadata.
  Include constructing identity, return links, saved positions and fixed lexical
  chain/foreach frames as logical state. Permit compact or implicit physical
  fields only with exact capability/liveness proofs under RT-02/GEN-03; record
  the general fallback and verify deterministic layout dependencies and no mirrors.
  Evidence: context schemas enumerate independent runtime domains and exact
  pool/constant counts; stable instance schemas retain constructing identity and
  the complete general control state. Fixed source-site operation/foreach frames,
  conditional cells and shared binding origins have exact parent identities and
  an allocation-free validation boundary independent of unfinished lowering.
  No physical state is elided and no payload mirror or method instance is added.
  Tests cover native intrinsic outputs, nested reuse versus sibling locals,
  permanent bindings, 128-level chains/traversals, independent-plan determinism,
  damaged schemas and every storage allocation. Full strict tests and ASan+UBSan
  passed; all 982 maintained inputs have coverage. Final Doxygen reproducibility
  is checked before commit; runtime cursor correction remains a later task.
- [x] **T043 — Lower entry, transfer and cleanup order.** Needs: T042. Emit
  explicit acquire/preflight/prepare/commit/result/release actions with distinct
  refusal and fatal edges. Test CALL-08 evaluation order and source-lifetime
  handoffs without inserting snapshots or whole-call rollback.
  Evidence: parameter/result/output replacements now share exact typed recipes
  with explicit preflight, fresh preparation, commit and release groups. A distinct
  aggregate input preflight follows caller-text source selection and precedes
  declaration-order installation; resume/read modes omit entry preparation.
  Recipe/group/invocation validators check construction choice, original operands,
  commit endpoints, source/install/output order and no interior entry. Tests cover
  exact literals, omitted noncopyable references, callee defaults, repeated output
  receivers, transfer modes, twelve damaged plans and every lowering allocation.
  Full strict tests, final focused IR checks and ASan+UBSan passed; 994 maintained
  inputs have coverage. Final Doxygen reproducibility is checked before commit.
- [x] **T044 — Lower structured control and result actions.** Needs: T043.
  Assign fixed loop repeat/break labels and depth-first result continuations.
  Test empty-loop single conversion, per-link handlers, nested exits and bounded
  frame use before C generation.
  Evidence: iterative block/operation work retains exact caller frames, selected
  targets, depth-first sibling continuations, retired body returns and whole-chain
  refusal cleanup before the failing link's handler. Loop records provide fixed
  header/named repeat roots and body/exit labels; both condition outcomes release
  header grants before body/exit. Foreach uses fixed enter/next/leave or empty
  validation, scoped return/restart cleanup and outer-first resume templates.
  Intrinsic methods stage typed descriptors without synthetic modules. New IR
  validators check loop/frame/chain/protocol identities. Static checking accepts
  complete plans; pending runtime adapters are explicitly rejected by C generation
  before any model/artifact allocation. Tests cover mixed modes, handler yield and
  outer breaks, intrinsic conditions, empty/declaration-only bodies, 128-deep
  chains/traversals, ten damaged plans and every lowering allocation. Full strict
  tests, ASan+UBSan and coverage of all 1013 maintained inputs passed; final Doxygen
  reproducibility is checked before commit. Runtime execution remains T066 onward.
- [x] **T045 — Verify complete frontend-to-plan composition.** Needs: T044.
  Preserve nominal types, source locations, ownership/capability facts and all
  control successors. Run real source-to-plan cases and invariant checks, including
  drafts, native contracts and invalid-project cleanup.
  Evidence: [frontend composition](docs/implementation/frontend-composition.md)
  compares independently owned semantic/storage/lifecycle/control plans against
  public checking, including every successor and original source identity. Real
  projects cover selected calls, traversals, continuations, shared values, inactive
  drafts and isolated ABI authoring. Eight invalid projects and failure of every
  allocation verify first-stage diagnostics and complete partial-owner cleanup.
  Full strict tests and ASan+UBSan passed; all 1021 maintained inputs have coverage.
  Final Doxygen reproducibility is checked before commit.

## Phase 6. Stable ownership storage before executing user bodies

Contracts: [references and copying](spec/language/values-and-references.md),
[lifecycle](spec/implementation/native-interface.md#complete-lifecycle-and-cleanup-protocol),
[pools](spec/implementation/runtime.md).

- [x] **T046 — Verify stable typed block pools.** Needs: T006, T011, T042. Reuse
  free slots before growth without moving live payloads; reserve final identities
  while constructing. Test multiple same-type instances and pool growth with
  live addresses, without implicit default instances.
  Evidence: [stable typed allocation](docs/implementation/instance-ownership.md#t046-stable-typed-allocation-and-publication)
  proves address preservation and tests published and private slots. New cases
  cover 33 live instances at three block sizes, reuse with allocation disabled,
  independent controls, final nominal identity, publication gates and both later
  growth allocation failures. Existing implementation satisfied these contracts;
  no alternative pool was introduced. Strict runtime composition and ASan+UBSan
  passed, with coverage of all 1025 maintained inputs. Final Doxygen reproducibility
  is checked before commit; generation/reclamation policy remains T047.
- [x] **T047 — Verify generations and reclamation history.** Needs: T046. Retire
  exhausted identities and reclaim only entirely free blocks while preserving
  staleness across regrowth. For interior reclamation, validate sparse directory
  holes, retained identity history and free-list removal with live later blocks.
  Exhaust small generation/state models and failure paths; keep capacity, free
  slots and live counts distinct and record the implemented reclamation policy.
  Evidence: [sparse reclamation](docs/implementation/instance-ownership.md#reclamation)
  replaces suffix-only release with every entirely free materialized block.
  Free links exclude reclaimed ranges before release; null historical directory
  entries retire those ranges permanently, and growth appends fresh identifiers.
  Tests preserve later live/private addresses through interior holes, both growth
  failures and repeated cleanup. An independent issued-identity/free-set model
  exhausts 93,750 length-six schedules at two block sizes and three generation
  limits, checking every prefix. Full strict tests and ASan+UBSan passed; all
  1029 maintained inputs have coverage. Final Doxygen reproducibility is checked
  before commit.
- [x] **T048 — Verify acquisition and destruction preflight.** Needs: T047.
  Check whole affected owned subtrees, current-owner authorization and ordinary
  busy/refusal transitions. Test available roots with busy descendants, foreign
  context handles and protection release on every supported failure.
  Evidence: [acquisition preflight](docs/implementation/acquisition-preflight.md)
  composes existing nine-node availability, caller refusal/preparation and foreign
  context checks with a new direct protection leaf. Explicit nested copy authority
  preserves the outer lease; unrelated/destructive acquisition refuses. Every
  capability/protection allocation and invalid owner/duplicate/type edge restores
  the complete service-state prefix without native effects. Non-owning busy
  referents remain outside the boundary. Strict runtime composition and ASan+UBSan
  passed; all 1034 maintained inputs have coverage. Final Doxygen reproducibility
  is checked before commit.
- [x] **T049 — Integrate ordered construction and destruction.** Needs: T012,
  T048. Initialize fields once, expose final self-identity only under constructing
  rules and finalize children/native payload in the prescribed order. Inject
  failures and test escaped self-handles never revive after failed construction.
  Evidence: [ordered self construction](docs/implementation/ordered-construction.md)
  adds an explicit typed runtime self source, formed without payload access or
  acquisition. Real program recipes use final private parent/child identities,
  preserve field/native order, select literals once and publish complete trees.
  Every native constructor and allocation is failed; issued identities remain
  stale across retries, exhaustion and sparse reclamation. Public program startup,
  Running identity formation and invalid-context gates are checked. Runtime C
  assembly includes the new source unit; verified project execution, full strict
  tests and ASan+UBSan passed. All 1041 maintained inputs have coverage. Final
  Doxygen reproducibility is checked before commit; generated self execution
  awaits the pending clone/replacement/layout adapters.
- [x] **T050 — Build exact clone maps.** Needs: T049. Clone independent owned
  trees with injective final-destination mapping and internal-reference fixups;
  preserve external handles and clear ordinary saved positions. Test self/peer
  links, duplicate external targets and failed preparation cleanup. This task
  establishes scalar-owned trees; T076 integrates membership-aware lists.
  Evidence: [exact scalar maps](docs/implementation/clone-preparation.md#t050-exact-scalar-tree-map-composition)
  proves the retained work-record map injective and checks all 8000 typed scalar
  reference substitutions against an independent positional oracle. Cases include
  stale generations sharing internal slots, cross-type slot/generation collisions,
  self/peer/ancestor cycles and duplicate busy external targets. Copies have fresh
  independent identities/resources, ordinary positions clear, source state stays
  unchanged and defaults never rerun. Every allocation and native clone failure
  cleans its prefix without publication. Existing separate-boundary/final-root
  phase tests compose with the new leaf. Strict runtime tests and ASan+UBSan passed;
  all 1047 maintained inputs have coverage. Final Doxygen reproducibility is
  checked before commit.
- [x] **T051 — Integrate staged replacement and overlap checks.** Needs: T050.
  Validate capability/identity/ancestry/source availability before replacement;
  preserve destinations on preparation failure. Test ordinary self no-op only
  after static capability validation and terminal cleanup failure after commit.
  Evidence: [staged replacement](docs/implementation/staged-replacement.md)
  distinguishes private preparation handles from final destination self identity.
  A synchronous scoped work borrow directs only matching self evaluation; ordinary
  field reads use prepared storage, every callback clears authority, and live
  pool slots gain no extra metadata. Real default/literal recipes retain root
  identity across repeated bank swaps, correct child owners and invalidate old
  children. Every preparation allocation/native failure preserves old values and
  service state; each destructive-hook failure finishes old/prepared cleanup and
  leaves the world terminal. Existing static self-copy, dynamic overlap and clone
  replacement checks compose with these cases. Full strict tests and ASan+UBSan
  passed; all 1052 maintained inputs have coverage. Final Doxygen reproducibility
  is checked before commit.
- [x] **T052 — Separate shared constants from pools.** Needs: T034, T051.
  Initialize immutable module storage per context in dependency order and clean it
  in reverse order. Test no constants-only instance pool, two independent contexts
  and startup failure before root creation. Establish scalar-owned constant trees
  here; T076 supplies concrete list copying and T087 integrates list constants.
  Evidence: [per-context scalar shared storage](docs/implementation/shared-storage.md)
  verifies a dependency-ordered literal/compound/whole-copy closure in two isolated
  contexts. Arrays, child values, payloads and native resources are independent;
  pool directories and root ledgers stay empty until explicit construction, and
  shared acquisition refuses. Address traces prove explicit-root cleanup before
  reverse shared declarations/children. All seven native constructors/destructors
  and every factory allocation are failed, with no partial identity publication,
  complete cleanup and successful retry. Strict runtime, semantic constants and
  storage-planning tests plus ASan+UBSan passed; all 1058 maintained inputs have
  coverage. Final Doxygen reproducibility is checked before commit.
- [x] **T053 — Compose runtime descriptors with planned C layouts.** Needs:
  T045, T052. Verify concrete sizes/alignment/offsets and nominal identity without
  raw-word universal casts. Compile descriptor checks and real native borrow/
  lifecycle probes against planned generated declarations. For any implemented
  compact/elided layout, compare observable transitions with the general model
  and measure actual size without demanding identical allocation-attempt counts.
  Evidence: [C layout composition](docs/implementation/c-layout-composition.md)
  compiles real planned fields/offsets/identities with 64-byte-aligned native State,
  address-sensitive resources, all four storage forms, namespace exclusion and
  pool growth. Verified native entry reads a typed parameter; an independent C
  probe checks actual sizeof/alignment/offsetof and generated borrowed views,
  rejecting an equal-layout foreign nominal handle before publishing a view.
  Four source-form substitutions also fail statically. Measured target sizes are
  State/plain/worker/root = 64/64/128/192 bytes; general representation remains
  materialized. Strict codegen parent/dispatch checks and ASan+UBSan for both the
  parent and standalone compiled probe passed; all 1067 maintained inputs have
  coverage. Final Doxygen reproducibility is checked before commit.
- [x] **T054 — Verify ownership services independently of dispatch.** Needs:
  T053. Exhaust small allocation/acquisition/clone/replacement transition models
  and run sanitizer/failure-injection compositions. Record evidence before using
  these services to execute Ama bodies.
  Evidence: [ownership foundation](docs/implementation/ownership-foundation.md)
  exhausts all 104,976 length-four words over eighteen explicit two-tree/caller
  actions with exact public identities, self/peer graphs, native data/resources,
  stable cells, stale history, service/grant/refusal facts and occupied counts
  checked at every prefix. Root versus descendant acquisitions distinguish whole
  tree refusal from effect-free admitted self-copy. A longer trace fails every
  mandatory allocation while preserving the model and clearing private ownership
  and borrowed construction authority. Earlier lifecycle/history/map/compiled-C
  cases supply complementary component evidence. Full strict tests and ASan+UBSan
  passed; all 1077 maintained inputs have coverage. Final Doxygen reproducibility
  is checked before commit. No Ama body or opaque saved selector is dispatched by
  this model; subsequent execution/list/context adapters retain their tasks.

## Phase 7. Deliver the first current-spec stackless vertical slice

Contracts: [generated C](spec/implementation/compiler.md#generated-project-organization),
[host outcomes](spec/implementation/runtime.md#root-execution-and-host-control).

- [x] **T055 — Emit reusable native declarations and scaffold contexts.** Needs:
  T010, T028, T040. Generate self-contained typed headers, optional untouched
  skeletons and C tooling configuration from validated interfaces. Independently
  check headers as first includes, reference cycles and prefix cleanup; preserve
  handwritten files without an explicit overwrite request. Bind PROTO-03's fixed
  typed `from_size` only for the exact standard-root `..integer` identity; reject
  missing/incompatible header declarations and keep the ABI table at six records.
  Plan all native error-provider namespaces with T010’s full-key planner before
  emitting matching header/body AMA_NATIVE_ERROR contexts; replace the frozen
  harness identities with T017’s checked current-source identities.
  Evidence: [native contract composition](docs/implementation/native-contract-composition.md)
  plans all current-source native error providers from complete root-aware keys
  before matching header/body contexts, with dependency-ordered owner teardown.
  Real Unicode/multiword root-distinct modules/reference cycles compile header-first
  and produce matching distinct codes without macro leaks or repeated arguments.
  Exact ..integer headers bind a checked typed from_size alias; absent/wrong
  prototypes and ABI-slot reuse fail, similar identities gain no contract, and
  explicit new skeletons remain fail-first with six ABI slots. Existing companions
  are preserved. Every planning/emission allocation and missing SDK dependency is
  checked. Full strict tests and ASan+UBSan passed; all 1087 maintained inputs have
  coverage. Final Doxygen reproducibility is checked before commit.
- [x] **T056 — Emit the single project assembly.** Needs: T045, T054, T055.
  Order declarations/types/constants/native bodies/runtime/labels under one
  context-parameterized execution function. Compile agreement checks with isolated
  scaffolds; never emit a C function per ordinary Ama module.
  Evidence: [single assembly contract](docs/implementation/project-assembly.md).
  The execution body now includes its own declarations and compiles directly;
  one context-parameterized function contains all ordinary Ama control labels.
  Actual GCC C11 checks at O0/O2 cover each native interface first and both
  interface/assembly include orders, exact host signature and macro cleanup.
  Existing execution/toolchain suites retain lifecycle and forbidden-entry checks.
  Full strict tests and assembly ASan+UBSan passed; all 1088 maintained inputs
  have coverage. Final Doxygen reproducibility is checked before commit.
- [x] **T057 — Execute startup, simple calls and returns.** Needs: T056. Create
  one explicit root after constants; dispatch between two module types/multiple
  instances using stable slots and validated return selectors. Begin with calls
  that have no inputs or requested outputs; complete entry bindings in T063–T064.
  Compile and run
  nested same-type calls and early completion without payload save/restore.
  Evidence: [stable-instance dispatch](docs/implementation/simple-call-dispatch.md).
  Return dispatch now checks the caller nominal type as well as exact selector
  membership. Compiled no-binding calls cross two ordinary types and independent
  instances, preserve native addresses/counters and skip bodies after early return.
  Host-side fault injection checks unknown and foreign-type return selectors,
  terminal status and complete pool/borrow cleanup. Existing same-type nesting,
  pool growth and startup lifecycle cases remain active. Full strict tests and
  generated-program ASan+UBSan passed; all 1089 maintained inputs have coverage.
  Final Doxygen reproducibility is checked before commit.
- [x] **T058 — Integrate native entry and lifecycle status.** Needs: T057.
  Enforce entry inlining and direct typed access; return normally from native C
  before status handling. Execute all constructor paths and permitted internal
  helpers; reject unsupported entry implementations with precise diagnostics.
  Evidence: [native execution boundary](docs/implementation/native-execution.md).
  Existing typed adapters, synchronous borrow closure and verified always-inline
  entry already supply the required implementation. Added actual compiled
  malformed-status checks for default, literal, clone and destroy, complementing
  entry/condition, native failure, recursive-helper and rejected-entry cases.
  The full strict execution leaf passed; actual layout/lifecycle and assembly
  composition remain covered by T053/T056. Coverage and local links passed;
  final Doxygen reproducibility is checked before commit. No new runtime path
  or permissive fallback was introduced.
- [x] **T059 — Integrate runtime refusal and source diagnostics.** Needs: T020,
  T058. Report active source/caller paths before cleanup and preserve per-caller
  refusal snapshots. Run busy/stale/pre-root/native-failure cases, including
  allocation-free reporting and secondary destruction errors.
  Evidence: [runtime diagnostic composition](docs/implementation/runtime-diagnostics.md).
  Added compiled active-chain and pre-root failures with secondary native
  destruction errors. Host assertions preserve the original code/message through
  terminal and repeated disposal and verify complete pool/borrow cleanup.
  Existing Unicode, refusal, absent/stale target and source-role cases remain
  active. Strict execution, context and host-lifecycle leaves passed, including
  allocation-disabled reporting with intact links/refusal snapshots. All 1090
  maintained inputs have coverage; local links passed and final Doxygen
  reproducibility is checked before commit.
- [x] **T060 — Expose check-only and verified compilation commands.** Needs:
  T055, T059. Use one shared frontend and explicit generated-output ownership.
  Test draft reporting, check/compile diagnostic agreement and atomic publication
  of `--emit ... --compile` outputs with the toolchain verifier.
  Evidence: [verified CLI composition](docs/implementation/verified-commands.md).
  Actual CLI tests now compare exact check/verified-compile diagnostics for
  missing modules and active drafts and require no failed output directory.
  Existing commands share the frontend; verified assembly is staged, compiled,
  inspected and linked before owned publication. Strict CLI check/execution,
  driver and publication leaves passed, including preservation of prior programs,
  unowned output and replacement-prefix rollback. Coverage/local links passed;
  final Doxygen reproducibility is checked before commit. No second frontend or
  additional compatibility path was introduced.
- [x] **T061 — Expose initial host completion/failure/disposal.** Needs: T060.
  Connect explicit context APIs to generated code and launcher policy. Test
  startup failure, normal completion, prohibited reentry and independent contexts
  with no process-global mutable runtime state.
  Evidence: [generated isolated contexts](docs/implementation/generated-contexts.md).
  Generated immutable schemas now feed the existing context factory/execution
  leases; assembly-global mutable descriptors/constants and the single-owner
  guard are removed. First-root execution constructs its interface once; later
  entry/resume uses checked admission. Rejected resume leaves a healthy host
  available, while accepted failure cleans its context. Pre-record allocation
  failures report the explicit source operation without allocating. Actual C
  threads overlap two generated executions and refuse same-host reentry/disposal;
  simultaneous leases verify independent pools/constants and reject foreign
  references with equal local triples. Live-lease disposal refusal preserves the
  host; a Busy destructor error instead consumes it and is never retried. The
  generated-context cases have their own independently runnable leaf. Every first-root allocation failure is
  cleaned. Full strict tests, focused contexts, runtime lifecycle ASan+UBSan and
  generated-program ASan+UBSan passed, including the final independent context
  leaf and accepted Busy destructor failure. The final complete strict suite
  passed after the empty-project compiler-warning correction. All 1094 maintained inputs have coverage;
  final Doxygen reproducibility is checked before commit. Only the existing
  monotone context-ID issuance frontier is shared identity bookkeeping.
- [x] **T062 — Accept the minimal runnable slice.** Needs: T061. Publish a small
  complete transitive Ama/native project with stable-address pool growth, return
  routing and cleanup. Compile/run it under supported flags and relevant sanitizers;
  record exactly which later features remain unsupported.
  Evidence: [minimal runnable acceptance](docs/implementation/minimal-runnable-slice.md)
  and the complete maintained `tests/integration/minimal/project` with a runnable
  local guide. Nine owned nodes force node/counter pool growth; one default and
  eight native clones own independent resources. Nested same-type calls preserve
  counts `1,1,2,3,2,4`; exact reverse cleanup releases all nine resources. Native
  callbacks validate final addresses and allocation markers. Real check/verified
  compilation and target ASan+UBSan passed through the recursive integration
  entry. No leak-sanitizer claim is made; lifecycle credits independently verify
  release. Pending result-tree/condition-chain/foreach/list-service/async/export
  execution is documented. All 1104 maintained inputs have coverage; local links
  passed and final Doxygen reproducibility is checked before commit.

## Phase 8. Complete calls, result chains and ordinary control flow

Contracts: [calls](spec/language/calls-and-results.md),
[control](spec/language/control-flow.md).

This execution slice covers scalar/reference targets and their control flow.
Concrete whole-list/indexed operands and list-service results complete in Phase 10;
their parsed and typed plans are already available from Phases 3–5.

- [x] **T063 — Complete entry preparation and defaults.** Needs: T062. Acquire
  target and protect sources before reset; construct supplied/default parameters
  in declaration order and then results. Test self-dependent inputs, omitted
  bindings and failures preserving only the effects promised by CALL-08.
  Evidence: [entry preparation composition](docs/implementation/entry-preparation.md).
  Existing preparation phases select/protect sources before declaration-ordered
  parameters/results, with individual replacement commits. The missing C self
  descriptor now emits exact nominal `AMA_SOURCE_SELF` without an access chain;
  its previous blanket execution guard is removed. Compiled cases distinguish
  caller self inputs from callee self defaults, exercise omitted/default/named
  inputs and expose stored-self clone remapping through a retained read before
  reset. A later constructor failure executes neither body nor refusal handler.
  Full strict tests, the final four-case preparation leaf, phase-level runtime
  tests and generated-program ASan+UBSan passed. All 1106 maintained inputs have
  coverage; local links passed and final Doxygen reproducibility is checked
  before commit. Other unfinished execution adapters remain explicitly guarded.
- [x] **T064 — Complete literal inputs and ordered result receivers.** Needs:
  T063. Select literal construction once, preserve payload bytes and resolve
  output receivers/indices in written order. Execute inferred scalar/reference
  receivers, placeholders and failures before body entry or result publication;
  integrate planned list receivers with their concrete storage in T076.
  Evidence: [compiled literal/output bindings](docs/implementation/compiled-bindings.md).
  Existing emitter/runtime paths now have independent native-byte and lifecycle
  effect observations. LF/CRLF/no-final-newline payloads preserve repeated spaces,
  `#`, `|` and UTF-8; repeated/empty literals select one constructor each. A later
  ordinary input reads the inferred receiver's stored bytes. Cases check owned/
  reference inference, placeholders, an output-updated index and an earlier
  committed receiver after a later conversion failure. Constructor/destructor
  effects prove no default-before-literal or whole-output rollback. Native
  literal failure enters neither body nor `:`. Full strict tests and target
  ASan+UBSan passed; all 1110 maintained inputs have coverage. Local links passed;
  final Doxygen reproducibility is checked before commit. No duplicate binding
  executor or new source syntax was introduced.
- [x] **T065 — Complete retained and constant result reads.** Needs: T064.
  Copy the complete permitted value state without executing/resetting the target;
  preserve ordinary copy clearing. Test qualified constants, inferred receivers,
  partial output progress and busy/stale refusal before receiver changes.
  Evidence: [retained/constant read composition](docs/implementation/retained-reads.md).
  Compiled reads preserve native module state without reset/body execution;
  ordinary owned copies clear saved positions while reference results leave the
  original resumable. Namespace/qualified/imported constants expose exact values
  and an alarm constructor detects any hidden default instance. Busy/stale targets
  preserve existing receivers. Destructor effects prove a body-mutated result and
  earlier committed output remain after a later conversion error, with no handler
  or body reexecution. Strict recursive read-leaf tests and generated-program
  ASan+UBSan passed. All 1116 maintained inputs have coverage; local links passed
  and final Doxygen reproducibility is checked before commit. Existing typed
  transfer paths already supply the implementation; no duplicate executor added.
- [x] **T066 — Execute direct-result operation trees.** Needs: T065. Act on
  actual retained result storage, follow returned references, sequence sibling
  methods depth-first and keep parent protections until all actions finish.
  Test stateful constructor/method examples with no intermediate clone.
  Evidence: [direct-result execution](docs/implementation/direct-result-execution.md).
  Generated fixed caller slots retain enclosing acquisitions while selected
  owned/reference/indexed targets execute in place. Result bases use the parent;
  all nested index operands, arguments and receivers retain the initiating caller.
  Actual returns/yields retire body connections before depth-first output actions;
  selected release restores the enclosing target and clears its transient slot.
  Slots are excluded from native/declared value cloning. Independent component
  tests verify every retained ancestor, exact restoration, pre-mutation refusal,
  invalid frame offsets and allocation-free transitions. Five compiled cases
  verify persistent method state, a failing clone sentinel, caller indices,
  referenced/owned resume, technical decoder failure and ancestor Busy release.
  Full strict tests and generated-program/runtime ASan+UBSan passed. Coverage
  includes all 1129 maintained inputs; local links passed and final Doxygen
  reproducibility is checked before commit. Intrinsic list/cursor/header adapters
  remain explicitly guarded; broader handler acceptance follows in T067.
- [x] **T067 — Complete local refusal-handler unwinding.** Needs: T066. Release
  only the initiating operation tree before its failing link's handler, skip
  unfinished siblings and retain prior committed effects. Test empty handlers,
  ancestor boundaries and caller lexical scope throughout nested result actions.
  Evidence: [local refusal handlers](docs/implementation/local-refusal-handlers.md).
  Six independently observable compiled compositions accept deep release before
  handler entry, retained output/method prefixes, skipped siblings at every
  ancestor, caller-name collisions, empty handlers and ancestor alternatives that
  cannot catch a descendant. A selected method handles its own deep tree refusal
  while preserving the unrelated dynamic caller's held chain/results. A yielded
  handler resumes its next statement without restoring abandoned actions; body
  failures bypass alternatives at every level. The existing typed refusal graph
  and T066 frames already supply the behavior, so no duplicate executor was added.
  Strict recursive handler tests and generated-program ASan+UBSan passed. All
  1135 maintained inputs have coverage; local links passed and final Doxygen
  reproducibility is checked before commit. Loop/header exits follow in T068.
- [x] **T068 — Complete branches, loops and exact repeat boundaries.** Needs:
  T067. Execute standalone negative branches, empty-body single checks, fixed-root
  linear condition commands and nested break/return. Test conversion errors,
  per-link refusal paths and protection release before loop bodies.
  Evidence: [condition-loop execution](docs/implementation/condition-loop-execution.md).
  Header entry follows the exact planned root; terminal truth uses its protected
  result owner and caller-local indices. Both truth paths release the complete
  checked frame ancestry before body/exit. The isolated condition component keeps
  ordinary named/negative branches and checked failure-output preservation.
  Eight explicit cases execute fifteen compiled programs: full/nested roots,
  true/false parsed-empty named/header bodies, body reacquisition, indexed terminal
  decoding once, root/intermediate/resume refusal, incoming outer-loop handler
  break, failed conversion, nested body break/return and body yield/resume without
  header replay. Full strict tests and generated-program ASan+UBSan passed.
  Coverage includes all 1146 maintained inputs; local links passed and final
  Doxygen reproducibility is checked before commit. Foreach and intrinsic-list
  adapters remain guarded; no new syntax, implicit truth or cached target added.
- [x] **T069 — Accept the synchronous language slice.** Needs: T068. Run
  multi-instance state/method/control examples and source-to-generated-program
  failure compositions. Check constant/default initialization order, no hidden
  retries and no unaccepted syntax introduced by implementation shortcuts.
  Evidence: [synchronous slice](docs/implementation/synchronous-slice.md) and the
  [constructor project acceptance](tests/integration/objects/main.c).
  The real CLI checks four active entries and verifies/publishes six normal or
  target-sanitized executables. Two objects capture independent owned method
  states and resume their actual retained results; original/shared seeds remain
  independent. Exact effects account for eighteen native resource credits,
  interface/default replacement order and reverse cleanup. Body failure bypasses
  its acquisition handler; invalid literal construction executes no body/default
  fallback. Complete foreach IR remains explicitly rejected before publication.
  Spaced module/variable names and output paths preserve exact argument identity.
  Strict independent and parent integration targets passed, including actual
  ASan+UBSan programs. All 1164 maintained inputs have coverage and the native
  header has a standalone probe; links passed and final Doxygen reproducibility
  is checked before commit. No additional executor or language syntax was added.

## Phase 9. Saved execution positions and independent continuation copies

Contracts: [continuations](spec/language/continuations.md),
[return links](spec/implementation/runtime.md#return-links-and-resumption-positions).

- [x] **T070 — Execute statement-boundary yield/resume.** Needs: T069. Save
  exact structured positions separately from active return links and make yielded
  instances available. Test repeated yields, nested immediate-caller return and
  the distinction between busy Waiting and available Yielded.
  Evidence: [saved statement positions](docs/implementation/saved-statement-positions.md).
  Three compiled cases observe two repeated yields to the immediate outer caller,
  three Busy refusals against that Waiting caller, preserved independent refusal
  snapshots, correct final return to the root and NoResume after completion.
  A yielded module exposes its native predicate through a declared reference;
  changing it false before resume does not recheck or replay its already chosen
  branch. Exact following-statement execution preserves persistent probe counts.
  Existing return/selector helpers already supply this behavior; no second
  coroutine executor was introduced. Strict parent position tests, independent
  runtime-call metadata tests and generated-program ASan+UBSan passed. All 1168
  maintained inputs have coverage; links passed and final Doxygen reproducibility
  is checked before commit. Foreach restoration and continuation-copy acceptance
  remain their later tasks.
- [x] **T071 — Complete restart and host yield policy.** Needs: T070. Restart
  entry without resetting persistent body fields; resume without inputs/interface
  reset. Test root completed/yielded/failed outcomes, explicit host resume/disposal
  and diagnosed standard-launcher rejection of unhandled root yield.
  Evidence: [restart and root policy](docs/implementation/restart-and-root-policy.md).
  Four compiled cases distinguish supplied/default interface values from retained
  body counters across entry and resume, including restart from a yielded state.
  Explicit generated-host checks preserve healthy roots across admission rejection,
  observe yielded/completed outcomes at stable identities and dispose without
  resuming. Accepted body failure remains terminal across later entry/resume and
  idempotent cleanup. The standard launcher reports unhandled root yield and
  consumes one native cleanup credit without executing the following statement.
  Existing preparation/context/launcher paths supply the behavior. Strict parent
  restart tests, independent host-lifecycle tests and generated-program ASan+UBSan
  passed. All 1173 maintained inputs have coverage; links passed and final Doxygen
  reproducibility is checked before commit. Checked foreign-component ABI and
  scheduler acceptance remain their separate tasks.
- [x] **T072 — Copy continuation control state explicitly.** Needs: T050,
  T071. Preserve allowed owned descendant positions for `%` copies and clear all
  positions for ordinary copies. Test distinct identity requirements, whole-source
  availability and absence of copied busy state, callers or refusal history in
  scalar-owned trees. Membership-aware list/cursor continuation copies complete
  in T084 after their storage and live traversal contracts are implemented.
  Evidence: [scalar continuation copies](docs/implementation/scalar-continuation-copies.md).
  Four compiled cases distinguish ordinary root/child position clearing from
  explicit independent preservation, retain native values and reset both source
  levels' refusal histories in the copy. A running external reference target does
  not extend the copied boundary; a Waiting owned descendant rejects before the
  first native clone. A reference alias to the destination rejects by actual
  identity with no clone callback, and cleanup observes the untouched payload.
  Existing preflight/mapping/service projection supplies the implementation.
  Strict parent snapshot tests, independent object clone/access tests and actual
  generated-program ASan+UBSan passed. All 1179 maintained inputs have coverage;
  links passed and final Doxygen reproducibility is checked before commit.
- [x] **T073 — Integrate continuation failure and cleanup paths.** Needs: T072.
  Reject native-stack capture, ensure failed contexts cannot resume and destroy
  yielded state without executing its continuation. Exercise native resource
  clone failures and mixed completed/yielded ownership trees.
  Evidence: [continuation cleanup](docs/implementation/continuation-cleanup.md).
  A later native clone frees its own partial allocation and fails after an earlier
  clone completed. Exact native traces prove private-prefix cleanup precedes
  unchanged old destination/source cleanup, consumes only completed credits and
  preserves the primary failure. Explicit host checks prohibit resuming the failed
  context or executing its saved source continuation during disposal. Two compiled
  mixed-state trees preserve only each node's own position. Native declarations
  reject Ama yield syntax, and a nonzero native NoResume status remains admitted
  technical failure rather than native yield. The shared fixture explicitly
  includes its C allocation/string dependencies. Full strict tests and actual
  generated-program ASan+UBSan passed; all 1184 maintained inputs have coverage.
  Links passed and final Doxygen reproducibility is checked before commit. The
  existing ledger/context machinery required no alternate cleanup executor.
- [x] **T074 — Accept resumable generated programs.** Needs: T073. Compile/run
  custom-host and nested same-type continuation examples, including ordinary and
  explicit independent copies. Verify source traces use the new active caller
  rather than historical copied return connections.
  Evidence: [resumable acceptance](docs/implementation/resumable-acceptance.md).
  Two additional compiled cases combine same-type nested yields, explicit C-host
  resume, independent continuation advancement, ordinary-copy NoResume and a
  copied continuation's new-caller failure trace. Ten explicit leaf cases cover
  the resulting scalar slice without an alternate executor. Strict compilation,
  actual generated-program ASan/UBSan, coverage of all 1185 maintained inputs,
  local links and reproducible Doxygen are checked before the task commit.
  Checked foreign ABI and membership-aware cursor copies remain separate tasks.

## Phase 10. Membership-aware typed lists and live traversal

Contracts: [lists](spec/language/lists.md),
[foreach scope](spec/language/declarations.md#name-06).

- [x] **T075 — Implement checked list membership records.** Needs: T037, T054.
  Replace or adapt reusable order storage to contiguous membership identifiers
  and stable record pools with current positions. Test duplicate reference
  targets as distinct entries, membership generations and collection incarnations.
  Permit compact local order indices with exact identity reconstruction; saved
  membership/origin references retain every required generation and incarnation.
  Evidence: [checked list memberships](docs/implementation/list-membership.md).
  A new metadata component supplies contiguous size_t order indices, stable
  block-pooled records and full saved owner/field/incarnation/index/generation
  identities. Prepared insertion and reference replacement publish atomically;
  shifts/swaps preserve occurrence identity, removal advances generations or
  permanently retires exhausted slots, reset checks incarnation before effects.
  Six independent cases exercise duplicates, record-address stability, exact
  inverse positions, complete namespace checks, counter exhaustion and allocation
  failure with counted resources. Strict and ASan/UBSan component runs passed;
  full strict tests, all 1197 input coverage, links and reproducible Doxygen are
  checked before commit. Generated list storage integration belongs to T076;
  list acquisition and cursor publication are not claimed by this component.
- [x] **T076 — Implement indexed selection and complete list copies.** Needs:
  T065, T075. Check conversion/bounds/membership before target effects and preserve
  exact owned/reference transfer rules. Integrate inferred list receivers and
  list-owned clone boundaries deferred by the scalar execution slice. Test
  stale/empty indices, fresh copied memberships, whole-list self no-op and
  replacement invalidation.
  Evidence: [list value integration](docs/implementation/list-values.md).
  Declared fields now use compact membership order and stable record targets;
  the obsolete handle-vector capacity implementation is removed. Construction,
  cloning, forest fixups, calls, cleanup and generated assembly use the same
  representation. Indexed reference stores revalidate their saved occurrence;
  whole-list and stable-root replacements reserve fresh incarnations before
  destruction. Tests cover independent inferred owned results, duplicate inferred
  reference results, actual continuation aliasing, stale/empty targets and index
  operands, unchanged failure outputs, exact self no-op and counter exhaustion.
  Strict component runs, real generated-program and component ASan/UBSan passed;
  the full strict suite, all 1201 input coverage, links and reproducible Doxygen
  are checked before commit. Intrinsic services and foreach keep their guards.
- [x] **T077 — Implement list service acquisition and method frames.** Needs:
  T066, T076. Hold owner/list protection across sibling methods/results, permit
  only exact direct structural helper token borrowing and finalize ephemeral
  result storage once. Test alias/callback reentry refusal and escaped stale views.
  Evidence: [list service frames](docs/implementation/list-service-frames.md).
  Linear caller-owned grants borrow exact current/outgoing owner protection;
  collection identity and original frame address validate every structural borrow.
  Ordinary selection refuses busy lists without inheriting a grant through caller
  identity. Typed method frames retain the parent busy interval across siblings
  and result cleanup, consume owned result credits once and expire escaped handles.
  Six component cases cover aliases, copied/wrong-field grants, callback reentry,
  selected owner protection, constructor/destructor failure and bounded frame slots.
  A source-to-plan-to-compiled-C probe accepts actual lexical frame layouts without
  enabling unfinished intrinsic dispatch. Component ASan/UBSan passed; full strict
  tests, all 1214 input coverage, links and reproducible Doxygen are checked before
  commit. Mutation adapters remain guarded until their following method tasks.
- [x] **T078 — Implement append and insert.** Needs: T077. Support omitted default
  creation versus explicit owning clone or reference-handle copy. Reserve counters
  and prepare before publication; test invalid positions, clone capability, busy
  sources, allocation overflow/failure and unchanged destinations on refusal.
  Evidence: [typed append/insert execution](docs/implementation/list-insertion.md).
  Resolved descriptors retain source order, exact parameter ordinals and unique
  staging sites. Typed helpers construct integer inputs once, validate the declared
  index protocol, distinguish pure preflight refusal from later native failures,
  and publish reserved membership/ownership state before direct result actions.
  Selected intrinsic values use actual stored targets and may park an empty root
  service context; exact frame ancestry releases method/input credits and service
  grants before a local handler. Six runtime cases and six compiled projects cover
  independent clones, direct structural indices, stored task entry/resume, cleared
  ordinary copy positions, skipped siblings, receiver preservation, noncopyable
  default/placeholder elements and technical Busy. Explicit empty copy sources
  fail statically; full-host-size/negative positions and every allocation prefix
  have component evidence. Actual generated-program and component ASan/UBSan
  passed; full strict tests, all 1231 input coverage, links and reproducible Doxygen
  are checked before commit. Remaining methods and foreach stay guarded.
- [x] **T079 — Implement remove and clear.** Needs: T078. Preflight full affected
  owned subtrees, retire memberships and release resources in specified order.
  Test busy descendants, reference removal without destroying external targets,
  atomic no-partial clear and terminal destruction failure.
  Evidence: [typed remove/clear execution](docs/implementation/list-deletion.md).
  Shared parameter preparation evaluates sources once before declaration-order
  installation; zero-argument clear creates no integer or conversion work.
  Checked removal separates bounds/availability refusals from technical destructor
  failure. Complete owning forests are protected before any destruction; reference
  removal only retires occurrences. Five runtime cases and four compiled projects
  cover busy descendants, survivor identity/order, running external targets,
  noncopyable elements, empty clear, reverse terminal cleanup and every preflight
  allocation prefix. Insertion regressions, component/generated ASan/UBSan, full
  strict tests, all 1244 input coverage, links and reproducible Doxygen are checked
  before commit. Swap, length and foreach retain explicit execution guards.
- [x] **T080 — Implement swap and length.** Needs: T079. Preserve membership
  identity through swap and construct length via the fixed `..integer` contract.
  Use PROTO-03's typed `from_size` bridge exactly once, without decimal text or
  double construction; test the full supported host-size conversion boundary.
  Test both method positions, representability, sequential service actions and
  selected-result failures without obsolete retained outcome state.
  Evidence: [typed swap/length execution](docs/implementation/list-ordering.md).
  Swap installs two ordinary integer inputs and publishes one membership
  transposition without target acquisition or lifecycle work. Length binds the
  exact standard integer's header-declared bridge through a typed payload adapter
  and selects one fresh size constructor after normal Ama field initialization.
  Four runtime cases cover membership/target preservation, self-swap, either bad
  position, technical conversion failure, ephemeral result retirement, bridge
  failure and full-host-size round trips through SIZE_MAX. Four compiled projects
  verify swapped order, copied versus current length, empty length, sequential
  methods and technical failures in construction/selected result execution.
  Component and actual generated-program ASan/UBSan passed; full strict tests,
  all 1251 input coverage, links and reproducible Doxygen are checked before
  commit. Foreach remains explicitly guarded for its cursor tasks.
- [x] **T081 — Implement live foreach selection and bindings.** Needs: T031,
  T074, T080. Store one fixed lexical cursor/cell per instance, validate source
  incarnation and use short acquisitions. Test empty bodies/lists, absent referents,
  compatible reuse, no-shadow errors and busy live targets not silently skipped.
  Evidence: [live traversal selection](docs/implementation/list-traversal-selection.md).
  Stable generated cursor slots retain exact collection identities and scalar
  boundaries; binding cells are shared by exact semantic symbol identity. Short
  acquisitions end before each body, with no index/condition protocol, allocation
  or target acquisition. Only eligible entries activate locals; nested reuse
  replaces the shared handle/origin without restoring an outer value. Seven
  runtime cases cover absent-only scans, empty bodies/lists, busy duplicates,
  actual owned children, reused references, replacement invalidation and checked
  lexical offsets. A compiled C layout probe verifies three independent cursors
  and two binding cells; existing semantic suites cover incompatible shadowing.
  Runtime ASan/UBSan passed; full strict tests, all 1262 input coverage, links and
  reproducible Doxygen are checked before commit. Complete source execution stays
  guarded until T082–T084 integrate mutation, origin/resume and continuation state.
- [x] **T082 — Correct every active or suspended boundary on mutation.** Needs:
  T081. Apply exact insert/remove/clear adjustments and unchanged swap boundaries
  in each complete publication. Test nested same-list scans, insert at the gap,
  append, remove/reinsert, clear/append and swap repeats/skips. Record either the
  exact O(K) lexical scan or O(A) active/suspended registration strategy; compare
  corrected states and cover registration cleanup/copy/restart if implemented.
  Evidence: [fixed-scan mutation correction](docs/implementation/list-traversal-mutation.md).
  Immutable per-field catalogs use actual generated cursor offsets; context/schema
  checks reject invalid extents and duplicate slots before allocation/effects.
  Each mutation validates matching boundaries before native work and applies all
  corrections in callback-free publication. Range removal counts removed past
  positions without overflowing sums; irreversible destructor failure still
  publishes complete correction. Ten runtime cases exhaust small real list/range
  states against an independent oracle and cover gap insertion, clear/append,
  repeats/skips, inactive/old incarnations, allocation/busy refusal, terminal
  native cleanup, independent contexts and real yielded-owner mutation through
  another caller's service. The C layout probe validates actual emitted catalogs.
  Runtime/generator ASan/UBSan passed; full strict tests, all 1270 input coverage,
  links and reproducible Doxygen are checked before commit. The O(K) fixed scan
  needs no active registration or copy/restart links. Foreach source execution
  remains guarded pending T083/T084 origin/resume and continuation integration.
- [x] **T083 — Enforce missing-membership guards and resume behavior.** Needs:
  T082. Preserve origin associations, detach only as specified and validate active
  loops outermost first after resume. Test removed current entries with live
  external referents, nested reused bindings and collection replacement failure
  before any resumed user statement.
  Evidence: [iteration origins and resume decisions](docs/implementation/list-traversal-origin.md).
  Resolved accesses select exact local/reused cells, and borrowed value views keep
  origins out of ordinary handle copies. Guards precede target acquisition and
  decoder effects; different explicit assignments detach while same-handle writes
  preserve association. Owning-copy inputs retain one fixed origin tuple for
  documented preflight. Immutable checked scope preorder drives outer-first
  validation, technical header errors, descendant cleanup and corrected repeat
  selection without restoring overwritten shared origins. Eleven runtime cases
  cover all target modes, decoder suppression, independent references, busy-list
  authority, missing entries/referents, nested reuse, collection replacement,
  valid saved dispatch and target-free scope reset. The actual C probe checks
  generated scope ancestry, shared locations and local/persistent access bindings.
  Runtime/generator ASan/UBSan passed; full strict tests, all 1277 input coverage,
  links and reproducible Doxygen are checked before commit. Complete source
  controller integration and continuation/ordinary-copy acceptance remain T084.
- [x] **T084 — Copy and accept live traversal state.** Needs: T083. Map full
  collection/membership/origin identities in `%` clones, preserve boundary values
  and discard ordinary-copy control state. Verify removed-origin nonrevival,
  membership/clone failure paths, candidate-potential bounds and complete compiled
  mutation/traversal examples without snapshot arrays or hidden visited sets.
  Evidence: [executable traversal and copy contract](docs/implementation/list-traversal-execution.md).
  Generated foreach entry/selection/exit/reset/resume now execute through checked
  helpers and labels; the obsolete execution barrier is removed. Continuation
  preflight validates protected owned trees, then maps live membership identities,
  dead origins, boundaries and local/installed/current handles after destination
  lists exist. Ordinary copies clear controls. Stable replacements preserve final
  identity; separate forest roots receive a final control-reference remap.
  Eleven runtime copy cases cover removed-record nonrevival, exited-inner origins,
  owned-child continuation, mixed initializer modes, every allocation-failure
  prefix and later native failure. Seven compiled scenarios cover original
  targets, past/future insertion, origin refusal/detachment, independent resume,
  sibling references and ordinary-copy NoResume. The CLI resource fixture runs
  foreach and verifies exact cleanup. An independent candidate-budget test
  complements exhaustive mutation correction. Runtime/generated ASan/UBSan,
  full strict tests, all 1294 input coverage, standalone headers, local links and
  reproducible Doxygen are checked before commit.

## Phase 11. Coherent library contracts and explanatory examples

Contracts: [library](spec/language/library.md),
[acceptance](spec/implementation/tools-and-conformance.md#acceptance-matrix).

- [x] **T085 — Validate the scalar and canonical integer library.** Needs:
  T069, T080. Publish actual paths/interfaces and literal/clone/truth/index
  capabilities and the fixed host-size bridge, including the full supported
  length range for `..integer`. Test malformed data and nominal typing without
  automatic numeric assumptions.
  Evidence: [scalar library contract](docs/implementation/scalar-library.md).
  The maintained `..integer` owns signed arbitrary-precision base-256 magnitudes,
  explicit six-protocol ABI, typed shared zero/one constants and a header-declared
  `from_size` bridge for every `size_t`, without default/decimal construction.
  Native protocol tests now compile the actual library components instead of
  duplicated algorithms. They cover every limb boundary, `SIZE_MAX`, independent
  clones, malformed counted bytes, negative/overflow index refusal and allocation
  failure in default/literal/clone/size constructors. Seven generated-project
  cases check actual length/position helpers, constants, truth, retained values,
  nominal project-name separation and all six directed standard-scalar type
  mismatches. Bounded `..std.index`/`..std.condition` and their checked consumers
  now use actual standard-root declarations/native names; their compiled suite
  verifies ordering, malformed values and bounded arithmetic. CLI
  `--standard-root DIR` explicitly supplies standard authority to every source
  command, preserving argument order/exclusions without path-name inference.
  The maintained CLI project checks missing configuration and verified lengths
  three/zero. Native/generated ASan/UBSan, full strict tests, all 1305 input
  coverage, standalone headers, links and reproducible Doxygen are checked before
  commit. Expected-failure/resource and arithmetic-result alignment remain T086–T087.
- [x] **T086 — Validate refusal and resource library interfaces.** Needs: T085.
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
  Evidence: [failure/resource contract](docs/implementation/failure-values.md).
  `..failure` owns its public `description` text result and a native presence
  discriminator; defaults and explicit reentry are false/empty, condition/read
  preserve the value, and owned copies retain the complete outcome. Ordinary
  `..failure.make` lets Ama code construct a present failure from one message.
  Typed native set/clear helpers prepare the description before publishing the
  discriminator and propagate technical failures unchanged. `..text` provides
  counted scalar-valid UTF-8, allocation-free empty values, immutable shared
  backing, checked share acquisition and preparation-before-release replacement.
  Seven resource cases cover both destruction orders, aliasing, allocation and
  UTF-8 rollback, reduced-limit exhaustion and real owned-tree prefix rollback.
  Seven generated-project cases cover state transitions, retained descriptions,
  independent/same-name results, UTF-8, empty descriptions, the real constructor,
  last-refusal separation and description OOM bypassing `:`. Status/refusal
  providers and their existing consumer now use actual standard-root identities;
  a maintained CLI project verifies construction, positive check, retained read
  and clearing. Native/generated ASan/UBSan, full strict tests, all 1334 input
  coverage, standalone headers, local links and reproducible Doxygen are checked
  before commit. Arithmetic expected-failure result alignment remains T087.
- [x] **T087 — Standardize and test library compositions.** Needs: T084, T086.
  Keep parameter/result ordering predictable without list action APIs.
  Apply the single condition-capable failure-value convention to operations
  reporting expected failures; keep intrinsic list refusals and ordinary false
  predicates distinct. Test no stale result processing after a refused call.
  Integrate list-valued constant startup/cleanup with T052's context-local scalar
  foundation. Compile real ownership/list/native combinations and document actual
  capability limitations without parser-level domain operation names.
  Evidence: [arithmetic/list composition contract](docs/implementation/library-compositions.md).
  Bounded next/add now return result then one owned failure value; range overflow
  is expected data, later success clears the outcome, and predicates remain
  ordinary Boolean values. Canonical `..integer.add`/`next`/`before` expose exact
  signed arbitrary-precision arithmetic through normal typed module interfaces.
  Native magnitude ordering, unsigned carry/borrow and prepare-before-replace
  helpers avoid host narrowing and preserve aliased operands on allocation failure.
  Four native cases use complete bounded sign/order combinations, actual limb
  boundaries, independent decimal expectations beyond C ranges, destination
  aliases and injected preparation failure. Seven generated-project cases verify
  bounded outcomes, refusal preserving prior receivers, unbounded arithmetic,
  false predicates, mutable copies of text-list constants, two live contexts,
  nested owned lists in constants and startup failure cleaning the completed
  prefix before root publication. This connects the existing context-local
  constant builder, recipe graph and cleanup path without a new representation.
  Native/generated ASan/UBSan, full strict tests, all 1351 input coverage,
  standalone headers, local links and reproducible Doxygen are checked before
  commit. No domain-operation parser syntax or implicit numeric conversion added.
- [x] **T088 — Publish complete synchronous and continuation examples.** Needs:
  T087. Supply reproducible modules for persistent state, self-reference,
  constructor-owned methods, mutable list traversal and independent simulations.
  Include file-operation failure handling with a returned failure module, a
  positive `?` and a retained read of its description, with no separate flag.
  Execute expected outputs/state changes and explain the governing contracts.
  Evidence: [executable examples](docs/implementation/examples.md), maintained
  `tests/integration/examples/project/` and the constructor/resource project.
  Real CLI scenarios check returned self-reference, independent copied traversal
  continuations that consume their own lists, future append/current removal,
  empty traversal, ordinary-copy resume rejection and owned file failures with
  repeated retained descriptions followed by success. Target ASan/UBSan,
  strict whole tests, all 1372 maintained-input coverage, standalone headers,
  local links and reproducible Doxygen are checked before commit. Native file
  resources stay within one synchronous activation; no new language rules.
- [x] **T089 — Review usability through the examples.** Needs: T088. Improve
  diagnostics, source navigation metadata and Doxygen explanations using concrete
  failures and repeated workflows. Record proposed language changes as questions;
  do not alter `spec/` or infer new semantics while improving implementation.

  Evidence: [example usability review](docs/implementation/example-usability.md).
  Transitive copy errors now retain the first real owned-field witness alongside
  the operation, receiver and exact native ABI seed, with allocation-free cause
  and capability guidance. Direct capability failures and unrelated diagnostics
  preserve their source partitions. Tests cover scalar/list/deep ownership,
  reference exclusion, recursive ownership and exact CLI positions. The README
  directs readers to current examples/task evidence rather than the original
  audit's historical limits. Strict semantic subtree and real CLI examples,
  semantic ASan/UBSan, all 1375 input coverage, standalone headers, local links
  and reproducible Doxygen are checked before commit. No language change proposed.

## Phase 12. Compiled Ama libraries and C-module embedding

Contracts: [EMBED-01–EMBED-08](spec/implementation/c-library.md),
[native boundary](spec/implementation/native-interface.md),
[host control](spec/implementation/runtime.md#rt-14).

- [x] **T123 — Resolve explicit C-library exports and build inputs.** Needs:
  T040, T061, T087. Implement the documented export manifest, explicit component
  identity and target/ABI metadata without scanning module names or exporting all
  internals. Resolve exported entries and supported value schemas through the
  existing typed project. Test missing/duplicate exports, identity collisions,
  unsupported schemas, dependency invalidation and preservation of handwritten
  inputs. Keep the executable launcher and library target separate.
  Evidence: [explicit library exports](docs/implementation/c-library-exports.md),
  `src/library/` and the real `--library-check` command. The explicit editioned
  configuration selects module/operation exports, complete field mappings,
  supported owned wire profiles, target/ABI/convention and C/link/dependency
  inputs. One typed closure includes only exports and explicit schema needs;
  a separate component consumer tracks exact source/configuration/file changes.
  Full framed ABI keys include field ordinals and build settings, with tested
  unequal-key digest collision rejection. Native protocol/ownership negatives,
  missing/duplicate/unsupported records, every parse/resolve allocation failure,
  stream failure, dependency invalidation and handwritten-source preservation
  have component and real-CLI evidence. Strict whole tests, library ASan/UBSan,
  all 1408 input coverage, standalone headers, links and reproducible Doxygen
  are checked before commit. Metadata explicitly reports resolved stage; public
  headers, execution and target-verified artifacts follow in T124–T128.
- [x] **T124 — Generate the public C consumer header.** Needs: T055, T085,
  T086, T123. Emit self-contained namespaced declarations, opaque host identities,
  documented ownership and exact ABI/version checks from one export schema. Keep
  native-authoring headers and private runtime layouts outside this public
  boundary. Compile multiple independent C consumers and incompatible-header
  negatives; cover ERROR-01's single condition-capable failure value through its
  independent C snapshot rather than separate Boolean and reason results.
  Evidence: [public C header](docs/implementation/c-library-header.md),
  `src/library/header/` and real `--library-header` output. One model emits
  namespaced control identities, readonly typed borrows, opaque independent
  snapshot owners, coherent failure views, selected operations and ownership
  comments without private SDK includes. C11/edition/explicit Linux ELF LP64
  System V guards reject unsupported targets and default calling conventions.
  The generated primitive ABI receiver compares identity, counts, every public
  size/alignment/field offset before structure-based calls. Real independent
  C translation units exercise matching/mutated/packed receivers, forced
  cross-component hash collisions with unequal complete keys, foreign
  headers, unavailable/private interfaces, exact Unicode/quote/trigraph bytes,
  repeatability and stream failure. Strict whole tests, generator ASan/UBSan,
  all 1426 input coverage, standalone headers, links and reproducible Doxygen
  are checked before commit. Runtime/snapshot bodies remain T125–T128 work.
- [x] **T125 — Implement exported context and instance lifecycle.** Needs:
  T013, T074, T124. Create independent contexts, initialize their constants and
  explicitly construct exported instances without executing their bodies.
  Implement quiescent release/disposal with checked owner, generation and ABI
  identities. Test partial startup, retained completed/yielded state, cross-context
  access with equal local numbers, nonreentrant execution, failure cleanup and
  exactly-once disposal without automatically running saved continuations.
  Evidence: [explicit C lifecycle](docs/implementation/c-library-lifecycle.md).
  Generated ABI-checked public thunks use real project descriptors, independent
  context registries and explicit stable-slot roots; factory construction never
  enters an Ama body. Exact service/context/root identities, nonwrapping IDs,
  executor leases and explicit destruction-admission provenance separate refusals
  from admitted native failures (including native Busy). Terminal failure cleans
  roots/constants once, preserves the primary cause and leaves only diagnostic
  access/cleanup. An independent public-header C consumer links to a real Ama
  assembly and checks stale/cross-context controls, factories, disposal and
  immutable diagnostic lifetime after cleanup. Address-sensitive kernel composition
  checks yielded-state retention, native reentry, overlapping threads, exhausted
  IDs, null/nonempty destinations, pre-retirement refusal and every measured
  allocation failure in startup, root creation and deletion. Full strict tests,
  focused ASan/UBSan (leak detection disabled), all 1450 input coverage, standalone
  headers, prose links and Doxygen generation pass. Final reproducibility is
  checked before commit. Typed snapshots/entry and artifact packaging remain
  T126–T128; no compiled library artifact is claimed by this lifecycle generator.
- [x] **T126 — Implement typed inputs and independent result snapshots.** Needs:
  T069, T084, T086, T125. Generate checked factories, argument preparation and
  public value reads using the documented supported schemas; publish outputs
  only after successful preparation. No consumer receives internal Ama addresses,
  reference handles or continuation storage. Test byte lengths, empty values,
  malformed/schema-mismatched data, overflow/allocation rollback, repeated retained
  reads and snapshots that remain usable after instance/context disposal. Verify
  a failure snapshot contains both its condition value and owned description.
  Evidence: [typed C transport](docs/implementation/c-library-transport.md).
  Exact scaffold/schema mappings generate private typed codecs, input-shape
  predicates and synchronous prepared recipes without default-then-overwrite
  construction. Public retained reads check export-bound controls, result
  availability and distinct empty destinations, prepare independent snapshots
  privately and publish only the complete set. All scalar profiles and every
  supported scalar-list product have explicit view/clone/release ownership;
  canonical binary arbitrary precision never narrows or uses decimal conversion.
  Real generated C consumers check overwritten input buffers, snapshots after
  instance/context disposal, failure condition/description transitions, empty and
  embedded-NUL values, schema declarations before dependencies, incompatible C
  schema arguments, busy/skipped outputs, output aliases and retained positions.
  Every measured list-clone/read allocation is failed with exact ownership and
  unpublished-output assertions. Native construction tests cover all magnitude
  lengths through 257 octets and malformed-success cleanup credits; lifecycle
  reference scenarios include the transport constructor. Full strict tests,
  ASan/UBSan of generators and generated C (leak detection disabled), all 1475
  maintained input coverage, standalone headers, links and Doxygen generation
  pass; final reproducibility is checked before commit. Public entry/resume
  execution and component artifacts remain T127/T128.
- [x] **T127 — Implement exported entry, resume and refusal outcomes.** Needs:
  T067, T074, T126. Preserve the three Ama execution outcomes and report invalid
  C requests or operation refusals through the separate API result. Test restart
  versus resume, output-only resume, busy/absent/stale selections, independent
  application-failure values and context-ending technical failure. Reject foreign
  owners and invalid requests before entering Ama or changing execution state;
  do not introduce a fourth Ama outcome or an implicit scheduling loop.
  Evidence: [checked C execution](docs/implementation/c-library-execution.md).
  Launcher/library hosts share one private label dispatcher with exact root-type
  and saved-selector checks. Linear lease moves and retained finalization keep
  one executor owner through argument preparation, dispatch, cleanup and atomic
  snapshots; prepared C borrows/trace arrays end before body execution. Generated
  selected entry/resume implementations distinguish effect-free API refusal from
  API OK with completed/yielded/failed, preserve native/declared body state and
  forget invalidated root mappings before releasing access. Real public-only C
  consumers exercise actual percent yield/resume/restart, multiple root types,
  read-only aliases, missing resume, stale/foreign/wrong-export controls,
  empty and overlapping destinations (including partial outcome/owner overlap),
  normal application-failure values, native callback exclusion and primary cause
  preservation despite Busy cleanup failure. Every measured entry/resume
  allocation failure leaves unpublished outputs empty and releases all resources.
  Both assertion-enabled and NDEBUG components, zero-schema interfaces and
  selected-mode C compilation rejection are checked. Full strict tests,
  generator/generated-C ASan/UBSan (leak detection disabled), all 1489 input
  coverage, standalone headers, links and Doxygen generation pass; final
  reproducibility is checked before commit. Artifact packaging remains T128.
- [x] **T128 — Build static and shared components with pinned code lifetime.**
  Needs: T008, T010, T017, T123, T124, T127. Compile the complete Ama closure as
  one C translation unit and package its object as a static or supported shared
  library. Apply the component namespace to every externally linked symbol and
  keep generated internals private. Link two independently generated components
  into a multi-translation-unit C host, including overlapping Ama module names.
  Test target/ABI incompatibility, repeatable build metadata and refusal to unload
  code while contexts, callbacks or component-owned values still require it.
  Exercise lifetime pins with a minimal native probe here; the full asynchronous
  and concurrent component compositions follow in T099 and T106.
  Evidence: `--library-build` stages one complete closure, checks raw native/control
  entry families and the exact isolated public definition set, then publishes
  deterministic static and supported ELF shared artifacts through the owned
  publisher. Declared compiler/link settings reach tools as exact arguments.
  Two overlapping Ama/native closures execute through separate C translation
  units and a combined-header consumer under static/shared linking and real
  repeated `dlopen`/`dlclose`. Exact snapshot/diagnostic/callback pins reject
  premature unload; foreign owners and stale reload controls are rejected before
  dereference. Receipt/pin allocation failures permit effect-free retry; retired
  load identity history and the external loader call barrier are documented in
  `docs/implementation/c-library-packaging.md`. Strict full `NDEBUG` tests pass;
  package/execution/snapshot/runtime suites pass ASan/UBSan, including generated
  debug/release libraries. All 1520 maintained inputs have named test coverage
  and standalone header checks; Doxygen and its reproducibility checks pass.
- [x] **T129 — Own a foreign Ama component inside a native Ama module.** Needs:
  T012, T073, T128. Implement an example native adapter that owns an opaque child
  context as a native resource in another compiled component. Configure child
  constants/instances explicitly, transfer only supported independent C values
  and release them on all construction/destruction paths. Omit native clone
  support unless a genuine independent resource clone is supplied; do not copy
  an opaque pointer as value cloning. Test that the parent and child cannot
  exchange checked Ama references or use each other's instance identities.
  Evidence: `tests/unit/library/nested` compiles a real isolated child archive
  and a native parent holding only an exclusive foreign resource. Factories run
  no bodies, initialize child constants/defaults, and unwind rejected/failed
  construction before native destructor credit exists. Two children retain
  independent state; counted UTF-8/zero-byte values cross through owned C
  snapshots, all released on success/failure. Child yield completes the native
  parent call, explicit later steps resume the child, and abandonment never
  resumes it. Public gates reject active-parent and foreign/stale controls;
  ordinary/continuation copies and reference transport are rejected statically.
  Every measured parent constructor/step allocation failure releases children.
  The composed constant-bearing export exposed and now regresses the exact
  semantic-symbol-to-planned-storage-slot mapping used by C inputs/outputs.
  Packaging/nesting and CLI build leaves are connected to common test dispatch.
  Strict full `NDEBUG`, generated-code ASan/UBSan, 1542 per-file coverage/header
  checks and reproducible Doxygen pass. See `docs/implementation/c-library-nested.md`.
- [x] **T130 — Exercise nested suspension and failure supervision.** Needs:
  T074, T086, T129. Execute the exact chain parent Ama entry → native C adapter →
  child component context without reentering either active context. Have child
  yield return normally through C; explicitly return or yield the parent and
  resume each context only after its previous native activation has ended. Test
  retained child state, explicit parent policy for child failure, copied diagnostic
  data surviving child cleanup, independent parent continuation and deliberately
  requested propagation as a parent technical failure. Cover child abandonment/
  disposal without automatically resuming its saved position or retaining borrows.
  Evidence: `tests/unit/library/supervision` executes the complete outer Ama →
  native adapter → isolated child Ama chain with two independent child/parent
  yields, retained native address/state, fresh entry and abandonment. Explicitly
  uninlined C helper intervals end before every returned yield. A scalar-only
  foreign C observer verifies both active-context gates, including idle sibling
  instances, while retaining no Ama borrow. Declared `propagate` selects parent
  technical failure or a coherent copied `..failure`; secondary child cleanup
  failure cannot replace its primary report. Supervised children become terminal
  until explicit reset, which reconstructs fresh context/roots. Own report/input/
  snapshot allocation failures remain parent technical failures and retire every
  child resource, including collateral sibling cleanup. Consumers compile without
  the SDK; generated code and the shared lifecycle regressions pass ASan/UBSan.
  Strict full `NDEBUG`, 1573 per-file coverage/header checks and reproducible
  Doxygen pass. See `docs/implementation/c-library-supervision.md`.
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
| CORE-01–CORE-05; ARCH-01–ARCH-06; TEST-01–TEST-08 | T001–T005, T132–T151 and the completion criteria; T116–T122 |
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
