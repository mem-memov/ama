# Ama implementation work queue

The whole [spec/](spec/README.md) directory is authoritative. The
[requirement index](spec/requirements.md) locates all 214 current requirements;
[spec/grammar.md](spec/grammar.md) defines source forms. This file orders the
active engineering work for those contracts. It neither changes the
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

The user reiterated on 2026-11-19: remove obsolete implementation code together
with tests dedicated solely to it. Retained tests must exercise the current
specification and implementation; keep independently useful regression coverage
when migrating a live consumer. Removal and exhaustive behavioral/failure checks
belong to each affected task's acceptance, not a compatibility layer.

## Current tasks

No active tasks remain in the static-module/template sprint. The complete
[task records and evidence](DONE.md) preserve every accepted task; the next
unused task ID is **T219**. Add newly authorized work here before implementation.

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

Relative paths, the [AMB-006](AMBIGUITIES.md#amb-006) foundational list rewrite,
new raw-storage APIs, generic native ABI, binary-only open-template distribution,
static natural-number parameters and dependent types remain outside this sprint.
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
