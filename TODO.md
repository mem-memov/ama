# Ama implementation work queue

The whole [spec/](spec/README.md) directory is authoritative. The
[requirement index](spec/requirements.md) locates all 241 current requirements;
[spec/grammar.md](spec/grammar.md) defines source forms. This file orders the
active engineering work for those contracts. It neither changes the
language nor grants permission to edit the specification.

## How to use this queue

Start with the first unfinished task whose prerequisites are satisfied. Before
implementation, review its contract against the current specification, decisions,
actual consumers and prerequisite evidence under the
[entry review](docs/development/README.md#review-the-next-task-against-the-current-state).
Revise stale scope and acceptance, then plan small verifiable steps. Record actual
results in the task and its owning contract, and commit the completed task before
beginning the next one. Follow prerequisites and presentation order rather than
numeric ID order.

[Completed tasks and acceptance evidence](DONE.md) belong in DONE.md. Keep
selected unfinished work here; future proposals belong in [BACKLOG.md](BACKLOG.md).
Once all required checks pass, move the complete task record to DONE.md in the same commit as its implementation, tests, documentation
and queue update. Do not discard decisions, failed-run facts or limitations.
Allocate IDs across TODO, BACKLOG and DONE; preserve old IDs and never reuse them.

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

The user reiterated on 2026-11-19: remove obsolete implementation code together
with tests dedicated solely to it. Retained tests must exercise the current
specification and implementation; keep independently useful regression coverage
when migrating a live consumer. Removal and exhaustive behavioral/failure checks
belong to each affected task's acceptance, not a compatibility layer.

## Current tasks

[T220](DONE.md#t220) specifies markers; [T225](DONE.md#t225) specifies declaration
type expressions, retained by [T227](DONE.md#t227). The next unused task ID is **T282**.
[T230](DONE.md#t230) specifies versioned projects, revised paths and independent
project tools. T222 joins the selected sprint by the user's 2026-11-20 request.
The user's 2026-11-20 continuation request authorizes implementation of this
queue in prerequisite order. Specification amendments still need their own
explicit authorization; T222's separately authorized notation amendment is complete.
[T229](DONE.md#t229) specifies the CMake/launcher/source-inclusion contracts;
T240–T242 and T246–T257 retain the pending build/launcher implementation.
[T239](DONE.md#t239), [T244](DONE.md#t244), [T245](DONE.md#t245) and
[T235](DONE.md#t235) provide the
accepted native-input, source/test closure, handoff and delivery foundations. The
[detailed implementation plan](docs/implementation/cmake-launcher-plan.md)
records foundation gates, task/step acceptance, migration and evidence duties;
[T243](DONE.md#t243) records its completed planning acceptance.

## Planned alignment: projects, paths and project tools

The user now authorizes implementation following the completed T243 plan. Execute in
prerequisite order across both alignment sections; listing order does not
override a dependency edge. Use granular component/composition tests, documented
invariants and a commit for every accepted task. T222 is complete with explicit specification authorization; see its scoped
acceptance in DONE.
T221 and T226 are accepted in DONE, together with the
T231/T222/T232 source/configuration foundations.
The earlier conformance matrix must not be refreshed to disguise missing support.

### Source operations: finish the pending implementation

[T274](DONE.md#t274) reviews the actual T224 state as of 2026-11-23. Existing
analysis, graph/image/configuration preparation, root-preserving planning,
freshness/leases, record codecs and private-tree ownership have component and
composition evidence. Public copy/delete/move, a durable transaction journal and
complete root-changing operation plans remain unfinished. Reuse the tested
foundations; do not restart them or infer public support from them.

The [current checkpoint and task graph](docs/implementation/source-operations.md#current-plan-after-t274)
identify the concrete owners, retained evidence and next action. The old step log
in that note is historical; its earlier "next" statements are not the current
queue. T275 is the next task for entry review. Each subtask below is a separate
implementation/test/documentation/commit unit and remains unchecked. T224 keeps
final acceptance; T236 still waits for it. Follow SRC-06, PKG-04/09, PROJECT-01
and TOOL-01..04 throughout; no task authorizes a new language rule.

<a id="t275"></a>

- [ ] **T275 — Separate semantic roots, physical changes and observation bounds.**
  Status: planned; first remaining T224 subtask. Prerequisites: T232, T233, T235,
  T272 and T273. Reuse the recorded T224 root-preserving foundation. Review the
  actual catalog/image/lease consumers, then separate original/final selected
  module registries, the physical mutation/removal set, effective registry
  retirement, and the stable physical observation/coordination envelope. An
  observed parent must not acquire module identities or write authority. Feed
  these distinctions through the existing capture and plan owners rather than
  introducing another resolver. Accept real augmented-original analysis with
  initial-name conflict refusal, new/relocated roots and same-identity whole-version
  promotion whose ignored external files remain intact. Check selection versus
  actual deletion safety, unchanged-root regressions, exact freshness, allocation
  failures and real compiler/catalog consumers. Deliver a tested representation
  consumed immediately by T276; this task does not publish source changes.

<a id="t276"></a>

- [ ] **T276 — Prepare one complete immutable copy/delete/move plan.**
  Status: planned. Prerequisite: T275. Extend the existing root-preserving plan
  owner to all supported operations and original/final roots. Join complete
  source/test/companion and declared-input closure, injective identity/physical
  maps, context obligations, original joint deletion/replacement safety, exact
  selector/alias/configuration edits and actual final compiler/binding proofs.
  Carry client entry/export/root-setting changes and owned build-invalidation
  intentions now so publication cannot discover them later. Every planned write
  needs an explicit owner and authorization. Preparation executes no module,
  test, native code, CMake, network or source mutation. Accept independent
  compilable-but-incorrect final-image refusals, unused/static/native/test edges,
  constants/storage suffixes, drafts/opaque bytes, missing client roots, allocation
  failures and retained root-preserving compositions. Unknown configuration formats
  are reported without patching. The finite owned plan is the common input to
  preview and publication; no second command-specific planner is allowed.

<a id="t277"></a>

- [ ] **T277 — Expose real source-operation previews.**
  Status: planned. Prerequisite: T276. Connect public ama-project copy/delete/move
  request recognition and --preview to the actual immutable plan. Enforce exactly
  one --preview/--apply, explicit dependency policies and safe --replace; report
  exact maps, removals, configuration/root/client changes, retained material,
  safety refusals and rebuild intentions. Accept all operations through the real
  executable, invalid/repeated/conflicting flags, cross-package modes, cycles,
  namespace/area boundaries and byte-exact no-mutation evidence including
  configuration presence/directory membership. Preview has no acquisition or
  test/native/CMake execution. Until T281 accepts application, --apply must give
  a clear unsupported outcome before mutation rather than falling through to a
  preview or claiming success. Document this finite preview-only implementation
  boundary; public apply/recovery remains owned by T281.

<a id="t278"></a>

- [ ] **T278 — Apply and recover one tree through a durable journal.**
  Status: planned. Prerequisites: T276 and T277. Build the first complete
  publication/recovery slice using a real prepared removal plan, existing record
  codecs, leases and private saved-tree ownership.
  Define a versioned journal, durable decision/state invariants, explicit recovery
  entry and recorded staging location. Reserve rollback resources before the
  first destructive step. Publish one removal and recover it in a fresh process,
  using the same production executor that T279 will extend. Accept forced process
  termination without exit cleanup before/after destructive and commit boundaries,
  repeated recovery/interrupted cleanup, every relevant I/O/allocation refusal,
  unknown ownership and retained recovery evidence. Define how a new process
  discovers unfinished work even after the dead writer's kernel locks disappear.
  Independently test file/directory persistence ordering, incomplete records and
  unsupported durability;
  distinguish a storage-fault model from a physical power-loss experiment.
  Ordinary uncommitted failure restores the original; committed cleanup failure
  retains the committed decision. This is a real backend slice, not acceptance of
  the full public command or a second synthetic test-only state machine.

<a id="t279"></a>

- [ ] **T279 — Publish and recover complete multi-tree source transactions.**
  Status: planned. Prerequisite: T278. Extend that same journal/executor to every
  addition, replacement, removal and prepared configuration edit in one plan,
  including copy, move and safe --replace. Stage on each destination's filesystem
  under stable observed parents, keep originals until the whole validated final
  state is durable and commit once. Compose common writer exclusion, fresh-input
  checks and reader coordination; no instantaneous cross-filesystem rename is
  assumed. Accept same/cross-filesystem real compositions, failures at every
  rename/synchronization/rollback/cleanup boundary, process interruption and
  conflicting external files, inodes or directory members. Refused moves leave no
  copy. Recover only proved operation-owned data and retain/report uncertainty;
  overlapping writers cannot publish concurrently, while permitted disjoint work
  remains independent. After writer death releases locks, a new participating
  reader must refuse the unfinished view without repairing it, and an overlapping
  writer must report the explicit recovery entry/location before new work.
  Do not maintain a parallel publisher or deferred test-only
  rollback implementation.

<a id="t280"></a>

- [ ] **T280 — Publish root/client settings and rebuild consequences coherently.**
  Status: planned. Prerequisites: T275, T276 and T279. Integrate the already
  prepared original/final root selection, selected physical-copy changes,
  documented writable client entry/export fields and owned build-invalidation
  intentions into the same transaction and final report. Return client-supplied
  root mappings without inventing a project-wide settings file; change only
  explicitly supported selected configuration fields. Accept new/moved/removed
  roots, identity-preserving promotion, original definitions that stay physically
  present but become ignored, and crashes between source and metadata updates.
  Final compiler/catalog checks must use the intended registry, not observation
  ancestors. Verify stale generated outputs cannot be presented as the new result,
  outside importers are not redirected, and unknown formats/unrelated bytes remain
  untouched and accurately reported. Root and metadata publication must recover
  as part of the whole operation, not a later best-effort fixup.

<a id="t281"></a>

- [ ] **T281 — Expose complete public application and recovery workflows.**
  Status: planned. Prerequisites: T277, T279 and T280. Wire --apply and the
  documented explicit recovery entry to the same accepted plan and executor.
  Choose and document concrete journal/staging/recovery options as engineering
  interfaces; permit an explicit recorded recovery location. Revalidate the full
  original observation under exclusion before mutation; never reuse a stale
  preview as authority. Accept the real executable for copy/delete/move, both
  dependency policies, safe replacement, root/client changes, contention, new
  importers/companions and process-loss recovery. Distinguish complete commit,
  successful rollback, incomplete/uncertain recovery and committed cleanup debt
  in outcomes/diagnostics. Demonstrate coordinated read-only compiler/editor/
  playground access to original or committed final views, or an unfinished-state
  refusal; compilation acquires no source-mutation/recovery responsibility.
  Document nonparticipating reader limits, supported filesystem/platform
  assumptions and retained paths. Remove the temporary unsupported-apply branch
  when its real public consumer is accepted. No false success or implicit recovery
  of unproved material is permitted.

<a id="t224"></a>

- [ ] **T224 — Accept resolved source copy, safe deletion and movement.**
  Status: unfinished parent; remaining implementation is T275..T281. Prerequisites:
  T232, T233, T235, T272, T273 and accepted T275..T281. Transferred from BACKLOG
  by T230; T268 aligns the scope with AMB-017/018 and AMB-021..026 resolve the
  later operation boundaries. Preserve the full original TOOL-01..04 acceptance:
  explicit preview/apply, complete source/test closure, no namespace/area change,
  name+/ topology, injective clone maps, original joint-removal safety, exact
  aliases/selector sets and authorized additions, client/root changes, drafts,
  opaque/declared inputs and recoverable cross-filesystem publication. Audit real
  public operations and all retained consumers; no guessed mask/prefix rewrite,
  hidden acquisition or partial copy after move refusal is allowed. Remove
  superseded compiler-side mutation/importer-rewrite APIs and tests dedicated
  solely to them after tracing/migrating real consumers. Retain shared current
  resolver foundations with explicit owners. Accept the complete command/mode/
  failure matrix and affected full source workflows, sanitizer/resource failures,
  per-file coverage, Doxygen/source links and actual final compiler/site proofs.
  Reuse scoped child evidence where inputs still match; T237 retains whole-sprint
  conformance and release. Archive/commit this parent only when every promised
  public behavior and its documentation has passed. Existing detailed evidence:
  [source-operation record](docs/implementation/source-operations.md).

<a id="t236"></a>

- [ ] **T236 — Acquire and publish coherent module-and-test snapshots.**
  Prerequisites: T235 and T224. Implement Git/process transport adapters,
  operation-owned partial/full download staging, input hashes, existing-version
  checks, publication/recovery and complete promotion under PKG-06/10 and
  PROJECT-03. Keep network effects outside compilation and never execute fetched
  code during discovery. Accept controlled local-remote integration with mutable
  branches/versions, overlapping requests, source conflicts, interrupted/failed
  publication, cleanup, concurrent jobs and byte-accurate provenance. Verify a
  dependencies-only project and a promoted packages version with their tests.

<a id="t237"></a>

- [ ] **T237 — Accept the complete project/path/tooling implementation.**
  Prerequisites: T224, T233..236, T221, T222, T226 and T242. Audit every retained
  current-rule component and per-file test/documentation owner; remove obsolete alternatives
  and tests that only preserve them. Exercise the canonical project/path matrix
  and reference-notation matrix through real preparation, compile/run,
  native/static/C-library, isolated tests,
  promotion and relocation consumers. Review correctness arguments and evidence
  before renewing conformance fingerprints. Pass required focused/composite/full
  checks, failure/sanitizer gates and final Doxygen/source-link/provenance checks;
  publish precise supported boundaries and archive each task in its own commit.

## Planned alignment: CMake applications and source inclusion

T229 specifies these contracts; T243 supplies the
[detailed task/step plan](docs/implementation/cmake-launcher-plan.md).
The user's continuation request authorizes implementation. Execute in order after
each row's actual prerequisites; IDs are stable, not a numeric ordering rule.
T231/T222/T232/T221/T226/T233 are accepted foundation gates. T222 received its
separate specification authorization. T233's independent test orchestration does
not depend on the final CMake runner; T242 composes them after both exist.

All tasks include contracts, exact invariants, Doxygen rule links, per-file
component/composition evidence, resource/failure checks and a separate commit.
The linked plan expands their step order and acceptance partitions. Earlier
complete-conformance fingerprints remain historical until T237's final review;
focused accepted evidence and documentation are required at every task.

<a id="t246"></a>

- [ ] **T246 — Generate preserving native CMake scaffolds.**
  Status: planned. Prerequisite: T239. Implement BUILD-02 with the existing
  CABI-06 author-file publication boundary: comments-only template, complete
  instructions/example/canonical link and current native/editor build guidance.
  Accept repeated requests, empty/nonempty existing files, collisions, controlled
  concurrency and injected prepare/publish/cleanup failures. Verify bytes are
  preserved, no active commands are generated and scaffolding runs no CMake.

<a id="t247"></a>

- [ ] **T247 — Implement scoped CMake component-target integration.**
  Status: planned. Prerequisite: T239. Implement BUILD-01's exact once-per-component
  includes, fresh function scope, supplied target, resource base and compatible
  imported-target contract. Support executable and supplied host-target consumers.
  Accept real CMake with a controlled local dependency, two components sharing a
  module, temporary-variable isolation, declared conflict refusal and meaningful
  include/link ordering. Never add native or included generated .c a second time;
  generation remains nonexecuting and required Ama settings remain enforced.

<a id="t248"></a>

- [ ] **T248 — Supply fresh root inputs during first construction.**
  Status: planned. Prerequisite: T245. Implement RT-11/CLI-02's initial supplied
  recipes in both context/program construction consumers before any argv startup
  uses them. Select default or supplied once, preserving declaration order,
  constants/results/lifecycle and the stateful host API's separate behavior.
  Accept constructor/resource traces, later defaults reading earlier supplied
  inputs, partial acquisition failures and ordered cleanup. Prove no default
  overwrite, second initial reset or body execution after preparation failure.

<a id="t240"></a>

- [ ] **T240 — Recognize argv and admit exact input bindings.**
  Status: planned. Prerequisite: T245. Preserve original launcher input ownership;
  T248/T249/T257/T256/T250 extract construction, acquisition and execution.
  Implement CLI-01 recognition, vector shape, first-equals splitting, exact
  names/transports and complete duplicate/unsupported/stdin admission before I/O.
  Accept zero/null and conventional argv0/sentinel, Unicode/multiword names,
  explicit empty, reordered tokens and all malformed/duplicate/unknown forms.
  Verify original tokens survive, no body/stream effect precedes admission and
  runtime values never infer Type specializations. Test valid readable shapes;
  fabricated dangling C pointers are not recoverable-input test fixtures.

<a id="t249"></a>

- [ ] **T249 — Construct text inputs through exact native capabilities.**
  Status: planned. Prerequisites: T240 and T248. Implement CLI-02 UTF-8 validation,
  exact byte/length literal construction and supplied/default integration.
  Accept explicit empty versus omission, literal/comment symbols as host data,
  malformed encoding and constructor/resource failure. Reject unsupported
  reference/list/compound text bindings, preserve defaulted complex fields and
  prove token ordering cannot change declaration-order construction.

<a id="t257"></a>

- [ ] **T257 — Acquire byte-exact files and borrowed standard input.**
  Status: planned. Prerequisites: T240 and T249. Implement CLI-03 stream/path
  selection, bounded owned views and cleanup independently of decoding.
  Accept empty/oversized data, CR/LF/TAB/BOM/NUL text, repeated ordinary-file
  reads, caller working directory, stdin versus ./- and declaration-order reads.
  Inject open/read/allocation/size and applicable close/mode-restoration failures;
  borrowed stdin stays open, host modes are restored and an earlier failure
  prevents later reads. Multiple stdin bindings fail before consuming bytes.

<a id="t256"></a>

- [ ] **T256 — Connect explicit checked binary input decoders.**
  Status: planned. Prerequisites: T245, T248 and T257. Implement CLI-03 through
  EMBED-04's exact nominal Type/schema/declaration admission and bounded borrowed
  decoder view. Invoke decoding at that input's initial construction in declaration
  order; publish only independently owned state, without eager pre-decoding.
  Accept embedded zeros, size/range/schema/decoder/partial-resource failures,
  same-layout wrong-Type refusal and borrow-expiration checks. First parse and
  validate the documented versioned binary-adapter metadata, including duplicate/
  unknown keys, exact input/Type references, format bounds and decoder declarations;
  reject unconfigured/unsupported descriptors before accepting generation.
  This keeps descriptor and actual adapter admission under one owner. No payload casts,
  persisted input addresses, helper-name discovery or new native ABI protocol.

<a id="t251"></a>

- [ ] **T251 — Encode exact component-qualified public entry names.**
  Status: planned. Prerequisite: T245. Implement EMBED-09/NAME-01's public
  exception with checked-length UTF-8 hex ownership separate from internal hashes.
  Accept independent ASCII/Unicode vectors, root/version/src/tests distinctions,
  aliases of one canonical path and different component identities. Prove
  injectivity and check target identifier/arithmetic/resource limits without
  truncation. Preserve private identity/collision and native namespace guarantees.

<a id="t250"></a>

- [ ] **T250 — Implement the shared fresh one-shot execution boundary.**
  Status: planned. Prerequisites: T249, T256 and T251. Compose admitted bindings,
  acquisition, first construction, execution and cleanup under CLI-04/EMBED-09.
  After complete admission, acquire/decode/construct each input in declaration
  order; a constructor failure must prevent later stream reads as well.
  Accept actual generated entry consumers for completion, fresh repeated runs,
  ordinary failure results, technical/preparation/cleanup failure and root yield.
  Verify no process exit, hidden instance, retained borrow, automatic resume or
  second reset; host execution continues after failure. Preserve explicit stateful
  entry/resume and use only conforming native fixtures for parallel context tests.

<a id="t241"></a>

- [ ] **T241 — Deliver source inclusion and the matching executable wrapper.**
  Status: planned. Prerequisites: T250 and T247. Preserve this task's delivery
  ownership; T251 extracts the public-name codec. Implement EMBED-09 complete
  implementation/header/manifest, no-main mode and forwarding main(argc, argv).
  Accept header-first compilation, own-main host, header-only second TU, two
  qualified components in separate TUs, coexisting public headers and native
  basename overlap. Compare actual executable/inclusion behavior and fresh state;
  adapt host targets without duplicate .c inputs. Remove superseded fixed launcher
  code/tests while retaining stateful C API and mandatory ABI/inlining checks.

<a id="t252"></a>

- [ ] **T252 — Plan explicit targets and complete declared build identities.**
  Status: planned. Prerequisites: T245, T239 and T247. Implement BUILD-04 profile,
  target/toolchain/configuration, declared resource/provider identity and exact
  dependency-tracking reuse decisions. Preserve meaningful link order/repetitions.
  Accept presence/content/alias/selected-copy/profile/dependency invalidation,
  native-default recording, host/cross distinction and explicit execution mapping.
  Runtime values and guidance-only README changes do not alter compilation;
  timestamps, target names or unrecordable effects cannot justify artifact reuse.

<a id="t253"></a>

- [ ] **T253 — Configure and build selected targets through CMake.**
  Status: planned. Prerequisites: T252 and T241. Implement BUILD-03's out-of-source
  project and exact configure/build subprocess adapters with stage/tool/command
  outcomes. Use argument vectors, not shell interpolation; select actual target
  and multi-configuration settings. Accept real local dependency include/link,
  spaced/Unicode paths and missing-tool/configure/compile/link failures. Preserve
  raw diagnostics and prove generation-only runs no CMake/C compiler operation.

<a id="t254"></a>

- [ ] **T254 — Verify and publish coherent owned build artifacts.**
  Status: planned. Prerequisite: T253. Implement BUILD-03/04 actual artifact and
  configuration checks, selected entry rather than unconditional main discovery,
  staged publication and cleanup. Accept real positive/negative native inlining
  and ABI controls, each meaningful failure transition and deterministic parallel
  output ownership. Published bytes and metadata must match; a previous good
  binary cannot become the new result after failure. Preserve primary diagnostics
  and retained recovery paths without promising arbitrary CMake-effect rollback.

<a id="t255"></a>

- [ ] **T255 — Compose build/run commands and precise diagnostics.**
  Status: planned. Prerequisites: T254 and T250. Implement BUILD-03/05 command
  orchestration through validate/emit/configure/build/verify/publish/run in order.
  Build does not execute; run forwards only application tokens and reports the
  actual child start/exit/signal with selected working directory. Accept options,
  defaulted input runs and every stage failure with no subsequent execution or
  stale artifact fallback. Report exact known module/span, raw tool diagnostics
  and effective README/directory; ambiguous ownership reports component context,
  never a guessed culprit, OS package or install command. No hidden acquisition.

<a id="t242"></a>

- [ ] **T242 — Accept complete CMake application and source-inclusion workflows.**
  Status: planned. Prerequisites: T246, T255, T241 and T236. Preserve this task's
  final functional integration ownership; T252..255 extract build stages.
  Accept real build/run/generation/inclusion for packages-only, dependencies-only,
  local whole-version precedence, native and concrete generic/static consumers.
  Connect delivered/promoted/relocated source/test snapshots and T233's isolated
  test orchestration to the accepted backend. Compose CLI transport/lifecycle,
  cache invalidation, stage/resource failures, controlled concurrency and actual
  ABI/inlining/header/host checks. Reconcile every file/consumer, remove obsolete
  alternatives and publish runnable examples plus actual supported versions.
  Complete scoped evidence/documentation; T237 owns whole-sprint conformance,
  clean release and final generated-reference delivery.

## Next sprint discussion

Static marker notation is specified by [TPL-14](spec/language/static-results.md#tpl-14);
its implementation is T221. [TPL-15](spec/language/templates.md#tpl-15) permits
ordinary declaration type constructors that yield one fully determined concrete
Type; T226 owns implementation. [T227](DONE.md#t227) retains that rule while
removing the phantom feature and stating [CORE-06](spec/introduction.md#core-06):
static modules generate concrete module combinations; module contracts and tests
establish behavioral evidence. Existing compiler safety checks remain mandatory.
Localization remains a separate backlog question; versioned paths and project
tools are specified by T230 and planned below. These
specification changes do not authorize compiler implementation.

## Sprint: static module templates

The sprint is complete. [T198](DONE.md#t198) closes its generated documentation;
[T197](DONE.md#t197) records the complete clean release and public consumers;
[T196](DONE.md#t196) records current requirement/acceptance evidence.
The [original plan](DONE.md#t167), [revised plan](DONE.md#t204),
[baseline audit](docs/implementation/audit/template-sprint.md) and individual
component notes retain dependencies, decisions, rejected probes and actual checks.

Delivered behavior includes ordinary compile-time specialization, exact typed
inference, partial constructors, named `@...` graphs, independently concrete
export selectors, specialization-aware constants/plans/execution/diagnostics,
concrete native and C-library consumers, checked source/editor/relocation tools,
and [generic algorithms over intrinsic lists](examples/template-lists/README.md).
No runtime Template objects or runtime Type generation are introduced.

[The later project/path alignment](#t224), the
[AMB-006](AMBIGUITIES.md#amb-006) foundational list rewrite,
new raw-storage APIs, generic native ABI, binary-only open-template distribution,
and static natural-number parameters remain outside this completed sprint.
They are scope boundaries, not silently completed implementation tasks.

## Acceptance routing

Use [the canonical requirement index](spec/requirements.md) and
[acceptance matrix](spec/implementation/tools-and-conformance.md#acceptance-matrix)
for obligations, [current conformance evidence](docs/implementation/conformance/README.md)
for reviewed sources/arguments/component and real-consumer witnesses, and
[the release guide](docs/implementation/conformance/release.md) for reproducible
commands, supported configurations and limits. This queue defines no alternative
language rules or implementation-coverage claims.

The generated file/interface reference is rebuilt from maintained Doxygen
comments with `make docs`; `make docs-reproducible` checks identical repeated
output. Preserve the source/staged-input manifest agreement after any later edit.
