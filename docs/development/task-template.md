# Task and step record template

Use this structure in the TODO entry for a small task or a linked task/component
note for a larger one. Replace prompts with concrete facts and omit sections
that do not apply, explaining a material omission. Do not create duplicate
requirements or a second backlog. This template is not acceptance evidence.
Follow the [development standard](README.md).

## Task identity and intended result

- ID and TODO link:
- Status: planned, active, awaiting clarification, or complete.
- Problem and observable result:
- Authorized scope and applicable specification rule links:
- Prerequisites and evidence that they are satisfied:
- Affected components and real consumers:
- Acceptance conditions and known environmental assumptions:

## Entry review against the current state

Complete this before starting the task; keep it brief and link existing evidence.
Update it when a material change invalidates its conclusions, without duplicating
the historical results below.

- Reviewed specification/decisions, repository state and prerequisite evidence:
- Actual delivered behavior, missing behavior and reusable or invalidated evidence:
- Revisions to scope, dependencies, finite deliverable/consumer and acceptance:
- Disposition: ready, split into linked stable-ID subtasks with parent acceptance
  retained, or awaiting a precise clarification/authorized additional audit.

For a requested additional audit, state its question, scope, expected result and
reason for extra effort; record the user's authorization and selected depth or
budget before that audit. Routine task review needs no separate approval. Apply
the [entry review rules](README.md#review-the-next-task-against-the-current-state)
and preserve specification authority and ambiguity handling.

## Contract and planned steps

Link the owning [component contract](../implementation/contracts/component-template.md)
for code changes. State what existing evidence can be reused and why. Plan the
relevant input partitions and failure observations before implementation.

| Step | Purpose and prerequisite | Deliverable / expected observation | Check and completion condition | State |
|---|---|---|---|---|
| 1 | Concrete question this step resolves | Reviewable result | Exact planned verification | Planned |

## Step results and decisions

For each meaningful completed step, record:

- Step, actual changes/findings, and reason for any changed plan.
- Contract/invariants established and links to the actual source or explanation.
- Decision, alternatives that matter, and why the chosen approach preserves the
  specified behavior. For a user decision, link the ambiguity and its resolution.
- Exact check, input revision/fingerprints, flags/environment and outcome.
- Independent expected observation and material limitations.
- Failures found, their cause and the exact scope of successful reruns.
- Remaining work; link a separate follow-up only when it is outside this task.

## Verification record

Distinguish planned commands from observed results. Record process exit status
and signal termination accurately. Link the per-file named-case mapping and
real composition evidence for changed code; include relevant tool versions.

| Check and input/configuration | Exact command and working directory | Observed result | What it establishes / limits |
|---|---|---|---|
| Name; revision or fingerprint; flags | Reproducible command | Not run until actually collected | Independent observation |

## Continuation checkpoint while work is active

- Current task/step and last verified repository state:
- Completed evidence and changed/reviewed input boundaries:
- Active jobs: process/session, command, output owner and log path:
- Open clarification/blocker and user decision awaited:
- Next action, remaining acceptance checks and safe rerun conditions:

## Final acceptance

State what is now true, why the contract is satisfied, the actual check results
and remaining limits. Record the TODO update, completed record in
[DONE.md](../../DONE.md), and commit boundary. Keep the
historical failed-run facts that explain the evidence; remove obsolete current
API descriptions. Mark complete only after required acceptance succeeds.
