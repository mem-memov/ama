# Guarded specialization closure

[T184](../../DONE.md#t184) establishes this compiler-phase contract from
`1b25657`. Authority is
[TPL-06](../../spec/language/templates.md#tpl-06),
[TPL-10](../../spec/language/templates.md#tpl-10),
[TPL-11](../../spec/language/templates.md#tpl-11) and
[COMP-02](../../spec/implementation/compiler.md#comp-02).
This note records implementation boundaries, not new language rules.

## Ordered work

1. Compose ordinary equations under an existing actual incoming context.
   Continue across unknown fields and operation targets; a later independent
   source equation can refine an earlier operation on a subsequent pass.
2. Enumerate every required Type and its real provenance. Keep reuse of supplied
   Types separate from constructor creation edges; retain validation dependencies
   in both cases. Never manufacture a selected root for an ordinary call.
3. Alternate ordinary equations, graph equations, independent exports, guarded
   demands and exact key normalization until no new semantic fact is available.
   Reject outstanding required unknowns; do not use iteration budgets.
4. Integrate owned closure into the real frontend, including check-only and
   multiple-root consumers. Verify allocation-prefix teardown, root-order
   equivalence, changing-key refusal, equal-key cycles and cache paths.

## Ordinary pass contract

`src/semantic/static/closure/ordinary/` borrows a healthy connection owner and
one original registered ordinary `StaticResultContext`. It owns no arena and
returns a report only after every child operation succeeds. Partial equations
and metadata remain owned by existing children on refusal. Diagnostics preserve
those children's original source locations. Copied or foreign contexts fail.

The pass attaches the context, submits annotation facts, default and assignment
facts, foreach equalities, and every source-owned call binding, including nested
and unreachable calls. It then merges proven congruent owner fields and validates
all actual field annotations and field-Type closure. It creates no callee root,
completes no candidate-local validation, and does not admit executable code.

Readiness has separate components: complete field Types and outstanding transfer
work. Literal capability checks may remain outstanding even with complete Types;
unknown call origins likewise remain outstanding. The whole scheduler must
separately enumerate actual callee/dependency requirements. A successful ordinary
pass alone is never whole-program closure.

Correctness follows composition of the child contracts in dependency order.
Pending children do not short-circuit independent evidence; failures do. Original
runtime events enumerate calls exactly once per pass, avoiding traversal through
non-owning AST control aliases. Each loop visits a finite retained source list;
the pass introduces no retry loop or speculative body expansion. Cost is the sum
of child component costs plus linear scans of retained events and typed fields.
Progress uses actual child facts/proofs and the existing publication progress
record; allocation count and retry count are excluded.

## Attributed reuse of a known Type

A required supplied Type needs its own definition checked, but reusing it does
not create a constructor edge from its consumer. Represent that requirement as
an immutable guard path with `required_from` pointing to the actual consumer,
original consumer source span and a fully normalized ordinary Type owner.
Its creation parent is null: it starts traversal of that Type's own outgoing
creation applications, not a newly selected compilation root. Selected roots
have neither parent nor `required_from`. Registry validation follows both kinds
of attribution; creation reachability follows only creation parents.

Only an already registered actual consumer and its original source span may
request this path. Unknown Types remain Pending without allocating another scope;
residual Templates and static producers cannot become ordinary required Types.
Repeated exact consumer/site/complete-Type requests reuse the same path. All
outgoing applications and cache checks of its target still run normally.
This distinction realizes TPL-11's separate reuse and validation relations rather
than inventing another creation edge or silently skipping a supplied Type.

## Evidence and continuation

The first ordinary pass has six named composition cases. Strict O0/Werror
`make -C tests/unit/semantic/static/closure/ordinary` passed, including every
allocation failure of the composed operation and zero-live-allocation teardown.
ASan/UBSan O1 also passed (`/tmp/ama-t184-ordinary-sanitizers.log`, collected
session 10750). LSan is unavailable in this environment; counted teardown is
separate evidence. The first build exposed two unused fixture helpers: the leaf
now includes the narrower connection fixture and defines its own needed field
expectation. An unanchored-literal fixture initially omitted the integer source
required by its shared fixture; an independent integer reference now loads that
metadata without constraining the unknown literal's Type. Production expectations
were unchanged. Logs are `/tmp/ama-t184-ordinary{,-failures}.log`.

The required-Type leaf has five named cases. Strict O0/Werror passed through
`make -C tests/unit/semantic/static/closure/requirements`, including same-key
outgoing recursion, changing-key refusal, unknown suspension, invalid provenance,
separate registry validation dependencies and every preparation-allocation
refusal. The passing final strict log is
`/tmp/ama-t184-requirements-outgoing.log` (collected session 20013).

Collected parent results: strict O0/Werror guard parent (session 27763),
registry parent (79611), and required-Type ASan/UBSan leaf (6819) all exited 0.
Their logs are `/tmp/ama-t184-{guards,registry,requirements-sanitizers}.log`.
The corrected docs-check also exited 0 (61470), reporting 2894 maintained inputs
and zero debts; both new ordinary headers compiled independently. Its log is
`/tmp/ama-t184-current-docs-check.log`. These are component evidence, not full
language acceptance or a generated-Doxygen-render claim.

Next implement the whole-worklist owner. Retain each actual incoming context;
expand one representative per closed canonical candidate, preserving all actual
creation paths and registry dependencies. Pending entries may merge only by exact
environment identity, not source name or assumed equal keys. Equate each original
scope with its canonical environment through the existing registry operation.
After body equations, require every field Type and immediate call target through
attributed known-Type paths, and independently close constant namespaces. The
fixed point must count source equations, category/requirements, incoming/creation
paths, known keys and first namespace proofs; scratch and retry activity do not
count. Returning to a closed candidate skips repeated body expansion while the
guard graph still checks every incoming/outgoing application.

The worklist owner now retains original actual ordinary contexts, registers
canonical variable equalities and chooses representatives by exact candidate
identity (or exact environment before closure). Static producer contexts stay
in graph publication. The collection operation does not run bodies or claim
candidate-local admission. A finite scan retains U contexts; representative
selection costs O(U squared), plus the documented guard/registry/typed-scope
child costs. Entry storage is O(U); incoming use and runtime identities remain
distinct. Representative selection terminates by a strictly bounded earlier-entry
scan; it follows no recursive Type definition.

Five worklist cases pass strict O0/Werror, including every collection allocation
refusal (`/tmp/ama-t184-worklist-formals.log`, collected session 82584). Two initial
test fixture attempts incorrectly used a section-local AST next link and a runtime
receiver-hole API for explicit declarations; corrected fixtures use the original
statement catalog and two independent declared Type formals. Production contracts
and expected sharing/refusal behavior were unchanged.

A subsequent strict parent and sanitizer launch incorrectly overlapped the
worklist output directory. Both exited 2; neither result is acceptance evidence
(`/tmp/ama-t184-{closure-parent,worklist-sanitizers}.log`, sessions 15621/92659
collected). The worklist directory was cleaned. The clean strict parent rerun exited 0
(session 5685, `/tmp/ama-t184-closure-parent-serial.log`), covering all three leaves
and 16 named cases. The sequential worklist sanitizer rerun also exited 0
(session 12884, `/tmp/ama-t184-worklist-sanitizers-serial.log`). Both corrected
runs are accepted within their recorded component/source boundaries.

## Whole-worklist refinement

The scheduler now collects actual contexts, canonicalizes scoped variables,
processes only exact-key/environment representatives, submits ordinary equations
and requires every field element and immediate call target Type. Deferred direct
constant transfers and saved reads independently determine namespaces before
receiver equalities. Static exports retain their supplying publication; no static
producer enters ordinary runtime-body work. Native capability checks, default
execution, constant value construction and executable candidate admission remain
later tasks.

A step snapshots exact equation/category/requirement and publication facts,
required contexts/paths/candidates and first namespace proof receipts. Newly
required contexts force another pass. Refinement repeats only after actual facts
change; a stable step requires all retained keys and solver Type obligations to
close. The shared registry's local_started/local_complete certificates stay
unclaimed for T185. A new resolve call rechecks current evidence rather than
trusting a stale closed flag.

The finite-expansion argument combines TPL-11 guard suspension before unknown
repeated-origin expansion, rejection of reachable unequal closed keys, and one
body representative per complete canonical candidate. Distinct known Type reuse
starts its own creation traversal with actual validation attribution. Original
finite written applications bound each representative's outgoing work; source
operations and equation/proof requests are interned by original scoped identity.
A retry cannot create a fresh anonymous application. Within that finite required
graph, facts grow monotonically. Thus an unchanged evidence snapshot is the
fixed point; neither allocation activity nor a numeric iteration budget can
justify another pass. Per-pass cost is the sum of collection/ordinary/guard/
namespace child costs and linear retained-entry/proof scans. Total cost is that
sum over the finite number of evidence-changing passes; no linear-time claim is
made for exact graph or structural unification work.

Ten scheduler cases passed strict O0/Werror (session 36159,
`/tmp/ama-t184-scheduler-expanded.log`). They cover generic input/result equations,
output inference from callee bodies, direct and saved constants, bad required
bodies behind empty references, unanchored literals, exact-key reference cycles,
independent roots in both discovery orders, static exports without runtime
producer instances and changing-key creation refusal. Every one of **450**
allocations in the composed constant-inference closure was refused in turn;
each refusal preserved diagnostics, published no closed result and unwound all
counted owners. Same-key resolution is idempotent in retained entry/path counts.
The earlier five-case scheduler build also passed (session 35939); this does not
replace the expanded acceptance.

The initial scheduler sanitizer leaf passed (session 84283). The allocation
matrix was then extended across four complete shapes: constants **450**, generic
calls **1084**, same-key reference cycles **804**, and static export publication
**410** positions. All **2748** refusal positions pass both strict O0/Werror and
ASan/UBSan O1 with counted zero-live-allocation teardown (sessions 96983/33646;
`/tmp/ama-t184-scheduler-four-phases{,-sanitizers}.log`). Original 450-position
evidence above is historical scope, not an additional disjoint count.

## Compiler owner and frontend handoff

`src/compiler/specializations/` owns bindings, identity names, terms, constraints,
applications, guards, registry, results, publication, constants, connections and
worklist in dependency order. All initializers are nonallocating; one outer
initialization credit covers the completed child chain. Teardown reverses that
order before the borrowed inventory/Project expires. Source diagnostics borrow
those retained owners and never a temporary error array.

Definition-time runtime name classification visits every loaded ordinary module,
including uninstantiated open definitions. Static graphs were checked by inventory
collection. Only the explicit ordinary root list registers actual contexts; an
empty list performs lexical/definition analysis without inventing Type defaults
or executable instances. Roots preserve exact original module pointers; copied
modules and static producers cannot become ordinary roots. The closure certificate
still does not complete candidate-local ordinary checks.

Six owner cases pass strict O0/Werror (session 98914,
`/tmp/ama-t184-compiler-specializations-fixed.log`), including **471** allocation
refusal positions across the complete owned stage. Tests use the public field
catalog instead of a private runtime-field lookup; the first compile correctly
rejected that undeclared private helper. Both explicit root orders pass, and an
unused open definition with an unbound runtime name fails before root registration.

The real frontend now owns this stage after source/native admission and before
ordinary semantics/planning. It selects its original entry when no explicit
multi-root analysis was already performed. Explicit multi-root completion can
be supplied to the same finish operation. Outer destruction releases plan and
semantic borrowers before Type analysis, source inventory and project bytes.
Concrete planning still uses the existing semantic consumer; T185 owns its
specialization-aware adapter. A complete Type candidate is not an assertion that
open source syntax can already reach lowering.

The first full compiler/check leaf exposed an obsolete stage assertion: an open
required Type now correctly fails Type closure before the raw concrete consumer.
The fixture now separately asserts source collection, Type-attempt/closure and
ordinary-consumer stage prefixes. It retains a closed export case that reaches
the still active concrete-consumer guard; that guard remains T185 work. No
language expectation was weakened and no specification file changed.

The corrected compiler/check and actual frontend specialization-composition
leaves both exited 0 (sessions 72311/61885,
`/tmp/ama-t184-frontend-{stage-check,specialization-composition}.log`). The four
new frontend cases inspect closed generic candidates before ordinary admission,
complete concrete planning with both explicit root orders, the public check entry
and early definition-time rejection in an otherwise uninstantiated loaded module.
Handwritten C companions contain `#error`, so invoking C compilation during
checking would visibly fail. Counted outer destruction succeeds in every case.

The first complete compiler parent exited 2 (61097): two old tests assumed
Type/storage contradictions and static roots reached ordinary semantics. The
composition failure oracle now independently invokes the new owned Type stage
against the same immutable inventory before comparing later semantic diagnostics.
Its complete frontend allocation-prefix case explicitly observes failures inside
Type analysis as well as semantic and plan preparation. The obsolete native-policy
support file `support/concrete.c`, dedicated to the old "bridge not implemented"
assumption, was deleted with its sole case and coverage rows. Current root and
selected-export boundaries remain covered by compiler/check and the new frontend
composition cases; native own-formal policy tests remain intact.

The corrected complete compiler parent passed strict O0/Werror (61559,
`/tmp/ama-t184-compiler-parent-current-stages.log`). Complete compiler and closure
parents also passed ASan/UBSan O1 (23256/26779,
`/tmp/ama-t184-{compiler,closure}-parent-sanitizers.log`), including full frontend
failure prefixes. Source/native C was not executed by checking.

An expanded docs-check initially rejected a missing canonical rule link on a
new first-error function comment. Every new compiler operation explanation now
cites its determining canonical rule and the actual invariant/lifetime. The
corrected gate passed (60748,
`/tmp/ama-t184-rule-linked-docs-check.log`), reporting **2920** maintained inputs
and zero debts. This includes independent compilation of all new headers; actual
rendered Doxygen validation is still pending. Static-root refusal retains the
original @... marker span, now explicitly checked by the owner leaf. The raw
concrete inventory comments name T185 as the descriptor-adapter owner.

Final interface review found a selected-root gap: finish always plans the loaded
entry, so a precomputed explicit root list must contain that exact entry pointer.
The frontend now rejects an empty or entry-omitting list without selecting a
fallback root or reaching semantics/planning. Standalone definition-only checking
remains available through CompilerSpecialization. The five-case frontend
composition leaf passed the added refusal and successful handoff paths under
ASan/UBSan (91071,
`/tmp/ama-t184-frontend-required-entry-sanitizers.log`). Both missing-list variants
leave the frontend safely destructible and a later finish still refuses them.
No language rule or specification file was changed.

The first optimized whole-project attempt stopped during compiler construction
(session 30107, `/tmp/ama-t184-whole-project.log`, exit 2), so no aggregate tests
ran. GCC O2 could not establish the private required-path helper's output
assignment through its external bool-returning error reporter. The overflow
branch now records that error and explicitly returns false, making every true
helper path visibly assign its output. No warning flag was weakened or fallback
Type selected. The required-Type leaf now also forces SIZE_MAX path-count refusal
and checks unchanged outputs/OOM status; its strict O2/Werror run passed (7357,
`/tmp/ama-t184-required-path-overflow-optimized.log`).

The corrected optimized full-project `make -k -j4 test` completed with exit 2
(session 34587, `/tmp/ama-t184-whole-project-current.log`). Exactly three unhandled
assertions remained: the pre-existing conformance digest refusal and two codegen
cases asserting that nominal Type conflicts reached the old semantic phase.
Intentional failing documentation-interface compiler probes were caught by their
successful negative harnesses; they are not extra implementation regressions.
All other independent branches ran, including actual compiled execution, native/
library/editor consumers and the workspace-to-/tmp relocation checks. This is a
failed aggregate, not a green full-test claim. Specification and conformance
fingerprints remain unchanged; T196 owns the existing review refusal.

## Final interface review and regression

An isolated original-source probe reproduced a stale aggregate `closed` flag after
refusing a later phase (9706, `/tmp/ama-t184-closed-proof-before.log`, exit 2 at
`assert(!plan.closed)`). It used its own /tmp object/fixture tree while the full
suite's inputs stayed frozen. A proposed copy passed all three cases (83481,
`/tmp/ama-t184-closed-proof-proposed.log`): direct phase refusal, borrowed dependency
failure with unchanged report output, and collection followed by renewed closure.

The maintained error handlers now expire the aggregate certificate on refusal
and dependency failure. Collection also invalidates prior whole-worklist closure,
even when it finds no new facts: collection prepares work and cannot substitute
for final resolution. Immutable Type descriptors remain owned for diagnostics;
no Type identity or runtime instance changes. Only a successful final resolve sets
closed again. This follows ARCH-03's failed-owner/complete-output boundary and is
now stated in the public owner/collection contract.

The maintained scheduler has eleven cases. Its updated ASan/UBSan run passes
(73164, `/tmp/ama-t184-final-scheduler-san.log`), including the three new certificate
cases and all 2748 four-shape allocation refusal positions. The foreign test term
stays alive until owner teardown; the test does not leave borrowed stack evidence
out of scope.

A second isolated probe compared new required-field metadata with independently
run ordinary semantic stages, sharing only immutable source identities. It passed
17/0/1/2 field comparisons across the four existing complete frontend scenarios
(42255, `/tmp/ama-t184-type-projection-probe.log`). The maintained projection now
has its own cohesive `support/types.c`: it checks every required field and local,
unique binding source interval, role, exact reference/list form, declaration
identity and full nominal origin. These fixtures have no Type formals; zero
captures are explicitly asserted, not generalized to arbitrary templates. The
normal compiler composition leaf passes this oracle and its full allocation-
prefix checks under strict O2/Werror (60221,
`/tmp/ama-t184-final-compiler-projection.log`).

Scalar-library and native-layout tests retain every rejection and runtime
assertion. Their nominal-conflict cases now require failed guarded Type closure,
no later semantic initialization and TYPE_MISMATCH diagnostics, instead of the
obsolete assertion that native/ordinary semantic checking ran first. The separate
noncopyable owning-list test still expects ordinary capability checking. Complete
scalar and layout leaves pass strict O2/Werror (12413/68218,
`/tmp/ama-t184-final-{scalar-stage,layout-stage}.log`), including actual generated
C execution and all negative namespace/layout/nominal cases.

## Final scoped acceptance

The final optimized complete suite finished with exit 2 solely from the unchanged
pre-existing conformance specification-digest review refusal (30152,
`/tmp/ama-t184-final-whole-project.log`). Every other independent branch passed,
including the corrected scalar/layout cases, generated C execution, complete
compiler/closure tests and native/library/editor/relocation consumers. The run
used `make -k -j4 test` with `-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L`. All 3135 recorded operational inputs stayed identical;
`/tmp/ama-t184-final-input-verification.json` retains that check. This failed
aggregate is not reported as successful language-wide conformance. T196 owns the
actual specification/evidence review; its fingerprints remain unchanged.

The final header/coverage gate checks 2921 maintained inputs with zero debt.
The first rendered review caught four canonical links missing from originating
file pages, even though nested type pages described those rules. Exact HTML audit
identified closure/plan.h (COMP-02/TPL-06), ordinary/pass.h (TPL-06) and the compiler
analysis header (TPL-05). Their file-purpose comments now explain those rules;
all bytes after each first comment remain identical to the optimized tested code.
No compiler operation, declaration, ABI, grammar or language rule changed.

An intermediate render also rejected a bare Doxygen command in a workflow
checkpoint: Markdown now formats such directive names as code. The rejected
review logs remain `/tmp/ama-t184-{rendered-docs-review,final-rendered-docs}.log`.
The XML description audit already checked all 8667 extracted interfaces. Final
rendering and source-hash validation use the exact proposed archive rather than
publishing a reference to outdated active-task text. Its receipt is
`/tmp/ama-t184-archive-docs.log`; the candidate is applied only after that command
and the final document/routing/navigation checks succeed. Doxygen alone produces
the file/interface reference. Source and rendered links must lead to the actual
canonical rule, including the owning source page.

The full ledger is archived in [DONE](../../DONE.md#t184). T185 remains responsible
for ordinary operation/ownership/capability checks and executable candidate
admission; T186 handles constant values/lifecycle, T187 onward concrete lowering
and execution, and T196 final conformance evidence. Closed nominal metadata is
never an executable certificate or shared runtime instance.
