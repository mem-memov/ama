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

## Current tasks

- [ ] **T158 — Resolve the remaining template design gates.**
  Status: ready for final design acceptance after T166; no implementation is authorized by
  this task. Prerequisites: T157's documented baseline, T159's named
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
  decision remains open. Finish the design acceptance from recorded decisions,
  canonical rules, the prerequisite-ordered implementation roadmap and named
  acceptance cases; do not infer implementation support from this queue item.
  Current checkpoint: AMB-004's inference, result-Type scope and constant design
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
  native/source-delivery rules. Current next action after T166's commit: review
  the final decision-to-rule/roadmap/acceptance links and archive this parent
  task in its own commit. No implementation work is authorized by this task.
  Acceptance: record user decisions and authorized canonical/EBNF corrections;
  derive the implementation roadmap and named positive/negative acceptance cases
  from those settled contracts. Preserve the previous conformance matrix as
  historical evidence until new implementation evidence is actually available.

The next unused task ID is **T167**.

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
