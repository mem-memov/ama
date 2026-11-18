# Structural static application and recovery

T177 is accepted on input 7e3afda. T173 supplies actual-interface snapshots and
source requests, T175 immutable scoped values, and T176 exact finite equations.
Authority: [TPL-07](../../spec/language/templates.md#tpl-07),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-12](../../spec/language/static-results.md#tpl-12),
[CONST-03](../../spec/language/declarations.md#const-03).
No specification change is authorized. Definition expansion and creation guards
remain T178/T179; executable/constant admission remains T180/T184/T186.

## Contract and intended operations

Operate on finite prepared source/application records, not runtime samples.
Borrow one healthy term store, its solver and binding plan; own application work,
validation obligations and diagnostics separately. Existing source and value
payloads remain immutable. A successful candidate is compile-time structure,
not an executable specialization or a published static output. Pending metadata
and unknown actual origins remain explicit; no expected interface substitutes
for an actual declaration order or proves an unknown nominal origin.

A known constructor supplies exact original origin, fixed captures and ordered
residual formals. Normalize requests against a snapshot of those actual residuals,
then merge new bindings without reopening fixed ones. Preserve every original
capture, including unused arguments. Source order and staged application cannot
change the normalized map. Check categories and complete recursive named
signatures independently of positional order. Check deferred descriptions only
when independent metadata is available; never fill an expected signature by
trying its first actual argument. Guarded later consumers may supply resolved
metadata without rewriting declarations.

Omission policy is explicit. Static arguments/results preserve local residuals.
Root runtime inputs require explicit coverage. Field/shared/runtime-result roots
generate deterministic T175 field inference terms. Constant namespaces introduce
no such terms and must become independently closed Types before member lookup.
Any static output phase, including a skip, instead generates per-expression
producer-input inference terms and requires the entire producer to close under
TPL-12; output definition evaluation/publication is outside this component.
These policies never create a source-visible owner parameter or execute a body.

Recover an unknown constructor only from known nominal application information
and its independently declared complete expected residual signature. Normalize
written positions using the actual original declaration order, never the order
of expected signature entries. Applied original names P must be fixed in actual
R = (D, B, S). Recover exactly `(D, B minus P, S union P)` and equate removed
bindings with their supplied expressions. Retain all other bindings and existing
residuals. Check the recovered signature and any earlier constructor binding.
Unknown origins/signatures remain Pending; there is no constructor or partition
search. Root input coverage and nested partial patterns remain distinct cases.

Nested unresolved applications need their own internal result variable. Its
identity is lexical environment plus original expression, separate from every
omitted producer input and field inference key. It adds no user placeholder or
owner formal. A later known result can drive recovery of an inner constructor;
an independently known head can drive forward application. Keep these operator
records outside the immutable value DAG, and add exact T176 equations when their
finite structure becomes known. No definition/layout is unfolded here.

## Ordered steps and acceptance

1. Add scoped internal expression-result identities and source-backed diagnostics;
   distinguish repeated expression uses, different sites/owners, producer-input
   variables and field variables. Refuse foreign syntax and every allocation.
2. Implement actual-origin forward maps and context-specific omission records;
   compose real T173 snapshots, skips, fixed filtering and T175 interning. Check
   direct/staged/reordered equivalence, independent fields, output-phase override,
   namespace/input coverage and known/deferred category/signature errors.
3. Implement exact nominal recovery and equality handoffs, including captured
   unused bindings, partial actuals, repeated disagreement, positional snapshots,
   missing fixed P slots and unknown-origin Pending outcomes. Compose nested
   recovery/forward applications through explicit expression-result equations.
4. Prove preservation, uniqueness, finite progress and ownership/cost bounds.
   Refuse each observed preparation allocation and counter edge; test actual
   parsed source and independent algebraic/order models. Register per-file/header
   evidence, run strict affected parents/sanitizers/build/docs, archive and commit.

## Preservation, progress and costs

For a known constructor `(D, B, S)`, input validation establishes distinct
original residual identities. The supplied non-skipped map P has a domain
disjoint from B. Partial application constructs exactly B union P, with remaining
original formals locally bound. Disjoint map union is associative and independent
of traversal order, so direct, staged and reordered applications intern the same
identity. All previously fixed bindings survive, including unused ones. Inference
adds only explicitly permitted, deterministic scoped variables; it never adds an
enclosing formal or runs an initializer. Coverage refuses missing root inputs
and namespace arguments. Independent expected metadata preserves each previously
known category/name; an actual argument cannot define its expected signature.

Recovery's candidate is unique: actual D fixes original identities, expected
signature E fixes possible open identities, and every actual residual S must be
inside E. Retain exactly B outside E. Re-normalize actual source requests against
that candidate, preserving RHS identity by original syntax. The complete forward
result must equal the independently supplied actual R. This enforces both that
every applied name is fixed in R and that no removed fixed name is left unapplied.
Thus E equals S union P and the retained map is B minus P. Known names/categories
and complete recursive signature disagreements fail immediately. Pending nested
metadata remains a separate obligation. Unknown origins remain pending without
searching a filesystem, choosing a constructor or enumerating partitions.

Nested composition connects original expressions through independent result
variables. T176's exact equations propagate captured RHS facts in either
direction; source scopes and original parameter identities remain unchanged.
Each operation traverses finite input/source/map lists, delegates finite DAG
unification, and returns. There is no source-depth C recursion or retry budget.
The later orchestration owner must schedule a finite source/environment work set
and resubmit only after genuine new structural facts; allocating another use
snapshot is not semantic progress. T178 separately guards definition expansion.

Let K be the actual origin's formal count, L its symbol-chain length, Q written
input count, and A prior application snapshots. Exclusive preparation/checking
work includes O(K L + K² + Q² + Q K + A) for ordinal selection, identity/membership
scans, exact map validation and RHS matching, plus borrowed source/value/solver
membership scans. Recovery adds O(K L + K²) for exact name-set membership. Actual
T173 normalization, T175 interning and T176 solving/signature comparisons retain
their separately documented costs; this component makes no near-linear claim.
Own retained storage per source/forward/recovery operation is O(K + Q), plus
source snapshots and canonical values owned by those components. Residual
metadata adds O(K) solver-owned headers/entries, borrowing immutable nested
entries until solver destruction. Complete facts and known generic metadata
are immutable. No entire source graph or candidate partition is duplicated.

Outputs and application publication remain untouched on failed preparation.
Borrowed owners may already contain valid equations/canonical child values;
independent metadata facts may be retained before a subsequent check fails.
There is no whole-operation rollback promise: the failed owner is destructible
only. All partial allocations stay arena-owned, counters/products are checked,
and error propagation allocates nothing. Destruction order is application plan,
solver, term store, binding plan, inventory/project/source and allocator. Do not
free independent signature/source storage while the solver borrows it.

## Step results and continuation

- Added an internal application-result variable role keyed by exact environment
  and original expression. Unlike field/producer holes it has no formal symbol
  and introduces no owner parameter. Result categories remain deferred until
  actual structural evidence exists. Diagnostic definitions now use the original
  lexical expression when such a variable has no named declaration.
- First identity run refused the fixture's alleged distinct expressions:
  `/tmp/ama-t177-expression-identity.log` (exit 2). Its occurrence selector counted
  an output receiver as another expression head. The selector now excludes
  receiver occurrences, as the existing binding fixture does. No production
  identity rule or expected distinctness was weakened.
- Strict expression identity rerun passed both cases (exit 0,
  `/tmp/ama-t177-expression-identity-rerun.log`), including distinct owners/sites,
  producer-input separation, source-backed final errors and allocation refusal.
- Consumer review found a real T175 ordering defect: the scoped symbol chain
  groups explicit binders before implicit ones, while nominal construction
  incorrectly used that chain as declaration order. A mixed input/explicit/
  result-only introduction regression failed at residual order as expected
  (`/tmp/ama-t177-mixed-order-before.log`, exit 2). Added a shared read-only
  `static_symbol_at` ordinal selector and use it for constructor signatures,
  actual binding snapshots and canonical nominal captures/residuals. Current
  T175 cost documentation now includes that explicit ordinal lookup.
- Shared ordinal repair passed the strict application parent (three cases,
  `/tmp/ama-t177-mixed-order-after.log`, exit 0). It covers original capture/
  residual/signature order against actual T173 snapshots, result-only free input
  introductions and nonpositional alias/out-of-range queries.
- Added application ownership, source-derived omission policy, actual-origin
  forward capture merging and exact known/deferred category/signature checks.
  The initial forward parent stopped at an unused alternative `setup_static`
  fixture helper under `-Werror` (`/tmp/ama-t177-forward.log`, exit 2). That optional
  two-module setup is now `static inline`, like other alternative fixture paths.
- Review also ensures a symbolic identity's later nominal captures are checked,
  and source-use pointers must match a published T175 record/environment/expression.
  No expected signature is chosen from the first actual argument.
- Forward parent passed eight cases (exit 0,
  `/tmp/ama-t177-forward-rerun.log`): direct/staged/reordered maps, skips, inherited
  rebinding, duplicates, foreign IDs, Type/Template separation, renamed signatures
  and later capture checks through an initially symbolic identity.
- Parsed policy parent passed twelve cases (exit 0,
  `/tmp/ama-t177-policy.log`): independent shared/result/body holes, static-result
  residuals, complete input/namespace coverage, output-skip override and nested
  producer versus containing-field identities. Containing parameter counts stay
  unchanged. Requirements remain pending until independent typed evidence closes
  every generated input; no native/default/body execution occurs.
- Added independent complete expected-signature facts per application/formal.
  They preserve known generic declarations and complete residual descriptions;
  generated Template holes receive their declared refined contract without a
  fabricated constructor origin. Descriptor snapshots have solver lifetime.
- The metadata parent caught a missing explicit declaration include for the
  existing exact-name comparator (`/tmp/ama-t177-metadata.log`, exit 2). Added
  its owning static-inventory internal header rather than duplicating comparison.
- Metadata rerun exposed a const-correctness boundary in the older mutable
  inventory descriptor representation (`metadata-rerun.log`, exit 2). Residual
  metadata now owns its root signature headers and borrows immutable independent
  entries under the existing solver lifetime contract. The next run exposed an
  overly strong test assumption that a first comparison with a newly composed
  description needs no memo entry (`metadata-rerun2.log`, exit 2). The test now
  checks publication idempotence separately from allocation-free warmed repeats.
  Strict metadata passed (`/tmp/ama-t177-metadata-rerun3.log`, exit 0).
- Actual source adapter re-normalizes fresh T173 snapshots and maps operands by
  original binding syntax. Scoped result variables connect nested application
  equations; unknown origins keep their raw provenance. Strict source checks
  passed (`/tmp/ama-t177-source.log`, exit 0).
- Recovery uses the declared residual set E to retain B outside E, then checks
  the entire forward-result equation. With actual open set S and applied set P,
  successful equality proves E = S union P, hence retained B outside E equals
  B minus P. Every P must be fixed in B; opening an already residual P produces
  incompatible complete maps, never an invented actual binding. Known heads are
  checked against prior facts rather than replaced. No origin/partition search.
- Strict recovery passed (`/tmp/ama-t177-recovery.log`, exit 0). Full application
  parent passed twenty named cases (`/tmp/ama-t177-applications-parent.log`, exit
  0): nested origin recovery, propagation to inner RHS results, existing residuals,
  and retained-capture conflict. No definitions/layouts/bodies are expanded.
- Fail-at-every-observed-allocation checks passed for source forward, recovery,
  field and producer generation, with untouched output/publication on refusal and
  counted zero-live-allocation teardown. Counter/sticky-failure checks passed
  (`/tmp/ama-t177-failures.log`, exit 0).
- Finite model checks passed: all 27 three-slot maps (absent/integer/distinct
  closed Type), all six orders and all prefix splits: 486 two-stage partitions.
  Another 27 partial actuals recover by actual source order despite reversed
  expected signature order, preserving an unused fourth fixed binding. Updated
  the second distinct Type's written syntax to name the actual prepared nominal
  operand, so the source handoff claims do not rely on a mismatched fixture RHS.
- Complete application parent passed 25 cases, including metadata-refinement
  allocation refusal (`/tmp/ama-t177-complete-parent.log`, exit 0). Full strict
  static parent then passed (`/tmp/ama-t177-static-parent.log`, exit 0); original
  inventory, signature, binding, native, term and constraint consumers remain
  valid after the shared ordinal repair.
- Added a final actual-source composition for separate input/output counters:
  a skipped output invokes producer inference even inside a static result
  definition; a skipped input obtains a distinct scoped producer hole; independent
  type evidence closes the producer without publishing an output definition.
  Strict final source leaf passed all three cases
  (`/tmp/ama-t177-source-output.log`, exit 0). Registry now names 26 cases.
- Root `make` passed (`/tmp/ama-t177-build.log`, exit 0). Complete documentation
  gate passed 2557 ready inputs and zero debt (`/tmp/ama-t177-docs-complete.log`,
  exit 0); the final source case is also in the refreshed per-file registry.
  Markdown audit passed 211 documents, 2907 local links, 214 rules and 108 unchanged
  grammar productions (`/tmp/ama-t177-markdown.log`, exit 0).
- All 26 application cases passed strict AddressSanitizer/UBSan
  (`/tmp/ama-t177-sanitizers.log`, exit 0). LeakSanitizer is disabled under the
  previously established container ptrace restriction; every fixture/refusal
  still asserts counted zero-live-allocation teardown independently.
- First full Doxygen generation passed (`/tmp/ama-t177-doxygen.log`, exit 0):
  2557 ready inputs/zero debt, 1928 maintained file pages, 7409 interfaces,
  423 actual destinations and 6068 generated pages. Actual editor navigation
  `code --goto /home/u/repos/ama/spec/language/templates.md:1311` passed
  (`/tmp/ama-t177-editor-navigation.log`, exit 0); this records command dispatch,
  not a claim of an observed manual GUI click.
- Full accepted record is archived in DONE. Final archive-input Doxygen refresh:
  `/tmp/ama-t177-doxygen-archive.log`; collect completion before explicit staging
  and the per-task commit. No functional jobs remain. Next ready task is T178.
  No specification change, semantic ambiguity or executable support claim.
- Initial archive refresh passed with the same counts
  (`/tmp/ama-t177-doxygen-archive-initial.log`, exit 0). Final registry review
  removed identity/ordinal-only cases from application-owner coverage: those
  cases use term/inventory fixtures, while actual application cases cover the
  application owner and its common fixture. All 26 remain in their true parent
  dispatcher and originating file mappings. Regenerate final documentation from
  this more precise registry before committing; no functional change or retest
  is needed for that evidence-only correction.

## Reproduction

- Strict parent: `make -C tests/unit/semantic/static C_COMPILER_FLAGS='-O0 -g
  -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`.
- Final source delta: the same flags with
  `make -C tests/unit/semantic/static/applications/source`.
- Sanitized applications: `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
  UBSAN_OPTIONS=halt_on_error=1 make -C tests/unit/semantic/static/applications
  C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic
  -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined
  -fno-omit-frame-pointer'`.
- Build `make`; complete per-file/header checks `make docs-complete`; actual
  generated reference/link checks `make docs`. Markdown audit is the recorded
  `/tmp/ama-t168-doc-check.py`; its evidence supplements the maintained gates.

Compiler/build/test commands above each had their exit status collected, not
inferred from log tails. Global executable conformance is scheduled in T196;
this component's acceptance does not refresh that evidence or imply template
syntax is already admitted through the complete compiler frontend.
