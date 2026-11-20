# Development standard

This is the working standard for contributors and coding agents in Ama. It
records the method used to complete the initial implementation: define a small
contract, plan its dependencies, justify the algorithm, implement cohesive
components, verify real behavior, and commit the result with its evidence.
Apply the method to maintenance and documentation as well as new features.

[AGENTS.md](../../AGENTS.md) makes this workflow mandatory for agents.
[CONTRIBUTORS.md](../../CONTRIBUTORS.md) defines source, comment and test
conventions. The entire [specification](../../spec/README.md) remains the
authority for product behavior. This guide organizes work; it neither defines
language semantics nor authorizes changing the specification.

## Start from an explicit task

Read the applicable instructions, relevant specification rules, current
[TODO](../../TODO.md), component contracts and relevant tests. Inspect repository
status before editing; preserve unrelated changes. Read source within the task's
dependency boundary and expand that boundary when a real consumer requires it.
Respect the project's exclusions for binaries and generated/build artifacts.
Verify generated behavior through the supported test and build commands.

Every requested change belongs to a task with a stable ID. If the request has no
task, add one before implementation. State the problem and intended observable
result, prerequisites, affected boundary and acceptance conditions. Link exact
canonical rule definitions when language behavior is involved. Workflow or
documentation tasks may have no language rule; say so instead of inventing one.

Arrange tasks by prerequisites: foundations before consumers, narrow components
before their compositions, and final acceptance after implementation. A completed
dependency means its deliverable and evidence are available, not merely that its
checkbox was copied. Keep IDs stable when splitting or reordering work; allocate
new IDs for independently accepted follow-up tasks. Preserve historical results
as dated/task-specific evidence rather than presenting old counts as current.

Keep future proposals in [BACKLOG.md](../../BACKLOG.md), selected active tasks
in [TODO.md](../../TODO.md), and completed task records in [DONE.md](../../DONE.md).
A backlog entry does not authorize implementation or specification changes.
Move an agreed item into TODO with its existing ID and decisions, without
duplicating its record. Allocate IDs across all three files without reuse.
Once acceptance passes, move the complete task record to DONE in the same commit
that updates the queue and delivers its work. Preserve its dependencies, decisions,
results and limitations. Do not archive unfinished tasks or unresolved
documentation-debt owners. Consult the archive when validating prerequisites;
keep the active queue short instead of duplicating completed history there.

Choose the first ready unfinished task in the authorized scope. Finish it before
starting another. When several tasks are authorized, continue through the ready
tasks after each commit; a progress report or completed substep is not a reason
to abandon the remaining work. Do not enlarge the user's scope or request new
approval for routine implementation choices already covered by that scope.

## Plan small steps with visible outcomes

Use the [task record template](task-template.md). The TODO entry is sufficient
for a small change. For a larger task, link its design/evidence note from TODO
and keep the active step and acceptance record there. Reuse an existing component
contract when it already owns the responsibility. Do not create a second copy
of requirements, a separate prose file for every command, or another backlog
beside [BACKLOG.md](../../BACKLOG.md).

Break the task into steps that each produce something assessable: a specified
interface, an invariant argument, an independently tested operation, a real
consumer composition, a resource-failure check or an updated usage example.
For each step identify its purpose, inputs/prerequisites, expected observation
and verification. Split by coherent responsibilities and failure boundaries,
not arbitrary line counts or empty folder levels. Keep implementation, tests
and explanation together; documentation is part of the work, not a final backlog.

Before coding, identify the important success, boundary and failure partitions.
Plan required component and composition checks and the cases needing real
generated execution, toolchain validation, sanitizers or failure injection.
Acceptable evidence must be known before observing the implementation's output;
do not change expected behavior just to obtain a passing test.

## Resolve meaning before selecting an implementation

Requirements determine behavior. If their observable meaning is missing,
ambiguous or contradictory, follow the
[ambiguity procedure](../../AGENTS.md#resolve-ambiguity-before-implementation):
record the affected task, exact rules, minimal example, question and alternatives
in [AMBIGUITIES.md](../../AMBIGUITIES.md), then ask and wait for the user's
decision. Read-only investigation may clarify the question. Do not implement a
guess or switch tasks to bypass the unresolved issue.

Record the answer and its source before resuming. A behavioral decision does
not authorize a specification edit unless that edit was explicitly authorized.
Ordinary choices of data structures, helper boundaries and test organization
remain the implementer's responsibility when they preserve specified behavior.

## Establish the contract and correctness argument

For a code change, use the
[component contract template](../implementation/contracts/component-template.md)
and [contract registry](../implementation/contracts/README.md). Describe the
actual boundary: inputs and outputs, representations, ownership and borrowing,
aliasing, effects, preconditions, postconditions and allowed failures. Include
environmental and toolchain assumptions and name the state preserved on refusal
or failed preparation. Do not promise rollback that the operation cannot provide.

Explain why the algorithm covers its complete domain. State invariants, the
progress or termination measure, arithmetic bounds, time and space costs, and
why composed children preserve the contract. Use exact algorithms; an example,
test count or successful symbol lookup is not a general correctness argument.
For an optimization, establish equivalence of observable behavior, including
identity, lifetime, failure and native effects, before relying on speed results.

Keep the argument proportional to the change. A small helper can share a parent
contract; it still needs a clear responsibility. Documentation-only work needs
consistency and navigation evidence rather than a fictitious algorithm proof.

## Implement and document the same boundary

Separate orchestration from recognition, validation, preparation, mutation,
publication and cleanup where these are distinct operations. Prefer narrow
explicit interfaces and cohesive nested components. Preserve the ownership and
failure rules across each handoff; an individually correct child does not prove
that its parent passes the right identity, value, source span or lifetime.

Follow the [Doxygen conventions](../../CONTRIBUTORS.md#explain-purpose-with-doxygen-comments).
Explain each maintained code file's purpose and each meaningful interface's
contract. Compiler explanations link the relevant canonical rule IDs to their
actual definition anchors. Explain why non-obvious operations exist rather than
narrating assignments. Preserve byte-exact fixtures and document their purpose
through the owning test contract when inline comments would change the case.

Update the existing usage and contract documentation when behavior changes.
Remove obsolete current API descriptions instead of retaining competing
contracts. Preserve explicitly historical audits and acceptance records. Let
Doxygen generate file/interface reference pages; do not handwrite replacements.

## Verify behavior and its failure boundary

Start with meaningful tests of the affected component, then its real consumers.
Use the project's explicit C/assert cases and recursive Makefiles. Exercise
actual stage handoffs and compile/run generated programs for execution claims.
Public headers need independent compilation and consumer checks. Map every added
or changed code input, including test support and build scripts, to named cases
and commands in the [per-file registry](../implementation/documentation/README.md).
Avoid tests that merely repeat the implementation or add a new layer of tests
of tests without an independent behavioral claim.

Cover relevant empty, boundary, malformed, stale, aliasing, ownership and failure
partitions. Use exhaustive finite model checks when the domain permits them;
record that domain explicitly. For concurrent behavior, use controlled ordering
and synchronization with stated invariants rather than timing guesses. Include
real resource failure, interruption or recovery checks where those are part of
the operation's contract.

Keep these claims separate: metadata is current; a C interface compiles; a test
actually ran; behavior matches an independently specified result; a correctness
argument covers the declared domain. None automatically establishes the others.
Review affected claims before updating
[conformance fingerprints](../implementation/conformance/README.md#updating-reviewed-evidence).
Never refresh hashes just to make a stale-evidence check pass.

Broaden checks when a changed boundary, failed check or new concern warrants it.
Run required full-project and clean-release gates at their planned checkpoints.
Do not repeat unrelated expensive suites merely to accumulate passing counts.
Documentation-only changes normally require link, structure, consistency and
applicable documentation checks, not an invented compiler regression claim.

## Keep runs isolated and report their real result

Independent jobs may run concurrently only when their inputs and output ownership
permit it. Do not overlap a recursive suite with one of its included leaves, or
build the same compiler/output tree under different flags at the same time.
Serialize shared builds or use separate clean worktrees/output directories. Use
fresh paths where example or publication commands require them.

For every acceptance run retain the exact command, working directory, relevant
flags/environment, input revision or reviewed fingerprints, outcome and important
observations. During a long run, record its log path and active process/session
handle so it can be resumed instead of accidentally relaunched. An asynchronous
launch is not a successful result: collect its completion status.

An expected technical/application failure inside a negative case can accompany
a successful harness exit. Conversely, a signal-killed process is not a managed
error, an assertion-free run is not behavioral evidence, and successful branches
do not turn a failed combined command into a pass. Preserve the original failed
result, describe the cause and fix, and record the exact successful rerun scope.

Record exact supported tool versions and relevant restrictions. If a sanitizer
or external tool fails, isolate the cause with a minimal probe when practical.
Use positive and intentional-negative controls where needed. State unavailable
instrumentation explicitly; do not reinterpret a tool crash as a clean run or
silently weaken compiler flags. Keep alternative ownership/resource evidence
distinct from claims about that unavailable tool.

## Leave a durable record after every meaningful step

Update the task record as work progresses: what the step was meant to establish,
what was changed or learned, the check actually performed, its result, and what
remains. Record material decisions with their reason, constraints and supporting
evidence. Keep this in versioned TODO/component/task documentation; transient
chat, private scripts and temporary logs must not be the only record of a
decision or completed acceptance. Store detailed logs when useful and permitted,
but summarize the reproducible command and conclusion in the durable record.

For active work, leave a concise continuation checkpoint: current task/step,
completed evidence, changed or reviewed inputs, active jobs and logs, blockers,
the next action and the checks still pending. Before resuming, verify repository
status and those inputs, collect any active job's result and reuse valid evidence.
Do not infer completion from a lost session handle or an old checkbox. If a run's
outcome cannot be established, label it unconfirmed and rerun safely in isolation.

Progress reports explain findings, uncertainty and the next useful check. They
complement the durable record rather than replace it. Report genuine blockers
with a precise question; do not repeatedly ask permission to continue already
authorized work.

## Close one task with reviewable evidence

Before marking complete, check that its contract and planned acceptance are
satisfied, affected tests and consumers passed, comments and usage are current,
per-file evidence is registered, and material limits are recorded. Unresolved
requirements or failing required checks leave the task unfinished. A discovered
independent improvement becomes a follow-up task rather than hidden scope.

Record acceptance in the task and link the owning contract/evidence note. Move
the completed record to DONE and update the active TODO queue. Inspect
the diff and staged paths, preserve unrelated work, and commit this task's
implementation, tests, explanations and final status together. Include its ID
in the commit message. Do not combine several independently finished tasks into
a later batch or use a commit as a reason to declare unfinished work complete.

When regenerating documentation, finish maintained inputs first, then run
[the documentation gates](../documentation.md). If an input, including
TODO, changes after generation, regenerate and recheck fingerprints. A delivered
reference must describe the final inputs; keep generated pages unversioned and
commit the generator inputs and evidence. Report exactly what was verified,
what remains and any relevant limits. Continue with the next ready authorized
task until the requested scope is complete or a real blocker requires a decision.
