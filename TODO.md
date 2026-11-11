# Ama implementation tasks

[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md) is the source of truth.
This is an execution order, not a new language design or a claim that any task
is complete. Follow [EBNF.md](EBNF.md), [VALUE_ABI.md](VALUE_ABI.md) and
[CONTRIBUTORS.md](CONTRIBUTORS.md). Do not change the specification without an
explicit user request.

Work in the order below. Finish each independently testable contract before
building its consumers. User-facing tooling reuses compiler components; it must
not introduce a second parser, resolver or type system.

## 1. Establish component contracts and the test/build foundation

- [x] Map existing source components to the specification and identify what can
  be reused, replaced or removed. Evidence and replacement decisions are recorded
  in the [component map](docs/implementation/components.md); existing compiler
  bodies and old tests were not analyzed.
- [x] Arrange responsibilities as nested directories with short orchestration
  functions, narrow interfaces and explicit contexts. Split components as they
  are implemented; avoid a separate repository-wide mechanical refactoring phase.
  The new driver separates request recognition, output and orchestration; old
  implementations and test cases have been removed. Build and test rules have
  been reset to support this foundation; their acceptance checks remain below.
- [x] Make source discovery and test dependency tracking support the full nested
  source tree and rebuild affected consumers after header changes. Exact command
  manifests additionally track compiler settings and link-input membership;
  [incremental-build tests and invariants](docs/implementation/incremental-build.md)
  cover the project and independent test leaves.
- [x] Retain two simple recursive C/Make harnesses: component tests and integration
  tests. Each leaf has `main.c`, descriptive `it_*` functions and explicit `run()`;
  guarded parent `common.c` fixtures use `assert`.
  [Harness acceptance](docs/implementation/test-harness.md) verifies parallel
  ordering, failure artifacts, explicit cleanup and assertions under release flags.
- [x] Establish and apply the component-contract procedure: domains, invariants,
  pre/postconditions, failure effects, complexity and a general mathematical/
  algebraic argument with boundary, failure and composition tests. The
  [procedure, template and registry](docs/implementation/contracts/README.md)
  cover existing components and require evidence for every subsequent one.
  No heuristics or example-specific exceptions (ARCH-05/ARCH-06, TEST-08).

Completed: nested components and their assembled parents build/test independently;
rebuild and harness acceptance tests verify dependencies and cleanup, and the
contract registry records the justification and executable evidence. The compiler
itself remains an implementation foundation; the next implementation tasks are in section 2.

## 2. Validate the native ABI and generated-C execution constraints

- [x] Select and document the supported C toolchain and exact native signatures,
  including byte views, representation includes and alignment requirements.
- [x] Implement common `AmaStatus` code/message/length, allocate distinct service
  codes and distinguish invalid-index refusal from technical conversion failure.
- [x] Build a small C harness for all six ABI bindings: mandatory `+`/`-`, optional
  `|`/`=`/`?`/`[]`, plus a void-return native entry with service status output.
- [x] Verify mandatory native-entry inlining while permitting internal native C
  calls and stack use. Verify labels and explicit return dispatch for Ama control.
- [x] Validate fresh/constructing/live/finalized bookkeeping, failed-constructor
  responsibilities, reverse cleanup, destroy errors and no double destruction.
- [x] Specify service metadata for handles, acquisition, return links, saved
  positions, source locations and host outcomes without hidden Ama links in C.
- [x] Apply NAME-05 to native authoring and ABI probes: register category-separated
  module prefixes, emit scoped `AMA_NATIVE(name)` contexts and qualified uses,
  and compile two companions with identical local names in one translation unit.
  The [namespace contract](docs/implementation/native-namespaces.md) records the
  exact naming argument, failure behavior and executable evidence.

Completed: [ABI](docs/implementation/native-abi.md),
[status](docs/implementation/native-status.md),
[hook harness](docs/implementation/native-hooks.md),
[toolchain](docs/implementation/native-toolchain.md),
[lifecycle](docs/implementation/native-lifecycle.md) and
[metadata](docs/implementation/runtime-metadata.md) contracts have executable evidence.
Frontend integration, real pool allocation and the complete execution dispatcher
remain later tasks. ASan/UBSan passed with LeakSanitizer disabled because ptrace
supervision prevents it from running; leak-check certification is not claimed.

## 3. Implement source storage, identities and structured diagnostics

- [x] Build a source manager with stable spans, original UTF-8 bytes, line tables
  and precise file/line/column reporting, including allocation-failure paths.
- [x] Implement strict UTF-8 and pinned Unicode XID tables, case-sensitive byte
  identity and deterministic filesystem lookup without normalization guesses.
- [x] Resolve extensionless module paths, physical `name+/` submodules and native
  companions. Keep logical identities distinct from physical diagnostic paths.
- [x] Generate collision-checked hashes from complete, unambiguously encoded
  identities; retain source names and stable Unicode test vectors.
- [x] Represent diagnostics structurally: primary span, cause, expected construct,
  related declaration/type spans and an actionable explanation when exact.
- [x] Render source excerpts and original module names, never internal hashes as
  the only explanation. Reuse this representation in later command-line/editor tools.

Ready when: source and identity errors are precise, deterministic and usable
without parsing generated C diagnostics.

Section 3 is complete. Contracts and executable evidence cover
[source storage](docs/implementation/source-storage.md),
[Unicode](docs/implementation/unicode.md),
[module paths](docs/implementation/module-paths.md),
[identities](docs/implementation/identities.md),
[diagnostic records](docs/implementation/diagnostics.md) and
[rendering/source maps](docs/implementation/diagnostic-rendering.md).
Unicode 17.0.0 tables and their C generator are vendored; internal C names use
versioned, length-delimited SHA-256 identities with exact collision rejection.
The POSIX catalog rejects symlinks and inode aliases, and takes explicit output
exclusions. Source/ABI UTF-8 errors and ambiguous paths are tested through real
filesystem-to-message composition. Source mapping is checked by compiling/running
C. ASan/UBSan passed with LeakSanitizer disabled; the full release/Werror test tree
passed. Parsing, required-draft semantics, dependency closure and CLI integration
remain their subsequent tasks; this section does not claim a working compiler.

## 4. Parse the complete grammar without semantic guesses

- [x] Implement CALL-07 entry literal inputs before starting section 5: recognize
  `|` only at the input operand boundary (positional/named), preserve all payload
  bytes through physical EOL, build AST_LITERAL_SOURCE with exact spans, count
  its input position, and replace literal-prohibition tests with positive/negative
  grammar tests. Distinguish empty `< |` from omitted `<`; keep literal outputs,
  assignments, conditions, indices and resume/read inputs forbidden. Extend
  source-to-AST/diagnostic composition and allocation-failure tests.
  Ama literal classification and ABI record classification now use explicit
  entry points: ABI `|` remains a protocol symbol, not a data boundary.

- [x] Read indentation, comments and literal boundaries without altering payload
  bytes, trailing spaces or deliberate line endings.
- [x] Parse sections, constants-only modules and the specified draft boundary.
- [x] Build an AST with spans for declarations, initialization, assignment,
  references, lists, qualified constants, entry/read/resume calls and control flow.
- [x] Parse empty positional `<`/`>` placeholders with independent arity counts;
  distinguish them from named bindings and retained `>>` requests.
- [x] Parse the fixed `.abi` records and report malformed, duplicate, unknown or
  missing mandatory bindings at their exact locations.
- [x] Add syntax-error explanations and deterministic expected-token information.
  Do not use declared-name lookup to decide which construct a line represents.

Ready when: complete valid and invalid grammar cases have independently runnable
parser tests; parsing does not depend on executable code generation.

Section 4, including CALL-07 source/AST support, is implemented.
[Frontend contracts and evidence](docs/implementation/parser-errors.md)
cover byte-preserving layout, exact draft activation/sections, a source-mapped AST,
iterative nested access/control parsing, call placeholders/modes and fixed ABI
records. Grammar expectations have stable keys and precise source diagnostics;
recognition never looks up aliases or creates hidden numeric values. Eight new
leaves contain explicit scenarios, including finite access/call models, deep
nesting, truncation and allocation failures. The composed suite loads actual files,
renders handwritten diagnostic expectations and compiles/runs typed native-binding
probes. Full release/Werror tests and focused ASan/UBSan passed (LeakSanitizer
remains disabled). Semantic checks and CLI integration stay in subsequent sections;
no code generator or runnable Ama program is claimed by parser completion.

## 5. Resolve dependencies, types, capabilities and storage plans

- [x] Load the transitive project/library closure once per logical module identity;
  report missing, ambiguous and required draft modules explicitly.
  [Closure contract and evidence](docs/implementation/project-dependencies.md):
  one AST per identity, occurrence spans and requester traces, iterative cycle-safe
  discovery; exhaustive three-module graphs, real UTF-8 diagnostic composition and
  allocation-failure cleanup. Native companion/type validation remains below.
- [ ] Enforce one base-name namespace across constants, interfaces and body fields.
- [ ] Resolve declarations, sources and interfaces into typed symbols; check
  defaults, literal/clone capabilities, binding arity and storage compatibility.
  For CALL-07, derive literal type from the selected owned scalar parameter and
  reject reference/list destinations or missing `|` support; retain binding and
  parameter/type declaration spans in diagnostics.
  Apply TYPE-05's nominal storage matrix without implicit dereference or list
  conversion. Reject duplicate named callee labels while preserving binding order.
- [ ] Distinguish dependency, eager ownership-layout and constant-initialization
  cycles from dynamic busy-instance reentry. Derive legal orders from each relation.
  Follow TYPE-04's eager edges, CONST-02's declaration-level dependency graph and
  PROTO-07's greatest fixed point for clone capability; recursive empty-list
  structures and benign self-typed constants are not automatically cycles.
- [ ] Validate mandatory `.h`/`.c`/`.abi` companions only after a required module
  is known to select `@C`; unused drafts remain inactive. Treat ABI function names
  as module-local identifiers under NAME-05.
- [ ] Plan shared constants separately from instance storage, including qualified
  reads, typed library enumerations, startup ordering and reverse cleanup.
- [ ] Build explicit typed lifecycle/control-flow IR and storage plans. Preserve
  spans and type identities through every handoff; never use C text as semantic IR.
- [ ] Add type-error messages showing both module types and related declarations.

Ready when: validated projects have inspectable layouts and IR, with no unresolved
interpretations deferred to the C emitter.

## 6. Expose fast checking and generate native scaffolding

- [ ] Provide an explicit check-only command using the same source manager, parser,
  resolver and checker as compilation, without generating or compiling C.
- [ ] Report draft status distinctly; a required draft remains a build error.
- [ ] Generate native headers/signatures and optional C/ABI/test skeletons from
  validated interfaces and ABI contracts instead of duplicating declarations.
- [ ] Make output ownership explicit. Preserve existing handwritten companions;
  show conflicts and require an explicit generation/overwrite request.
- [ ] Test that checking and compilation agree on frontend validity and diagnostics,
  and that generated native declarations compile against the ABI harness.

Ready when: contributors can validate interfaces and obtain correct scaffolding
before the execution backend is complete. These tools do not extend the grammar.

## 7. Implement stable instances and ownership operations

- [ ] Build stable block pools per executable module type, typed slot/generation
  handles, empty references, free-slot reuse and generation-exhaustion retirement.
- [ ] Reclaim free trailing blocks without reviving stale handles after regrowth.
- [ ] Construct owned fields and native payloads in the specified order; implement
  partial failure cleanup, at-most-once destruction and terminal cleanup errors.
- [ ] Implement independent ownership-tree cloning with an injective clone map,
  internal-reference remapping and unchanged external targets.
- [ ] Prepare replacements before destructive commit; preserve destinations on
  preparation failure and finalize resources correctly after commit failure.
- [ ] Check source-tree availability, ownership overlap and copy capability;
  preserve ordinary self-assignment and reject continuation self-copy.
  Preflight the complete affected old ownership subtree before deletion/reset/
  replacement, including busy descendants of an available root (REF-08).

Ready when: small exhaustive state models and failure-injection tests establish
handle, lifetime, clone and replacement invariants independently of module execution.

## 8. Connect the smallest complete stackless program

- [ ] Emit one project assembly containing required includes, generated types,
  constants, native bodies and Ama labels, included by a launcher translation unit.
- [ ] Execute directly over heap instances without working-state mirrors or
  implicit default objects. Initialize constants before creating the explicit root.
- [ ] Implement CALL-07 literal construction in entry preparation: select `|` once
  for supplied owned scalar parameters without default-then-overwrite, preserve
  normal construction/commit cleanup, construct on every call, and report failures
  with call/binding/type context outside `:`. Test skipped/default inputs, literal
  failures, acquisition release and prevention of body entry after failure.
- [ ] Implement CALL-08 evaluation order for every call/read mode: acquire the
  selected target, resolve/protect input sources before reset, construct parameters
  from supplied/default values in declaration order, then construct results.
  Resolve output receivers/indices in binding text order; reject conflicting
  ownership without implicit snapshots. Resume/read skip entry preparation and
  no whole-call rollback is promised. Initial root startup constructs once.
- [ ] Lower entry calls, interface reset/defaults, retained results, result routing,
  branches, loops and return links to explicit transfers and service metadata.
- [ ] Integrate native execution/status and lifecycle code; prohibit C dispatch of
  Ama modules and persistent borrowed views in opaque native state.
- [ ] Exercise two module types with multiple instances, nested same-type calls,
  busy refusal, pool growth during calls and persistent body state.
- [ ] Implement acquisition refusal snapshots and their read-only library reader;
  preserve caller-context ownership across successful and nested calls.
- [ ] Emit source maps and execution-path diagnostics before cleanup, including
  failure before root entry and reporting without a newly allocated trace buffer.

Ready when: a real project compiles to C and runs with verified native inlining,
label transfers, stable instance addresses and correct cleanup.

## 9. Add suspension and continuation copying

- [ ] Save explicit branch/loop positions at `%`, separating available yielded
  state from a busy instance waiting for a nested call.
- [ ] Implement output-only resumption, entry restart and completed/yielded/failed
  host outcomes. Failed execution is not resumable; root resume is host-explicit.
- [ ] Clear positions throughout ordinary copies and preserve them throughout `%`
  copies, without copying callers, busy state or refusal history.
- [ ] Test repeated yields, nested owned states, busy descendants, clone failures,
  host disposal and rejection of unhandled root yield by the standard launcher.

Ready when: continuation semantics compose with ownership and failures without
capturing a native C activation or adding an implicit scheduler.

## 10. Complete lists and a coherent standard library

- [ ] Implement ordered contiguous handle arrays over stable element pools,
  checked geometric capacity growth and zero-based index conversion.
- [ ] Implement the specified four-input/three-output interface and six actions,
  default Inspect, positional skips, owning construction/clone and reference lists.
  Supplying an owning source input requires static element clone capability
  even for an empty/unused handle; omission permits default noncopyable creation.
  Initial retained results represent successful inspection; copies retain last
  completion/outcome data without acquisition/refusal history (LIST-09/LIST-11).
- [ ] Preflight destructive mutations and distinguish ordinary refusal from
  technical failure; maintain retained length/results and order-preserving removal.
- [ ] Publish scalar, condition/index, action/outcome and refusal-reader modules
  with documented paths and typed shared constants.
- [ ] Establish consistent parameter/result ordering across related library
  interfaces and test it. Keep domain operation names out of parser rules.
- [ ] Cover traversal, empty references, busy deletion, Clear preflight, pool
  reclamation and stale-reference rejection through real compiled projects.

Ready when: library interfaces compose predictably without new core operators.

## 11. Make the language easy to explore and debug

- [ ] Create small complete examples for persistent state, native I/O, object lists,
  and stepwise simulation with independent scenario copies.
- [ ] Use the examples to review diagnostics, repeated paths and auxiliary-variable
  burden. Record concrete friction before proposing any language change.
- [ ] Add editor highlighting, module-path completion, definition navigation and
  signature/type display using the existing frontend and source spans.
- [ ] Add rename support using resolved references, including qualified constants;
  verify affected files rather than performing unchecked textual substitution.
- [ ] Provide explicit debugger inspection of instance type, persistent fields,
  retained interface values, acquisition state, saved position and refusal reason.
  Reuse runtime metadata and respect native borrowing and opaque-payload boundaries.

Ready when: a contributor can understand, navigate and diagnose a complete example
without decoding internal C names or consulting unrelated implementation files.

## 12. Finish acceptance and reproducibility

- [ ] Complete the specification's acceptance matrix at component, composition
  and generated-C integration levels, with recorded formal arguments and test evidence.
- [ ] Verify deterministic output, transitive dependency invalidation, Unicode paths,
  collisions, allocation arithmetic, output failures and resource exhaustion.
- [ ] Run the full build/tests, relevant ASan/UBSan and supported-toolchain checks;
  confirm both mandatory entry inlining and permitted internal native calls.
- [ ] Remove nonconforming mechanisms and obsolete descriptions; publish actual
  conformance limitations, supported toolchain versions and reproducible commands.

## Verification throughout the work

Run focused tests after each component and composition tests at its next handoff.
Compile and run generated C for execution/ABI claims. Successful leaves clean their
outputs; failures preserve debugging artifacts. Use `make`, `make test` and
`make test-clean`, plus relevant individual suites described in the contributor guide.
Documentation-only work requires link, structure and consistency checks rather
than unrelated compiler test claims. Do not mark a task complete without evidence.
