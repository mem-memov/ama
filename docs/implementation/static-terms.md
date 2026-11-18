# Immutable scoped static terms

T175 was accepted on 2026-11-18 against input 5c00e82. T172 supplies lexical identities and signatures;
T173 supplies immutable foreign interface/request records. This task represents
terms and environments; it does not apply constructors (T177), solve equations
(T176), expand definitions (T178/T184), allocate runtime instances or emit code.
Authority: [TPL-05](../../spec/language/templates.md#tpl-05),
[TPL-08](../../spec/language/templates.md#tpl-08),
[TPL-10](../../spec/language/templates.md#tpl-10).

## Algebra and ownership contract

Use a finite immutable graph with two value forms: scoped variables and nominal
records. A nominal record retains the exact original module, every fixed original
formal/value pair in original declaration order, and remaining original residual
binders. Its category is Template exactly when residual binders remain; otherwise
it is Type, which may still depend symbolically on surrounding variables. Do not
unfold module bodies/layouts or drop unused captures. Residual binders are local
constructor declarations, not free inference variables and not missing values.

Variable roles remain explicit: original owner formals, derived static results/
aliases, field inference and producer-output inference. Original formals and
derived names reference exact T172 symbols in an immutable environment. Inference
identities retain the owner environment, actual lexical declaration/expression
and original residual formal. Repeated lookup of the same identity shares a term;
different declaration/expression occurrences do not. Deferred derived metadata
remains explicit and is never assigned Type by default.

An environment describes a definition or an independently supplied normalized
owner Type. Definition environments retain free scoped parameter references;
application environments map original formals to their supplied terms. Symbolic
owners remain symbolic rather than choosing a runtime sample. Later substitution
and specialization orchestration normalize supplied owner terms before treating
inference variables as belonging to a concrete canonical specialization. Source
application provenance is separate from value identity: equal values can have
multiple different application/ancestry records, all of which later guards need.

Structural term equality compares original origins, complete normalized captures
and residual formal identities; variable identity is scoped, not name-based and
not a claim about its future solution. Equation solving/occurs checking belongs
to T176. Closedness distinguishes known partial constructors (locally bound
residuals are allowed) from unsolved free variables. It is not a layout or source
specialization-creation-path check. Signature metadata obligations remain owned
by their existing analysis stages rather than being treated as wildcards.

One term owner borrows a valid StaticInventory/Project and immutable binding
plans used by application records. Arena ownership covers partial construction;
no variable solution mutates the term graph. Constructors may reference only
already constructed values, making value construction acyclic. Module reference
cycles remain outside this graph. No runtime state belongs in any record.

## Planned steps and independent partitions

1. Implement/test owner lifetime, exact definition/application environments and
   scoped symbol/inference identity. Check same spelling in distinct modules,
   repeated uses, independent fields/producer expressions, deferred metadata,
   invalid foreign identities and every actual allocation refusal.
2. Build normalized nominal records from already supplied original captures,
   retaining order, unused values and explicit residual binders. Intern finite
   graphs and cache closedness without C recursion or source-definition expansion.
   Check reordered input records, distinct origins/captures, symbolic versus
   closed values, known partial constructors, deep/shared graphs and failures.
3. Import immutable application/input records from actual T173 requests and
   explicitly prepared value terms. Keep skips and unknown actual origins as
   records for T177/T179, not fresh guessed types. Separate provenance from value
   identity and preserve original binding IDs/order, source spans and owner scope.
4. Run narrow/component compositions, exact invariants/cost arguments, relevant
   parent regressions and sanitizers; register actual named per-file/header
   evidence, validate source/local/generated links, archive and commit T175.

No specification edit is authorized or required. Ordinary representation choices
must preserve these identities; any actual missing observable language rule
uses the AMBIGUITIES procedure before implementing dependent behavior.

## Construction invariant and cost

The store is a canonical finite DAG, not a mutable equation graph. Every variable
key is interned by its complete role/environment/symbol/occurrence tuple. Every
nominal uses an accepted original module and distinct registered original formal
keys, and references only previously published store-owned child terms. Before
publication, original declaration order partitions all input formals into captures
and residuals; all supplied captures survive, including unused ones. No source
body is followed. Application category checking remains T177's responsibility;
this representation admits symbolic/deferred operands without solving them.

Induction over publication order proves uniqueness: variable key equality is
exact; nominal keys agree exactly when origin, capture key set and canonical child
pointers agree. The residual complement is determined by that map. Thus equal
records intern to the same pointer, regardless of supplied array order or first
source site. This is structural equality, not equivalence after future solving.
Groundness is false for a variable, and the conjunction of captured children's
groundness for a nominal. A closed partial constructor remains ground because its
residuals are local binders. Empty conjunction is true. Membership queries check
exact published handles before dereferencing foreign records. No hash collision,
name search, expansion budget, recursive C walk or runtime default participates.

Let V be published values, S registered source symbols, M registered modules,
E environments, P original formals and C supplied captures. Membership is linear
in its owning registry. An environment lookup costs O(M + V + E); a variable
lookup O(E + S + V), plus source occurrence membership for inference sites.
Nominal input validation costs O(M + C(S + V + C)); canonical search costs
O(V(1 + C²)); ordered preparation costs O(P(1 + C)), with O(P) new records.
Equality/groundness queries cost O(V) for checked handles, then O(1). There is no
recursion-depth dependence, including destruction. Input counts are bounded by
accepted origin/request counts; public arena counters are checked against
SIZE_MAX before incrementing. Partial allocation remains in the arena; the
first error is sticky and no failed operation publishes an output or changes a
previous immutable record.

Application imports borrow a published healthy T173 static request, registered
in the supplied lexical environment. They require a bijection from explicit
nonomitted inputs to owned prepared terms. Source order, original actual slots,
unknown slots, skips and output syntax survive; prepared array order is irrelevant.
Each import creates a distinct provenance record, even for identical values.
Neither omitted inputs nor pending actual origins generate guessed inference
terms. With I source inputs and A supplied operands, validation/preparation costs
O(I A + A(V + A)) plus plan/environment/source membership; O(I) new records.
The binding plan must outlive these borrowed application records.

## Step evidence and continuation

- Owner/environment/scoped variable operations and independent identity cases
  passed strict C11 with warnings as errors (`make -C
  tests/unit/semantic/static/terms`, log `/tmp/ama-t175-identity-expanded.log`).
  This includes independent same-spelled modules, repeated identities, distinct
  field sites, producer roles, deferred aliases, copied-record refusal and every
  observed environment/variable allocation refusal, with counted full teardown.
- Initial normalized nominal cases passed the same strict command (log
  `/tmp/ama-t175-terms.log`): reordered maps, unused captures, partial residuals,
  applied owner formal lookup, symbolic closedness, 512 nested immutable records,
  duplicate key refusal and every observed nominal allocation refusal.
- The first expanded composition build refused an unused shared fixture helper
  under `-Werror`; the actual source-span identity assertion now consumes it.
  The next run reached the application fixture and correctly rejected a standalone
  static expression with no output actions. The allocation scenario now includes
  an explicit output skip; no parser/production rule was weakened. Both failed
  attempts remain recorded in `/tmp/ama-t175-composition.log` and
  `/tmp/ama-t175-components-final.log`.
- The strict term parent rerun passed all eight then-current cases (exit 0,
  `/tmp/ama-t175-components-rerun.log`). Added a ninth independent case for
  reordered prepared operands, foreign application/scope refusal and saturated
  application counts. All nine cases are mapped to their actual maintained
  source/header/build inputs in the ready registry.
- Full strict static parent passed all 76 independent cases, including nine T175
  cases, with warnings as errors (exit 0, `/tmp/ama-t175-static-parent.log`).
  Root `make` passed (exit 0, `/tmp/ama-t175-build.log`). Complete per-file and
  standalone-header gate passed: 2481 ready inputs, zero debt (exit 0,
  `/tmp/ama-t175-docs-complete.log`). Markdown audit passed: 209 documents,
  2894 local links, 214 canonical rules, 108 unchanged EBNF productions.
- ASan/UBSan passed all nine term cases (exit 0,
  `/tmp/ama-t175-sanitizers.log`) with the documented `detect_leaks=0` container
  restriction. Counted fixtures additionally assert complete teardown.
- First Doxygen generation refused four test `main` functions without their own
  extracted documentation (`/tmp/ama-t175-doxygen.log`). Each now has an explicit
  entry contract. File purpose blocks also enumerate all canonical rules used
  by extracted member contracts, preserving originating-page navigation.
- Doxygen rerun passed (exit 0, `/tmp/ama-t175-doxygen-rerun.log`): 1870
  maintained file pages, 7160 documented interfaces, 421 exact canonical
  destinations and 5896 generated pages. Actual editor navigation
  `code --goto /home/u/repos/ama/spec/language/templates.md:1026` passed (exit 0);
  the generated HTML validator checks real originating pages and destinations.
- Final review clarified file-level rule explanations without changing algorithms.
  T175 is accepted and its full task record is archived in DONE. The archive-input
  documentation refresh uses `/tmp/ama-t175-doxygen-archive.log`; collect that
  result before the per-task commit. No functional jobs remain. Next task T176
  consumes this finite algebra for equations; it must not reinterpret pointer
  identity as equality after solving. No spec change or executable support claim.

## Reproduction and coverage

All commands run from the repository root. Strict parent acceptance:

```sh
make -C tests/unit/semantic/static C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make -C tests/unit/semantic/static/terms C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'
make
make docs-complete
make docs
```

The nine named cases and all 25 new maintained source/header/test/build files are
mapped explicitly in [ready.json](documentation/ready.json); headers have actual
standalone probes. `identity/main.c` owns scope, producer/derived metadata and
owner/variable refusal partitions; `nominal/main.c` owns canonical maps, deep
values and nominal allocation refusal; `application/main.c` composes actual T173
requests with skips, pending origins, reordered operands, provenance and refusal;
`failures/main.c` checks copied handles and saturated counters. The recursive
Makefiles execute these independent leaves; counted source/project/arena owners
assert zero remaining allocations on success and each refused preparation.
Full compiler-template conformance/release gates remain T196–T198; this task
neither refreshes old conformance hashes nor claims executable integration.
