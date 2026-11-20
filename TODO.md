# Ama implementation work queue

The whole [spec/](spec/README.md) directory is authoritative. The
[requirement index](spec/requirements.md) locates all 231 current requirements;
[spec/grammar.md](spec/grammar.md) defines source forms. This file orders the
active engineering work for those contracts. It neither changes the
language nor grants permission to edit the specification.

## How to use this queue

Start with the first unfinished task whose prerequisites are satisfied. Plan
small verifiable steps, record the actual results in the task and its owning
contract, and commit the completed task before beginning the next one. Follow
prerequisites and presentation order rather than numeric ID order.

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
type expressions, retained by [T227](DONE.md#t227). The next unused task ID is **T239**.
[T230](DONE.md#t230) specifies versioned projects, revised paths and independent
project tools. T222 joins the selected sprint by the user's 2026-11-20 request.
T221/T226 and T231–T237/T224/T222 implementation has not been requested.

<a id="t221"></a>

- [ ] **T221 — Implement and accept the specified static type markers.**
  Status: planned; awaits a separate implementation request, outside T220's
  specification-only authorization. Prerequisites: T220, T231 and T232.
  Rule: [TPL-14](spec/language/static-results.md#tpl-14) and its
  [acceptance scenario](spec/implementation/tools-and-conformance.md#static-module-templates).
  Steps: classify empty static files, preserve nominal marker Type metadata
  through graph equations/aliases and source tools, and reject runtime storage
  and producer/constructor misuse at their actual admission boundaries.
  Test direct/aliased equality, distinct identities, layout boundaries, graph
  forwarding, all four runtime storage forms and entry/native/C export refusals,
  including substituted/selected marker Types; retain ordinary producer behavior.
  Accept: component/composition/failure tests and actual consumers pass; review
  changed requirement/acceptance arguments and fingerprints with real evidence,
  then pass current conformance and documentation gates. The T196/T197 matrix
  remains an earlier accepted baseline until this alignment; do not refresh
  fingerprints merely to make its newer-specification refusal disappear.

<a id="t226"></a>

- [ ] **T226 — Implement complete declaration type expressions.**
  Status: planned; awaits a separate implementation request. Prerequisites:
  T221, T225 and T227. Contract: [TPL-15](spec/language/templates.md#tpl-15) and its
  [acceptance scenarios](spec/implementation/tools-and-conformance.md#static-module-templates).
  T227 withdraws the former phantom implementation scope; ordinary declaration
  expressions remain required, as explicitly reaffirmed by the user on 2026-11-20.
  Steps: inventory actual expression/interface consumers; implement structured
  parsing, scoped introductions and exact normalized inference; require one
  concrete ordinary runtime-capable Type per used concrete specialization;
  preserve full specialization identity and ordinary storage/interface rules;
  align source, editor and relocation tools with the same parsed structure.
  Accept: meaningful granular/component/composition and failure tests cover
  nested named applications, partial intermediate constructors, selected exports,
  bare placeholders, aliases, exact inference and all required producer inputs.
  Reject residual/ambiguous final Types, unknown body placeholders, direct marker
  storage and native own parameters; preserve ordinary lifecycle and C transport.
  Renew conformance evidence only after implementation/argument review, then
  pass the required consumer, conformance and documentation gates. The earlier
  accepted matrix remains historical evidence, not support for these new rules.

## Planned alignment: projects, paths and project tools

The user authorized T230 specification work, not implementation of this queue.
All tasks below remain planned until an implementation request. Execute in the
listed prerequisite order, with granular component/composition tests, documented
invariants and a commit for every accepted task. T222 is selected for this sprint;
align its specification only after explicit authorization for that amendment.
T221/T226 remain required; perform them after T231/T222/T232 establish the
source/configuration foundations.
The earlier conformance matrix must not be refreshed to disguise missing support.

<a id="t231"></a>

- [ ] **T231 — Implement versioned source identity and revised lexical forms.**
  Prerequisite: T230. Implement PKG-01/02 and SRC-01..05 plus TPL-01: slash
  paths, version/variant components, single-dot static names and physical name+/
  storage. Select complete package versions before indexing; remove superseded
  dotted-root code/tests after tracing consumers. Keep parsing independent of
  alias/name lookup. Accept granular recognition/registry tests and real parser,
  native, static, constant and source-tool consumers; cover duplicate selection,
  missing local holes, Unicode and malformed version/separator boundaries.

<a id="t222"></a>

- [ ] **T222 — Require a space after the reference-formation operator.**
  Status: selected for this sprint by the user's 2026-11-20 request; transferred
  from BACKLOG with its original ID. Prerequisite: T231. The intended spelling
  was recorded earlier on 2026-11-20; implementation and the required
  specification amendment still need separate explicit authorization.
  Desired spelling: exactly one ASCII space between prefix `&` and its owned
  operand: `& item`, `& items[position]`, and `input& < & item`.
  The reference suffix stays adjoining (`target&`); standalone self-reference
  `&` has no operand and gains no trailing space. Reference semantics stay intact.
  Current contracts still require the joined prefix under
  [REF-01](spec/language/values-and-references.md#ref-01),
  [SYN-06](spec/language/source-and-names.md#syn-06) and the
  [grammar](spec/grammar.md#accesses-and-sources).
  Steps when separately authorized: align those rules, examples and acceptance
  scenarios; update recognition and affected source tools/fixtures using source
  structure, preserving literal/comment data; document and verify the change.
  Complete this narrow notation change before T232 and later consumers adopt
  the revised source forms; keep it independent of reference ownership/lifetime.
  Accept: test one-space formation with simple, multiword and indexed operands;
  reject joined operands, repeated spaces, TAB and a line break in place of the
  separator. Preserve suffix/self-reference parsing and all reference behavior.
  Complete component/composition, source-tool and conformance/documentation
  checks before marking this language change implemented.

<a id="t232"></a>

- [ ] **T232 — Implement shared configuration, scoped aliases and normalization.**
  Prerequisites: T231 and T222. Implement PKG-03/04 and SRC-06 through narrow
  UTF-8/INI, table/scope, parsed-path and normalization components shared by
  compiler/tools.
  Enforce package/local alias boundaries and explicit src-to-tests dependencies
  under PKG-09 while preserving caller-supplied test types. Track effective
  config/companion presence in caches. Accept positive, malformed, duplicate,
  scope/base/root-underflow, alias/static distinction and reordered-input cases;
  compose real ordinary/static/constant/native readers and dependency loading.
  T221/T226 consume these forms before the later end-to-end tool tasks.

<a id="t233"></a>

- [ ] **T233 — Separate project commands and create guided empty projects.**
  Prerequisites: T232, T221 and T226. Implement PROJECT-01/02 and the independent
  test-orchestration part of PROJECT-03. Separate command/process ownership from
  the compiler pipeline; expose narrow read-only sharing. Implement the exact
  ama-project init scaffold, destination preflight, staging/publication and
  Markdown hints; preserve an explicit entry selection. Run test microprograms
  with isolated outputs. Accept real command tests of new/empty/conflicting and
  concurrent destinations, failure cleanup, guidance links, no hidden network or
  compiler mutations, and independently compiled tests with recorded outcomes.

<a id="t224"></a>

- [ ] **T224 — Add relative module paths and support them in directory relocation.**
  Status: agreed target, transferred from BACKLOG by T230; implementation awaits
  a separate request. Prerequisites: T232, T233. Rules: SRC-06, PKG-04/09,
  PROJECT-01 and TOOL-01..04. Use the shared parsed configuration/path resolver;
  recompute references when importer, target or both move, including external
  importers, aliases and selector sets. Preserve whole-version selection,
  static-name identity, member selection, drafts and all unrelated bytes.
  Accept preview/preflight/apply and recoverable failure tests for namespace
  switches, nested name+/, multiword/Unicode names, shared aliases, config bases,
  source/test boundaries and exact graph/set preservation. No guessed mask or
  textual-prefix rewrite is allowed; unprovable plans fail before mutation.

<a id="t234"></a>

- [ ] **T234 — Implement exact dependency masks and repository routing.**
  Prerequisites: T232 and T233. Implement PKG-05/06 as independent finite-set
  matcher, requester-scoped source router and snapshot/source-contract planners.
  Preserve multi-depth whole-component stars, component-local embedded stars,
  version validity and all matches; route by longest full-component prefix with
  explicit conflicts and no failure fallback. Accept exhaustive bounded models,
  overlapping/reordered mappings, empty masks, skipped absent members and actual
  config consumers; one source contract must govern each acquired version.

<a id="t235"></a>

- [ ] **T235 — Compute minimal structural source-and-test delivery closure.**
  Prerequisites: T234, T221 and T226. Implement PKG-07..09 through source-unit,
  companion, topology-context, test-association and dependency-expansion children.
  Retain complete selected subtrees; distinguish topology-only parents from
  active units. Build the least finite closure of needed source/test/support
  inputs without tests-of-tests or guessed assets. Accept isolated and composite
  checks for child-only selection, parent promotion, shared/cyclic dependencies,
  mirrored tests, template/static/native inputs, missing tests and injected test
  types. Every emitted membership must have a recorded reason and snapshot.

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
  Prerequisites: T224, T233..236, T221, T222 and T226. Audit every retained
  current-rule component and per-file test/documentation owner; remove obsolete alternatives
  and tests that only preserve them. Exercise the canonical project/path matrix
  and reference-notation matrix through real preparation, compile/run,
  native/static/C-library, isolated tests,
  promotion and relocation consumers. Review correctness arguments and evidence
  before renewing conformance fingerprints. Pass required focused/composite/full
  checks, failure/sanitizer gates and final Doxygen/source-link/provenance checks;
  publish precise supported boundaries and archive each task in its own commit.

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
