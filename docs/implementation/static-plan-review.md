# Specification consistency and implementation plan

## Task identity and scope

- Task: [T204](../../TODO.md#t204).
- Status: active after AMB-015 approval, resumed 2026-11-19; input revision
  `155c9da`. [T205](../../DONE.md#t205) delivers the lifecycle clarification first.
- Authorization: review the entire specification after T202; if no
  contradictions are found, revise the work plan and commit it.
- Authority: [specification index](../../spec/README.md),
  [214 canonical requirements](../../spec/requirements.md), and
  [grammar](../../spec/grammar.md). The original planning request did not authorize
  semantic edits. The user's 2026-11-19 explicit answer separately authorizes
  T205's narrow LIFE-05 clarification, not other specification changes.
- Existing work: unfinished T182 source, tests, notes and coverage changes stay
  outside this planning task. No implementation acceptance is inferred from
  earlier mixed-interface tests or from document checks.

## Contract and planned steps

| Step | Purpose and deliverable | Check | State |
|---|---|---|---|
| 1 | Review all specification chapters and interactions | Independent reviews, exact requirement links and minimal cases for real conflicts | Review finished; AMB-015 confirmed |
| 2 | Derive foundation-first task boundaries from the accepted contracts | Explicit prerequisite graph and acceptance ownership, without changing semantics | Active after the accepted T205 correction |
| 3 | Reconcile unfinished tasks and preserve historical evidence | No reused IDs, false completions or discarded T182 work | Planned |
| 4 | Verify and deliver the planning records | Links/anchors, unique IDs, acyclic task dependencies, acceptance routing, documentation gates and staged diff | Planned |

## Review coverage and decision procedure

The review covers the language and implementation chapters, not compiler source
or generated artifacts. It compares syntax with semantic permission, compile-time
metadata with runtime state, and every shared boundary with its counterpart.
Different stages or separate graph relations are not automatically contradictory.
A concrete conflict requires one source situation with incompatible required
outcomes, or required observable behavior for which no outcome is specified.
Routine algorithm and component design choices remain implementation work.

| Review boundary | Inputs | Status |
|---|---|---|
| Static equations and ordinary Types | Templates, static results, declarations, compiler specializer, grammar and template acceptance | Checked; no additional conflict found |
| Runtime identity and control | Values/references, calls, control, continuations, lists, runtime and diagnostics | Checked; no additional conflict found |
| Native and concurrent boundaries | Native ABI, compiled libraries, async/parallelism and library contracts | Checked; AMB-015 correction authorized through T205 |
| Source and product tools | Source/names, grammar, dependency/loading, editor/relocation, conformance and roadmap | Checked; no additional conflict found |
| Cross-chapter and planning synthesis | Introduction, glossary, requirement index and existing task evidence | Checked; planning resumes after lifecycle reconciliation |

The review found a concrete cross-chapter conflict, recorded as
[AMB-015](../../AMBIGUITIES.md#amb-015). LIFE-05's unqualified prohibition on
disposing another context conflicts with the owned-child destructor obligation
in CABI-05/EMBED-07, including outer technical-failure cleanup. Two independent
reviews confirmed the same minimal case. On 2026-11-19 the user explicitly
approved the precise correction and continuation. T205 delivers it separately. Rechecking LIFE-05 against CABI-05, EMBED-07,
ASYNC-09 and the corresponding acceptance row leaves the same nonexecuting
owned-child cleanup outcome, while independent peer contexts remain live. No
other blocking contradiction was found in the bounded chapter reviews. T204
does not change language semantics or implementation files.

The completed chapter reviews otherwise found no competing required outcomes.
An independent CommonMark check covered 24 specification chapters, 671 actual
local links and 214 unique canonical requirements with valid destinations.
DEP-01/DEP-03 could more explicitly enumerate static graph dependencies and
compile-time-only nodes; these are explanatory coverage improvements, not a
second supported outcome or authorization for a specification edit.

## Continuation checkpoint

The document checker `python3 /tmp/ama-t204-check.py .` passed at the checkpoint:
29 documents, 1118 local links, 214 unique indexed rules, 11 EBNF blocks and 108
productions; negative undefined-name/alignment/group controls rejected as expected.
`git diff --exit-code -- spec` returned 0, confirming the canonical requirements
are unchanged. No compiler or generated-execution test is claimed by this audit.

Current step: finish the T205 document-generation/commit gate, then rebuild the
dependency/acceptance plan. All chapter reviews and lifecycle reconciliation are
finished; the bounded review found no remaining blocking contradiction.
Independent planning reviews have identified an early-registry prerequisite
needed to avoid a task dependency cycle. T204 remains unfinished until its plan
and document/dependency/coverage checks pass. No implementation build is running.
