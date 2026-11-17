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
