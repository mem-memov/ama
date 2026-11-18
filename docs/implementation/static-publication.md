# Complete static producers and immutable publication

**T201 alignment:** [Creation guards](static-guards.md#t201-alignment) records
the current nominal-cycle/result/publication evidence. Own nominal Type results
can close their specialization graph, while direct/mutual static projections
still fail the definition guard. A repeated unknown producer ancestry stays
Pending and cannot publish until independent evidence closes it. A warm foreign
value proof cannot bypass that check: unknown incoming ancestry requires original
RHS checking in the actual context. T180's dated
evidence below remains its original baseline. Full producer closure and immutable
publication still follow [TPL-12](../../spec/language/static-results.md#tpl-12).

T180 is accepted on input fa1cd3e. Authority:
[TPL-12](../../spec/language/static-results.md#tpl-12),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11),
[ARCH-03](../../spec/implementation/compiler.md#arch-03).
T180 did not authorize specification changes.

## Contract

Borrow T179's result plan and its exact source/context identities. Require the
whole producer Type, its static dependencies and every declared output to close
before exposing any selected Type/Template or immutable received alias. Returned
Template residuals remain local binders; they are not missing producer inputs.
No runtime code, default, native initializer, implicit instance or constant value
is executed. A new received alias supplies no independent anchor.

Existing determined receivers can supply equality evidence. Explicit result and
previous alias definitions are evaluated as independent guarded roots, rather
than adding fictitious definition edges just because a name occurs as a receiver.
Use original receiver symbols/definitions and registered actual contexts. Do not
choose origins from names or expected signatures. Keep input omission variables
from T177's source/environment keys unchanged across attempts.

One attempt evaluates the source and independent receiver definitions through
T179, solves exact equations, and checks selected closure. Repeat only after new
constraint equations, independent metadata, requirements or checked source
contexts are published. Merely
allocating another application snapshot is not progress. A stable unresolved
state fails with original producer provenance, never a default or guessed inverse.
The outer whole-project scheduler and ordinary validation remain T184/T185 work.

Publication owns immutable descriptors, not new nominal types or runtime state.
Keep producer identity separate from returned value identity and exact receiver
symbol scope. Equal repeated publication must agree, including existing aliases;
failed operations leave public outputs untouched and partial records owned.

## Ordered implementation and acceptance

1. Add a public guarded definition-evaluation entry to T179 for independently
   known receiver evidence; share its explicit stack and test guard/cache behavior.
2. Add a narrow closure owner with evidence-driven attempts and selected producer/
   dependency checks. Test receiver inference, fresh aliases, unused inputs,
   skips, unknown origins and noninvertible output definitions.
3. Publish exact output/alias descriptors only after closure. Test equal/different
   owner scopes, repeated immutable aliases, partial returned constructors and
   allocation/counter refusals. Compare contextual omissions with/without outputs.
4. Register per-file/header evidence, run strict components/composition, sanitizers,
   build and documentation gates, archive and commit before T181.

## Invariants, termination and cost

- A publication is keyed by the exact source use, not a returned value or method
  name. Canonical equal returned Types do not merge producer occurrences or runtime
  instances. Alias keys are original `(environment, symbol)` pairs.
- Receiver evidence evaluates already declared result/alias definitions as roots.
  A fresh alias adds no independent fact. T179 retains actual RHS dependency
  edges and T178 guards before any definition or producer is expanded.
- Every successful publication has a READY checked expression, closed transitive
  source dependencies and a complete selected whole producer value. Every actual
  declared static output is independently queried, including skipped/unselected
  outputs. No selected constant hides an unused unknown producer input.
- A returned Template's residuals are locally bound constructor parameters; free
  surrounding variables still fail selected closure. Output identity is the actual
  constructor origin/captures/residuals, never a synthetic producer-derived Type.
- Late equations are solved before returning a cached publication. Conflicts
  preserve previously published immutable descriptors. Failure leaves output
  untouched and partially prepared records owned until destruction.

Let `F` be the number of new exact equation pairs, independent metadata facts,
requirements and checked actual source contexts introduced during this source's
finite guarded expansion. Each nonterminal iteration must strictly increase at least one corresponding checked
counter. Thus there are at most `F + 1` attempts; application/term/allocation counts
are not the progress measure. T179's source keys, original AST operands and T178's
creation/definition guards bound expansion; repeated attempts reuse their terms
and known-origin work. T176 deduplicates exact pairs/requirements/metadata. No
pass, depth, timeout, candidate search or guessed inverse is used.

If `C_i` is the T179/T176/T178 work for attempt `i`, closure costs the sum of those
kernel costs plus source-action receiver scans and the visited proof-graph walk.
For `N` reached source proofs and `E` retained checked edges, linear exact queue
membership costs `O(N^2 + EN)`, in addition to guarded definition evaluation.
For `O` declared outputs, `B`
output bindings and `L` stored aliases/publications, descriptor construction and
linear exact lookup cost O(O + B(O + L) + L), excluding selected-value queries.
New retained descriptor storage is O(O + new aliases + 1); iteration continuations
are operation-local T179 scratch. Output sequence counters start at zero and are
bounded by the accepted original result count; plan/alias publication counters
are explicitly checked before increment/allocation.

Finalization is a selected-source boundary. A separate unresolved root remains
an obligation for T184 and cannot block an independently closed producer here.
Conversely, this operation must be called only when the outer caller has supplied
its available evidence: stable local underdetermination is an error, not an
incremental Pending result. Earlier stages use T179 to retain Pending work.
Ordinary executable/lifecycle/ownership validation remains T184/T185 work.

## Step record

T179 commit fa1cd3e passed 31 result and 27 affected guard cases, strict and
ASan/UBSan acceptance, root build and final Doxygen refresh. Working tree was clean.
No jobs active. Current step: independent receiver-definition evidence without
invented receiver definition dependencies. No ambiguity or specification edit.

- Added independent definition evaluation to T179's public owner, sharing its
  explicit operation stack. The applied-scope result case now checks the actual
  derived definition value through that entry. Strict result evaluator acceptance
  passed all 13 cases (`/tmp/ama-t180-independent-definitions.log`, collected exit 0).
- Added publication ownership/first-error propagation, original receiver evidence,
  exact closure attempts and canonical output/alias descriptors. Closure retries
  are justified only by equation, metadata or requirement additions. Complete
  returned constructor metadata retains its local residuals.
- Initial strict closure leaf passed three cases (`/tmp/ama-t180-closure.log`,
  collected exit 0): an existing result closes an omitted identity input; a fresh
  alias, skip and constant result of an unused input all fail without guessing;
  a returned partial constructor preserves residuals in an applied owner.
- Current job: strict publication parent at `/tmp/ama-t180-publication-strict.log`.
  Collect before functional edits. Added value/identity and allocation/counter/
  late-conflict cases; source registration, further inference/error composition,
  contextual omission coverage, sanitizer/build/docs acceptance remain.

- The first strict publication parent passed all 12 cases
  (`/tmp/ama-t180-expanded-strict.log`, collected exit 0). The six-context matrix
  compares shared declarations, ordinary arguments, runtime returns, fields,
  static returns and constant namespaces, with and without output syntax.
  Every output phase uses producer inference; existing independent receiver
  evidence closes its omitted inputs. Bare static returns preserve residuals;
  fields require anchors and inputs/namespaces require explicit coverage.
- Allocation refusal now covers two full-operation partitions: explicit producer
  arguments with a new alias and omitted producer arguments constrained by an
  existing result definition. Added lost-input/noninvertible output checks,
  unknown actual-origin rejection, conflicting existing alias values and a
  complete selected producer alongside a separate unresolved root.
- Current job: final strict publication parent with 16 named cases at
  `/tmp/ama-t180-final-strict.log`; collect before functional edits. Explicit
  per-file/header evidence is registered. Sanitizers, affected result evaluator,
  root build and documentation gates remain.

- Full strict publication parent passed 16 cases
  (`/tmp/ama-t180-final-strict.log`, collected exit 0). Added a final source-backed
  test that two explicit output phases infer different Types for the same original
  formal through distinct omitted-input terms. Its strict values leaf is running
  at `/tmp/ama-t180-values-final-strict.log`; root build is running at
  `/tmp/ama-t180-build.log`. Collect before functional edits. The per-file registry
  now names 17 publication cases. Next: sanitizer acceptance of all 17 and the
  affected 13-case result evaluator, documentation gates and task archive/commit.

- Read-only closure review identified a necessary additional receiver obligation:
  an existing alias may have a known constant selected value while its original
  producer still has an unresolved unused input. The current source must not
  publish that alias merely because its own producer closes. Independent receiver
  definitions must contribute checked RHS/dependency closure, not only equations.
  TPL-12 already specifies this; no ambiguity or specification change exists.
  Add a regression after active jobs complete, confirm refusal, then include
  receiver-definition closure in the exact attempt predicate and rerun acceptance.

- The original-receiver regression failed as intended against the earlier
  implementation (`/tmp/ama-t180-original-receiver-before.log`, collected exit 2):
  a new complete producer incorrectly published a known constant alias whose
  original producer still had an unknown unused input.
- Retained exact required expression/guarded-definition edges in T179's stable
  work records. Definition cache entries retain their original checked RHS proof;
  cache hits retain that witness separately from the current lexical RHS.
  These are source-check dependencies, not additional definition-cycle edges.
- Publication now uses an operation-local visited worklist over exactly reachable
  source proofs, including nested operands, guarded head/output reads and existing
  receiver definitions as independent roots. It follows cached RHS witnesses and
  requires every reached source producer to close. Unrelated roots remain outside
  this graph. Receiver equality cycles are visited once rather than rejected or
  recursively defined as a conjunction that can never become true.
- The direct receiver regression passed
  (`/tmp/ama-t180-original-receiver-after.log`, collected exit 0). Strict values
  acceptance then passed the transitive cached-receiver regression and an anchored
  receiver equality cycle (`/tmp/ama-t180-receiver-graph-strict.log`, collected
  exit 0). The latter keeps the actual static-definition graph acyclic.
- Added exact dependency-edge identity/idempotence, copied-handle and counter
  boundary checks to the result context leaf. The affected full result parent
  (32 cases) is running at `/tmp/ama-t180-required-graph-results-strict.log`.
  No functional edits until completion. The publication registry now has 20 cases;
  final strict/sanitized publication acceptance must include the new graph.

- Full affected result acceptance passed all 32 cases under strict and ASan/UBSan
  (`/tmp/ama-t180-required-graph-results-strict.log`,
  `/tmp/ama-t180-graph-result-sanitizers.log`, collected exits 0). Root build passed
  (`/tmp/ama-t180-graph-build.log`, collected exit 0). The 20-case publication parent
  passed strict acceptance (`/tmp/ama-t180-graph-publication-strict.log`, exit 0).
- Added an exact pre-anchored foreign-result case: all value equations and required
  Types already exist, but the first attempt registers a previously missing actual
  source context. The old progress measure rejected it prematurely
  (`/tmp/ama-t180-context-progress-before.log`, collected exit 2). Including new
  checked context registrations as provenance evidence passes
  (`/tmp/ama-t180-context-progress-after.log`, collected exit 0). Unchanged snapshots
  remain excluded; no origin or implicit context is invented.
- Current job: full 21-case strict publication parent at
  `/tmp/ama-t180-complete-strict.log`. Collect before functional edits. Remaining:
  final publication sanitizers, final root build, complete per-file/header/docs
  acceptance and archive/commit. No ambiguity or spec change.

- Full strict publication acceptance passed all 21 cases
  (`/tmp/ama-t180-complete-strict.log`, collected exit 0). Final root build passed
  (`/tmp/ama-t180-complete-build.log`, collected exit 0). The affected 32-case
  result kernel had already passed ASan/UBSan after required-edge/cache-proof
  changes; subsequent functional edits affect only publication closure.
- Current acceptance jobs: final 21-case publication ASan/UBSan at
  `/tmp/ama-t180-complete-sanitizers.log` and complete documentation/header gate at
  `/tmp/ama-t180-complete-documentation.log`. Markdown audit passed 214 files,
  2928 local links, 214 rules and 108 unchanged EBNF productions
  (`/tmp/ama-t180-complete-markdown.log`, collected exit 0). Collect active jobs;
  then generate/validate Doxygen, archive T180 and refresh final reference before
  its separate commit. No user decision is needed.

## Final acceptance

All 21 publication cases passed strict C11 warnings as errors and ASan/UBSan
(`/tmp/ama-t180-complete-strict.log`, `/tmp/ama-t180-complete-sanitizers.log`,
collected exits 0). All 32 affected result cases passed strict and ASan/UBSan
(`/tmp/ama-t180-required-graph-results-strict.log`,
`/tmp/ama-t180-graph-result-sanitizers.log`, collected exits 0). The final root
build passed (`/tmp/ama-t180-complete-build.log`, collected exit 0).

The receiver-proof and provenance-progress regressions were observed failing
before their fixes, then passed with positive controls. Exact required edges,
cache witnesses, compatible receiver equality cycles and separate unresolved roots
are covered without guessed origins, depth budgets or source-name approximations.
Two full-operation allocation partitions cover explicit arguments/new aliases
and omitted arguments/existing result evidence; counted fixtures reach zero.
LSan remains disabled under the documented container restriction; counted ownership
checks are independent of that unavailable instrumentation.

Complete per-file/header acceptance checks 2655 ready inputs with zero debts
(`/tmp/ama-t180-complete-documentation.log`, collected exit 0). Markdown checks
214 files, 2928 local links, 214 rules and 108 unchanged EBNF productions.
Doxygen 1.11.0 validates 2006 maintained file pages, 7775 interfaces,
426 canonical destinations and 6339 pages (`/tmp/ama-t180-doxygen.log`, exit 0).
The archive/status changes receive a final reference refresh before commit.
No specification changes were made. T181 is the next ready task; whole-project
admission, ordinary validation and executable conformance remain separately owned.
