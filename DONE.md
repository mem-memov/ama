# Completed Ama work

This file preserves completed task records, their stable identifiers,
prerequisites, decisions and acceptance evidence. The initial roadmap and its
reported baseline below are historical checkpoints, not an alternative language
specification or a promise that old inventory counts describe later revisions.
The current product authority remains [spec/](spec/README.md).

Use [TODO.md](TODO.md) for active tasks, current completion criteria and useful
verification commands. Allocate new IDs across both files; never reuse an
archived ID. Move a completed task here with its full evidence in the same
commit that removes it from the active queue. Do not archive unfinished work or
an unresolved documentation-debt owner. The
[development standard](docs/development/README.md) defines the workflow.

T001–T155 were transferred from TODO.md without rewriting their task records.
Their presentation/dependency order and the initial requirement coverage map
are preserved. Follow the [current conformance evidence](docs/implementation/conformance/README.md)
for current implementation claims rather than inferring them from old checkboxes.

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
- [x] **T131 — Accept standalone and nested C-library consumption.** Needs:
  T089, T128, T130. Publish reproducible static/shared-library host and nested
  Ama/native/Ama examples with generated headers, manifest and exact build/link
  commands. Run independent-header, multi-C-file, two-component, ownership,
  yield/resume, failure, teardown and supported sanitizer tests; map every
  EMBED-01–EMBED-08 rule to current component and composed evidence. Explain the
  difference between same-context Ama calls and a separately compiled child
  component, and document actual supported targets and loader limitations.
  **Completed:** the CLI example builder publishes exact native authoring maps,
  public headers/manifests/symbol inventories, explicit configurations and ordered
  command records. Four no-SDK multi-file C executables cover standalone/nested
  static/shared linkage, two yields, independent snapshots, copied primary failure,
  child reset and balanced native retirement. Bootstrap/final SDK records compare
  complete identity keys; malformed models, existing outputs and failed tools are
  rejected. Strict full `NDEBUG`, generated-code ASan/UBSan and 1584 per-file
  coverage/header checks pass. Doxygen includes the published C hosts and validates
  every rule destination; reproducibility is checked before commit. Actual target,
  loader boundaries and EMBED-01–EMBED-08 evidence are documented in
  `docs/implementation/c-library-examples.md`.

## Phase 13. Explicit cooperative tasks and native events

Contracts: [ASYNC-01–ASYNC-12](spec/language/async-and-parallelism.md).

- [x] **T090 — Define and implement the ordinary task-phase interface.** Needs:
  T074, T087. Use typed Ready/Awaiting/Completed results through existing calls,
  not new syntax or universal task casts. Test phase/yield consistency, first
  entry versus later resume and explicit propagation of nested waiting. Keep
  phase separate from ERROR-01's failure value; test valid terminal description
  data and ensure false failure does not turn an awaiting task into completion.
  **Completed:** `..task.phase` exposes ordinary owned ready/awaiting/completed
  predicates; named constructor modules publish exactly one predicate without
  task execution, hidden references or compiler special cases. Independent tests
  exhaust choices and reject invalid publication before writes; a real standalone
  native consumer checks the generated write-view boundary. Generated execution
  covers retained reads, independent copies, explicit reset, first entry/resume,
  restart, nested waiting propagation, completed-task refusal and terminal failure
  description separately from phase. Both component scales pass ASan/UBSan;
  strict full `NDEBUG` and 1606 per-file coverage/header checks pass. Doxygen rule
  links and reproducibility are checked before commit. See
  `docs/implementation/task-phases.md`; enrollment and native events remain T091+.
- [x] **T091 — Implement visible scheduler ownership and enrollment.** Needs:
  T090. Store task references/owners, run epochs and registration generations in
  declared Ama state. Test restart/removal invalidation, nonwrapping exhaustion
  and rejection of stale enrollments without hidden hooks on ordinary calls.
  **Completed:** declared scalar `..task.generation`, its typed equality module
  and `..task.flag` support an executable typed Ama owner/registration graph.
  Owners retain tasks in persistent local storage; registrations hold only checked
  references and scalar snapshots. Replacement, restart, removal and reenrollment
  reject old generations/epochs; recreating an owner with matching numbers does
  not revive its old task reference. Ordinary entry/resume leaves protocol counters
  unchanged. Small-bound exhaustion, counted literals, actual native lifecycle,
  both exhausted identity counters and malformed native flags are covered.
  ASan/UBSan at component/composition scales, strict full `NDEBUG`, 1632 per-file
  coverage/header checks and Doxygen rule links pass; reproducibility is checked
  before commit. See `docs/implementation/task-enrollment.md`; native tokens and
  event reconciliation follow in T092–T093.
- [x] **T092 — Implement native submission and completion storage.** Needs:
  T012, T091. Keep native buffers/resources and service/submission tokens separate
  from Ama identities. Test synchronous completion during submission, token reuse
  and worker/callback paths that queue records without entering Ama.
  **Completed:** `src/native/events/` provides fixed native cells, immutable
  domain/service/submission tokens, creator-image callback operations, retained
  FIFO results and independently owned binary buffers. Prepare precedes arm;
  synchronous/worker completions only queue native data without allocation or
  Ama execution. Exact cell/service reuse, finite counter/refcount exhaustion,
  every initial allocation failure, duplicate callbacks and close/ack races are
  tested independently; actual code pins survive owner release until the final
  callback/byte owner retires. Separate and assembled forms pass ASan/UBSan,
  release-mode assembly and ThreadSanitizer through the supported POSIX harness.
  Strict full `NDEBUG`, 1660 per-file coverage/header checks and Doxygen rule links
  pass; reproducibility is checked before commit. See
  `docs/implementation/native-events.md`; the generated Ama access bridge and
  wait/readiness reconciliation remain T093.
- [x] **T093 — Implement registration and readiness reconciliation.** Needs:
  T092. Publish waits before arming, retain completion records, recheck before
  awaiting and match full identities on import. Exhaust small interleavings for
  completion before/after recheck, duplicate/late events and no lost wakeups.
  **Completed:** noncopyable `..task.events` owns actual selected native methods;
  its explicit manual notification source returns ordinary `..task.token` and
  `..task.packet` values with independently owned binary `..bytes` data. Typed Ama
  task/registration/mailbox composition publishes before arm, checks the live
  registration and complete identity on import, and reconciles readiness before
  enqueueing. Revocation overrides earlier readiness; an early resume rechecks
  and yields until mailbox delivery. Native tests exhaust six callback boundaries,
  including synchronous arm completion; generated execution covers six schedules,
  duplicate/stale notifications, revoked waits, stamp/token mismatches, retained
  packet copies and invalid service/method lifecycle operations. Standalone methods
  include their native core; the literal-binding suite now registers its standard
  root distinctly from its project `.bytes` fixture. Strict full `NDEBUG`, native
  and generated ASan/UBSan, native ThreadSanitizer, 1716 per-file coverage/header
  checks and Doxygen rule links pass. Documentation reproducibility is checked
  before commit. See `docs/implementation/task-registration.md`; unique ready
  membership, bounded reactor rounds and cancellation remain subsequent tasks.
- [x] **T094 — Implement unique FIFO ready membership.** Needs: T093. Coalesce
  readiness and append newly ready/yielding tasks at the tail. Test observed
  completion order, stable ordering within unordered batches and multiple typed
  queues without hidden references or duplicate task steps.
  **Completed:** the executable typed Ama queue schema owns explicit entries with
  checked record references and epoch snapshots. Enqueue coalesces shared membership;
  pop authenticates the head and clears its flag before stepping; Ready yields rejoin
  at the tail. Ordered native completions enqueue in acceptance order; unordered
  batches reconcile in stable registry append order. `..task.sequence` publishes
  nonwrapping snapshots from persistent private history, including initializer
  reentry after completion. Tests cover two nominal task domains, two matching
  queues, empty queue references, completed/late readiness, stale epochs and removed
  then replaced owners. Native instrumentation rejects execution while still queued,
  including under `NDEBUG`. Strict full tests, generated ASan/UBSan, 1737 per-file
  coverage/header checks and Doxygen rule links pass; reproducibility is checked
  before commit. See `docs/implementation/ready-queues.md`. Bounded rounds, reactor
  waits, detachment and copy/failure boundaries remain subsequent tasks.
- [x] **T095 — Implement bounded scheduling rounds and reactor waits.** Needs:
  T094. Enforce finite positive step/import budgets and queue service bounds;
  import before blocking and report quiescence. Test continuously replenished
  work, wakeup races and event fairness without treating live foreach as a budget.
  **Completed:** the actual typed Ama round validates `1..SIZE_MAX` quotas, bounds
  queue decisions/task steps and native imports, and alternates two nominal domains
  with a persistent cursor. Backlog gets another bounded round; an idle audit checks
  task phases, liveness and both wait stamps before Waiting/Quiescent. Standard
  `..task.budget.check` and `..task.require` reject invalid states. `..task.events`
  exposes actual poll/wait methods backed by an exact armed-cell partition and one
  retained POSIX wake credit. Native tests force the check-to-sleep gap, retry EINTR,
  inject initialization/wait errors and prove finite stale-credit drainage and final
  retirement. Generated execution covers perpetual Ready replenishment, event/queue
  fairness, finite backlog, actual gated worker wake, zero/negative/overflow quotas,
  empty maximum quotas and broken live-task teardown without continuation execution.
  Strict full `NDEBUG`, native/generated ASan/UBSan, native ThreadSanitizer, 1788
  per-file coverage/header checks and Doxygen rule links pass; reproducibility is
  checked before commit. See `docs/implementation/scheduling-rounds.md`. Complete
  detachment/cancellation and loadable-provider host acceptance remain T096–T099.
- [x] **T096 — Implement registration detachment and native record retirement.**
  Needs: T095. Exact full-token detachment unlinks prepared, armed and completed
  native cells without changing survivor FIFO order or notification credits;
  queued bytes retire outside the gate and callback rights remain independently
  owned until acknowledgment. Checked synchronous reference visitors let the
  noncopyable `..task.operation` acquire native rights from an explicit source
  without retaining Ama views. Restart and lifecycle cleanup retire its old token.
  Tests cover each FIFO position, stale/foreign/closed identities, exact native
  resource counters, a real late worker after immediate cell reuse, source
  reconstruction, queued-result removal, suspended task removal and context
  teardown with source-first destruction. Existing checked identity/stamp
  validators exclude stale visible registrations before delivery. Strict full
  `NDEBUG`, native/generated ASan/UBSan, native ThreadSanitizer, 1805 per-file
  coverage/header checks, Doxygen rule links and prose checks pass; final
  documentation reproducibility is checked before commit. See
  `docs/implementation/task-detachment.md`. Cancellation and loadable-provider
  acceptance remain T097–T099.
- [x] **T097 — Implement cooperative cancellation terminal outcomes.** Needs:
  T096. `..task.outcome` commits an absorbing Pending/Cancelled/Completed value at
  the explicit task step boundary; arrival/import alone does not commit completion.
  `..task.operation.detach` retires delivery through a checked reference visitor
  without interrupting workers or consuming their independent callback rights.
  Ordinary typed cancellation requests wake Awaiting tasks through the existing
  coalescing FIFO. The maintained Ama composition invalidates its visible wait
  before native detachment, clears owned mailbox bytes and publishes Completed
  phase with a separate ordinary outcome. Full transition algebra, cold requests,
  both commit orderings, available-but-uncommitted bytes, premature resume,
  idempotent cleanup and fresh enrollment after cancellation pass. The restart
  case explicitly resets the private loop predicate and rejects old native tokens.
  Strict full `NDEBUG`, component/generated ASan/UBSan, 1832 per-file coverage and
  standalone header checks, Doxygen rule links and prose checks pass; final
  documentation reproducibility is checked before commit. See
  `docs/implementation/task-cancellation.md`. Destruction remains ownership cleanup
  and never runs the remaining continuation.
- [x] **T098 — Enforce copy and failure boundaries for tasks.** Needs: T072,
  T097. `..task.enrollment` makes an owning record/registry noncopyable through
  declared native capability checks, including empty owning record lists; tasks
  and external references remain ordinary copyable values. Explicit snapshot
  adoption publishes a fresh record epoch and requires separate enqueue. Native
  operations remain noncopyable; cloned tokens/shared references create neither
  another operation nor an additional single-consumer completion right. Fixed a
  runtime regression that cleared explicit continuation input positions: selected
  source mode now reaches scalar replacement, `% reference&` snapshots its target,
  and continuation controls validate before any input field replacement. Explicit
  reference acquisition keeps its separate reference-shaped mode. A host probe
  proves both Awaiting tasks detach before Failed, repeated resume never enters
  Ama, and retained native bytes/endpoints can acknowledge afterward without
  running suspended bodies. Strict full `NDEBUG`, selection/preparation/generated
  ASan/UBSan, 1858 per-file coverage and standalone header checks pass, including
  the decomposed fixture/copy/capability/failure test support. Prose/rule links
  pass; final Doxygen reproducibility is checked before commit. See
  `docs/implementation/task-copy-and-failure.md`.
- [x] **T099 — Accept the asynchronous library and supported host.** Needs:
  T089, T098, T131. Published runnable typed-task and SDK-free native-host examples
  through actual product commands. Native services receive a move-only code
  credit independent of context lifetime; event services retain it transitively
  through endpoints and byte buffers, including constructor-failure cleanup.
  Actual unload/reload tests cover immediate/delayed completion, cancellation,
  restart with stale events, pending context disposal and exported snapshots;
  the host joins active callbacks before testing unload readiness. Registration,
  bounded-round and cancellation suites supply the remaining ASYNC races and
  scheduling bounds. Strict full `NDEBUG`, native/component ASan/UBSan and TSan,
  public example commands and negative CLI/metadata cases pass. Per-file coverage
  and standalone headers cover 1902 maintained inputs with no documentation debt.
  Doxygen rule links and prose checks pass; final documentation reproducibility
  is checked before commit. See `docs/implementation/asynchronous-host.md`.

## Phase 14. Isolated parallel contexts and explicit messages

Contracts: [PAR-01–PAR-08](spec/language/async-and-parallelism.md#parallelism-uses-isolated-execution-contexts).

- [x] **T100 — Audit context-local generated and native state.** Needs: T061,
  T099. Recorded exact private pool/constant/control/diagnostic ownership and the
  synchronized identity, registry, component-code and native-event metadata.
  Audited shipped scalar/resource hooks, context-confined text/bytes share counts,
  constant lifecycle and native dependency obligations. The asynchronous example
  backend requires one serialized host; unaudited dependencies need documented
  serialization or process isolation. New real-runtime barriers force concurrent
  constant construction/destruction, checking disjoint resources/resolvers,
  colliding local triples, foreign-origin rejection, private diagnostics/control,
  clone/reuse without stale revival and exact resource cleanup. Strict isolation,
  context, shared-storage, library, event and generated-context suites pass, as do
  the new lifecycle probe under ASan/UBSan and TSan. Per-file coverage, standalone
  headers, prose/rule links and final Doxygen reproducibility are checked before
  commit. See `docs/implementation/context-isolation-audit.md`.
- [x] **T101 — Execute two contexts concurrently.** Needs: T100. The actual
  generated dispatcher already takes an explicit admitted view; new acceptance
  runs its independent startup/yield/resume on POSIX threads and keeps assertions
  effective in the generated `NDEBUG` consumer. Checks all context-local constant
  storage, equal root triples with foreign-origin rejection, scalar slot reuse,
  busy reentry/disposal and exact binary packet import while a peer remains in
  an active native entry. The peer then fails technically and closes both old
  and current native services; callback rights retire afterward while the healthy
  context restarts and completes again. The shared fixture now propagates TSan
  separately from ASan/UBSan to the generated executable. Strict parallel,
  existing generated-context and execution suites, generated ASan/UBSan and TSan
  pass. Per-file coverage checks 1914 inputs with no debt; prose/rule links and
  final Doxygen reproducibility are checked before commit. See
  `docs/implementation/parallel-context-execution.md`.
- [x] **T102 — Implement quiescent executor transfer.** Needs: T101. The
  existing release/acquire context gate composes with an explicit synchronized
  host owner/command protocol. Documented that moving C lease storage does not
  itself publish thread ownership or end native borrows. Compiled acceptance
  uses two permanent POSIX workers: active native entry and retained inspection
  refuse transfer, then yield/resume crosses zero-to-one-to-zero with unchanged
  context/root identity, preserved selectors and pending native event delivery.
  A separate real pthread-tagged native resource declares pinned affinity;
  transfer is refused and entry/destruction stay on its original worker. Strict
  generated/runtime context checks, root build, generated ASan/UBSan and TSan
  pass. Per-file coverage and standalone headers check 1922 inputs without debt;
  prose/rule links and final Doxygen reproducibility are checked before commit.
  See `docs/implementation/executor-transfer.md`.
- [x] **T103 — Implement typed message encoding and decoding.** Needs: T086,
  T101. Added an explicit native record profile for bounded scalar/list condition,
  canonical arbitrary-precision integer, UTF-8 text and binary fields. Fixed wire
  format, exact schema name/version, complete size/shape/content validation and
  allocation-free admission precede independent receiver storage. Message owners
  retain native creator-code credits, contain no Ama identity/control/resource
  serialization, and clean every allocation prefix. Actual typed Ama bindings
  construct owned receiver fields, expose malformed data as an ordinary failure,
  and prove exact cleanup after a technical error following child construction.
  Independent wire vectors, all truncated prefixes, every one-byte substitution
  of the one-condition grammar, canonical/limit/empty cases and allocation-failure
  injection pass. Strict full `NDEBUG`, native/generated ASan/UBSan and exact
  generated allocation counters pass. Per-file coverage/standalone headers check
  1948 inputs without debt; Doxygen/prose/rule links pass and final reproducibility
  is checked before commit. See `docs/implementation/typed-messages.md`.
- [x] **T104 — Implement bounded single-sender/single-receiver channels.** Needs:
  T103. Native move-only creator-dispatch endpoints own a copied typed schema,
  fixed ring and independent code credit. Send validates whole envelopes before
  atomic count/byte admission and preserves every rejected owner; take moves only
  the FIFO head and refunds its exact encoded length before decoding. Sender drop
  closes and preserves accepted data; receiver drop aborts and retires detached
  owners outside the gate. Taken data outlives both ports. Added explicit public
  schema configuration checking, without inferring it from dummy wire data.
  Count/byte/variable-length limits, ring reuse, schema independence, allocation
  prefixes, independent channels, 4096 real native SPSC messages and 128 abort/send
  races pass. Actual isolated Ama contexts expose Full as an ordinary failure and
  decode queued values after sender-context disposal. Release accounting covers
  compiler-folded calloc as well as malloc/free. Strict full `NDEBUG`, optimized
  native, native/generated ASan/UBSan and native TSan checks pass. Per-file/header
  coverage checks 1971 inputs without debt; Doxygen/prose/rule links pass and final
  reproducibility is checked before commit. See `docs/implementation/bounded-channels.md`.
- [x] **T105 — Integrate channel waits and endpoint shutdown.** Needs: T097,
  T104. Added one-shot sender/receiver readiness registrations with exact byte
  predicates, atomic check/install, independently owned wait handles and full-key
  cancellation. Event admission distinguishes armed, prepared, completed, retired
  and closed tokens; callbacks post only native records outside channel gates.
  Close/abort wakes peers; native metadata/code survive context and endpoint
  teardown until the last registration acknowledges. Native tests cover reference
  exhaustion, replacement generations, both readiness timings, host blocking and
  128 registration/publication plus 128 cancellation/publication races. Compiled
  Ama tasks cover unrelated ready work, premature resume, cancellation, sender
  failure, receiver failure and drain after sender disposal. Strict full NDEBUG,
  native/generated ASan/UBSan and native TSan pass. Documentation covers all 1994
  maintained inputs without debt; Doxygen/prose links pass, with reproducibility
  checked before commit. See `docs/implementation/channel-waits.md`.
- [x] **T106 — Accept isolated parallel execution and supervision.** Needs:
  T102, T105. Added a maintained public two-actor host with real simultaneous
  executors and explicit continue-peer versus cancel-peer policies for the same
  receiver failure. Shared exact Ama/native fixture installation with T105;
  probe roles are constructed per payload before concurrency, and native-only
  coordination cannot select another context's fields. Normal delivery and both
  policies check typed outcomes, closed sources and exact atomic allocation
  ownership. Strict/optimized NDEBUG, generated ASan/UBSan and TSan pass; the
  original channel-wait composition regression passes. Ran the supported dynamic
  native-worker example through the product CLI with actual completion/unload.
  The PAR matrix maps isolation, identity, transfer, codecs, queues, callbacks,
  separately generated namespaces and nested child resources to their executable
  evidence. The recursive native synchronization/affinity/code-lifetime audit
  explicitly preserves serialized observer preconditions and requires processes
  when native failure containment is needed. No unrestricted nested-adapter
  concurrency is inferred from namespace separation. The preceding full strict
  T105 baseline remains unchanged in production code; 1998 maintained inputs have
  documentation/test evidence without debt. Final Doxygen reproducibility is
  checked before commit. See `docs/implementation/parallel-supervision.md` and
  `examples/parallel/README.md`.

## Phase 15. Reusable exploration, debugging and relocation tools

Contracts: [tools](spec/implementation/tools-and-conformance.md),
[diagnostics](spec/implementation/diagnostics.md).

- [x] **T107 — Expose reusable editor analysis results.** Needs: T060, T089.
  Added an immutable editor snapshot API over the production frontend, source
  catalog, lexer and checked semantic graph. Exact original spans support lexical
  highlighting, definition/type navigation, visible-name completion, declared
  signatures, intrinsic operation/binding metadata and structured errors/warnings.
  Draft/invalid input exposes no guessed partial semantic graph. Added exact
  comment spans and a shared pure constructor/lexical visibility predicate,
  preserving compiler diagnostics. The nonrecursive O(R log R) occurrence index
  distinguishes caller receivers, callee labels, selected returned methods,
  qualified constants, nested indices and separately scoped foreach identities.
  Synthetic output-transfer metadata creates no invented source occurrence.
  Tests cover multiword/Unicode names, CRLF/literals/comments, incomplete drafts,
  changed-file snapshots retaining old definitions, inferred introduction order,
  constructor availability, virtual list signatures, warnings, missing inputs and
  every measured allocation-failure prefix. Strict full NDEBUG and all editor
  ASan/UBSan checks pass; 2019 maintained inputs have registered evidence without
  debt. Doxygen/prose/standalone-header checks pass, with final reproducibility
  checked before commit. See `docs/implementation/editor-analysis.md`.
- [x] **T108 — Implement identity-aware rename.** Needs: T107. Added complete
  active-workspace analysis, including unused importers and opaque draft version
  checkpoints, through shared prepared-catalog frontend entry points. Rename plans
  select exact checked declaration identities, validate complete replacement names
  and preserve every non-name byte, reference/list suffix, comment, literal and
  line ending. Fresh validation checks source identity/bytes and tracked native
  dependencies, reparses/reresolves all active modules, then proves declaration,
  occurrence, initializer, binding/action and control graph correspondence.
  A compiling candidate that captures a foreach cell is rejected; disjoint local
  and later inferred names remain valid. The all-file client document transaction
  is revalidated immediately before publication and never invoked on failed
  validation or no-op. This is the editor core contract; filesystem recovery and
  user-facing command integration remain in their later tooling tasks. Tests
  cover cross-file qualified constants and callee labels, real standard types,
  Unicode, CRLF/literals/comments, indices/intrinsic labels, returned-method
  refusal, branches/loops, stale source/native inputs, rejected atomic document
  transactions and every measured workspace/plan/candidate allocation ordinal.
  Strict full NDEBUG and full editor/source-catalog ASan/UBSan pass. All 2050
  maintained inputs have named coverage without debt; Doxygen/prose/header
  checks pass, with final reproducibility checked before commit.
  See `docs/implementation/editor-rename.md`.
- [x] **T109 — Expose explicit debugger inspection.** Needs: T084, T099, T106.
  Exclusive linear inspection sessions expose actual live types, declared
  parameter/result/persistent fields, list acquisition/memberships, saved
  positions, lexical cursor/binding roles, refusal and context state. Declared
  task records use the same field projections; opaque native state requires an
  explicitly registered synchronous scalar reader. The compiler emits original
  UTF-8 names by exact nominal/storage/scope identities, independently of C names.
  Terminal cleanup retains status rather than fabricated instances or history.
  Tests cover actual competing threads, foreign-origin collisions, stale/busy
  instances, callback lifetime, invalid catalogs/readers, malformed previews and
  a native payload protected with PROT_NONE. Generated Russian-named traversal
  tests inspect two successive yields without advancing execution, distinguish
  rejected resume from accepted native failure, and check cleared caller links.
  Strict full NDEBUG and runtime/generated ASan/UBSan pass. All 2069 maintained
  inputs have named coverage without debt; standalone headers and Doxygen links
  pass, with final reproducibility checked before commit. This is the inspection
  core; user-facing integration remains T115.
  See `docs/implementation/debugger-inspection.md`.
- [x] **T110 — Build complete relocation discovery and preview.** Needs: T018,
  T040, T107. The read-only core owns exact endpoint/root settings, the complete
  selected physical tree, every original source and a projected final registry.
  It includes unused/external importers, opaque draft bodies, existing destination
  sources and companions/assets without requiring an executable entry. The
  filesystem view redirects D to S, removes S and inserts D without staging files.
  Exact Unicode candidate checks, original inode identities and final SRC-05
  anchors reject aliases, collisions, uncovered sources and orphan plus storage.
  Tests cover carried root containers, ordinary/standard family changes, explicit
  new roots, empty/constants-only domains, Unicode/multiword paths, long cwd,
  shared prefixes, parent write permission, draft anchors and opaque link cycles.
  Every measured allocation ordinal preserves originals and releases its prefix;
  failure reporting itself allocates nothing and preserves original IO causes.
  Strict compiler/preview and source/project/editor regressions plus preview
  ASan/UBSan pass. All 2094 maintained inputs have named coverage without debt;
  standalone headers and final reproducible Doxygen are checked before commit.
  Reference edits, graph proof, staged configuration and recoverable publication
  remain T111–T115. See `docs/implementation/relocation-preview.md`.
- [x] **T111 — Derive relocation identity and reference maps.** Needs: T110.
  Compute complete physical/root/module mappings and resolved path-span edits.
  Test injectivity and graph preservation across cycles, qualified constants,
  unchanged logical identities and importers outside the moved directory.
  Completed: captured source catalogs and entry-free project domains feed the
  ordinary parser/semantic stages; every resolved explicit/implicit edge maps
  through exact physical/root identity records. Original path-prefix edits
  deduplicate and reject overlap; qualified members/suffixes and other bytes are
  preserved. AMB-002 is resolved by the user's explicit specification decision:
  newly covered sources join original analysis and initial name conflicts reject.
  Evidence: strict compiler build and full suite; focused preview/reference,
  source-catalog and project-closure suites; reference/catalog ASan+UBSan with
  exhaustive allocation failure and exact live-allocation checks; standalone
  headers, complete per-file documentation and reproducible Doxygen. Final
  staged graph validation/publication remain T112–T115. See
  `docs/implementation/relocation-references.md`.
- [x] **T112 — Validate staged relocation bytes and configuration.** Needs:
  T111. Preserve all non-path bytes and opaque companion/assets; update only
  documented project settings and invalidate generated outputs. Reparse and
  reresolve staged active files; test line endings, comments/literals and drafts.
  Completed: exact original-span rewriting preserves all other source bytes;
  captured opaque files and literal links retain their bytes. The final catalog,
  native companion snapshots, ordinary parser/semantic stages and translated
  edge comparison independently prove the staged graph equals F(G). Explicit
  entry/library fields and foreign-file referents are prepared through existing
  configuration rules; generated-output invalidations reject protected inputs,
  roots, aliases and overlapping requests. No source publication occurs here.
  Evidence: strict compiler build/full suite; staged/reference/configuration
  regressions; staged/catalog ASan+UBSan; every measured allocation failure;
  exhaustive two-span byte/boundary model; stale-input/redirected-edge refusals,
  opaque binary/long/cyclic links, new/empty domains and actual write denial;
  complete per-file/standalone-header checks and reproducible Doxygen. Recovery
  and publication remain T113/T114, commands T115. See
  `docs/implementation/relocation-staging.md`.
- [x] **T113 — Apply relocation as a recoverable transaction.** Needs: T112.
  Require exclusive access and unchanged original file identities/bytes; stage
  moves/importer/configuration edits across supported filesystems. Inject I/O
  failures and verify exact rollback with originals retained until commit.
  Completed: descriptor-backed exclusive directory leases cover the full plan;
  independent discovery/binding/staging checks run before and after private copy
  preparation. Source trees, external importer/configuration files and explicit
  output retirement use filesystem-local saved originals and no-replace renames.
  Ordered inverses restore precommit changes; failed inverses and uncertain commit
  retraction retain both states and separate causes. Committed journals remain as
  outcome receipts while private backups retire. Automatic interruption replay
  and completed recovery reporting remain T114; tool integration remains T115.
  Evidence: strict compiler/full suite; real same/cross-filesystem application and
  ordinary frontend reload; all 254 measured I/O and 1280 application allocation
  refusals; complete lease/revalidation failure prefixes; ownership/byte/attribute
  and ambiguous-commit tests; exact original file identities/bytes/modes, descriptor
  and memory checks; all four transaction leaves under ASan+UBSan; standalone
  headers, complete per-file coverage and reproducible Doxygen. See
  `docs/implementation/relocation-transaction.md`.
- [x] **T114 — Implement relocation recovery and reporting.** Needs: T113.
  Persist commit/rollback state and recover idempotently after interruption. Test
  incomplete rollback retains recovery data and reports affected paths; never
  report success for a partly applied move or rewrite text twice.
  Completed: independent owned scope/settings/identity/file reports and strict
  framed preparation/plan journals survive compiler/executor destruction. Durable
  slot reservations precede creation, identity acknowledgements precede copied
  data, and READY precedes publication. Validated uncommitted inverses restore
  originals; committed recovery acknowledges its decision before retiring backups.
  Restored/retired parents synchronize before terminal receipts; repeated terminal
  recovery acknowledges the receipt without touching later user edits. Conflicts
  retain both states and diagnostics; no source rewrite is executed during recovery.
  Evidence: strict compiler/full suite, followed by final strict transaction/recovery
  reruns after the receipt/retirement synchronization guard; real same/different
  filesystem recovery with settings, invalidations, opaque binary bytes, cyclic
  links and drafts; all 529 writer plus 49/33 recovery process-interruption points;
  pending/terminal I/O refusals 49/33/8/8 and allocation refusals 258/252/162/162;
  expanded application 1066 I/O and 1324 allocation refusals, exact bytes/identities,
  live allocations and descriptors; report, all four recovery and all four
  transaction leaves under ASan/UBSan. All 2240 maintained inputs have named
  coverage without debt, standalone headers and local prose links pass, and two
  complete Doxygen generations are byte-identical (6424 documented interfaces).
  User-facing tool workflows remain T115. See
  `docs/implementation/relocation-recovery.md`.
- [x] **T152 — Expose source-tooling commands.** Needs: T107, T108.
  Add exact CLI requests for editor snapshot queries and a validated identity-aware
  rename plan. Reuse the production APIs, explicit roots and original UTF-8 byte
  offsets; provide lossless structured output and versioned full-file replacements
  for a client-owned atomic document transaction. Test complete/draft/invalid input,
  Unicode/control bytes, absent queries, stale/conflicting edits, output/allocation
  failure and actual command processes. Document the one-shot filesystem snapshot
  boundary and avoid claiming unsupported editor plugins or filesystem rename apply.
  Completed: exact `--editor`/`--rename-plan` requests reuse immutable compiler
  analysis and fresh whole-workspace rename validation. Lossless JSON preserves
  UTF-8/control/arbitrary bytes, full-width identities, original source spans,
  declared interfaces, exact point definitions/completions and nominal/intrinsic
  binding metadata. Plans contain complete original/replacement versions and
  original declarations; no source write or native execution occurs. Client
  whole-workspace authority and atomic publication requirements remain explicit.
  Evidence: strict compiler/full suite; independent standard JSON decoding;
  actual process queries and three-file renames including an unreachable importer,
  no-op/refusal and unchanged opaque drafts/C; related/Unicode/CRLF/source-state
  tests, all 201 editor and 635 rename allocation refusals and output failures;
  JSON/driver-source tests under ASan/UBSan and semantic-list/rename-core regressions.
  All 2274 maintained inputs have named evidence without debt, standalone headers
  and prose links pass; two complete Doxygen generations are byte-identical
  (6511 documented interfaces). See `docs/implementation/source-tool-commands.md`.
- [x] **T153 — Expose relocation commands.** Needs: T114, T152.
  Add explicit read-only preview, transactional apply and journal recovery commands.
  Carry supplied root kinds/exclusions, destination-root settings, optional entry,
  supported library configurations and explicit output invalidations through the
  existing complete plan; report exact owned scope and durable outcome. Exercise
  real commands, refused inputs, changed/foreign state, repeated recovery and
  output failure after commit without claiming that the move rolled back.
  Completed: explicit preview/apply/recover commands preserve supplied root kinds,
  exclusions, configurations, entry and invalidations through the existing exact
  planner and durable transaction. Lossless JSON reports owned scope, original
  source edits, retained receipt, cleanup and commit independently of stdout
  acknowledgement; unvalidated recovery authority leaves the outcome unknown.
  Evidence: strict compiler build and all unit suites; the complete integration
  suite passed after replacing two obsolete fixed-size help-test buffers with
  exact-size reads. Real processes cover Unicode paths, opaque/draft/binary
  preservation, configuration rewrites, output invalidation, added-root initial
  ambiguity and repeated recovery. Component tests cover post-commit output
  failure, unknown recovery and all 287 preparation allocation refusals, also
  under ASan/UBSan. All 2294 maintained inputs have named evidence without debt;
  standalone headers and local prose links pass, and two complete Doxygen
  generations are byte-identical (6565 documented interfaces).
  See `docs/implementation/relocation-commands.md`.
- [x] **T154 — Provide an executable debugger client.** Needs: T109, T152.
  Supply a documented client for retained generated project assemblies, using the
  checked runtime inspection API and explicit run/resume/dispose boundaries.
  Inspect actual names, declared fields/lists/cursors and source positions at rest;
  keep native previews adapter-controlled and inspect without advancing execution.
  Test real compiled projects, repeated yields, terminal/refused execution, invalid
  commands and output failure. Describe the generated-assembly build boundary,
  unsupported external debugger/editor integration and actual native limitations.
  Completed: the shipped retained-assembly launcher and reusable heap-free loop
  support exact run/resume/inspect/dispose/quit commands, with exclusively created
  JSON response files separate from native stdout. Checked inspection streams
  original names, context-qualified identities, declared scalar/list fields,
  occurrences, retained cursors and exact next-operation source positions. Native
  previews require explicit host-authorized readers; the default grants none.
  Every valid exit releases inspection and attempts idempotent host disposal.
  Evidence: strict compiler build, complete unit/integration tool subtrees,
  runtime and generated-debugger regressions; client tests under ASan/UBSan.
  Independent JSON decoding covers repeated real yields, unchanged observations,
  four native preview formats, separate preview/program failures, terminal and
  disposed hosts, command-byte boundaries, IO failure during observation and after
  execution, EOF and preserved existing response files. All 2313 maintained
  inputs have named evidence without debt; standalone headers and prose links
  pass, and two complete Doxygen generations are byte-identical (6630 documented
  interfaces). See `docs/implementation/debugger-client.md`.
- [x] **T115 — Accept user-facing tools through real projects.** Needs: T108,
  T109, T114, T152, T153, T154. Run editor/rename/debugger/relocation workflows against complete
  projects and regenerate verified output afterward. Document exact scope,
  preserved opaque files, unsupported editors and actual tool limitations.
  Completed: an actual process workflow queries a Russian constant, prepares and
  exclusively publishes a three-file declaration-identity rename including an
  unreachable importer, verifies/debugs execution, previews/applies a logical
  subtree move, then carries all ordinary/standard roots and real native
  companions with unchanged logical identities. Fresh verified builds and the
  shipped debugger execute two yields and native entries after every transition.
  Complete byte inventories preserve comments, CRLF drafts, opaque C, assets and
  native implementation helpers. Supported export rewrites, explicit artifact
  invalidation, returned root kinds/paths, repeated receipt recovery and historical
  receipt replay after a later root move are checked. Client authority, external
  editor/native-C migration limits and fresh-output boundaries remain explicit.
  Evidence: strict compiler/process builds and the complete integration-tools
  parent pass; all 2324 maintained inputs have named evidence without debt,
  standalone headers/prose links pass, and two complete Doxygen generations are
  byte-identical (6645 documented interfaces).
  See `docs/implementation/tool-workflows.md`.

## Phase 16. Complete coverage, conformance and release evidence

Contracts: [acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix),
[exact algorithms](spec/implementation/compiler.md#exact-algorithms-and-formal-justification).

- [x] **T116 — Close every code-file documentation and test gap.** Needs: T005,
  T106, T115, T131. Complete the inventory for compiler/runtime/library/tools,
  exported components, native embedding adapters and headers;
  map every code file to meaningful behavioral or compile tests. Generate complete
  Doxygen output and check every substantial component's contract/proof/evidence.
  Completed: reconciled all 2324 maintained inputs with 1235 distinct named C
  cases, 238 executable command registrations, 291 standalone headers and 138
  actual contract destinations. Reviewed source/IR/ownership/list/context/native/
  library/export/async/tool/build contracts and real independent/parent consumers;
  corrected stale claims that completed features and test homes were still planned.
  Updated six source/header purpose comments without changing C operations or
  canonical specifications. Historical audits retain their original scope.
  Evidence: strict compiler build, independent documentation positive/negative/
  rendering/coverage tests, complete zero-debt inventory and actual test-path/prose
  link checks. Complete Doxygen extraction documents every maintained code page
  and interface; two generations are byte-identical. Requirement/release acceptance
  remains the separate T117–T122 gate.
  See `docs/implementation/documentation/closure.md`.
- [x] **T117 — Close all requirement and composition gaps.** Needs: T116.
  Published the current successor to T001's frozen starting audit: all 200
  requirements and 108 canonical acceptance rows have individual reviewed
  arguments and 41 concrete source/contract/component/composition evidence groups.
  `make conformance-check` verifies exact canonical IDs/anchors, ordered complete
  criteria, maintained-input/registry/argument fingerprints and actual named C
  case signatures. Six independent C/assert cases test malformed/open claims,
  missing/changed obligations, stale provenance, wrong interfaces and real current
  repository closure; the recursive unit parent includes this leaf.
  Closed the compiler-comment gap: 2600 existing Doxygen explanations in 667
  compiler files now carry their own canonical-rule links. Exact comparison
  confirms unchanged executable bytes and physical line counts. Documentation
  tests check all 900 compiler inputs, reject fake/example links and preserve
  opaque binary fixtures without requiring UTF-8 decoding.
  Evidence: strict compiler build; complete semantic, object and async parents;
  compiled result-tree, binding, entry-preparation and execution leaves;
  documentation/conformance positive and negative cases; complete 2331-input
  zero-debt registry; local prose links; reproducible Doxygen generation with
  1746 maintained file pages and 6685 documented interfaces. T118–T120 retain
  their broader product/resource/toolchain acceptance scope.
  See `docs/implementation/conformance/README.md` and its checked `matrix.json`.
- [x] **T118 — Verify deterministic and incremental product behavior.** Needs:
  T117. Added three real CLI/verified-program integration cases over Unicode
  multiword roots and nested physical submodules. Repeated and clean output
  builds reproduce generated source, tooling and ownership bytes; transitive
  Ama/header/native/ABI and C-setting changes produce independently expected
  executable effects. Conflicting roots preserve the previous working product;
  moving a call updates its original-source runtime trace. Complete authored
  input inventories remain unchanged by every build; binaries are not used as
  source-analysis or reproducibility oracles.
  Evidence: strict new product leaf, complete rebuild parent, exact project-input
  graph and native-output ownership suites, actual CLI library builds, identity/
  native namespace/public-header collision probes, current conformance gate,
  complete zero-debt documentation and reproducible Doxygen extraction.
  Current evidence incorporates the new cases without changing canonical rules
  or production compiler behavior. See `docs/implementation/product-reproducibility.md`.
- [x] **T119 — Verify resource and cleanup boundaries.** Needs: T118.
  Verified the complete runtime, native ABI and async parents with ASan/UBSan,
  including the independent 104976-word ownership model and measured allocation
  prefixes. Sanitized real generated cleanup, library compositions and task
  boundaries cover clone/destructor failures, overflow-as-data, partial constants,
  nonresumable context failure and detached waits. The full relocation parent
  covers 1324 application allocation refusals, 1066 I/O checkpoints, 529 writer
  process interruptions and repeated recovery through rollback/commit and
  recovery-allocation/I/O failures. Sanitized CLI tests additionally cover all
  287 command-preparation allocations and output refusal after committed movement.
  Strengthened real process observers and added an independent signal-termination
  control: a crash cannot pass as an expected managed technical failure.
  Strict product cases, current conformance, complete documentation/link checks
  and reproducible Doxygen generation pass. LeakSanitizer is explicitly disabled
  in this environment; counted owners and failure-prefix checks remain active.
  No language or production compiler behavior changed.
  See `docs/implementation/resource-acceptance.md` for invariants and exact scopes.
- [x] **T120 — Run full supported-toolchain and sanitizer acceptance.** Needs:
  T119. Clean strict GCC 15.2.0 release build and all 26 integration / 194 unit
  case translation units pass, including native-entry disappearance and forbidden
  C-dispatch probes at O0/O2. Explicitly initialized the ABI permutation test's
  empty seed so all 1957 cases compile under O2/Werror without weakening flags.
  ASan/UBSan compiler-parent checks complement T119's full resource coverage.
  TSan passes actual generated parallel contexts, context admission/factories,
  native callbacks, FIFO channels and channel-wait races; independent positive
  and deliberate-race controls confirm the detector runs. The context test uses
  POSIX launch/join after an isolated C11-thread probe reproduced a crash inside
  the installed TSan; shared fixtures retain C11 declarations for their other
  consumers, all six of which pass strict rechecks. Strict/ASan/TSan context
  checks preserve the same tested identity/admission behavior.
  LeakSanitizer's separate probe fails explicitly under ptrace; it is reported
  unavailable, with counted ownership checks active. Current conformance,
  standalone/test-interface documentation checks, links and reproducible Doxygen
  generation pass. No production compiler or canonical language behavior changed.
  See `docs/implementation/toolchain-acceptance.md` for exact versions, commands,
  branch results, supported target and instrumentation limits.
- [x] **T121 — Publish reproducible examples and conformance report.** Needs:
  T120. Document supported toolchains, library/native/host APIs, complete usage
  commands and evidence-backed limitations. Rebuild example outputs and generated
  documentation from a clean checkout; request permission for any needed spec change.
  Acceptance: a fresh detached checkout at implementation commit 94b1c0c
  rebuilds under strict O2/Werror/NDEBUG and passes all seven recorded example
  commands: async emit/run, static/shared and nested C consumers, async host,
  language/object integration and parallel supervision. Clean-checkout Doxygen
  generations are byte-identical. The new release guide connects 200 requirements,
  108 acceptance rows and the current test inventory to runnable commands, public
  library/native/host APIs, supported toolchain and evidence-backed limitations.
  Local links, complete per-file documentation, current conformance and two
  identical main-tree Doxygen generations pass. No specification or implementation
  behavior changed. See docs/implementation/conformance/release.md.
- [x] **T122 — Finish the implementation release checkpoint.** Needs: T121.
  Remove obsolete mechanisms and conflicting derived descriptions, verify links
  and complete clean build/test/documentation reproduction. Reconcile all task
  statuses through this checkpoint and commit the final acceptance evidence
  without claiming unverified behavior or automatically publishing outside the
  repository. All implementation changes must be complete before T135.
  Acceptance: make clean followed by the complete make -j4 test with strict
  O2/Werror/NDEBUG exits zero in one combined run: all 194 unit and 26 integration
  test programs pass, including the real native O0/O2, generated execution, task,
  parallel, library/host and tooling compositions. Relocation repeats 1324
  allocation, 1066 I/O and 529 interruption checkpoints. Retired list-action paths
  remain absent; current typed service admission is distinguished from the removed
  selector dispatcher. Conflicting future/incomplete descriptions now point to
  actual pool, component and conformance evidence; historical audits are intact.
  The documentation generator also delivers its checked manifest with the site,
  including configuration fingerprints and exact tool versions; the affected
  documentation C/assert leaf passes after that packaging-only change. All
  154 unique task records reconcile, with only final delivery T135 remaining.
  Current conformance, zero-debt per-file coverage, local links and two identical
  complete Doxygen generations pass. No compiler/runtime or spec semantics changed.
  See docs/implementation/conformance/release.md for final acceptance evidence.
- [x] **T135 — Deliver the final technical documentation folder.** Needs: T122.
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

  Acceptance: after the final implementation checkpoint, make docs regenerates
  docs/technical solely through Doxygen. All 1749 maintained file pages and 6703
  extracted interface descriptions pass their gates; all 397 canonical/page
  destinations resolve on their originating pages. The delivered manifest matches
  the checked report and fingerprints all 1947 rendered/configuration inputs,
  including the exact final source and Doxyfile, with actual tool versions.
  Complete per-file coverage remains 2335 reviewed inputs and zero debt. Two
  final complete generations, including manifest and final task statuses, are
  byte-identical. The entry page, source browser, architecture/contracts and
  test evidence are generated/navigable. Outputs remain unversioned; generator
  inputs and final acceptance evidence are committed. All 154 roadmap tasks
  are now complete; no canonical specification change was needed.

## Development process

- [x] **T155 — Establish the project development standard.** Needs: T135.
  Consolidate the practiced development, dependency planning, correctness,
  documentation, verification and per-task commit workflow into a readable
  contributor/agent standard. Provide a reusable task/step evidence template,
  connect it to existing instructions and preserve specification authority.
  Acceptance: instructions and templates agree with existing contracts; local
  links, current conformance and complete documentation checks pass; regenerate
  Doxygen from the final inputs and check byte reproducibility. This is a
  documentation-only task; do not change compiler behavior or spec requirements.
  Step 1: reviewed AGENTS, contributor conventions, queue completion criteria,
  component contracts and evidence/documentation procedures. Their existing
  semantic authority and per-task commit obligations are preserved.
  Step 2: wrote the development standard and task/step template; linked them
  from agent, contributor, overview and queue entry points. Reused the component
  contract template instead of adding duplicate language or algorithm contracts.
  Step 3: local prose links and document consistency pass. From the repository
  root, make conformance-check exits zero (200 requirements, 108 acceptance rows,
  41 groups), and make docs-complete exits zero (2335 ready inputs, no debt).
  make docs-reproducible exits zero with two byte-identical complete generations:
  1749 maintained file pages, 6703 documented interfaces and 399 canonical/page
  destinations. Both new guides are included and all manifest input fingerprints
  match. Repeat final generation after recording these final task statuses.
  Acceptance: the standard now requires dependency-based tasks and small
  verifiable steps, explicit contracts and correctness arguments, contemporaneous
  documentation, truthful command/results and failure records, durable continuation
  checkpoints, isolated builds and one evidence-bearing commit per completed task.
  The record is proportional to the change and reuses the existing per-file and
  conformance registries. No compiler/runtime behavior or spec file changed;
  unrelated functional suites were not rerun for this documentation-only task.

## Archive maintenance

- [x] **T156 — Archive completed work without losing evidence.** Needs: T155.
  Move completed tasks to DONE.md with stable IDs, dependencies and acceptance
  records intact. Keep TODO.md as the active queue and update workflow/navigation
  for the two files. Do not change specification, compiler or historical results.
  Step 1: reviewed the queue, development instructions and documentation debt
  ownership. All 155 existing tasks are complete, there are no open documentation
  debts, and no maintained links target moved TODO section anchors.
  Step 2: created DONE.md, retained active planning/completion criteria in TODO,
  and updated agent, contributor, overview and development instructions. Compared
  every archived entry against TODO at a2c449f: all 155 original task records are
  identical and occur once; only T156 is active. IDs are allocated across both
  files and completed entries move with their full evidence in the task commit.
  All 2402 local prose links in 202 Markdown files resolve.
  Step 3: from the repository root, make conformance-check exits zero (200 rules,
  108 acceptance rows, 41 groups), and make docs-complete exits zero (2335 ready
  inputs, no debt). make docs-reproducible exits zero with two byte-identical
  complete generations including the new archive. The final generation is repeated
  after moving this completed record and leaving the active queue empty.
  Acceptance: all original task records remain identical to a2c449f and unique;
  all 156 completed task IDs are in DONE, with no active task or unresolved debt
  hidden by the move. Current instructions preserve this archive workflow and
  allocate the next ID across both files. Final documentation fingerprints cover
  the archive and queue. No compiler, spec or functional fixture changed;
  functional suites were not rerun for this documentation-only move.

## Static-template design

- [x] **T157 — Specify static module templates and their design boundaries.**
  Status: complete (specification/design documentation only). Prerequisites: T156 and the user's 2026-11-17 authorization
  to update the specification/EBNF and collect open template questions separately.
  Scope: documentation only; no compiler, runtime, native ABI or test-fixture
  implementation changes. Record agreed type-placeholder spelling and scope,
  ordinary-argument inference, nested specialization, identity and lifecycle,
  template-constructor/partial-substitution contracts, and the list implementation
  boundary. Preserve unsettled choices as explicit questions rather than inventing
  source syntax or runtime behavior.
  Steps: (1) audit related rules and record the settled/pending boundary;
  (2) write the canonical chapter, grammar and affected cross-references;
  (3) independently review semantics, grammar, navigation and examples; record
  actual checks and implementation-evidence limits, then commit this task.
  Acceptance: every normative rule has one stable indexed definition; grammar
  and examples agree with that definition; an immediately visible open-question
  index links precise issues in AMBIGUITIES.md; existing list/native/continuation
  guarantees remain intact; no documentation claims template implementation.
  Results: added canonical TPL-01 through TPL-10, type-line-only placeholder
  grammar, examples, acceptance scenarios and list implementation boundaries.
  Independent semantic/grammar reviews passed; clarified static default checking
  versus suppressed runtime default construction. AMB-003 through AMB-008 remain
  explicitly open under T158. No compiler/native/test implementation was inspected
  or changed. Current support notices preserve the old matrix as historical.
  Checks: `make docs-complete` passed (2335 inputs, zero debt); the independent
  Markdown/grammar audit passed (203 Markdown files, 2508 local links, 210 unique
  indexed rules, 96 closed nonterminals, aligned EBNF). `make conformance-check`
  exited 2 as expected: specification changed, conformance review required. Old
  fingerprints were intentionally not refreshed; no template support is claimed.
  Generated-reference check: first `make docs` exited 2 because the open-question
  index's explicit anchor duplicated the automatic heading label. Retained the
  stable explicit anchor and gave the heading a distinct spelling; no semantic
  change. Full Doxygen rerun is required before acceptance.
  Generated-reference rerun: `make docs` exited 0 after the anchor correction,
  with Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0: 1749 maintained
  file pages, 6703 documented interfaces and 411 checked destination links.
  The final archive/queue inputs require the same generation check before
  committing; rendered output remains generated and unversioned.
  Acceptance: 210 rules are uniquely indexed, all 96 EBNF nonterminals close
  and definition operators align at column 30; placeholder expansion changes
  only declaration type lines. Independent semantic, grammar and list-boundary
  reviews passed. T157 adds ten template acceptance rows and documents nested
  calls, independent/fixed field specializations, hygienic partial bindings
  and the list primitive boundary without inventing constructor syntax.
  The six unresolved decisions remain Open under T158, not archived as done.
  Existing evidence stays scoped to its 200-rule/108-row baseline; the current
  target has 210 rules and 118 acceptance rows and is not fully implemented.
  No functional suites were rerun for this documentation-only task. Canonical
  rules are in [the template chapter](spec/language/templates.md); pending
  decisions are in [the open index](AMBIGUITIES.md#open-template-design-questions).

- [x] **T159 — Specify named type constructors and partial application.**
  Status: complete (specification/design documentation only). Prerequisite:
  T157. The user's 2026-11-17 approval and instruction explicitly authorize
  specification/EBNF changes for the agreed named constructor model and a review
  of open questions. No compiler implementation or selection of pending
  additional-evidence, ABI or recursive-generation policies is included.
  Contract: preserve scoped full parameter names; distinguish Type and Template
  parameters; specify signatures, ordinary-input decomposition, static named
  application and forwarding; normalize partial bindings without merging state.
  Steps: (1) reconcile the agreed source forms with existing TPL rules and record
  exact scope/kind/inference boundaries; (2) update the canonical chapter and
  indentation-parameterized EBNF, related cross-references and acceptance cases;
  (3) resolve AMB-003, narrow related pending questions, and verify navigation,
  grammar closure/alignment, documentation generation and historical conformance
  boundaries. Acceptance requires positive/negative and nested/partial examples,
  independent semantic review, document checks and a task commit. Remaining
  undecided behavior stays explicitly gated under T158.
  Step 1 result: reviewed the clean b5fea45 baseline, development standard,
  existing canonical rules/grammar and the agreed discussion. A static binding
  selects the exact complete foreign name; canonical parameter order does not
  enable positional static binding. A Template needs a finite recursively named
  signature; fresh bare right-hand input placeholders introduce Type, not a
  category guessed from the first actual. Root input types explicitly cover all
  residuals, including concrete template-path heads, preventing hidden owner
  parameters. Body/result fields retain per-declaration inference variables.
  Step 2 result: expanded [TPL-01 through TPL-10](spec/language/templates.md)
  without adding duplicate requirements. Added explicit static Template formals,
  recursive signatures, declaration type expressions and nested named binding
  blocks to [the EBNF](spec/grammar.md). Static maps precede ordinary initializers;
  static formals consume no runtime input slots. Definition/application examples
  show a dictionary family, direct/static application, three-level composition,
  forwarding with a fixed key, independent field inference, staged-equivalent
  identities and invalid applications. Exact nominal recovery retains every
  unselected binding and existing residual, with no constructor search. Updated
  declaration/call/name/module/list/compiler explanations and acceptance rows.
  Step 3 result: [AMB-003](AMBIGUITIES.md#amb-003) is resolved with the user's
  decision/source/authorization. AMB-004 through AMB-008 remain Open under T158;
  AMB-006 now concerns the concrete list interface and primitives, since general
  named static application is settled. Three independent read-only reviews
  found and resolved wording gaps for bare known-Template inputs, concrete-path
  root coverage, infinite inferred signatures and argument reordering versus
  changes to a definition's own introduction order. No blocking inconsistency
  remains in the settled scope. No compiler/native/test implementation was
  inspected or changed, and grammar examples are design evidence, not execution.
  Verification from the repository root: `python3 /tmp/ama-t159-doc-check.py`
  passes against the current maintained Markdown set (203 files, 210 uniquely
  indexed requirements, 104 closed EBNF nonterminals, definition operators at
  column 30 and template-example spacing). The checker is a temporary independent
  audit, not a shipped compiler component. A separate graph check confirms all
  104 unique productions are reachable from `module-file` or `abi-file`.
  `git diff --check` passes. `make docs-complete` exits 0 (2335 ready inputs,
  zero explicit debts). `make docs` exits 0 with Doxygen 1.11.0, Python 3.14.4
  and markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented interfaces,
  411 checked canonical/referenced destinations and 5524 generated pages.
  `make conformance-check` exits 2 as expected: the specification changed and
  implementation conformance needs review. No fingerprints were refreshed and
  no template implementation support is claimed. Current targets remain 210
  requirements and 118 acceptance rows; old 200/108 evidence stays historical.
  An initial temporary inventory assertion mixed cell rows with table-separator
  lines (128 versus 138); the corrected Markdown table audit passes with 118
  acceptance rows and unchanged inventory. Archive checks confirm unique task
  IDs T001 through T159, only T158 unfinished, and T001–T157 records unchanged.
  Final archive/queue checks and Doxygen generation are repeated on these final
  inputs before the task commit. Functional suites are not rerun for this
  documentation-only task. Acceptance: named constructors and partial application
  are specified with scoped, deterministic constraints and examples; the five
  remaining design questions stay visible and unfinished in TODO.

- [x] **T160 — Specify interface-wide type inference and result Type parameters.**
  Status: complete (specification/design documentation only). Prerequisites:
  T157/T159 and the user's approved additional inference evidence plus explicit
  `@>` scope decision in [AMB-004](AMBIGUITIES.md#amb-004). Scope: update
  TPL-02/03/06/10, TYPE-06 and linked grammar/examples so all agreed typed
  connections constrain one fixed specialization and results may introduce Type
  parameters. Preserve lexical receiver classification, storage suffixes,
  runtime initialization order and body-introduction prohibition.
  Steps: (1) reconcile scoped introductions and static constraints with the
  receiver algorithm; (2) add canonical positive/negative/later-evidence examples
  and acceptance cases; (3) review links, EBNF and Doxygen, record evidence and
  commit. Parameter-dependent constants/access remain Open; no proposed constant
  notation or compiler implementation is included.
  Step 1 result: Type parameters may be introduced in `@<` and `@>`; Template
  introductions/signatures remain only in `@<`. Collect interface scope before
  checking uses; preserve input-then-result introduction order for identity.
  New body parameters remain forbidden. Input/output transfers, typed defaults,
  declaration initializers, assignments and reference/list/alias connections
  contribute exact scoped type equations. Existing typed constants remain valid
  evidence without authorizing dependent constants or a new namespace.
  Step 2 result: [TPL-03](spec/language/templates.md#tpl-03) describes global
  equality propagation, including later typed evidence and result-only owner
  parameters. New inferred receivers supply equations, never a type by themselves;
  literal bytes and capabilities still do not choose types. TYPE-06/FLOW-05 bind
  lexical names/storage before solving their symbolic terms; a later anchor cannot
  repair an earlier unresolved name or change reuse/local classification. The
  proof separates decreasing name dependencies from bidirectional type equations.
  Constructor recovery accepts the same typed connections, using a declared
  signature and deterministic source/generated bindings without module search.
  Examples cover two independently specialized producers, typed output/literal
  inference, later assignment anchors, generic box results, per-field omissions,
  defaults and rejection boundaries. Updated canonical cross-references and
  existing acceptance cases; no parser production change is necessary.
  Step 3 result: three independent semantic/grammar/consumer reviews pass after
  correcting stale statements requiring an immediately complete receiver type
  and forbidding all callee-body inspection. Ordinary typed Ama body assignments
  may refine declared interface terms; executing bodies for inference, searching
  for undeclared members or guessing from native/runtime contents remains
  forbidden. Typed defaults constrain types even when runtime construction is
  suppressed; persistent fields still construct before body execution. The
  remaining part of AMB-004 concerns constant declarations/access only.
  Verification from repository root: `python3 /tmp/ama-t160-doc-check.py` passes
  for 203 maintained Markdown files, 2531 local links before archive navigation,
  210 unique indexed rules, 104 closed EBNF nonterminals at column 30 and example
  spacing. This is a temporary independent documentation audit. A separate
  Markdown comparison confirms every EBNF production block is byte-identical to
  39b0aaf and all 118 acceptance rows remain, with expanded scope/inference cases.
  `git diff --check` passes. `make docs-complete` exits 0 (2335 ready inputs,
  zero debt). `make docs` exits 0 with Doxygen 1.11.0, Python 3.14.4 and
  markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented interfaces,
  411 checked canonical/referenced destinations and 5524 generated pages.
  `make conformance-check` exits 2 as expected because reviewed implementation
  evidence predates the specification; no fingerprints or support claims change.
  Final archive/queue checks and documentation generation repeat on final inputs
  before commit. No compiler/native/test code was inspected or changed and no
  functional suites were rerun for this documentation-only task. Acceptance:
  the approved inference/scope change is explicit and consistent across chapters;
  constant design and the other T158 questions remain visible and unfinished.

- [x] **T161 — Specify dependent constants and explicit type namespaces.**
  Status: complete; specification/documentation only. Prerequisites: T157, T159,
  T160 and the user's explicit selection of constant design option 2 on
  2026-11-17. Authorized result: constants may reuse interface type parameters,
  and programs may explicitly select the constants of a substituted concrete
  type or specialization, without importing/inheriting names. Preserve the
  motivation, examples, initialization and ownership limits in the project.
  Boundary: canonical declarations/templates/grammar, affected indexes and
  acceptance scenarios, AMB-004 and this task's evidence; no compiler changes
  and no adoption of value-indexed/dependent types.
  Steps: (1) record the decision and exact scope; (2) define shared constants,
  namespace resolution and syntax, with rationale and positive/negative examples;
  (3) reconcile cross-references and pending gates; (4) check links, requirements,
  grammar, documentation and the final diff, then archive and commit this task.
  Acceptance: one canonical rule per behavior, no implicit name lookup or runtime
  input dependency, explicit specialization identities and lifecycle preservation,
  named acceptance cases, and an accurate distinction between specified design
  and implementation support. The documentation checks below supply acceptance evidence.
  Steps 1–3 result: recorded the approval/source before canonical edits; added
  CONST-06 and explicit static-expression namespaces in CONST-03/EBNF. Wrote the
  rationale and examples for empty lists, neutral values and bounds, without
  imports/inheritance or runtime-dependent values. Constant initialization now
  distinguishes full specialization identities. Reconciled scope/inference,
  indexes, roadmap and acceptance cases; AMB-004 is resolved, with four other
  gates still open. No compiler/native/test implementation was inspected or
  changed. Functional suites are outside this design-only scope.
  Step 4 result: two independent semantic reviews found no blocking ambiguity.
  Corrected the distinction between ordinary literal rejection at startup and
  permitted equivalent earlier validation, and explicitly included shared
  declarations in TPL-08's per-declaration inference without adding owner
  parameters. Namespace selection creates no such inference variables.
  The informative rationale in [CONST-06](spec/language/declarations.md#const-06)
  explains own dependent constants, explicit access to another type's values,
  and why automatic imports/inheritance would introduce conflicts and hiding.
  Type/Template composition remains distinct from numeric value parameters:
  ordinary integer contents and constant values do not enter specialization keys.
  Verification from the repository root: `python3 /tmp/ama-t161-doc-check.py`
  passes for 203 maintained Markdown files, local links, 211 uniquely indexed
  requirements, 104 closed EBNF nonterminals at column 30 and example spacing.
  This is a temporary independent audit, not a shipped implementation component.
  A separate grammar/matrix comparison passes: only the constant-read production
  changes, all 104 productions remain reachable, all 118 prior acceptance areas
  remain, and three specialized-constant areas bring the target to 121 rows.
  Its initial auxiliary run searched for an obsolete matrix heading and failed;
  using the existing `Acceptance matrix` heading makes the full rerun pass.
  `git diff --check` passes. `make docs-complete` exits 0 (2335 ready inputs,
  zero explicit debt). `make docs` exits 0 with Doxygen 1.11.0, Python 3.14.4
  and markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented interfaces,
  412 checked canonical/referenced destinations and 5524 generated pages.
  `make conformance-check` exits 2 as expected: reviewed implementation evidence
  predates these specification changes. No fingerprints are refreshed and no
  template/constant implementation support is claimed. Final archive/queue
  navigation and Doxygen generation repeat on these final inputs before commit;
  generated pages remain unversioned.
  Acceptance: the user's selected option 2 is specified with motivation, concrete
  namespace ownership, pure per-context initialization and positive/negative
  acceptance cases. AMB-004 is resolved; AMB-005 through AMB-008 remain Open
  under T158. No dependent-type/vector extension or compiler code is included.

- [x] **T163 — Specify finite composition and nonrecursive specialization creation.**
  Status: complete; specification/documentation only. Prerequisites: T157, T159,
  T160, T161 and the accepted [AMB-005](AMBIGUITIES.md#amb-005) decision.
  Explicit authorization on 2026-11-17: "Вноси правила рекурсии в спецификацию
  и делай коммит." This extracts the settled recursion boundary from T158;
  positional forms and the other template gates remain unfinished work.
  Boundary: canonical templates, grammar guidance, requirements/navigation,
  compiler/roadmap guidance, acceptance scenarios and decision/task records.
  Steps: (1) define creation edges, finite roots and source-origin path checks;
  (2) reconcile higher-order/partial/inferred applications, cache behavior and
  separate type/ownership checks; (3) update related guidance and acceptance;
  (4) review semantics, validate documentation, archive and commit this scope.
  Acceptance: finite repeated nesting is permitted; recursive creation is
  rejected under an exact terminating rule; original identities, independent
  branches/specializations and cache-order independence are explicit. No
  compiler implementation or functional template support is claimed.
  Steps 1–3 result: [TPL-11](spec/language/templates.md#tpl-11) defines the graph
  on concrete types and applications originating in their instantiated owners.
  Nested argument applications retain their lexical owner; reusing a supplied
  Type creates no edge, while every supplied Type is still checked. Actual
  Template origins, partial captures and inferred applications cannot hide
  repetition. Cached dependency paths remain subject to the new ancestry;
  unrelated concrete bindings cannot be merged into fictitious source cycles.
  Added complete Ama box/nesting examples and an invalid growing-reference
  example, private dictionary/set composition, partial-origin and independent
  binding cases, motivation and the finite-branching/creation-path termination
  argument. Recorded the user's algorithm-development purpose without assuming
  a polygon data architecture. AMB-005 is resolved; T158's other gates remain.
  Step 4 result: independent semantic review found no blocking ambiguity and
  confirmed scope/grammar, provenance, finite captures, ordinary roots and cache
  behavior. Refined the order-invariance wording to cover equivalent normalized
  bindings and reordered independent arguments. Verification from repository
  root: `python3 /tmp/ama-t163-doc-check.py` passes for 203 maintained Markdown
  files and their local links, 212 uniquely indexed canonical requirements,
  104 closed EBNF nonterminals aligned at column 30 and example spacing.
  A separate comparison verifies all 10 EBNF production blocks are byte-identical
  to e4e2d6a: this change is semantic, not new syntax. All 121 previous acceptance
  areas remain, with five creation-path partitions bringing the matrix to 126.
  `git diff --check` and `make docs-complete` pass (2335 ready inputs, zero debt).
  `make docs` passes with Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0:
  1749 maintained file pages, 6703 documented interfaces, 413 checked canonical
  and referenced destinations, and 5524 generated pages. Archive/queue inputs
  are included in a final documentation audit and Doxygen generation before
  commit; generated outputs remain unversioned. `make conformance-check` exits
  2 as expected with "specification changed; conformance review is required";
  no implementation evidence or fingerprints are refreshed. No compiler/native/
  test code was inspected or changed, and functional suites were not rerun for
  this specification-only task. Pre-existing unfinished positional-proposal
  work-log changes are preserved outside this task's commit.

- [x] **T162 — Design positional static arguments and returned type bindings.**
  Status: complete; specification/documentation only. Prerequisites: T157,
  T159, T160 and T161; T163 supplies the finite-creation baseline. Explicit
  authorization on 2026-11-17 covers static Type/Template results and positional
  input/output bindings with skips. The user selected textual static-output
  order in `@>`, immutable receiver aliases visible from their receiver onward
  through the module, and complete producers before static result publication.
  Decisions and their conversation sources are retained in
  [AMB-009](AMBIGUITIES.md#amb-009)/[AMB-010](AMBIGUITIES.md#amb-010).
  Boundary: canonical static inputs/results, structural inference, scope,
  identity, grammar, related chapter/implementation guidance, acceptance and
  task/decision records. No compiler, native, fixture, numeric-static-parameter
  or dependent-type implementation is included.
  Plan: (1) record the actual decisions; (2) reconcile named/positional views,
  skips, categories, output definitions and alias scope; (3) update readable
  canonical rules, rationale and examples; (4) independently review, verify
  documentation, archive and commit. Acceptance requires deterministic original
  interface identities, exact category/equality checks, no heuristic type or
  origin search, independent runtime/static positions and complete producer
  closure while preserving returned constructor residuals.
  Step 1 result: AMB-009's output sequence and receiver scope are resolved.
  Composition review exposed a separate omitted-producer-input boundary rather
  than guessing it. Work paused for AMB-010; the user explicitly selected full
  producer determination, allowing exact result constraints to infer omitted
  inputs and rejecting any that remain unknown, including unused inputs.
  Step 2 result: [TPL-12](spec/language/static-results.md#tpl-12) owns the new
  static-result chapter. Two-line results define Type/Template values; input
  formal signatures use indentation without a hanging `<`. Positional inputs
  normalize through the actual origin's original residual order, positional
  outputs use explicit static `@>` declaration order, and named/positional
  modes, independent counters, skips and arity are exact. Output phases create
  deterministic per-expression producer inference terms; no-output contexts
  retain their prior partial/field/namespace rules. Declared output binders and
  source-ordered receiver aliases remain distinct from free owner parameters.
  Explicit interface binders are collected first; lexical alias classification
  prevents later interface uses from creating hidden owner parameters, while
  genuine owner formals retain whole-interface visibility.
  Step 3 result: TPL-02/05/07/08/10/11 and the EBNF are reconciled. Examples
  show concrete dictionary Types, returned key-bound families, equivalent named
  and positional applications, skips, subsequent composition and output-driven
  inference. Rationale explains static library interfaces and information lost
  by noninjective outputs. Exact result identity, captures, complete checking,
  unselected outputs and source/projection provenance remain explicit. Active
  definition and known-origin creation paths are guarded before expansion,
  including during inference and cache reuse, rather than after generating an
  unbounded chain. The target acceptance matrix preserves its previous 126
  areas and adds 14 static-result partitions: 140 total, 32 about templates.
  Step 4 result: independent semantic and grammar reviews found no remaining
  blocker. They caught and corrected pre-expansion guard ordering, closed-Type
  producer provenance, interface receiver classification and a hypothetical
  grammar example's conflicting producer name. Verification from repository
  root: `python3 /tmp/ama-t162-doc-check.py` passes for 204 maintained Markdown
  files and their local destinations, 213 uniquely indexed canonical rules,
  108 closed EBNF nonterminals at column 30, example spacing and 32 template
  acceptance areas. A separate exact graph/matrix audit confirms all 108
  productions are reachable and all 140 acceptance-area names are unique.
  These are temporary independent document audits, not shipped compiler tests.
  `git diff --check` and `make docs-complete` pass (2335 ready maintained inputs,
  zero explicit debt). Initial `make docs` exits 0 with Doxygen 1.11.0,
  Python 3.14.4 and markdown-it-py 3.0.0: 1749 maintained file pages, 6703
  documented interfaces, 415 checked canonical/referenced destinations and
  5526 generated pages. The archive/queue and final canonical refinements are
  included in a final successful audit and Doxygen regeneration before commit;
  generated output remains unversioned. `make conformance-check` exits 2 with
  "specification changed; conformance review is required", as expected for
  evidence predating these language changes. No implementation fingerprints are
  refreshed and no template support is claimed. Compiler/native/test code was
  neither analyzed nor changed; functional suites were not rerun for this
  specification-only task. T158 retains AMB-006 through AMB-008; their list,
  native/export and naming decisions remain outside this completed task.

- [x] **T164 — Remove list-foundation reimplementation from the template sprint.**
  Completed 2026-11-17; documentation and planning only. Prerequisites: T157's
  [TPL-09](spec/language/templates.md#tpl-09) boundary and the user's explicit
  instruction to exclude the base-list rewrite from this sprint and commit the
  decision. Scope: AMBIGUITIES.md, TODO.md and this completed record; no compiler,
  native, test, build, grammar or canonical specification changes.
  Plan and acceptance: record the actual decision and rationale, update the open
  index and T158, verify local destinations/task IDs/scope, then archive and commit
  before discussing the next open question. AMB-006 must no longer block the
  sprint; AMB-007/AMB-008 must remain open and T158 unfinished.
  Step 1 result: [AMB-006](AMBIGUITIES.md#amb-006) is deferred outside this sprint,
  with the quoted user instruction and its source. This preserves the existing
  intrinsic list foundation and ordinary template algorithms over lists. No
  provider, raw storage interface or protected mutation mechanism is selected;
  future base-list work needs its own explicit decision. The rationale records
  the publication/lifecycle costs and why whole-order copying on every append
  would violate the required amortized O(1) bound. This is not a permanent ban.
  Step 2 result: the open index and T158 now retain AMB-007/AMB-008 as the remaining
  sprint gates, with AMB-007 next. T158 remains unchecked; no unrelated design or
  implementation task is claimed complete. Independent content review confirms
  the scope decision and preserved semantic boundaries.
  Verification from repository root: `python3 /tmp/ama-t164-doc-check.py` passes
  initially for three maintained files, 253 local links and 164 unique task IDs;
  it checks deferred/open statuses, T158's unfinished state and unchanged `spec/`.
  This is a temporary document audit, not a shipped compiler test. `git diff
  --check` and `make docs-complete` pass; 2335 maintained inputs are ready with
  zero explicit debts. Initial `make docs` exits 0 with Doxygen 1.11.0, Python
  3.14.4 and markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented
  interfaces, 415 checked canonical/referenced destinations and 5526 generated
  pages. The final scope/link audit and Doxygen regeneration also pass after
  archiving this record; generated outputs remain unversioned. Compiler/native/
  test code was neither analyzed nor changed; functional suites were not rerun
  for this planning-only task. Current conformance evidence is not refreshed.

- [x] **T165 — Specify concrete native leaves and source-distributed templates.**
  Completed 2026-11-17; specification only. Prerequisites: T162's static
  interfaces, T163's specialization identity rules and T164's sprint scope.
  Authorization: after stating that `@C` modules should remain leaves without
  templates, the user accepted the complete concrete-native/facade/source-delivery
  proposal and instructed, "Принимаем. Внеси в спецификацию." The answer and source
  are recorded in [AMB-007](AMBIGUITIES.md#amb-007).
  Scope: canonical template/native/export rules, EBNF guidance, explanation and
  examples, cross-references, roadmap, acceptance matrix and task/decision records.
  No compiler, native implementation, fixture, build or ABI-generation code is
  included. Preserve concrete native composition, existing lifecycle/transport
  rules, deferred AMB-006 and open AMB-008.
  Plan and acceptance: (1) record the exact boundary; (2) reconcile canonical
  chapters and grammar; (3) independently review scopes, examples and navigation;
  (4) run documentation gates, archive and commit. AMB-007 must be settled without
  generic C companions, runtime open types or guessed export/source formats.
  Step 1 result: [TPL-13](spec/language/templates.md#tpl-13) rejects an `@C`
  definition with any own Type/Template parameter after interface collection,
  including explicit, implicit and unused formals, even when every use binds
  them. Concrete C types remain arguments to Ama templates. Closed dependency
  types, declared Ama fields/references, bound static aliases and returned
  constructors' own residuals are distinguished from free native-owner formals.
  The rationale explains why a native implementation leaf is not an ownership
  leaf and why a generic C header/lifecycle system is unnecessary.
  Step 2 result: CABI-01 retains its concrete checked native interface and excludes
  static results from runtime C arguments/storage. EMBED-01 exports ordinary
  concrete module paths, including already concrete native roots and Ama facades
  with internal closed specializations. Direct specialization selectors remain
  outside the sprint. Complete `.forward`/`.integer facade` examples keep normal
  state ownership and supported explicit transport; no generic C ABI is invented.
  Open templates are delivered as Ama source with normally resolved dependencies;
  full logical identities, root kinds, static scopes and checked closure remain
  authoritative, with no new binary open-template or runtime generation format.
  Step 3 result: grammar productions remain unchanged; semantic guidance links
  TPL-13 rather than banning every `...` spelling on native interfaces. The
  registry contains 214 unique definitions/index entries. Seven new acceptance
  areas retain every previous area: 147 total, including 37 template areas.
  Independent semantic, grammar and cross-reference reviews pass; they caught
  and corrected differing formal names in repeated `.forward` examples and
  wording that grouped deferred AMB-006 with the open naming gate. T158 remains
  unfinished for AMB-008; no implementation support is claimed.
  Verification from repository root: `python3 /tmp/ama-t165-doc-check.py` passes
  initially for 204 Markdown files, 2705 local links, 214 canonical/indexed rules,
  108 unique aligned EBNF rules in 10 blocks byte-identical to the previous
  grammar, 147 uniquely named acceptance areas and 165 unique task IDs. The
  temporary checker's first run omitted parameterized EBNF names; its extraction
  was corrected without a grammar change and the complete audit passed. This
  is a document audit, not shipped compiler tests. `git diff --check` and
  `make docs-complete` pass (2335 ready maintained inputs, zero explicit debts).
  Initial `make docs` exits 0 with Doxygen 1.11.0, Python 3.14.4 and
  markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented interfaces,
  416 checked canonical/referenced destinations and 5526 generated pages.
  The final archived-input audit and Doxygen regeneration also pass before
  commit; generated outputs remain unversioned. `make conformance-check` exits 2
  with "specification changed; conformance review is required", as expected for
  evidence predating these rules. No implementation fingerprints are refreshed;
  compiler/native/test code was neither analyzed nor changed, and functional
  suites were not rerun for this specification-only task.

- [x] **T166 — Specify single internal ASCII hyphens in complete names.**
  Completed 2026-11-17; specification only. Prerequisites: SRC-03/TPL-01's shared
  name profile and the user's explicit AMB-008 answer, followed by "Вноси это в
  спецификацию" in this conversation. The answer/source and distinct canonical
  authorization are recorded in [AMB-008](AMBIGUITIES.md#amb-008).
  Scope: canonical name spelling, EBNF, examples/rationale, related navigation,
  roadmap, acceptance and task/decision records; no compiler/native/test code.
  Plan and acceptance: update SRC-03 and name-word together; explain exact word,
  path and static-name boundaries; add named acceptance partitions; review and
  verify documents; archive and commit. Preserve original start/continuation
  character sets, exact identity, separate native ABI and payload contexts.
  Step 1 result: [SRC-03](spec/language/source-and-names.md#src-03) permits
  isolated internal U+002D hyphens in every name word, including several separated
  hyphens, without leading/trailing/consecutive or space-adjacent forms. `item-2`
  is valid; `item 2` is not. Underscores and permitted combining continuations
  retain their original position classes. The EBNF consumes `-` with an ordinary
  following continuation and leaves the continuation class itself unchanged;
  the explanatory grouping argument establishes the exact same word language.
  Step 2 result: examples distinguish `a-b`, `a_b` and `a b`, preserve exact
  physical path spelling and static whole-name identity, and reject confusable
  punctuation without normalization. No subtraction operator is introduced.
  Native `.abi` destruction, authored C identifiers and literal/comment payloads
  retain their own rules. Placeholder examples and the roadmap share SRC-03;
  three new acceptance areas preserve all 147 previous areas: 150 total, with
  all 37 template areas retained. AMB-008 is resolved; AMB-006 remains deferred.
  Independent semantic/grammar reviews found no contradiction; wording explicitly
  distinguishes isolated hyphens from a one-hyphen-per-word limit.
  Verification from repository root: `python3 /tmp/ama-t166-doc-check.py` passes
  initially for 204 Markdown files, 2718 local links, 214 indexed canonical rules,
  108 unique aligned EBNF productions and 166 task IDs. Only the accepted
  name-word production changes; all other grammar content in the 10 EBNF blocks
  is preserved. The temporary document-model audit independently compares the
  grammar-shaped expression with SRC-03's position predicates over all 97656
  strings of lengths 0..7 using five character-role representatives; all agree.
  This is a bounded document check, not compiler or Unicode-table conformance.
  `git diff --check` and `make docs-complete` pass (2335 ready maintained inputs,
  zero explicit debts). Initial `make docs` exits 0 with Doxygen 1.11.0, Python
  3.14.4 and markdown-it-py 3.0.0: 1749 maintained file pages, 6703 documented
  interfaces, 416 checked destinations and 5526 generated pages. Final archived
  inputs also pass the audit and documentation regeneration before commit;
  generated outputs remain unversioned. `make conformance-check` exits 2 with
  "specification changed; conformance review is required"; no implementation
  fingerprints are refreshed. Source/native/test code was neither analyzed nor
  changed and functional suites were not rerun. T158 remains for separate final
  design acceptance; this task claims no template implementation support.

- [x] **T158 — Resolve the remaining template design gates.**
  Completed 2026-11-17; design acceptance only. No compiler implementation was
  authorized or performed by this task. Prerequisites: T157's documented baseline, T159's named
  constructor/signature forms, T160's interface-wide inference rules and T161's
  dependent constants/explicit namespaces. Questions and alternatives
  are indexed in [AMBIGUITIES.md](AMBIGUITIES.md#open-template-design-questions):
  AMB-003 is resolved by T159; T160/T161 settle AMB-004.
  T163 resolves AMB-005's specialization-creation boundary. T162 resolves
  AMB-009/AMB-010 through static results, positional bindings, immutable aliases
  and complete producer inference under [TPL-12](spec/language/static-results.md#tpl-12).
  T164 records the user's exclusion of the base-list rewrite from this sprint:
  AMB-006 is deferred and no longer blocks this sprint. Preserve the existing
  intrinsic list foundation; ordinary template algorithms over lists stay in
  scope. No implementation provider, new storage primitive or protected mutation
  API is selected. T165 settles AMB-007 under
  [TPL-13](spec/language/templates.md#tpl-13): native modules have no own static
  parameters, templates are distributed as Ama source, and C exports select
  concrete ordinary modules/facades rather than template specialization maps.
  T166 settles AMB-008's isolated internal ASCII-hyphen profile under
  [SRC-03](spec/language/source-and-names.md#src-03). No current sprint design
  decision remains open. Final design acceptance checks the recorded decisions,
  canonical rules, the prerequisite-ordered implementation roadmap and named
  acceptance cases; this record establishes no implementation support.
  Established design checkpoint: AMB-004's inference, result-Type scope and constant design
  are approved and specified. Explicit namespaces avoid name inheritance/imports;
  static parameters remain Type/Template, without numeric value parameters.
  The answers/source are recorded in AMBIGUITIES. Any newly discovered gate
  needs its own explicit decision before affected implementation. T163 specifies the user's
  accepted finite-composition rule in [TPL-11](spec/language/templates.md#tpl-11),
  with the creation-path check, termination argument and acceptance partitions.
  AMB-005 is resolved; it no longer blocks implementation of that boundary.
  The user also clarified the polygon's purpose: automatic development and
  improvement of Ama algorithms from textual descriptions and examples in other
  languages; the base data model remains undecided. Other canonical changes
  still need their own explicit authorization. T162's archived acceptance,
  T164's scope decision and T165's specification record are in [DONE.md](DONE.md);
  T166's accepted naming decision preserves settled static result and
  native/source-delivery rules. Final step after T166's commit: the decision-to-rule/roadmap/acceptance links
  were reviewed before archiving this parent in its own commit. No implementation work is authorized by this task.
  Acceptance: record user decisions and authorized canonical/EBNF corrections;
  derive the implementation roadmap and named positive/negative acceptance cases
  from those settled contracts. Preserve the previous conformance matrix as
  historical evidence until new implementation evidence is actually available.
  Final evidence map: AMB-003/T159 defines signatures and partial composition in
  [TPL-07/TPL-08](spec/language/templates.md#tpl-07); AMB-004/T160/T161 defines
  interface inference/scope under TPL-02/TPL-03 and dependent values/namespaces
  under [CONST-03/CONST-06](spec/language/declarations.md#const-06).
  AMB-005/T163 defines TPL-11's finite-composition/creation boundary;
  AMB-006/T164 explicitly excludes the list-foundation rewrite from this sprint.
  AMB-007/T165 defines concrete native leaves, source delivery and ordinary
  export facades under TPL-13/EMBED-01. AMB-008/T166 defines SRC-03's uniform
  isolated internal hyphen profile. AMB-009/AMB-010/T162 defines static results,
  positions, aliases and complete producer inference under TPL-12.
  The [Stage 11 roadmap](spec/implementation/plan.md#stage-11-static-templates-with-explicit-design-boundaries)
  orders recognition, scoped interfaces, exact inference/composition, output
  closure/cycles, creation validation/caching, lifecycle/constants, emission,
  diagnostics and native/source/export checks. The
  [acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix)
  names positive/negative partitions across 150 areas, including 37 template
  areas and the hyphen profile. Independent read-only review confirms that these
  satisfy this task's design deliverable; old conformance remains historical.
  Verification from repository root: `python3 /tmp/ama-t158-doc-check.py` passes
  on the final archive/queue for document links, 214 uniquely indexed rules,
  108 unchanged aligned EBNF productions, preserved 150 acceptance-area names,
  unique task IDs and settled/deferred statuses. It also confirms `spec/` is
  byte-unchanged from T166 commit c9e56b9 and only the three root planning records
  change. `git diff --check` passes. T166's successful `make docs-complete` evidence
  (2335 ready inputs, zero debt) remains applicable because no code inputs or
  coverage registry change; final `make docs` reruns coverage and successfully
  regenerates the archive/queue with the supported Doxygen 1.11.0/Python 3.14.4/
  markdown-it-py 3.0.0 toolchain before commit. No functional suites, source audit
  or implementation evidence refresh is claimed. T167 is a planned next-stage
  task breakdown, not started compiler work or a new specification change.


<a id="t167"></a>

- [x] **T167 — Decompose the approved template roadmap into implementation tasks.**
  Completed 2026-11-17; planning only, authorized by the user's sprint-planning
  request. Reviewed input revision: a1022bd; initially clean working tree. Prerequisites: T158's completed design review and
  T166's naming profile. No compiler implementation or specification edit is
  included. Use [Stage 11](spec/implementation/plan.md#stage-11-static-templates-with-explicit-design-boundaries)
  and the [acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix)
  to derive small, stable, prerequisite-ordered implementation tasks.
  At that stage review the maintained implementation boundary and record existing
  support/gaps, without analyzing excluded generated/binary artifacts. Assign
  each task canonical rule links, an exact contract/invariants, its real component
  and consumer boundaries, positive/negative and failure cases, documentation/
  per-file coverage requirements, and independently verifiable acceptance.
  Cover recognition/shared names, kinds/signatures/results, static/runtime
  positions/aliases, exact constraints/patterns/partials, output closure/cycles,
  specialization creation/cache, type/lifecycle/constants, emission/diagnostics,
  concrete native composition, source delivery and facade exports. Keep the
  base-list rewrite outside the sprint. Deliverable: an executable TODO breakdown
  with no new language rules, guessed behavior or unsupported implementation claim.

  Result: [T168–T198](TODO.md#sprint-static-module-templates), 31 independently
  accepted implementation tasks, in prerequisite order. Each task identifies
  canonical rules, component/consumer boundary, concrete steps and positive,
  negative and failure acceptance. The queue defines the per-step record,
  continuation checkpoint, documentation/test evidence and per-task commit rules.
  All implementation tasks remain planned and unchecked; T168 starts the later
  implementation phase. No new language behavior or spec edit was selected.

  | Step | Purpose and actual result | Verification and decision |
  |---|---|---|
  | 1 | Read the development standard/template, canonical Stage 11 and current contract/evidence inventories; inspect only maintained source/test directory topology | Existing conformance remains pre-template evidence; old component prose includes historical subsets. Exact source/function and functional gap audit belongs to T168; no unsupported support claim |
  | 2 | Allocate T168–T198 from foundations to product consumers and release; assign one primary owner to each of the 37 template acceptance areas | Three independent read-only reviews confirm scope and sequencing; base-list rewrite stays deferred; native/source/export and editor/relocation consumers are included |
  | 3 | Check canonical links, rule/grammar preservation, task IDs, dependency graph and acceptance routing | 31 planned tasks, 105 prerequisite edges, no cycle or forward prerequisite; 198 unique task IDs; 37 canonical area names routed exactly once; hyphen and facade obligations routed separately |
  | 4 | Run documentation gates, fix the observed formatting error, archive this planning task and regenerate final inputs | make docs-complete and corrected make docs pass; final archive/queue links and Doxygen generation are rechecked before commit |

  Review decisions: lexical alias classification precedes final free-formal
  collection; native admission uses that complete inventory. Nonexpanding term,
  equality and application components precede guarded expansion. Active symbolic
  ancestry checks precede producer expansion and remain effective on cache hits.
  T175 owns normalized-record representation; T177 owns source-application
  normalization. T183 interns closed candidates; T185 supplies ordinary checks
  before executable admission. Unresolved projected kind/signature obligations
  retain fixed lexical identities. T198 rechecks final conformance freshness
  after release/documentation/archive changes instead of trusting T196 hashes.

  Verification from repository root: python3 /tmp/ama-t167-plan-check.py checks
  all tracked Markdown destinations, canonical rule/index bijection, unchanged
  EBNF blocks, unique IDs, ordered acyclic prerequisites, complete task record
  fields and exact acceptance-area ownership. The final audit covers 204 Markdown
  documents, 214 indexed rules, 108 unchanged aligned grammar productions and
  150 acceptance areas, including 37 template areas. Its algorithm and results
  are recorded here; 2837 local links pass after archival. The temporary audit
  script is not a new project tool or the
  sole record of decisions. git diff --check passes. Only planning/archive and
  conformance-navigation prose changes; spec, compiler/native/test code, coverage
  registry and reviewed conformance matrix remain unchanged.

  Actual documentation results: make docs-complete reports 2335 ready inputs
  and zero explicit debts. The first make docs returned exit 2 because a plain
  native-selector marker in TODO was interpreted as an unknown Doxygen command.
  Inline-code formatting fixed it; the exact make docs checkpoint rerun passed, checking
  1749 maintained file pages, 6703 documented interfaces, 416 rule/page
  destinations and 5526 generated pages. Supported tools: Doxygen 1.11.0,
  Python 3.14.4 and markdown-it-py 3.0.0. The final make docs generation follows
  this archival edit, rechecks coverage and describes the final maintained inputs.
  No functional tests, sanitizer runs, compiler source-body audit or executable
  template support is claimed. Current implementation readiness: T168 is planned;
  no implementation or test jobs are active. This task's commit closes planning;
  later tasks retain their own implementation authorization and acceptance gates.


<a id="t168"></a>

- [x] **T168 — Audit the template delta and assign component ownership.**
  Completed 2026-11-18; audit and migration ownership only. Input 88f5867.
  Evidence: [source/API audit](docs/implementation/audit/template-sprint.md).
  Prerequisites: T167, T158, T166.
  Rules: [ARCH-01](spec/implementation/compiler.md#arch-01),
  [ARCH-06](spec/implementation/compiler.md#arch-06),
  [TEST-06](spec/implementation/tools-and-conformance.md#test-06) and Stage 11.
  Boundary: maintained source, native declarations/companions, tests, build/tool
  consumers and [current contracts](docs/implementation/contracts/README.md).
  Steps: (1) trace actual stage/API handoffs and compare current rules with old
  evidence; (2) map retained/adapted/obsolete responsibilities, exact test homes
  and each sprint acceptance area to its owning task; (3) characterize affected
  existing behavior through focused tests/probes and reconcile current prose.
  Accept: every gap has a task owner, narrow interface and real consumer; unchanged
  support is distinguished from observed failures and unverified behavior.
  Preserve immutable historical audits; do not delete working code just because
  templates are absent. Record exact commands, revision and supported assumptions.
  No binary/build/generated-artifact analysis and no spec edits.

  Actual result: concrete-only AST/semantic/identity assumptions traced to
  production interfaces; all T169–T198 responsibilities have component and real
  consumer test homes. Immutable source ownership, pinned Unicode, configured
  roots, ordinary lifecycle/IR/ABI and tool transaction contracts are retained.
  No code is obsolete merely because it lacks template support. Corrected the
  current AST guide's stale missing-feature paragraph; historical audits unchanged.
  Verification: make and fourteen focused leaves passed (twelve unit, two actual
  CLI/parser integration); exact commands and input partitions are in the audit.
  Independent controls accept ordinary/multiword names; valid hyphens, a typed
  template call and concrete static results expose recognition gaps. The first
  template probe mixed named/positional runtime bindings and correctly failed;
  the corrected named fixture reaches the placeholder error. make conformance-check
  returned expected exit 2 for stale pre-template evidence; no fingerprints refreshed.
  Document audit checks 205 Markdown documents, canonical/index links, 214 rules,
  108 unchanged EBNF productions and 37 template areas; git diff --check passes.
  make docs-complete passed with 2335 ready inputs and zero debt; make docs passed
  with 1749 maintained file pages, 6703 interfaces and 417 checked destinations.
  Final generation after archival rechecks final maintained inputs before commit.
  No compiler/test/native code or spec changes, full-suite/sanitizer or template
  implementation claim. This commit closes T168; T169 is the next ready task.

<a id="t169"></a>

- [x] **T169 — Share the exact complete-name and path-component profile.**
  Completed 2026-11-18; input 755bc97, initially clean tree.
  Owner: [shared word contract](docs/implementation/unicode.md#shared-ama-word-prefix).
  Prerequisites: T168.
  Rules: [SRC-03](spec/language/source-and-names.md#src-03),
  [SYN-06](spec/language/source-and-names.md#syn-06),
  [TPL-01](spec/language/templates.md#tpl-01).
  Boundary: source Unicode/name recognition, lexer/parser and module-path/tool
  consumers; retain original bytes and spans.
  Steps: (1) define one word/name recognition contract; (2) adapt ordinary,
  qualified and static-name consumers with explicit root/delimiter recognition;
  (3) exercise filesystem and ordinary/static syntax compositions.
  Accept: internal isolated ASCII hyphens, multiword/Unicode/underscore names and
  exact distinct identities pass; malformed UTF-8, joins, repeated/end hyphens,
  unsupported dash/minus punctuation lookalikes and invalid component aliases
  fail. Valid XID homographs remain accepted and byte-distinct.
  Literals/comments retain data,
  `a-b` remains one name, and .abi destructor/C identifiers retain their own syntax.
  Check ordinary grammar, physical catalogs and identity collision rejection.

  Steps/results: the bounded prefix contract and general invariant/progress/cost
  argument precede the implementation in the owning document. word.c owns maximal
  prefix/full validation; identifier.c owns pinned properties; lex_name delegates
  to the same kernel as complete names. Ten explicit test source lists include
  the new source; directory-based discovery handles ordinary builds. No fallback,
  normalization, locale, scalar-property or C-identifier change.
  Tests compare 97656 finite strings with an independent three-state grammar,
  every post-hyphen code point/truncation against independently read Unicode 17
  data, and exact UTF-8/boundary/error-output partitions. Actual token/path/root,
  structural submodule, nominal identity, frontend and rename consumers exercise
  the shared handoffs. Per-file evidence covers both new files and all changed
  headers/test/build consumers; independent header compilation passes.
  Final make, sixteen unit leaves and two integration leaves passed (session 18872;
  exact commands/results in the owner). An emitted/compiled/executed program with
  distinct hyphen/underscore module types, owned/list/reference fields and calls
  exits zero with empty stdout, using source/output paths containing spaces.
  ASan/UBSan Unicode leaf passes with strict warnings and detect_leaks=0;
  positive/intentional-overflow controls confirm instrumentation. LeakSanitizer
  is unavailable under ptrace: a nonleaking minimal control reproduces its fatal
  shutdown. No successful leak-sanitizer or whole-release claim is made.
  Failures preserved: the new rename test initially used the wrong status-member
  name; correction to validated passed. parser/composition fails its stale text-
  fence inventory before parsing, identically on an isolated unchanged 755bc97
  archive. No case skipped or expected count weakened; T170 owns canonical
  inventory/context reconciliation and static syntax. This known baseline gap
  is outside T169's name-profile acceptance; broad parser conformance is unclaimed.
  Documentation checks: canonical/local links, unchanged spec/EBNF and git diff
  --check pass; docs-complete reports 2337 ready inputs, zero debt. make docs
  passes with 1751 maintained file pages, 6711 interfaces and 417 destinations
  under Doxygen 1.11.0/Python 3.14.4/markdown-it-py 3.0.0. Final regeneration after
  this archival edit rechecks final inputs. No spec, Unicode table or reviewed
  conformance fingerprint changed. Commit T169 separately; T170 is next.

<a id="t170"></a>

- [x] **T170 — Parse immutable static interfaces and application trees.**
  Status: completed 2026-11-18. Scope: structural AST/parser only; no static
  name/kind resolution, substitution or template execution. Steps:
  (1) document source-span ownership, structural classifications and iterative
  nesting invariants; (2) recognize static prefixes/names and represent expression,
  binding, signature and result trees; (3) compose declarations/interfaces,
  namespaces and standalone projections with the existing parser; (4) reconcile
  canonical example contexts, exercise allocation/depth/failure partitions and
  real ordinary consumers; (5) per-file evidence, Doxygen, archival and commit.
  Baseline acceptance owner: reconcile the explicit canonical-example inventory
  in tests/unit/parser/composition/support/documented.c with the current
  specification, including newly added static forms and each example's declared
  module/body/ABI context. T169 found a pre-existing fence-count assertion before
  parser invocation: declarations 9 versus 16, lists 3 versus 4, grammar 8 versus
  20 and C-library 1 versus 3. Preserve the failure evidence and validate contexts;
  do not skip unsupported examples or guess wrappers to obtain a pass.
  Prerequisites: T169.
  Rules: [grammar](spec/grammar.md#static-template-syntax-and-design-boundaries),
  [COMP-01](spec/implementation/compiler.md#comp-01),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: AST lifetime and parser section/declaration/type-expression/binding
  children, composed through the reusable frontend.
  Steps: (1) represent formals, recursive signatures, two-line static results,
  applications/projections and skips with source spans; (2) implement structural
  parsing separately from lookup; (3) test actual section/body/nested compositions.
  Accept: valid categories' syntax, nested arguments/results/namespaces and
  interleaved runtime declarations parse without symbol knowledge; bad indentation,
  missing result definitions, invalid suffixes/markers and malformed nesting fail.
  Check allocation failures, safe partial-tree destruction and finite traversal
  without an arbitrary depth limit; native syntax uses the same parser.

  Results: source-owned static expression/binding/signature/result/projection
  records replace bare type-path spans. Separate iterative parser components
  preserve exact source heads, nested ownership, source-order phases and skips;
  section/body composition performs no lookup or kind inference. Ordinary
  concrete consumers retain their behavior; the old semantic inventory refuses
  unresolved static syntax until T184 composes the resolver before it.
  Evidence: [current component contract](docs/implementation/syntax-tree.md#separate-static-syntax),
  updated by T206. The acceptance below records T170's historical input revision.
  Ten independent static cases cover exact names, nested and positional operands,
  interface/native/body composition, rejection partitions, 600-level expression
  and signature trees, every actual expression/module allocation-failure prefix
  and counted cleanup. The explicitly contextualized canonical manifest checks
  119 fences, including the documented syntax rejection; no example is skipped
  to hide an unsupported form. Complete parser, token, semantic names/inference/
  composition, project closure, compiler check, editor and codegen assembly
  checks pass, as do parser/grammar and check integration leaves and make.
  The static leaf passes ASan/UBSan/Werror with the previously established
  container LeakSanitizer limitation; counted ownership still reaches zero.
  Failures and corrections: include the standard snprintf declaration; supply
  explicit interface-fragment markers instead of duplicate section markers;
  preserve nullable namespaces in editor indexing; include the header's full
  rule boundary on its Doxygen file page. Corrected affected checks pass.
  No heuristics, spec changes or semantic decisions were introduced.
  docs-complete: 2352 ready inputs, zero debt. Corrected Doxygen generation:
  1765 maintained file pages, 6765 interfaces, 417 canonical destinations,
  5573 pages, all actual destinations validated. Source navigation resolves
  src/ast/static.h links to their exact spec files/anchors; installed VS Code
  code --goto templates.md:759 returns zero for TPL-07 (no mouse-click claim).
  The archived inputs receive final regeneration before the separate task
  commit. Reviewed language-conformance fingerprints remain owned by T196;
  full template execution is not claimed by structural parsing. T171 is next.

<a id="t171"></a>

- [x] **T171 — Load template definitions through the exact source registry.**
  Status: completed 2026-11-18. Input f6f80c0. Record:
  [static dependency extraction](docs/implementation/project-dependencies.md#static-expression-dependencies-t171).
  Existing catalog/registry already enforces complete root identities, physical
  aliases, once-only parsing and finite source cycles. Extend structural path
  extraction only; do not infer actual types, generate specialization records or
  change native admission rules. Independent component tests precede real
  configured-root/diagnostic/failure compositions and final documentation checks.
  Prerequisites: T170.
  Rules: [DEP-01](spec/implementation/compiler.md#dep-01),
  [DEP-02](spec/implementation/compiler.md#dep-02),
  [DEP-03](spec/implementation/compiler.md#dep-03),
  [TPL-05](spec/language/templates.md#tpl-05).
  Boundary: source catalog, project dependency closure and immutable parsed
  definition registry; virtual specializations are a separate later registry.
  Steps: (1) trace concrete module-path dependencies in static expression trees;
  (2) integrate exact loading/origin lookup; (3) compose configured-root loading
  with the new parser and requester diagnostics.
  Accept: both root kinds and name+/ storage preserve complete logical identity;
  atomic placeholders are never filesystem paths. Resolve each required concrete
  definition once; reject conflicts, missing definitions and aliases exactly.
  Distinguish source cycles from specialization/ownership cycles; retain original
  spans and native-companion requirements without fabricating specialization files.

  Results: one explicit-frame expression component collects only concrete
  MODULE_PATH heads and traverses input children beneath both concrete and
  placeholder heads. The AST collector composes declaration types, constant
  namespaces, static result definitions and standalone projections. Atomic
  formals/signatures/output aliases never become filesystem requests. Existing
  source-position sorting, exact roots, module registry and native selection
  remain the admission boundary; no virtual module source file is created.
  Evidence: [complete step record and proof](docs/implementation/project-dependencies.md#static-expression-dependencies-t171).
  Five named component/composition cases check independently built expression
  trees, original repeated spans, null/skips/aliases, all four real source
  positions, Project/Standard root order and hyphens, physical name+/ storage,
  unimported draft parents, source cycles and shared AST identities. Missing,
  draft and invalid nested arguments preserve exact requester chains and never
  retry another root family; native companions are selected and diagnosed.
  Real 768-level static syntax produces 769 distinct source requests to one
  parsed definition. Success, missing and native-error paths survive every
  actual allocation-failure prefix, with zero counted live allocations.
  make, complete project/source-catalog/compiler-check/editor suites and check
  integration pass. The entire closure leaf also passes ASan/UBSan/Werror with
  the established container LeakSanitizer limitation. Initial fixture failures
  and corrections are retained in the owner: draft lexical spacing and the
  previously uncreated .factory source in the T170 semantic-admission scenario.
  Required behavior/assertions were preserved; no heuristic, spec edit or
  unresolved semantic interpretation was introduced.
  docs-complete passes: 2358 ready inputs, zero debt. Doxygen passes: 1771
  maintained file pages, 6779 interfaces, 417 canonical destinations and 5588
  pages. Every deep-source link resolves to its canonical rule; code --goto
  compiler.md:630 returns zero for DEP-03 (no GUI click claim). Final generation
  rechecks the archived/staged input bytes before this task's separate commit.
  Conformance fingerprints remain owned by T196; static semantics and generated
  specialization execution remain the later sprint tasks. T172 is next.

<a id="t199"></a>

- [x] **T199 — Clarify output-only interface Type introductions (AMB-011).**
  Status: completed, 2026-11-18; user explicitly confirms the recommended Type
  introduction and authorizes the precise specification clarification.
  Prerequisite: T171. This decision unblocks T172, without completing it.
  Rules: TPL-02, TPL-07, TPL-12 and grammar static-template explanation.
  Steps: record the decision/source; define introduction eligibility independently
  of static outputs; distinguish input-pattern labels from output labels; add an
  explained example and parser/context regression; verify links/grammar and
  documentation, archive and commit this clarification separately.
  Acceptance: no new syntax production or output-derived free formal; output-only
  interface heads introduce Type, output labels never define Template signatures,
  and ordinary alias/closure/scope rules remain linked to their canonical owners.
  Steps/results: user decision recorded before edits. TPL-02 owns the exact
  no-static-input definition and complete explained example; TPL-12/grammar
  distinguish that introduction from output actions. No EBNF production changes.
  Existing known-symbol exclusions and shared/body/namespace restrictions remain
  explicit. parser/static passes a new four-part implicit/explicit/named/skip
  regression; parser/composition checks all 120 contextualized canonical fences.
  Logs: /tmp/ama-t199-static.log and /tmp/ama-t199-examples.log. These are syntax
  checks; T172's semantic parameter/alias assertions remain required. Next:
  canonical/link checks, complete documentation gates, archive and task commit.

  Acceptance: both parser leaves pass; canonical/link audit checks 205 Markdown
  files, 2883 local links, all 214 rules, unchanged aligned 108 EBNF productions
  and the unchanged 150-area acceptance matrix (37 template areas). Only the
  authorized TPL-02/TPL-12 chapters and grammar explanation changed in spec.
  docs-complete passes with 2358 ready inputs and zero debt; Doxygen validates
  1771 maintained file pages, 6780 interfaces and all 417 canonical destinations.
  T172 remains unfinished and owns actual Type/alias inventory assertions.
  Final documentation regeneration checks archive inputs before the required
  separate task commit. No compiler-inference support is claimed by this edit.

<a id="t172"></a>

- [x] **T172 — Collect scoped static symbols, kinds and result definitions.**
  Status: accepted 2026-11-18; input 2562706; implementation follows committed T199.
  The user resolves [AMB-011](AMBIGUITIES.md#amb-011): an output-only interface
  head introduces Type. The precise rule
  is recorded and verified. Step record: [static symbols](docs/implementation/static-symbols.md).
  Prerequisites: T171, T199.
  Rules: [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: static semantic inventories and lexical binding, separate from
  runtime fields and from callee formal/result label lookup.
  Steps: (1) inventory explicit formals and static result binders; (2) classify
  receiver aliases versus genuine implicit interface introductions by source
  order, then finish whole-interface formal collection; (3) build/check finite
  recursive named Type/Template signatures and bound result-expression records.
  Accept: output-only Type heads (named and skipped outputs, with an explicit
  Type-formal control), result-only Type introductions, existing Template reuse, nested
  signatures and aliases have exact scopes; same-spelled foreign names never
  capture. Reject body-only free names, implicit result Template introductions,
  duplicate/category/signature errors and alias use before its receiver.
  Later alias uses do not invent owner formals; retained output definitions have
  no runtime storage or evaluation. Fix lexical identities now but retain
  category/signature obligations whose projected producer origin is unresolved
  until T179 can determine it. Allocation failure leaves destructible owners.

  Evidence: all 36 static cases in six leaves pass strict C11 with ASan/UBSan
  (LeakSanitizer disabled for the documented container limitation), including
  original scopes/order, every real allocation refusal, 120 sort permutations
  with every prefix, 900-deep signature construction/comparison, a 700-level
  actual parser-to-inventory signature and shared-DAG memoization.
  Logs: /tmp/ama-t172-sanitizers.log, /tmp/ama-t172-project.log,
  /tmp/ama-t172-build.log. Compiler make and the complete project parent pass.
  Documentation: 2405 ready inputs, zero debt; Doxygen validates 1811 file pages,
  6939 interfaces and all 418 canonical destinations. Local audit checks 206
  Markdown files/2884 links, 214 rules and 108 unchanged grammar productions.
  Actual editor navigation opens the canonical TPL-02 location successfully.
  Failed helper-comment documentation coverage was corrected before acceptance;
  all decisions, algorithms, bounds and checks are in the component record.
  Handoff: pending application metadata belongs to T173/T175/T177; projected
  aliases belong to T179, with exact syntax and pending records retained.
  This task certifies scoped static inventory, not executable-template support;
  full frontend integration remains T184. No specification changes in T172.

<a id="t173"></a>

- [x] **T173 — Normalize separate static and runtime binding sequences.**
  Status: accepted 2026-11-18 on input e4c8d92. Step record:
  [binding sequences](docs/implementation/binding-sequences.md).
  Prerequisites: T172.
  Rules: [TPL-12](spec/language/static-results.md#tpl-12),
  [CALL-06](spec/language/calls-and-results.md#call-06).
  Boundary: semantic application/interface binding descriptors, consumed by later
  constraint generation rather than parser symbol-dependent decisions.
  Steps: (1) retain actual-origin formal/output order and runtime role ordinals;
  (2) validate modes/order/arity and normalize named/positional requests and skips;
  (3) compose nested applications with parser and scoped interfaces.
  Accept: snapshot the residual static-input sequence before processing bindings,
  include interface-inferred formals, order static outputs by `@>` source order,
  and keep four independent static/runtime input/output sequences. Check every
  skip position, fixed captures, reversed expected signatures and nested modes.
  Reject mixed modes, excess/duplicate positions and input-after-output; unknown
  actual origins remain explicit obligations, never guessed from output labels.

  Evidence: all 26 binding cases plus all 36 prior T172 cases pass the full
  affected static parent with strict C11 warnings as errors and ASan/UBSan.
  This covers actual/reversed expected order, all skip prefixes and first excess
  positions, captured-formal filtering, nested modes, static/runtime/shared and
  intrinsic surfaces, original diagnostic contexts, counter boundaries and every
  actual allocation refusal while preserving previous successful publications.
  Native list composition loads the real Standard integer dependency; existing
  fixture domains and old scope tests remain passing. Compiler make and the
  complete project parent pass. Logs: /tmp/ama-t173-sanitizers.log,
  /tmp/ama-t173-build.log, /tmp/ama-t173-project.log.
  Documentation: 2440 ready inputs, zero debt; 207 Markdown files/2887 local
  links, 214 canonical rules and 108 unchanged grammar productions. Doxygen
  validates 1841 file pages, 7048 interfaces and all 419 canonical destinations;
  actual editor navigation opens TPL-12. The component record retains initial
  fixture failures, exact corrections, ownership/termination/cost invariants,
  all acceptance results and stage limits. No specification change in T173.
  Handoff: records preserve raw operands and unresolved actual interfaces;
  immutable type/application terms and value closure remain T175/T177/T179,
  full executable frontend integration remains T184. No runtime target,
  native callback, default initializer or hidden instance is executed here.

<a id="t174"></a>

- [x] **T174 — Reject native definitions with own free static formals early.**
  Status: accepted 2026-11-18 on input 11d0b7e. Step record:
  [native admission](docs/implementation/native-static-admission.md).
  Prerequisites: T172, T173.
  Rules: [TPL-13](spec/language/templates.md#tpl-13),
  [CABI-06](spec/implementation/native-interface.md#cabi-06).
  Boundary: semantic native admission and authoring/scaffold preparation.
  Steps: (1) consume the completed own-formal inventory; (2) reject prohibited
  definitions before specialization-specific header preparation; (3) compose
  check-only and native-authoring commands with precise formal/definition errors.
  Accept: explicit, pattern-introduced, implicit input/result and unused free
  Type/Template formals fail even when all uses would bind them. Bound aliases,
  returned constructors' own residuals and concrete dependencies are not mistaken
  for owner formals. Existing concrete native declarations remain admissible;
  generated concrete template/native composition is accepted later in T190.

  Evidence: five pure policy, five frontend/lowering and two real CLI cases
  cover every introduction kind, unused/fully bound definitions, aliases/foreign
  residual controls, original marker/formal diagnostics, required/catalog and
  normal/authoring paths, no artifacts on create/overwrite refusal, every new
  frontend allocation refusal and original diagnostic lifetime. Pure/frontend
  leaves pass strict C11 ASan/UBSan; compiler parent passes strict warnings as
  errors. Existing library, complete relocation and concrete CLI check/scaffold
  regressions pass. Relocation includes the full 1340-position application
  allocation matrix and all interruption/recovery failure partitions.
  Logs: /tmp/ama-t174-admission.log, /tmp/ama-t174-frontend-admission.log,
  /tmp/ama-t174-compiler-parent-final.log, /tmp/ama-t174-cli.log,
  /tmp/ama-t174-policy-sanitizers.log, /tmp/ama-t174-frontend-sanitizers.log,
  /tmp/ama-t174-library.log, /tmp/ama-t174-relocation.log,
  /tmp/ama-t174-concrete-check.log, /tmp/ama-t174-concrete-scaffold.log.
  Integration required an explicit unresolved-static lowering guard for the
  three T170 AST kinds and a shared frontend source list for nine consumers.
  The old concrete-boundary regression now checks precise earlier kind errors
  versus remaining legacy unsupported forms, without weakening rejection.
  Documentation: 2456 ready inputs, zero debt; 208 Markdown files/2890 links,
  214 canonical rules and 108 unchanged EBNF productions. Doxygen validates
  1850 file pages, 7077 interfaces and all 420 referenced destinations. Actual
  editor navigation opens TPL-13. The component record retains corrections,
  owner/cost proofs, all collected results and explicit handoff limits.
  No specification changes or default native specialization. Fully concrete
  storage/header resolution and generated interoperability remain T184/T190.

<a id="t175"></a>

- [x] **T175 — Represent scoped static terms and normalized binding records.**
  Status: accepted 2026-11-18 on input 5c00e82. Step record:
  [static terms](docs/implementation/static-terms.md).
  Prerequisites: T172, T173.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-10](spec/language/templates.md#tpl-10).
  Boundary: a cohesive static-term model and immutable definition/application
  environments, separate from AST, concrete layouts and mutable runtime state.
  Steps: (1) define scoped Type/Template, inference and residual-binder terms;
  (2) construct/compare immutable normalized origin/capture/residual records
  without unfolding definitions; (3) test inventory-to-term handoffs with
  explicitly prepared records and failure cleanup.
  Accept: original formal identities/order and unused captures survive record
  construction and comparison; application normalization is owned by T177.
  Residual binders differ from unresolved inference variables; independent
  declaration occurrences receive distinct terms while repeated uses share the intended term.
  Explain representation ownership, exact equality inputs and termination/cost.

  Evidence: nine independent cases cover exact lexical identities, repeated and
  independent field/producer uses, deferred aliases, capture ordering/unused
  captures, local residuals, applied owner lookup, symbolic groundness, 512-level
  finite graphs, actual known/pending T173 imports, skips, reordered operands,
  distinct use provenance, copied/foreign identities, saturated counters and
  every observed preparation allocation refusal with counted full teardown.
  Full strict static parent passes 76 cases; nine term cases pass ASan/UBSan.
  Root build, real standalone headers and 2481 ready inputs/zero debt pass.
  Doxygen validates 1870 file pages, 7160 interfaces and 421 canonical targets;
  Markdown audit validates 209 files/2894 links, 214 rules and 108 unchanged
  EBNF productions. Actual editor navigation opens TPL-08. Component record
  retains the finite-DAG interning proof, precise costs, ownership, failed fixture
  and comment checks, exact commands/results and stage handoffs. No spec changes.
  Pointer equality is structural identity within an owner, never solver equality.
  Constraint solving, application normalization and expansion remain T176–T178;
  no executable specialization or runtime state is introduced by this component.

<a id="t176"></a>

- [x] **T176 — Solve finite structural type constraints exactly.**
  Status: accepted 2026-11-18 on input 5d735be. Step record:
  [static constraints](docs/implementation/static-constraints.md).
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

  Evidence: 24 independent constraint cases cover symmetric records, original
  source/parent diagnostics, later anchors, anchored/unanchored cycles, nominal
  origins and unused captures, partial residual binders, Type/Template and
  recursive signature conflicts, pending metadata and independent exact facts,
  direct/indirect/Template occurs failures and nominal-reference-cycle controls.
  An independent Boolean graph oracle checks 720 graph/order combinations.
  Shared 64-level normalization and 100 repeated queries establish finite shared
  work and immediate scratch release; stable solves allocate nothing. Actual
  T173/T175 and standard integer facts compose through original producer slots.
  Every observed allocation across seven operation partitions and all meaningful
  count/lookup boundaries is refused with counted full teardown and sticky errors.
  Full strict static parent passes its then-current 99 cases; final closure leaf
  passes the added metadata-undertermination partitions. All 24 current constraint
  cases pass strict C11 ASan/UBSan; root build passes. LSan's established container
  restriction remains explicit, separate from counted ownership evidence.
  Documentation: 2518 ready inputs/zero debt, standalone headers; 210 Markdown
  files/2901 links, 214 rules and 108 unchanged EBNF productions. Doxygen validates
  1899 file pages, 7283 interfaces and 422 canonical targets. Actual editor
  navigation opens TPL-10. The component note retains exact commands, input/logs,
  fixture corrections, solution-set/progress proofs, conservative costs and scope.
  No source definitions are unfolded, no default type/origin is chosen and no
  runtime state is merged. T177 owns application/recovery, T178–T180 own guarded
  expansion/projection closure, and T184 owns full frontend admission. No spec edit.

<a id="t177"></a>

- [x] **T177 — Apply partial constructors and recover nominal input patterns.**
  Status: accepted 2026-11-18 on input 7e3afda. Step record:
  [static applications](docs/implementation/static-applications.md).
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

  Evidence: 26 application cases cover direct/staged/reordered capture maps,
  mixed explicit/implicit source ordinals, exact Type/Template contracts,
  independently refined pending signatures, source-backed omission policies,
  positional skips, actual filtered interfaces and independent output counters.
  Exact recovery preserves unused captures, actual residuals and previous
  constructor equations; nested result equations determine inner constructors
  and RHS Types without origin or partition search. Independent finite models
  cover 27 three-slot maps, six orders and every prefix split (486 partitions),
  plus 27 positional partial recoveries with reversed expected-signature order.
  Every observed allocation across source/recovery/field/producer/refinement
  operations is refused with counted teardown; checked counters and sticky
  errors preserve publication. A real T175 mixed-introduction ordering defect
  is fixed through one shared original-ordinal selector and regression evidence.
  Full strict static parent and final source cases pass; all 26 cases pass
  AddressSanitizer/UBSan. LSan remains disabled under the established container
  restriction; counted zero-live-allocation ownership is checked independently.
  Root build passes. Documentation checks 2557 ready inputs, zero debts and
  standalone headers; Doxygen checks 1928 file pages, 7409 interfaces and 423
  destinations. Markdown navigation and actual editor opening of TPL-10 pass.
  The component record retains exact commands/logs, failure corrections,
  preservation/uniqueness/termination arguments, costs and consumer boundaries.
  No specification edit, definition expansion, runtime execution or executable
  specialization admission is claimed; T178–T180/T184 retain those boundaries.

<a id="t178"></a>

- [x] **T178 — Guard static dependencies and specialization creation paths.**
  Status: accepted 2026-11-18 on input 8cfe3f6. Step record:
  [static dependency guards](docs/implementation/static-guards.md).
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

  Evidence: 25 guard cases cover exact symbolic owner identity, active
  creation ancestry before complete keys, direct/mutual/equal/permuted/growing
  applications, finite nested/sibling reuse, partial metadata, supplied Types,
  ordinary producers and captured original constructors. Concrete key refresh
  retains incoming provenance, checks cached descendants under new ancestors,
  and rechecks scopes after independent equations close their keys. Distinct A/B
  specializations do not create a fictitious source-name cycle; ordinary-only
  creation cycles remain separate from static definition cycles.
  Creation reachability agrees with independent matrix closure for every
  four-vertex graph in both discovery orders: 131072 combinations. The separate
  scoped result/alias checker agrees with exhaustive topological-order checking
  for all three-node relations in both orders: 1024 combinations. Source-backed
  cycles, cached foreign outputs, alias scope, anchored equality controls and
  definition-key merging retain original diagnostics and full witness paths.
  Eight public-operation and four graph-kernel allocation partitions refuse
  every observed request; counters/products, copied/foreign handles and sticky
  borrowed errors are checked with complete counted teardown. One hundred
  unchanged refreshes allocate nothing. No heuristic depth/pass/time limit.
  Strict guard parent plus final captured-origin delta and all 25 cases under
  strict C11 ASan/UBSan pass; root build passes. LSan's established container
  restriction remains explicit alongside independent counted ownership checks.
  Documentation checks 2597 ready inputs, zero debts and standalone headers;
  212 Markdown files/2914 links, 214 rules and 108 unchanged EBNF productions.
  Doxygen validates 1959 file pages, 7554 interfaces and 424 destinations;
  actual editor navigation opens TPL-11. The component record retains exact
  commands/logs, scope, invariant/termination/cost arguments and failure laws.
  No specification edit, output evaluation, runtime execution or executable
  specialization admission is claimed; T179/T180/T184 retain those boundaries.

Historical component record retained by T210; this evidence refers to the
original task inputs and source contract, not the current supported interface.

### T178 historical component evidence

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

<a id="t200"></a>

- [x] **T200 — Specify closed specialization cycles and fixed constant namespaces.**
  Status: accepted, 2026-11-18; input 78a2099 with unfinished T181 work preserved.
  Prerequisites: accepted template design and the explicit user decisions for
  AMB-012 option 2 and exact same-key recursion, followed by authorization:
  "Вноси изменения в спецификацию с безопасными послаблениями. Проверь
  согласованность и непротиворечивость требований в спецификации."
  Rules: TPL-10/TPL-11/TPL-12, CONST-03/CONST-05, TYPE-04, COMP-03 and the
  canonical grammar/acceptance matrix. Boundary: specification, decision log,
  task routing and documentation; no compiler implementation or support claim.
  Steps: (1) audit canonical and derived specification text and record decisions;
  (2) define exact closed-key recurrence, unresolved obligations, namespace reuse,
  cache-independent checks and termination, with explained examples;
  (3) reconcile grammar guidance, implementation/acceptance contracts and TODO
  consumers, verify links/rules/grammar and documentation, archive and commit.
  Acceptance: no path connects distinct concrete keys of the same source
  template; exact-key cycles remain subject to separate eager construction,
  substitution, static-definition and constant-value checks. Both ordinary
  no-output namespace spellings reuse the fixed Type. Unknown keys are never
  guessed, caches do not hide outgoing paths, and the finite-graph argument
  covers all admitted creation paths. Record actual checks, not compiler claims.
  Step 1 findings: TPL-12's pre-key origin rejection, blanket cycle wording in
  CONST-05 and same-key negative acceptance cases require coordinated changes.
  Results: coordinated nine specification files, explanations, four new Ama
  fences and eight existing acceptance areas. TPL-11 owns the exact graph
  predicate, incremental predecessor/successor check, full-key comparison,
  unresolved-obligation policy and finite-branching/simple-path proof. TPL-12
  retains static-definition cycle rejection; CONST-05's former blanket dependency
  cycle wording now refers specifically to constant-value initialization.
  The A(text)/B/A(integer) example distinguishes fixed namespace reuse even
  after same-key cycles become legal. Closed-cycle outgoing-branch rejection,
  independent siblings, references, empty lists, ownership and capture boundaries
  are explicit. Grammar productions and stable requirement/acceptance identities
  are unchanged. AMB-012 records user decisions and authorization; AMB-005
  preserves its dated original decision and identifies the later refinement.
  Verification: make -C tests/unit/parser/composition exits 0, including all
  124 canonical Ama/ABI examples and unchanged ownership/prefix/failure cases
  (log /tmp/ama-t200-parser-examples.log). The existing per-file named consumer
  covers support/documented.c; only its four explicit example contexts changed.
  Pre-archive canonical/link audit exits 0: 213 Markdown pages, 2942 local links, 214 rules,
  108 byte-identical EBNF productions and 150 unchanged acceptance-area identities.
  A temporary independent graph model (python3 /tmp/ama-t200-four-vertex-model.py,
  log /tmp/ama-t200-model.log, exit 0) enumerates all 65536 four-vertex graphs,
  including self-loops, for two origin profiles. Independent BFS and incremental
  closure agree in 262144 acceptance comparisons and 9824 accepted full closures;
  131072 ascending/descending order comparisons and six explicit controls pass.
  Profiles accept/reject 784/64752 (two template pairs) and 4128/61408
  (one template pair plus two ordinary vertices). This finite model supplements
  the canonical proof; it does not test compiler behavior or arbitrary graph sizes.
  Documentation acceptance uses an isolated HEAD source tree with only T200
  document/example changes overlaid at /tmp/ama-t200-docs-x4btfgw_/tree, excluding
  unfinished T181 code. With GIT_INDEX_FILE=/tmp/ama-t200-docs-x4btfgw_/index
  and GIT_WORK_TREE set to that tree, make docs-complete exits 0: 2655 reviewed
  code/fixture inputs, zero debts, independent headers and named test interfaces
  (log /tmp/ama-t200-docs-complete.log). On those checked code inputs,
  python3 tools/documentation/build.py --doxygen doxygen exits 0: 2006 maintained
  file pages, 7775 interfaces, 427 checked destinations and 6341 pages (log
  /tmp/ama-t200-doxygen-initial.log). Tools: Doxygen 1.11.0, Python 3.14.4,
  markdown-it-py 3.0.0. Final archived link/structure/whitespace checks also exit
  0: 213 pages, 2943 local links, the same rule/grammar/acceptance identities,
  and unique TODO/DONE task IDs with T200 archived, T201 planned and T181 unfinished.
  Delivery regenerates the archived inputs through make docs in the isolated
  tree (log /tmp/ama-t200-doxygen-final.log) and compares its manifest with every
  corresponding working-tree document/example byte before the task commit.
  Implementation remains pending: T201 owns changed guards and result/publication
  consumers; T181 remains unfinished behind that prerequisite. Component records
  explicitly distinguish prior accepted evidence from the revised target. No
  compiler/runtime support or conformance fingerprint refresh is claimed.

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

<a id="t179"></a>

- [x] **T179 — Generate guarded static result projections and equations.**
  Status: accepted 2026-11-18 on input e48fc3e. Step record:
  [static result equations](docs/implementation/static-projections.md).
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

  Evidence: 31 named result component cases cover original scoped source/alias
  identities, persistent checked uses, complete definition caching, shallow
  operands, exact actual-origin refinement/recovery, iterative nested evaluation,
  returned partial constructors, named/positional output equations, skipped and
  unselected outputs, existing receivers, invalid fields/arity, Pending origins,
  creation/definition cycles and every observed evaluation allocation refusal.
  Source/selected values remain distinct and all accepted fixtures unwind to zero.
  Repaired T178 cached-descendant validation across unresolved ancestry with two
  source regressions; all 27 affected guard cases and exhaustive graph oracles
  pass strict and ASan/UBSan acceptance. Initial 30 result cases plus final
  13-case evaluator delta pass strict and ASan/UBSan; root build passes. LSan's
  established container restriction is separate from counted ownership checks.
  Documentation passes 2633 ready inputs/zero debts and standalone headers;
  Markdown/rule/unchanged-grammar checks and Doxygen links pass. Detailed
  commands, limits, proof and final archive refresh are in the linked step record.
  No spec changes; T180 owns final producer closure and alias publication.

<a id="t180"></a>

- [x] **T180 — Finalize complete producers and immutable result aliases.**
  Status: accepted 2026-11-18 on input fa1cd3e. Step record:
  [static publication](docs/implementation/static-publication.md).
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

  Evidence: 21 publication cases check independent receiver inference, fresh
  alias/skip/unused-input rejection, noninvertible outputs, unknown actual origins,
  canonical output order, immutable alias agreement/conflict, scoped owner identity,
  returned partial constructors and six original contexts with/without output
  syntax. Distinct output phases retain distinct omitted-input terms. Exact
  required-source edges and cached RHS witnesses preserve transitive original
  receiver producer closure while accepting compatible equality cycles. New checked
  contexts count as actual provenance evidence; unchanged snapshots do not.
  The receiver-proof and provenance-progress regressions failed before their
  fixes and pass with positive controls. Strict and ASan/UBSan pass all 21
  publication and 32 affected result cases; root build passes. Two full-operation
  allocation partitions, invalid handles, counter limits, late conflicts and
  counted zero-leak teardown pass. LSan's container limitation remains explicit.
  Documentation passes 2655 ready inputs/zero debts and standalone headers;
  Markdown/rule/unchanged-grammar checks and Doxygen validate 2006 file pages,
  7775 interfaces, 426 canonical destinations and 6339 pages. Detailed commands,
  proofs, source boundaries and final archive refresh are in the linked record.
  No spec changes. Whole-project admission and ordinary validation remain T184/T185.

<a id="t201"></a>

- [x] **T201 — Align creation guards and consumers with closed-key cycles.**
  Status: accepted on input 7469fd4, 2026-11-18; authorized by the user's request to
  continue under the agreed plan. Step record: [creation guards](DONE.md#t201-alignment).
  existing T178/T179/T180 evidence describes the earlier stricter contract.
  Prerequisites: T178, T179, T180, T200.
  Rules: [TPL-11](spec/language/templates.md#tpl-11),
  [TPL-12](spec/language/static-results.md#tpl-12),
  [CONST-03](spec/language/declarations.md#const-03),
  [CONST-05](spec/language/declarations.md#const-05).
  Boundary: creation guards, their result/publication/application consumers,
  focused tests and current component comments/contracts; preserve T181's local
  partial work and replace its unaccepted edge adapter only as required.
  Steps: (1) specify exact closed-key/pending/reuse states and finite progress;
  (2) implement reachability and cycle-safe checked contexts, keeping static
  definition cycles separate; (3) align result/publication/namespace consumers,
  documentation and per-file evidence, run composition/resource/build checks.
  Accept: direct/mutual same-key reference/empty-list cycles close without body
  re-expansion or premature validation. Reachable distinct keys of one origin
  still fail, including differing unused captures, unequal permutations and
  cycle-outgoing branches. Unknown comparisons wait for actual constraints,
  then close or fail without guessing. Both fixed ordinary namespace forms
  retain independent checking without creation edges; output phases retain
  producers. Cold/warm caches and reordered roots/edges agree. Exhaustive finite
  graph tests, allocation failures and real guard/result/publication consumers
  validate the changed behavior; static-definition/occurs/eager/value checks
  remain separate. Update stale component prose rather than claiming T178's
  earlier checks establish support. Commit before resuming T181.
  Acceptance: 32 guard, 32 result and 29 publication cases pass their affected
  strict and ASan/UBSan compositions, including 131072 independent creation
  graph/order checks, the separate definition oracle, all observed allocation
  refusals and counted teardown. Root make -j4 passes without warnings. LSan
  remains unavailable under the established container restriction; its absence
  is not counted as sanitizer evidence.
  Cold/warm foreign cached proofs now preserve actual unknown ancestry, with
  positive unrelated-ancestor and resource-refusal regressions. Full failed-run,
  invariant, complexity and exact command/configuration evidence is retained in
  the linked component note. Specification bytes are unchanged.
  Per-file/header acceptance passes 2658 ready inputs with zero debts in an
  isolated HEAD-plus-T201 tree, excluding unfinished T181 additions. Doxygen
  rerun passes 2008 maintained file pages, 7795 interfaces, 427 actual canonical
  destinations and 6346 generated pages. Markdown audit passes 215 documents,
  2965 local links, 214 rules and 108 unchanged EBNF productions; queue/archive
  IDs are unique. Editor source-navigation command exits 0 (dispatch evidence,
  not an observed manual click). Final archive-input Doxygen refresh must pass
  before staging; its collected outcome is recorded in this task's commit.
  The seven replaced unused draft adapter inputs are preserved with hashes at
  /tmp/ama-t201-input-7469fd4. T181's constant sources/tests remain unfinished
  local work and are excluded from this commit. Next ready task: T181.

Historical component record retained by T210; this evidence refers to the
original task inputs and source contract, not the current supported interface.

 <a id="t201-alignment"></a>

### T201 historical component evidence

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
GUI click. Complete task evidence is archived in [DONE](DONE.md#t201).
Final archive-input regeneration uses /tmp/ama-t201-doxygen-archive.log; collect
its outcome before explicit staging and record it in the task commit. No active
functional jobs. T181 is next; its partial sources/tests remain excluded from
T201. Specification bytes have not changed.

<a id="t181"></a>

- [x] **T181 — Resolve dependent constant types and explicit namespaces.**
  Status: accepted on implementation input 83603d5, 2026-11-18; preserved
  earlier partial input 78a2099 and resolved AMB-012/T201 decisions.
  Step record:
  [static constants](docs/implementation/static-constants.md).
  Prerequisites: T174, T177, T180, T201.
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
  Acceptance: 35 strict C11/-Werror and ASan/UBSan constant cases; all 54 full
  static dispatcher leaf executables; every observed allocation refusal with
  counted teardown; root make -j4 without warnings. Both namespace spellings
  agree without false creation edges across distinct caller specializations.
  Typed local/qualified initializers infer Types; literal bytes supply no anchor.
  Original source/dependency identity, root kind, own member/list storage, missing
  scopes/members, open namespaces, forbidden sources and invalid handles/counts
  are covered. Full commands, proof/cost and failed-run corrections are retained
  in the owning component record. LSan's established container restriction is
  explicit and is not claimed as passing leak instrumentation.
  Complete per-file/independent-header checks pass 2690 ready inputs and zero
  debts. Doxygen passes 2032 file pages, 7914 interfaces, 427 actual destinations
  and 6439 pages. Markdown audit passes 215 documents, 2969 links, 214 rules and
  108 unchanged grammar productions. Comment/EOF-only final cleanup changes no
  C tokens. Final archived reference must regenerate before staging; record its
  collected outcome in the commit. No spec edits or executable template claims.
  Next ready task: T182, typed connections and nested-field constraints.

<a id="t202"></a>

- [x] **T202 — Specify separate static modules and resolve AMB-013.**
  Completed 2026-11-18; input 43afdd9. The user's explicit specification/EBNF
  authorization and final clarification approve a static own-result pair:
  `...dictionary type`, then `.dictionary` and its indented named inputs.
  Relative paths are expressly deferred; no such syntax/resolution rule is added.
  Record: [static-module specification](docs/implementation/static-modules-specification.md).
  Scope: 17 canonical specification files plus decisions and implementation
  handoff records; no compiler source, tests, registry hashes or T182 private
  implementation notes are delivered by this task.
  Changes: exclusive `@...` structural graphs; named-only static inputs/outputs;
  public immutable pair/receiver results and multiple-output selection; whole-
  graph scope/input collection; minimum requirements inferred from all constructor
  uses while preserving actual residuals/captures; complete producer closure;
  independent single named concrete-export selectors in ordinary declarations
  and constant namespaces. Ordinary interfaces retain runtime name/type pairs
  and bare Type parameters. Native definitions remain concrete; no static ABI,
  runtime template object, value-indexed Type or list-foundation rewrite appears.
  Preserved: exact ordinary typed inference and field specialization, full nominal
  identity without shared object state, exact-key closure and unequal-key path
  rejection, and separate definition/occurs/eager-ownership/value checks.
  AMB-013 is resolved by excluding computed ordinary interfaces, not adding a
  preview exception. AMB-014 records the final approved pair spelling. T203 must
  audit/plan alignment of earlier components before incomplete T182 resumes.
  Evidence: cross-review of the chapters, grammar and collateral requirements;
  all 214 unique requirement IDs retain their canonical files and indexed anchors;
  11 EBNF blocks contain 108 defined, aligned and reference-closed productions.
  Markdown checks include every spec chapter, TODO/DONE, ambiguity registry and
  task note; undefined-production/alignment/group negative controls reject as
  expected. The initial link check found three references to the renamed Stage 11
  heading; restoring its stable anchor fixed them and the rerun passed.
  `make docs-complete` passed in the existing workspace (2743 inputs, zero debts;
  includes unfinished T182 and makes no code-support claim). A HEAD-plus-task
  snapshot excluding that partial implementation passed `make docs`: 2690 ready
  inputs, zero debts, 2032 maintained file pages, 7914 documented interfaces and
  428 checked destinations, Doxygen 1.11.0, Python 3.14.4, Markdown parser 3.0.0.
  Final archive/record inputs are regenerated and document/staged whitespace
  checks are required to pass before this task's commit. No compiler execution
  or updated conformance-evidence claim is made by these documentation checks.

<a id="t205"></a>

- [x] **T205 — Clarify disposal of explicitly owned foreign child contexts.**
  Completed 2026-11-19 on specification input 155c9da. The user's explicit
  AMB-015 answer approves the independent-peer/owned-child clarification and
  continuation of the review and plan. T204 remains a separate unfinished task.
  Rules: [LIFE-05](spec/implementation/native-interface.md#life-05),
  [CABI-05](spec/implementation/native-interface.md#cabi-05),
  [EMBED-07](spec/implementation/c-library.md#embed-07).
  Changed LIFE-05's blanket other-context prohibition to prohibit implicit
  disposal of independent peers while requiring owning native destructors to
  dispose exclusively owned foreign children exactly once. Preserved acyclic
  ownership, no Ama body execution/reentry, yielded/failed child disposal and
  detached native callback/code lifetimes. Added the A/W/B versus independent C
  example and corresponding canonical acceptance evidence; no ABI or grammar
  syntax is changed. AMB-015 records the decision and authorization.
  Evidence: the previously conflicting lifecycle/embedding/native/async rules
  now prescribe the same outcome for the minimal example; independent source
  review and document checks pass. `python3 /tmp/ama-t204-check.py .` checked
  29 documents, 1128 links, 214 indexed rules, 11 EBNF blocks and 108 productions;
  undefined-name/alignment/group negative controls reject. `git diff --check`
  passes. Final records are checked again and Doxygen generation from an isolated
  HEAD-plus-task snapshot must pass before the task's commit. No compiler source,
  maintained tests, T182 private files or evidence fingerprints are changed.
  The audit/planning note is committed as T205's prerequisite/handoff record,
  explicitly retaining T204 as incomplete; it is not an accepted new work plan.

<a id="t204"></a>

- [x] **T204 — Audit specification consistency and rebuild the implementation plan.**
  Completed 2026-11-19; review input 155c9da, plan input 40c27b5 after T205.
  The user requested a whole-specification review and, if no contradictions
  remain, a revised plan and commit. The original request authorized planning
  records, not semantic edits; the later explicit AMB-015 answer separately
  authorized T205's narrow clarification and continuation.
  Record: [specification audit and planning](docs/implementation/static-plan-review.md).
  Steps completed: reviewed all canonical chapters and cross-chapter boundaries;
  stopped for the real LIFE-05/EMBED-07 conflict, recorded the explicit answer
  and resumed after T205; rebuilt the prerequisite/acceptance plan; reconciled
  historical evidence and unfinished work; checked links, IDs, dependencies and
  coverage. The bounded review found no other blocking contradiction, not a
  formal proof of specification completeness or an implementation acceptance.
  Added T206–T213 and reordered the 26 remaining implementation tasks by their
  80 explicit prerequisite edges. T203 audits actual source/test evidence first;
  source/scopes/lookup precede metadata/equations/guards, early T183 candidate
  identities precede graphs and the ordinary bridge, and executable admission
  remains in T185 after ordinary closure. All 37 canonical template acceptance
  areas have current unfinished owners. Consumers, useful examples, evidence,
  release and generated reference follow their prerequisites. Every task keeps
  component/composition/failure tests, purpose/contract documentation, per-file
  evidence and a separate commit; T214 is the next unused ID.
  Preserved completed historical records and all unfinished T182 files, partial
  test results, failed-run evidence and local continuation note. T204 changes
  neither spec nor compiler/test sources or evidence fingerprints. T203 remains
  future work and must establish the actual reuse/adapt/remove map before coding.
  Evidence: independent chapter/plan reviews; exact CommonMark link/anchor and
  rule-index checks; all 11 EBNF blocks/108 productions checked for defined
  references, balanced groups and alignment; unique task IDs, resolved acyclic
  prerequisites, topological presentation and exact 37-area ownership checks.
  Negative controls reject undefined productions, shifted alignment, unbalanced
  groups, cycles, unknown dependencies, missing areas and obsolete owners.
  Final archived records pass: 29 documents, 1200 local links, 214 indexed rules,
  26 active tasks, 80 dependency edges and 37/37 current acceptance owners.
  Whitespace and unchanged-spec checks pass. Doxygen generation from an isolated
  HEAD-plus-four-record snapshot is a final commit gate; its outcome and tool
  versions are recorded with that commit. No compiler execution or implementation
  support is claimed.

<a id="t203"></a>

- [x] **T203 — Audit existing components against the revised static model.**
  Completed 2026-11-19 on 99ffbed plus the fingerprinted unfinished T182 inputs.
  Actual source/test audit only; no new-model implementation acceptance.
  Record: [actual implementation alignment](docs/implementation/audit/static-module-alignment.md).
  Prerequisites: T202, T204, T205.
  Rules: [ARCH-01](spec/implementation/compiler.md#arch-01),
  [ARCH-06](spec/implementation/compiler.md#arch-06),
  [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: actual maintained source/test/component contracts, their consumers,
  and the exact uncommitted T182 input revision and file fingerprints.
  Steps: inventory components and existing evidence; classify each affected
  responsibility as reusable, requiring adaptation, obsolete or deferred;
  assign its source/tests/documentation to a task below and record any necessary
  finer decomposition before implementation. Inspect the recorded O2 warning
  and actual lifecycle/embedding acceptance affected by T205.
  Accept: a per-component/file map names real consumers and executable checks;
  source, AST, scopes, metadata, constraints, guards, registry, graph results,
  constants and product boundaries all have explicit owners. Historical
  T169–T181/T201 evidence remains dated to its accepted contract, never promoted
  to acceptance of removed mixed interfaces, exact signatures or static positions.
  Preserve T182's partial files, run/failure evidence and continuation note.
  Reuse them only through an identified cohesive task with tests and comments;
  do not silently depend on uncommitted APIs, delete unfinished work or commit it
  as a side effect of this audit. No semantic change or implementation is part
  of the audit itself. Follow the common evidence/documentation/commit criteria.
  The user's continuation instruction requires deleting unused/obsolete code
  and its sole-purpose tests, not retaining alternative implementations. Record
  exact removal/replacement ownership here before source edits; each responsible
  implementation task must remove superseded paths and test the current rules.
  Evidence: 1730 unique paths in 25 explicit boundaries, all 73 original dirty
  hashes preserved, seven valid focused leaf invocations and root build exit 0.
  The invalid direct nested/consumer command exits 2 and is recorded separately;
  its actual host runs through the successful nested parent. Five source probes
  preserve independent target expectations and identify unresolved frontend/parser
  gaps. Reproduced the existing O2 warning without hiding or fixing it in an audit.
  Added T214 for the precisely traced unused preview API before T206; each
  obsolete branch/test family has an explicit replacement owner. No code, test,
  spec or evidence-registry changes are included. Final navigation, task graph,
  whitespace and isolated Doxygen checks must pass before the task commit.

<a id="t214"></a>

- [x] **T214 — Remove the unused ordinary interface-preview application API.**
  Completed 2026-11-19 on 3289336 plus the explicitly adopted result bundle;
  original pre-existing inputs and removal ownership are recorded by T203.
  Record: [live operand preparation](docs/implementation/static-operands.md).
  Prerequisites: T203.
  Rules: [TPL-02](spec/language/templates.md#tpl-02),
  [TPL-12](spec/language/static-results.md#tpl-12),
  [ARCH-03](spec/implementation/compiler.md#arch-03).
  Boundary: `src/semantic/static/results/pattern/apply.c`, its public declaration
  and unused cached-application field, and the sole-purpose result-pattern leaf.
  T203 traced every occurrence: only that leaf calls `static_result_pattern_apply`;
  actual expression evaluation consumes operand preparation without this API.
  Steps: remove the unused API/state and obsolete leaf; retain source/environment
  operand preparation through its real expression consumer; reconcile purpose
  comments, recursive test routing and per-file evidence without importing T182
  implementation as an unrelated side effect.
  Accept: no declaration/call/definition or dormant compatibility wrapper remains.
  Meaningful preparation identity, failure/cleanup and actual expression checks
  remain covered through independently runnable live-consumer tests. Run affected
  results, publication and constant compositions, relevant sanitizers and root
  build; preserve the pre-existing warning as T182-owned until its real fix.
  The user's 2026-11-19 unused-code instruction authorizes removing these exact
  unfinished T182 files; retain their fingerprints and historical failed runs,
  not obsolete executable code. Follow common documentation and commit criteria.
  Evidence: dormant API/state and four original untracked code/test files removed;
  nine cohesive live result source/header inputs and two prepare-test inputs
  adopted without other T182 changes. Private collection/cache contracts preserve
  exact lexical identity, incoming validation, failure publication and teardown.
  The initial eight-case live prepare leaf passes. On an isolated HEAD-plus-task
  snapshot, all 35 results, 29 publication and 35 constant cases pass both strict
  Werror and ASan/UBSan configurations; root make passes without warnings.
  Executable C inputs remain those exercised by the six parent runs; a later
  header-purpose comment repair produces identical compiler-preprocessed bytes.
  New/private headers and every changed source/support file have named consumers.
  Final links/task routing, selected evidence merge, Doxygen and staged-byte
  checks are commit gates. No spec change or template-language acceptance is
  claimed; T206 is next. T182's separate local O2 warning remains recorded.

<a id="t206"></a>

- [x] **T206 — Parse separate static modules and simple ordinary Type forms.**
  Status: source contract accepted on 2026-11-19, based on c28d8a0. Record:
  [current static source parsing](docs/implementation/static-source.md).
  Prerequisites: T203, T214.
  Rules: [SYN-03](spec/language/source-and-names.md#syn-03),
  [SYN-04](spec/language/source-and-names.md#syn-04),
  [COMP-01](spec/implementation/compiler.md#comp-01),
  [TPL-12](spec/language/static-results.md#tpl-12);
  [canonical EBNF](spec/grammar.md).
  Boundary: shared lexical/layout components, module-kind activation, immutable
  AST nodes and source/path spans; no symbol-table-dependent recognition.
  Steps: recognize `@...` and exclusive module kinds; parse result pairs, named
  producer uses and finite nested constructor arguments; parse one named export
  selector in ordinary declarations/constant reads and expose actual path spans.
  Accept: empty graphs, comments, final virtual newline, Unicode/multiword/hyphen
  names and arbitrary finite nesting obey exact spacing/indentation. Multiple
  outputs require named selections. Reject mixed annotations, incomplete pairs,
  empty/positional static bindings, inputs after outputs, trailing captures and
  ordinary inline static applications/projections. Runtime positional forms are
  unchanged. Keep lexical/layout/parser responsibilities separately testable and
  compose them through the real AST. Cover malformed input and every allocation
  failure; remove superseded syntax branches with their replacements. Meet the
  common per-file tests, Doxygen links and same-task commit criteria.
  Evidence: exclusive static graphs and simple ordinary selectors parse through
  the shared lexer/layout/AST; obsolete formal/signature/position/skip machinery
  and its sole-purpose tests are removed. Exact metadata comparison has one live
  scratch API and actual constraint-cache consumers. Current-source graph,
  ordinary, dependency, native, metadata, named-request and allocation cases pass
  in an isolated HEAD-plus-task tree: eleven initial command groups under strict
  Werror and ASan/UBSan, followed by nine dependent groups under both after final
  metadata cleanup. Root build passes without warnings. Original literal/source
  bytes, arbitrarily finite nesting, explicit work frames and failure ownership
  are preserved. Complete Doxygen passed before final cleanup; the same-input
  final archive refresh, explicit staging and staged-byte comparison are commit
  gates. No spec file changed; no complete static semantic/execution claim.
  Higher live algebraic fixtures retain explicit T207–T212/T182 migration owners.

<a id="t207"></a>

- [x] **T207 — Collect ordinary Type formals and whole-graph static scopes.**
  Status: accepted on 2026-11-19, based on 46524d5. Record:
  [scope contract and current steps](docs/implementation/static-symbols.md#t207-whole-graph-scope).
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
  Evidence: pair headers precede first-receipt collection; every binder is public
  and whole-module, and graph inputs are the exact remaining local head set.
  Alias/public counters and deterministic record indices match actual output
  snapshots. Ordinary input/result Type introductions, shared/body/namespace
  reuse and native own-formal refusal preserve their independent contracts.
  The obsolete lookup position and classifier-created aliases are removed.
  Exact provenance, all six independent statement orders, repeated/forward
  receipts, cross-module names, duplicates, overflow and every measured
  allocation refusal are tested. Ten command groups pass strict Werror and
  ASan/UBSan on a task-only tree; root build passes without warnings. Initial
  Doxygen passes 2700 ready files, zero debts, 2042 file pages, 7917 interfaces
  and 432/432 destinations. Final archive generation and exact staged-byte
  comparison remain commit gates. No spec change or executable-template claim;
  actual metadata, source kinds and producer closure retain later task owners.

<a id="t213"></a>

- [x] **T213 — Resolve static graph dependencies and source module kinds.**
  Prerequisites: T206.
  Rules: [SRC-01](spec/language/source-and-names.md#src-01),
  [SRC-02](spec/language/source-and-names.md#src-02),
  [DEP-01](spec/implementation/compiler.md#dep-01),
  [DEP-02](spec/implementation/compiler.md#dep-02),
  [DEP-03](spec/implementation/compiler.md#dep-03).
  Evidence: occurrence roles survive discovery and are checked after the complete
  closure has parsed. Thirty-two role/kind combinations, cycles, requester spans,
  every measured allocation refusal and actual stale-input propagation pass.
  Project/native/scope/explicit/classify groups pass strict and ASan/UBSan; the
  isolated compiler build passes. Final documentation and staging remain commit gates. See
  [the dependency contract](docs/implementation/project-dependencies.md#t213-static-path-roles).
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

<a id="t208"></a>

- [x] **T208 — Model actual static metadata and minimum usage requirements.**
  Prerequisites: T207, T213.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08).
  Evidence: all-use membership preserves complete actual residuals/captures;
  bare forwarding classes propagate requirements without expansion or allocation.
  Producers are separate from ordinary nominal values. The unused expectation
  injection API, three source files and sole-purpose leaf are removed. Nine
  current-source groups pass strict and ASan/UBSan; root build/native admission
  and 2704-file complete documentation checks pass. Final rendering and exact
  staged-byte comparison remain commit gates. See [the metadata contract](docs/implementation/static-metadata.md).
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

- [x] **T209 — Apply named constructors and solve nonexpanding equations.**
  Prerequisites: T208.
  Rules: [TPL-04](spec/language/templates.md#tpl-04),
  [TPL-07](spec/language/templates.md#tpl-07),
  [TPL-08](spec/language/templates.md#tpl-08),
  [TPL-10](spec/language/templates.md#tpl-10).
  Evidence: named-only static maps, stable field inference and exact B-minus-P
  recovery preserve actual captures/residuals and all minimum uses. Retired static
  positional/skip and producer/input omission branches and sole-purpose tests
  are removed; ordinary runtime positions remain covered. Five parent/groups
  (24 leaves) pass strict and ASan/UBSan; actual metadata refinement is rerun in
  both configurations. Root build/native admission and complete documentation
  checks pass (2708 inputs, zero debts). Final rendering/staging remain commit gates. See [the equation contract](docs/implementation/static-equations.md).
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

<a id="t215"></a>

- [x] **T215 — Give static producer owners exact compiler-only identities.**
  Prerequisites: T207, T209, T213.
  Rules: [TPL-11](spec/language/templates.md#tpl-11),
  [TPL-12](spec/language/static-results.md#tpl-12).
  Boundary: full producer input maps and shared guard owner normalization;
  no runtime Type/Template representation for static modules.
  Steps: intern raw scopes, normalize every input through the solver, generalize
  creation/definition keys and expose a source-backed producer entry guard.
  Accept: complete map identity, permutations, unused inputs, lexical membership,
  unknown repeated-key suspension before new scope preparation, same/unequal
  closed-key recurrence, independent definition-cycle rejection, exhaustive
  concrete creation-graph oracle and allocation refusal cleanup. Strict and
  sanitizer checks, per-file documentation coverage and generated links pass.
  Evidence (2026-11-19): eight producer cases and ordinary owner checks pass
  strict C11 and ASan/UBSan. Both ordinary and producer-key graph oracles exhaust
  65536 graphs in two insertion orders each (262144 graph/order/category cases
  per configuration). Immutable-term parent and final compiler build pass.
  Complete documentation check reports 2713 ready inputs, zero debts. Rendered
  link validation passes after adding the producer rule to the file-level purpose
  comment; final rendering/staged-byte review remain commit gates.
  Contract: [producer owner foundation](docs/implementation/static-guards.md#t215-producer-owner-foundation).
  This prerequisite separates the representation correction from T210's remaining
  source-fixture migration and full expansion/cache acceptance.

<a id="t210"></a>

- [x] **T210 — Guard definition expansion and every specialization path.**
  Prerequisites: T207, T209, T215.
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
  Evidence (2026-11-19): all nine guard leaves and 43 named cases pass strict
  C11 and ASan/UBSan. Each configuration checks 262144 creation graph/order/kind
  combinations and 1024 definition graph/order combinations against independent
  oracles. Actual current source roles cover finite nesting, unknown/equal/
  unequal recursion, distinct source uses, warm descendants, late closure,
  first/equality receipts, all public allocator cutpoints and both graph outcomes.
  Empty producer maps now settle queued constraints before key normalization;
  the failed idempotent-read migration check exposed this defect, and direct/
  cached contradiction regressions pass. Producer map copies read only formal/
  value fields. The five affected leaves pass both final reruns; compiler build
  passes. Complete per-file/interface check: 2713 ready inputs, zero debts.
  Rendered-link and exact staged-byte review remain final commit gates; final
  counts/outcomes are recorded in the commit. Current contract and full commands:
  [guard model](docs/implementation/static-guards.md). Obsolete mixed-interface
  fixtures/sole-purpose captured-Template case and stale case registrations are
  removed. Historical T178/T201 input/run records are preserved in DONE.
  T183 owns canonical registry/body/cache composition; T211/T212 own actual
  output equations and ordinary export bridges. No full-language admission or
  full static-parent acceptance is inferred from these guard tests.

<a id="t216"></a>

- [x] **T216 — Encode complete specialization keys and collision-checked names.**
  Prerequisites: T209, T210.
  Rules: [TPL-05](spec/language/templates.md#tpl-05),
  [NAME-01](spec/implementation/compiler.md#name-01),
  [NAME-02](spec/implementation/compiler.md#name-02).
  Boundary: normalized applied owner maps, exact UTF-8/postorder descriptors and
  the existing SHA-256 identity service; no candidate/body/runtime admission.
  Steps: validate closed maps; discover shared nominal values in original formal
  order; measure/write a lossless versioned descriptor; compose existing complete
  identity framing and collision checks. Configure test roots from actual leaf
  authority so test decomposition has no fixed-directory-depth assumption.
  Accept: exact ASCII/Unicode byte/hash vectors, original zero-input names,
  Type/Template/producer separation, source-backed staged/alias equivalence,
  root and full residual/capture distinctions, shared DAG depth without C
  recursion, checked arithmetic, malformed/open handles, every allocation
  refusal and forced unequal-key digest collision. Strict/sanitizer, real
  compiler/header/documentation/link checks and same-task commit are required.
  Contract: [key representation](docs/implementation/static-registry.md).
  Evidence (2026-11-19): 16 key cases pass strict C11 and ASan/UBSan.
  The actual nested parent target, old guard-owner smoke leaf and root compiler
  build pass. Vectors independently verify descriptor bytes and SHA-256 names,
  including Unicode; laws cover real staged applications, solved aliases, root/
  residual/capture distinctions, construction-order independence and shared
  metadata DAG depths through 1024. All measured codec/full-name allocations,
  invalid/open handles, exact size/counter boundaries and forced digest collision
  are checked with counted teardown and stable receivers. Initial missing link
  dependencies and missing standalone-header registrations were corrected with
  actual checks, not changed expectations. Complete documentation gate: 2729
  ready inputs and zero debts. Final rendered-link/staged-byte gates and counts
  are recorded in the task commit. Full contract/commands/logs:
  [key encoding and registry](docs/implementation/static-registry.md).
  Candidate/use states, dependency closure and executable admission remain T183
  and its consumers; naming creates no runtime instance or module source file.

<a id="t183"></a>

- [x] **T183 — Register exact specialization identities and guarded cache reuse.**
  Prerequisites: T208, T209, T210, T213, T216.
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
  Historical implementation checkpoint (2026-11-19, superseded by acceptance below): implement candidate/use association
  before body/cache closure. Published guard paths retain separate use handles;
  unknown keys retain no candidate/name, while closed normalized maps intern one
  identity. Registration remains NEW rather than body/executable checked.
  Core ownership, first-error propagation and key promotion APIs are implemented;
  four source/term/guard/naming compositions pass strict C11. The initial repeat
  query assertion exposed redundant normalization of an already closed use;
  after a successful guard refresh, its immutable key association is now reused.
  Rerun: `/tmp/ama-t183-candidates-key-reuse.log`, collected exit 0; initial
  failed run retained at `/tmp/ama-t183-candidates-initial.log`. Current isolated
  snapshot: `/tmp/ama-t183-registry-fbj6fq5_`; selected-input list:
  `/tmp/ama-t183-registry-task-paths.json`. No active jobs remain.
  Canonical scope association and finite group checks are implemented. The
  registry automatically retains actual parent chains and every guard-demanded
  path, keeping validation dependencies separate from constructor-creation edges.
  Four affected leaves pass strict C11 after that change (including an independent
  8192-composition reachability oracle). Earlier complete registry strict and
  ASan/UBSan runs also passed; recorded intermediate evidence is in the contract.
  Intermediate strict, ASan/UBSan and root-build runs passed, including every
  automatic-ancestry allocation refusal and the one-time analysis claim. Final
  maintained code is frozen in `/tmp/ama-t183-registry-16pf7nj_` and its `-san`
  sibling. Entire 33-case registry acceptance: strict session 1992,
  `/tmp/ama-t183-accept-strict.{log,exit}`; ASan/UBSan session 91462,
  `/tmp/ama-t183-accept-san.{log,exit}`. Documentation/build-only snapshot
  `/tmp/ama-t183-registry-x5yh4acy` has task-only coverage (2755 rows, excludes
  unrelated T182 work): docs-complete session 91637,
  `/tmp/ama-t183-doc-complete.{log,exit}`; root build session 18573,
  `/tmp/ama-t183-accept-build.{log,exit}`. Collect all outcomes before closure.
  No specification ambiguity is open. Final generated documentation/link gates,
  evidence review, archival and commit remain unfinished.
  [Registry contract](docs/implementation/static-registry.md) records the boundary.

  Acceptance (2026-11-19): all jobs above were collected; no job remains active.
  All 34 named registry cases pass strict C11 and ASan/UBSan, including the
  independent 8192-composition graph oracle in both configurations. Complete
  parent exits `/tmp/ama-t183-accept-{strict,san}.exit` are 0; the final additional
  counter-boundary case and full candidate leaf exits
  `/tmp/ama-t183-counters-{strict,san}.exit` are 0. Root build exit
  `/tmp/ama-t183-accept-build.exit` is 0. Complete per-file coverage gate passes
  with 2755 ready rows and zero debt. The contract records exact flags, ownership,
  mathematical invariant/termination bounds, failure fixes, source-backed cached
  graph compositions and the unavailable-LSan boundary.
  Final documentation inputs include this archive and the next-task queue;
  `make docs-complete` and `make docs` must pass on this prospective final tree
  before this record is copied into the working tree or committed. Their collected
  logs are `/tmp/ama-t183-doc-final-complete.log` and
  `/tmp/ama-t183-doc-final.log`; final rendered counts belong to the task commit.
  No source/executable acceptance is inferred for T211/T182/T184/T185, and no
  spec change or compatibility layer was introduced. Unrelated T182 work remains
  untouched. The earlier pending-checkpoint text is historical, not active work.
