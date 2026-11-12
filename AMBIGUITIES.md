# Questions requiring a user decision

This registry records ambiguities, contradictions, or missing observable behavior
that block a task from [TODO.md](TODO.md). It is a work log, not a language
specification. Only [spec/](spec/README.md) defines the product contract.

When such a question arises, stop the affected task before implementing a guessed
interpretation. Read-only investigation may continue to make the question precise.
Add an entry, ask the user, and wait for an explicit answer. Do not switch TODO
tasks to avoid the blocker or mark the affected task complete.

Use stable, never-reused IDs in the form `AMB-001`, increasing numerically. Update
an existing entry for the same issue instead of opening duplicates. Preserve the
question and the user's actual decision; do not replace them with an inferred
resolution. Routine engineering choices that preserve a clear specified contract
do not belong here.

Each entry must contain:

- **ID and title:** the stable issue identifier and a concrete description.
- **Status:** `Open` until an explicit user decision is recorded; `Resolved` after
  recording that decision and completing any authorized specification correction
  needed before implementation can resume.
- **Task:** the exact TODO task ID and the implementation work that is paused.
- **Requirements:** links to the relevant canonical requirements and the precise
  wording in conflict or the behavior that is missing.
- **Minimal example:** a short Ama fragment, input, or state transition exposing
  the ambiguity, with assumptions sufficient to reproduce the reasoning.
- **Question:** the specific decision requested from the user.
- **Possible answers and consequences:** describe the distinct observable outcomes
  and any effect on safety, usability, lifecycle, or implementation dependencies.
- **User decision:** the answer, date, and a traceable conversation or other user
  instruction reference. Leave unset while the issue is open.
- **Specification authorization:** whether the user's instruction explicitly
  authorizes the particular `spec/` edit; link any resulting authorized change.
  A behavioral answer alone must not be treated as permission to edit `spec/`.
- **Resume evidence:** the corrected contract, affected tests, and remaining work
  to verify before the TODO task may be completed and committed.

No issues are currently recorded. This is not a claim that all future implementation
questions have already been resolved.
