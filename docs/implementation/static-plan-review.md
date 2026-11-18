# Specification consistency and implementation plan

## Task identity and scope

- Task: [T204](../../DONE.md#t204).
- Status: review and replacement plan completed 2026-11-19. The review started
  at `155c9da`; the final plan uses `40c27b5`, after
  [T205](../../DONE.md#t205) delivered the lifecycle clarification separately.
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
| 2 | Derive foundation-first task boundaries from the accepted contracts | Explicit prerequisite graph and acceptance ownership, without changing semantics | Completed; eight foundation tasks added |
| 3 | Reconcile unfinished tasks and preserve historical evidence | No reused IDs, false completions or discarded T182 work | Completed; T182 retained, earlier acceptance dated to its original contract |
| 4 | Verify and deliver the planning records | Links/anchors, unique IDs, acyclic task dependencies, acceptance routing, documentation gates and staged diff | Structural and independent reviews passed; final snapshot generation and staging are commit gates |

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
| Native and concurrent boundaries | Native ABI, compiled libraries, async/parallelism and library contracts | Checked; AMB-015 corrected by T205 |
| Source and product tools | Source/names, grammar, dependency/loading, editor/relocation, conformance and roadmap | Checked; no additional conflict found |
| Cross-chapter and planning synthesis | Introduction, glossary, requirement index and existing task evidence | Checked; revised prerequisite and acceptance plan reviewed |

The review found a concrete cross-chapter conflict, recorded as
[AMB-015](../../AMBIGUITIES.md#amb-015). LIFE-05's unqualified prohibition on
disposing another context conflicts with the owned-child destructor obligation
in CABI-05/EMBED-07, including outer technical-failure cleanup. Two independent
reviews confirmed the same minimal case. On 2026-11-19 the user explicitly
approved the precise correction and continuation. T205 delivers it separately in
`40c27b5`. Rechecking LIFE-05 against CABI-05, EMBED-07,
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

## How the replacement plan follows the contracts

The old remaining queue started with ordinary connection inference. That skips
new prerequisites: separate module kinds, whole-graph binders, producer metadata,
named partial applications and the ordinary export selector. Eight new tasks
T206–T213 give these responsibilities explicit owners. The revised queue begins
with T203, the actual source/test audit. It then establishes source structure and
binding (T206/T207/T213), finite terms and metadata (T208/T209), and expansion
guards (T210) before asking graph or ordinary consumers to use them.

An important ordering decision is to move T183 before graph output evaluation.
If the canonical registry waited for T182 ordinary connections, T182 would wait
for T212 export selection, T212 for T211 graph outputs, and T211 for the registry.
Instead T183 owns closed candidate identities and pending validation state; it
does not admit executable Types. T211/T212 can expose known nominal identities
with obligations still pending. T182 generates ordinary connections, T184 closes
the group, and T185 performs final ordinary admission. This breaks the planning
cycle without weakening the language's closure requirement.

The plan distinguishes the relations that determine these separate operations:
source dependencies, structural term equality, static output dependencies,
definition expansion, returned-constructor creation, eager storage and constant
value dependencies. None is substituted for another. Independent reviews checked
the candidate/admission boundary and found no circular wait on T185. T213 owns
real module lookup and dependency closure; T209 does not expand definitions;
T210 guards expansion before it occurs, including relevant cache paths.

Each later consumer has a current owner: constant declaration/namespace alignment
in T212 and lifecycle in T186; resolved storage/control in T187; stackless
execution in T188; diagnostics in T189; concrete native integration in T190;
source delivery in T191; editor and relocation in T192/T193; checked C facades
in T194; useful intrinsic-list algorithms with static factories in T195.
T194 explicitly covers T205's owned-child cleanup and independent peers, and
rejects static modules as export roots even when their graphs are closed.
T189 follows COMP-04's precise failed-output and handwritten-file protections;
the plan does not impose a new ban on retaining diagnostic artifacts.

T196 reviews current evidence, T197 accepts release behavior, and T198 generates
the final reference after all maintained inputs are finished. Earlier tasks must
already supply component contracts, narrow cohesive implementation, per-file
tests and Doxygen explanations with clickable canonical rule links. Final
documentation is not a place to defer those obligations. Every task records its
steps, observations, failure results and next action and receives its own commit.

## Evidence ownership and limits

The routing table covers all 37 current template acceptance areas with current
unfinished owners. It also names owners for internal hyphens, real source
dependencies, native headers, compiled storage/control, C export exclusion,
lifecycle, editor operations and relocation. T196 must enumerate the actual
canonical rules and acceptance rows at execution time; the current 214-rule,
150-row, 37-template-area totals are a baseline, not a substitute for that review.

T168 and T169–T181/T201 remain historical records of their original contracts.
They are not current acceptance of removed mixed interfaces, exact expected
signatures or static positions. T203 must inspect the real implementation,
assign each component/file to reuse, adaptation, removal or deferral, and refine
the engineering steps from actual evidence. T204 did not inspect compiler
function bodies, update implementation fingerprints or run implementation tests.

Unfinished T182 source, tests, its local continuation note, partial sanitizer
results and recorded O2 warning remain outside the planning commit. T203 must
establish their exact input fingerprints and ownership before implementation.
The replacement queue neither discards them nor silently depends on uncommitted
APIs. Relative paths, dependent natural parameters and a rewrite of the intrinsic
list remain outside this sprint's approved model.

## Verification and delivery record

The document check uses CommonMark parsing outside code fences to resolve local
links and explicit/heading anchors, compares the requirement index with unique
canonical definitions, and checks EBNF production references, group balance and
aligned definition operators. The plan check reads TODO/DONE task anchors and
prerequisites, resolves every edge, uses depth-first cycle detection and checks
presentation against the dependency order. It parses the actual Markdown tables
and compares their acceptance-area sets and current owners exactly.

Commands used for the bounded checks are `python3 /tmp/ama-t204-check.py .`,
`python3 /tmp/ama-t204-plan-check.py .`, `git diff --check` and
`git diff --exit-code -- spec`. A pre-archival checkpoint passed 29 documents,
1199 local links, 214 rules, 11 EBNF blocks and 108 productions; the plan had
27 active tasks including T204 and 80 explicit prerequisite edges. Intentional
negative controls rejected an undefined production, shifted EBNF alignment,
unbalanced group, dependency cycle, unknown dependency, missing acceptance area
and obsolete completed owner. Independent plan reviews found no blocking scope,
ordering or admission issue. Their small wording corrections are incorporated.

After archiving T204, the checks pass on 29 documents and 1200 local links, with
the same 214 rules, 11 EBNF blocks and 108 productions. The queue contains 26
implementation tasks and the same 80 prerequisite edges, with T203 the first
ready task. All 37 acceptance areas have current owners; every negative control
still rejects. T214 is the next unused ID. Whitespace and unchanged-spec checks
pass. T204 changes no files in `spec/`; the lifecycle correction remains isolated
in T205.

Final Doxygen generation runs with `make docs` from an isolated archive of
`40c27b5` plus only TODO, DONE, AMBIGUITIES and this note. This keeps unfinished
T182 code and its evidence registry outside the delivery inputs. Generation must
pass its coverage, interface and canonical-link gates on those final bytes;
its exact outcome, counts and tool versions accompany the commit. The staged
paths and contents must match that checked snapshot. No compiler execution or
new-model conformance claim follows from these documentation checks.
