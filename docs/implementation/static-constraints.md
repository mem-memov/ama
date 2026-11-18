# Finite static constraint solving

T176 was accepted on 2026-11-18 against input 5d735be; T175 is committed and supplies the finite
canonical term algebra. Authority: [TPL-03](../../spec/language/templates.md#tpl-03),
[TPL-07](../../spec/language/templates.md#tpl-07),
[TPL-10](../../spec/language/templates.md#tpl-10),
[ARCH-05](../../spec/implementation/compiler.md#arch-05).
No specification changes are authorized. Constructor application/recovery belongs
to T177; definition/output expansion and creation guards belong to T178/T179.

## Contract and representation

Borrow one mutable term store and its immutable source inventory. Own equations,
variable substitution records, prepared metadata facts and explicit required values
in a separate arena; query/comparison work uses immediately released local arenas. Existing term payloads and source
records remain immutable; normalization may append canonical terms through T175.
Destroy the solver before its borrowed term/source owners. All refused partial
work remains arena-owned; the first failure is sticky and diagnostics allocate
no additional storage. A failed solver is destructible, not resumable. No runtime
state, body expansion, search for an origin or execution occurs. Callers/allocator
callbacks must not mutate borrowed graphs or reenter an operation. The borrowed
term owner remains healthy for valid calls; normalization propagates its failures.

Original source symbols remain distinct from solutions. A substitution maps a
scoped variable to another term; follow variable links iteratively. Equating two
unbound variables links the greater original term ordinal to the lesser, avoiding
alias cycles. Before binding a variable to a nominal, an iterative visited walk
through current substitutions and fixed capture values performs the occurs check.
Residual formals are local binders and do not enter that walk as free variables.
Nominal reference/layout cycles are outside this finite capture graph.

Store each distinct equation pair once, including generated corresponding capture
pairs, with its originating source site/parent. Retain original equations after
refinement: every pass rechecks each original variable's declared category and
signature against its current representative. This prevents an alias to a
currently deferred variable from discarding a known Type/Template restriction.
Nominal origins and complete original capture/residual key sets must agree before
child pairs are queued. Different origins cannot agree through equal layouts.

Reuse the existing exact iterative named-signature comparator with an independent
arena entry point. Its existing inventory-facing operation retains its contract;
new consumers must not mutate an accepted borrowed inventory to obtain scratch.
Known mismatches win over pending entries. Template signatures compare complete
foreign names and recursive categories independently of declaration order.
A deferred category/signature is retained as an obligation, never a wildcard
certificate. Independently supplied fully resolved metadata facts may refine a
term's deferred source metadata only after preserving every already known name
and category; facts do not replace a conflicting known declaration. There is no
selection of an expected signature merely from the first supplied actual.

The caller explicitly marks values required to close. A stable equation graph may
contain unrequired free variables; required unresolved values and pending metadata
return Pending during incremental solving. Additional finite equations/metadata
can resume the same healthy owner, including later concrete anchors. Finalization
reports underdetermination instead of inventing a solution. Required closedness
walks current substitutions and captured values, not runtime samples. Resolving a
closed result reconstructs canonical nominal values bottom-up without C recursion;
this permits comparison of normalized solutions across equation permutations.

## Invariant and progress obligation

Every published variable link preserves all original equations and adds strictly
more equality information; it never overwrites a solved binding. Occurs checking
keeps the substitution/capture graph acyclic. Each distinct pair is queued once
and checked without unfolding definitions. A pass repeats only after an actual
new variable link, equation pair or validated metadata fact. With the caller's
finite term/constraint input, at most V variable links and V² distinct pairs can
be introduced. A pass with no refinement terminates, including unresolved input.
No arbitrary iteration/depth/time budget is permitted. Failure of uniqueness is
reported, not resolved by traversal order. Signature work memoizes exact pairs
and traverses finite accepted signatures; no recursive C activation is needed.
Actual per-operation costs and allocation bounds will accompany implementation.

## Ordered steps and independent acceptance partitions

1. Extract the existing signature comparator's arena-owned read-only entry point;
   preserve all inventory callers and tests. Define solver owner, checked finite
   equation/requirement records and first-error interfaces.
2. Implement exact variable refinement, nominal decomposition, original metadata
   rechecking and iterative occurs checking. Test identity, distinct nominal
   origins, conflicting captures, wrong categories, anchored/unanchored alias
   cycles, direct/indirect infinite terms and residual binders.
3. Add explicit pending metadata refinement, required closure and bottom-up
   canonical result reconstruction. Test later anchors, recursive/reordered
   signatures, pending versus known conflict, deep/shared graphs and actual
   T173/T175 handoffs. Enumerate finite equation permutations against an
   independent equivalence-class model; no implementation-derived expectations.
4. Refuse every actual allocation and bounded-counter edge, retain source/parent
   diagnostics and prove cleanup. Run strict component/affected parent tests,
   sanitizers, root build, standalone headers and documentation gates. Register
   every maintained file against actual named cases. Archive and commit T176 only
   after all required results are collected.

## Step results and continuation

- Extracted `static_signature_compare_arena` over the unchanged exact iterative
  signature algorithm; the old inventory wrapper preserves its sticky status/
  diagnostic contract. Two independent tests passed strict C11 warnings as
  errors (exit 0, `/tmp/ama-t176-signature-scratch.log`), including recursive
  reordered signatures, each actual scratch refusal, unchanged accepted inventory
  arena/status and complete counted teardown.
- Added separate solver owner, first-error diagnostics, immutable original/
  derived equation records and explicit required-value records. Symmetric pairs
  deduplicate by original term ordinals. Checked exact term membership precedes
  dereference; saturation/refusal never publishes a partial record. No solving
  or executable acceptance is yet implemented or claimed.
- The strict owner/record parent passed five cases (exit 0,
  `/tmp/ama-t176-records.log`). The initial equality parent passed eleven cases
  (exit 0, `/tmp/ama-t176-solve-initial.log`): unanchored/later-anchored equality
  cycles, complete/partial nominal decomposition, origin conflicts, inconsistent
  repeated bindings, direct/indirect occurs failures, deferred alias classes and
  recursive reordered/mismatching signatures. Known restrictions are checked
  across every active variable in a class, not just its ordinal representative.
- Added independent fully resolved metadata refinements, required finalization
  and iterative shared result reconstruction. A fully described variable still
  has no nominal origin. Closed reconstruction memoizes resolved vertices and
  reuses T175 canonical records, preserving all original captures/residuals.
  The strict parent passed all fifteen current cases (exit 0,
  `/tmp/ama-t176-closure.log`), including Pending-to-resolved metadata, explicit
  final underdetermination and a 64-level shared diamond whose result equals
  independently constructed ground structure without exponential expansion.
- Extended strict parent passed the nominal-reference-cycle control and all 720
  independent three-edge/five-vertex graph/order scenarios. The resource leaf
  then correctly rejected its inconsistent baseline: it compared a three-level
  symbolic nominal with a one-level concrete nominal after anchoring. The
  fixture now compares matching one-level structures, preserving the intended
  structural-decomposition test; no expected failure was weakened. Failed run:
  `/tmp/ama-t176-model-failures.log` (exit 2).
- The next expanded parent passed the model/resource leaves; composition refused
  a missing `<stdint.h>` include and an unused alternative fixture setup under
  `-Werror` (`/tmp/ama-t176-expanded.log`, exit 2). Added the explicit include;
  the genuinely optional shared `setup_requests` fixture is now `static inline`.
  Source helpers still load actual project/standard roots rather than fake types.
- Added ordinal-indexed variable lookup with checked geometric growth, cached
  class representatives and exact immutable signature-pair results. Pending
  results stay Pending; a new independent description selects a new immutable
  signature pointer. Earlier accepted inventories are never modified. Diagnostics
  now distinguish original module/formal definitions from an interning site's
  first application. The strict indexed parent passed
  `/tmp/ama-t176-indexed.log` (exit 0).
- Query-local occurs/closure/materialization work is released immediately, and
  unchanged solves reuse their checked outcome. New equations, requirements,
  actual bindings and metadata facts invalidate that outcome. A hundred repeated
  shared-graph value queries retain exactly the same counted live allocation
  total. Strict query-lifetime parent passed
  `/tmp/ama-t176-query-lifetime.log` (exit 0).
- Full strict static parent passed its then-current 99 cases, including 23 T176
  cases and every existing scope/binding/native/term leaf (exit 0,
  `/tmp/ama-t176-static-parent.log`). Root build and complete documentation gate
  passed `/tmp/ama-t176-build.log` and `/tmp/ama-t176-docs-complete.log` (2518 ready
  inputs, zero debt). Added one final closure case: a ground constructor may still
  need resolved signature metadata; both required-value and final signature-only
  underdetermination paths are checked. Strict closure leaf passed
  `/tmp/ama-t176-final-closure.log` (exit 0).
- Final comparison scratch also releases immediately after computing its cached
  result. All 24 current constraint cases passed strict C11 ASan/UBSan (exit 0,
  `/tmp/ama-t176-sanitizers.log`). This includes every observed allocation refusal
  across seven operation partitions, exact counter bounds, both result categories,
  720 independent graph/order models, nominal-reference cycles, Type/Template
  occurs failures, metadata refinement and actual T173/T175/standard-type handoff.
  LeakSanitizer remains disabled under the established container ptrace restriction;
  counted owner/refusal teardown and query-lifetime checks are separate evidence.

## Why the operations are exact

Mathematical notation below is not Ama syntax. A substitution maps scoped free
variables to terms. A nominal is `(origin, fixed original map, local residuals)`.
For `X = T`, after following existing links, the occurs check requires that X is
absent from T's free captured values. Assigning X to T preserves exactly the
finite solutions of that equation; every solution already has to make both sides
equal. A variable-variable link chooses an ordinal representative, never a type.
For two nominals, unequal origins or original binding sets cannot have equal
identity. Equal keys reduce equality exactly to corresponding capture equations.
Residual declarations are not free variables. Hence a source `Node` containing a
reference to `Node` is outside occurs checking, while `X = Box(X)` is infinite.

Active alias classes retain every original declared restriction. Cached
representatives are recomputed after each structural pass; all class declarations
are compared with their representative and one another. A deferred alias cannot
hide a Type-versus-Template contradiction. Signatures match complete foreign name
sets and recursive categories; pointer memoization caches this exact result,
not nominal constructor equality. Pending entries cannot hide known mismatches.
An independent fully known description can fill unknown metadata only while
preserving every known name/category. It does not provide a nominal origin.

Each structural step preserves the same solution set, so processing independent
equations in another order preserves normalized closed values or rejection,
assuming successful allocation. This is completeness for the stated finite
nominal structural rules; it is not constructor search, higher-order inversion,
inhabitance checking or executable admission. An unresolved required variable
cannot be filled by choosing among available modules. The caller supplies further
independent constraints or explicitly finalizes with underdetermination.
T177/T179 retain their distinct application/projection obligations.

Reconstruction memoizes each resolved vertex, completes its captured children
before its parent, and asks T175 to intern the original key with substituted
values. Thus shared paths do not expand exponentially, unused captures survive,
and equal normal forms have one identity without merging runtime state. Ground
records contain no variables and can be reused directly. Independently refined
metadata transfers to an equivalent reconstructed record and consistency is
checked again before publishing the value. A selected value query certifies only
that value; unrelated required producer inputs still need their own closure.

## Progress, cost and failure boundaries

Let N be term vertices, A captured edges, U active variables, E distinct unordered
equation pairs, R explicit requirements, M independent descriptions, and C cached
immutable signature pairs. Variable-only links decrease original ordinal; links
to nominals pass the occurs check. No solved binding is overwritten. Each repeated
pass adds a variable link or a distinct pair. There are at most U links and
N(N+1)/2 pairs, so a stable unresolved pass terminates without a budget.
Metadata refinement publishes at most one fully known description per term.
No semantic node is manufactured repeatedly just to keep iteration alive.

Ordinal lookup is O(1), representative traversal O(U), and retained geometric
lookup storage O(N), including old arena-owned arrays. Adding an equation costs
O(N + E) for membership/deduplication; requirements cost O(N + R). An unchanged
solve costs O(1). Conservatively, each refining pass costs
`O(E(U + M + C + K E) + U²(1 + M + C))`, plus new recursive signature work,
where K bounds captures per nominal. This deliberately includes linear pair/fact
lookups; it makes no unproved near-linear complexity claim. Signature pairs are
computed once and memoized symmetrically; their iterative recursive comparator
visits finite signature pairs and scans complete name sets. For P recursive pairs,
L maximum entries and B maximum name bytes, its conservative time is
`O(P + P L² B + P²(1 + L))`, with O(P) local scratch.

A graph query takes `O((N + A)(U + N) + N(M + C))`, plus any newly computed
signature comparisons. Reconstruction visits each vertex/capture once at the
semantic level; memo lookup is linear, and canonical record construction retains
T175's explicit costs. No C recursion depends on graph/signature depth.
Persistent solver storage is O(E + U + N + M + C); temporary graph/reconstruction/
comparison storage is O(N + A + P) and is released on every exit. Newly normalized
canonical values remain owned by the term store. All increment/multiplication/
growth boundaries are checked before publication.

A failed solve may already have refined mutable solver links or appended valid
canonical child values; no whole-solve rollback is promised. The failed owner is
only destructible. Source and prior immutable value payloads remain unchanged;
output values/outcomes are untouched on failure. First-error diagnostics allocate
nothing and retain actual application/definition source spans. Source parents,
original equations and distinct T175 use records remain available to later
chain diagnostics; interning/deduplication does not replace those provenance
relations.

## Acceptance commands and current checkpoint

Commands run from the repository root:

```sh
make -C tests/unit/semantic/static C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'
make -C tests/unit/semantic/static/constraints/closure C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make -C tests/unit/semantic/static/constraints C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'
make
make docs-complete
make docs
```

[ready.json](documentation/ready.json) maps all 37 new maintained source/header/
test/build files, the existing signature API and shared request/parent consumers
to actual named cases and standalone header commands. Final root compilation
passed `/tmp/ama-t176-build-final.log` (exit 0). Final first Doxygen pass passed
`/tmp/ama-t176-doxygen.log` (exit 0): 2518 ready inputs/zero debt, 1899 maintained
file pages, 7283 documented interfaces, 422 exact canonical destinations and
5985 generated pages. Markdown audit passed: 210 documents, 2901 local links,
214 canonical rules, 108 unchanged EBNF productions. Actual editor navigation
`code --goto /home/u/repos/ama/spec/language/templates.md:1311` passed (exit 0).

T176 is accepted and its complete task record is archived in DONE. No functional
jobs remain. The archive-input reference refresh uses
`/tmp/ama-t176-doxygen-archive.log`; collect its actual result before the task
commit. Next ready task is T177. No specification change, executable specialization
support or full-sprint conformance refresh is claimed.
