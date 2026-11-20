# Static bindings and specialization identities during directory relocation

T193 follows T213, T184, T189, T191 and T192. The authority is
[TOOL-01](../../spec/implementation/tools-and-conformance.md#tool-01),
[TOOL-03](../../spec/implementation/tools-and-conformance.md#tool-03),
[TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) and
[TPL-01](../../spec/language/templates.md#tpl-01). No specification change is
authorized. Existing preview, reference, staging, publication and recovery
contracts remain required; this task does not invent a new directory-move model.

## Steps and acceptance

1. Establish actual original/staged static graph analysis with narrow fixtures:
   real constructor/producer module paths change, while atomic placeholder,
   parameter, result, alias and selector names preserve their bytes and bindings.
2. Replace relocation's remaining raw one-module/one-Type semantic projection
   with the common checked frontend. Keep independent original/final source owners
   and complete source/native version checks. Preserve entry-free complete-domain
   analysis, including open source definitions and empty/draft domains.
3. Verify exact original-to-final source and complete specialization mapping.
   Moving a producer alone changes its identity while an unchanged ordinary
   constructor's closed Type identity stays fixed. Moving a constructor changes
   only the corresponding nominal origins, with every capture retained.
4. Exercise nested applications/namespaces, roots, hyphens and name+/ storage;
   actual relocated compilation/execution; original-name conflicts/new-root files;
   refusal, allocation failure, interruption and rollback/replay consumers.
5. Remove actually replaced dead paths/tests, update live contracts and per-file
   evidence, run affected strict/sanitizer/documentation gates, archive and commit.

## Initial inspection

References and staging each own a Project plus a raw SemanticProgram and call
`semantic_analyze` directly. That path has no static graph/guarded specialization
closure. The project dependency collector already distinguishes written module
paths from atomic descriptive static names (T213). Its exact resolved-edge map
and byte-interval rewriting must remain authoritative; no text replacement is
needed. The next step is a focused failing static relocation composition case,
then migration to the common catalog frontend. No production change yet.

The first strict static staging case fails before edits with the actual diagnostic
`Static syntax requires resolution before the concrete declaration inventory`
(`/tmp/ama-t193-static-staging-initial.log`, collected exit 2). It independently
expects a moved producer use and unchanged dotted/hyphenated atomic names,
comments, CRLF and returned constructor source. The failed leaf retains its
fixture; clean it before rerunning cases using exclusive fixture setup.

The common normal frontend also captures native files by physical path and
requires an entry policy input. That cannot be used unchanged for future native
paths or an entry-free relocation domain. The general checked solution is an
explicit captured-catalog analysis entry point: retain common admission, guarded
root selection, semantic checks and planning, while leaving its input graph
uncaptured/unsealed. The relocation owner already owns and validates source/native
physical versions; no absent future path is probed or replaced with a guessed
old file. Normal compilation keeps its existing complete input capture. Root
selection must omit an absent entry rather than pass a fabricated null module;
empty domains remain actual empty root sets. This is an implementation interface
choice under ARCH-03/TOOL-04, not a language rule change.

The first common-frontend probe reached executable storage planning and refused
an absent entry (`Storage has no exact executable entry identity`,
`/tmp/ama-t193-shared-snapshot-staging.log`, collected exit 2). Entry-free source
validation must not invent an entry merely to run executable planning. Extracted
the common guarded ordinary semantic composition into `compiler/resolve.c`;
normal executable finish still builds storage/control plans and seals its captured
inputs, while the explicit snapshot operation stops after common Type/operation/
constant checks and leaves its plan/input graph unavailable. Relocation owners
now retain/destroy one CompilerFrontend instead of manual semantic/project credits.
All borrow dependencies retire through its existing reverse destructor.

The complete strict existing staging leaf passed with the new static producer
case (`/tmp/ama-t193-shared-semantic-staging.log`). Its strengthened full-key test
also passed (`/tmp/ama-t193-static-return-identity.log`): moving only the static
producer preserves the exact registered returned `.box` owner encoding including
all captures. Descriptive three-dot names, comments and CRLF remain byte-exact.
The full original reference leaf passed
(`/tmp/ama-t193-original-reference-regressions.log`). A direct captured-catalog
component leaf passed (`/tmp/ama-t193-captured-catalog-fixed.log`): closed static
exports with captured native ABI and nonexistent future C/header paths; empty
and open definition-only domains; no fabricated entry, runtime Type, storage plan
or sealed input-cache graph. Its first compile required the missing explicit
stdio include (`/tmp/ama-t193-captured-catalog-initial.log`); corrected and rerun.

No acceptance jobs currently remain active. Next: broaden static constructor and
nested/root/storage cases, exercise actual publication/rebuild/recovery, update
per-file evidence, and audit the now-retired raw semantic orchestration and its
sole-purpose tests before declaring T193 complete. No specification changes.

The nested constructor fixture initially lacked the required sibling `storage`
anchor for `storage+/` and was correctly refused by preview
(`/tmp/ama-t193-nested-constructor-origins.log`). Added the specified sibling
source rather than relaxing discovery. Its complete strict staging rerun passed
(`/tmp/ama-t193-nested-constructor-origins-anchored.log`): both inner and outer
constructors retain two distinct concrete captures, original atomic formal names
stay exact, all real nested module paths acquire the mapped origin, and physical
`+` stays outside logical identities. All these jobs are collected.

### Retired concrete orchestration audit

After migration, no production relocation consumer calls `semantic_analyze`.
Its source-only callers are tests plus its own authoring wrapper. The raw
`semantic_names` implementation allocates one Type per source module and explicitly
mentions T193 migration; it is a competing old projection, not a template-aware
source path. The old inference orchestration also retains its source-event branch
beside a closure-certificate fast path. Production specialized inventory already
obtains those fields from the exact closed source/typed connection plan.

Before T193 acceptance, retire that raw orchestration and its unused dependencies.
Migrate independently useful semantic/planning/type-diagnostic cases to actual
current guarded preparation. Delete comparisons and stage-prefix/allocation tests
whose sole purpose is the old pipeline; retain or replace meaningful language and
failure observations against the current boundary. Do not hide the old pipeline
behind a test helper or preserve a compatibility API. Current remaining direct
caller groups are semantic composition/common/names/inference/selected/lists/
control/type fixtures, compiler composition's explicit-stage comparison, planning
storage/transfers/structured fixtures, one empty library rebinding fixture and
one Unicode integration diagnostic leaf. Inspect shared helper dependencies before
removal. This is required cleanup for the just-replaced live consumers; T193
remains in progress until its tests, source retirement and final consumers pass.

Continuation: no active jobs. Current next step is the concrete-orchestration
retirement and test migration described above. Publication/rebuild/recovery static
cases, final broad strict/sanitizer checks and documentation/archive/commit remain
pending. The existing conformance fingerprint issue remains T196's owner.

### Current guarded fixture migration and retirement checkpoint

Removed the raw semantic orchestration/source-module inventory and isolated
ordinary event replay implementation; live lexical local/initializer lookup and
AST walking remain. Namespace reads now require retained closed source proof,
without a path-spelling escape hatch. The ordinary inference prerequisite checks
the actual closure certificate instead of running a second classifier. No calls
or declarations of the retired analyze/names/path-link APIs remain.

Low-level semantic/planning tests now prepare StaticInventory, native-definition
admission and the production CompilerSpecialization child before ordinary checks.
All borrowed graph owners retire after projected semantics and before Project.
The old raw-composition leaf was removed; its live comparator moved to the current
compiler composition leaf, whose explicit ordinary stages consume guarded closure.
That compiler composition and the empty-library rebinding consumer passed strict
checks (`/tmp/ama-t193-compiler-current-stage-composition.log`,
`/tmp/ama-t193-library-empty-current-owner.log`). All planning leaves passed
(`/tmp/ama-t193-planning-current-preparation.log`).

Names/inference migration isolated actual boundaries rather than relaxing source
rules. AST visitation uses a parsed syntax owner without claiming Type validity.
Removed the old complete-inventory fixture whose operation names were intentionally
unbound; current specialized-inventory role/capture/default tests own that real
boundary. The duplicate matrix now isolates permitted declaration forms instead
of demanding duplicate-error priority for an independently invalid reference
constant. A simple declaration-order fixture now uses declared parameter operands.
Constructor-local diagnostics assert their original token rather than an obsolete
prose fragment. Names and inference passed strict checks, and the new current
source-preparation failure sweep actually refuses **1954** measured allocations
(`/tmp/ama-t193-current-name-bound-uses.log`,
`/tmp/ama-t193-current-inference-allocation-sweep.log`).

The first broader semantic runs exposed tests assuming first-enumerated Types or
partial inventories after upstream Type failure. Fixture queries now select the
exact original entry, and absent output on upstream failure remains absent.
Cross-root transfer matrices retain every rejection but permit failure at the
actual source equation stage. List rejection checks retain original operand
locations instead of an old prose fragment. Duplicate-binding note lookup is by
its exact role rather than array order; actual interface notes retain original
name spans. These are test boundary corrections, not specification changes.

The Unicode integration leaf now uses the current complete frontend. Its byte-exact
expectation retains the same offending source interval, both distinct Unicode
nominal origins and their original equality/variable-definition evidence. The
updated run passed (`/tmp/ama-t193-unicode-current-attribution.log`); the former
expectation's failure remains in `/tmp/ama-t193-current-unicode-diagnostic.log`.

Current job: full strict semantic parent, session 92894; log
`/tmp/ama-t193-semantic-source-owner-regressions.log`. All earlier jobs are
collected. Current per-file registry is being migrated with the actual removed
files/cases; final metadata acceptance has not run. Publication/rebuild/recovery
static cases and final broad sanitizer/documentation acceptance remain pending.


The shared guarded preparation now replaces all raw analyze/names/type-link
entry points in code and tests. Ordinary native/projection fixtures keep their
actual phase boundaries; the compiler comparison consumes accepted guarded
closure. The all-planning suite, updated compiler composition and empty-library
consumer passed. Updated current naming/inference leaves passed; the real source
preparation sweep retains all 1954 measured refusal points. The old event-replay
allocation case was replaced rather than retaining a now-vacuous failure loop.

The Unicode integration exact-output case passed after replacing old ordinary
mismatch prose with current source-equality evidence: the same offending caller
interval, both full native origins and their original binding/definition sites.
Duplicate saved-constant labels can be rejected during raw structural normalization
before namespace origin is solved (CONST-03); their test requires the actual earlier
and later binding sites, without demanding a fabricated actual formal. Known
ordinary interfaces still require their original declaration as a separate note.

A compiled relocation leaf is in progress: real filesystem publication, two
closed exports/native consumers, a relocated literal-failure source path and a
physical whole-root move preserving native body bytes. The first fixture assumed
a fixed scalar layout for the actual arbitrary-precision Standard integer and
failed real C compilation. Replaced that assumption with a typed call to its
actual declared index protocol; no production native ABI change was needed.
A subsequent compile corrected the actual interface query function name before
execution acceptance. Current execution job: session 46842; `/tmp/ama-t193-compiled-relocation-execution.log`.
The broader semantic migration still has a list diagnostic test crash under
investigation; sanitizer diagnostic job is session 1106,
`/tmp/ama-t193-current-list-diagnostic.log`. Neither T193 nor
its documentation/conformance acceptance is complete.

Compiled execution reached both relocated values and the relocated failure trace.
The physical whole-root rebuild then correctly refused the old output receipt:
`Output scope differs or is malformed; choose a separate directory or explicitly
overwrite a valid scope` (`/tmp/ama-t193-compiled-publication-evidence.log`).
The fixture had omitted the explicit generated-output invalidation required by
TOOL-03; it now declares `generated/output` in the actual relocation staging
request and verifies its removal after publication. The test does not weaken the
native output-scope guard or overwrite an unrelated receipt. Its recovery
container is also explicitly created, as required by the transaction API; the
first missing-directory probe is retained in
`/tmp/ama-t193-compiled-relocation-execution.log`.

Known-interface duplicate notes now use exact note roles. A raw constant-read
duplicate is structurally contradictory before its namespace has an actual slot;
that test retains both written binding locations without fabricating a formal.
Standard integer rejection diagnostics now retain both full nominal origin notes
instead of dereferencing an absent legacy primary subject. Sanitizer pinpointed
the former test assumption at `lists/support/capabilities.c:35` before correction
(`/tmp/ama-t193-current-list-diagnostic.log`). The broader semantic run has one
remaining old nominal-diagnostic test assumption to migrate; source-equality
integration already verifies the current complete attributed diagnostic.

### Collected functional acceptance after source retirement

The real compiled relocation leaf passed
(`/tmp/ama-t193-compiled-relocation-invalidated-outputs.log`): original and
logically relocated programs both print `7:1:0` and `value=23`; the relocated bad
literal fails before any native body output and names the new template source
path. A whole-root physical move preserves the three actual native body files
byte-for-byte across an independent rebuild/run. The transaction explicitly
invalidates its declared generated-output directory, and both old endpoints and
old generated artifacts are absent immediately after publication. No fixture
assumption about Standard integer payload layout, native output-scope overwrite,
C-source rewriting or generated-name compatibility was added to production.

The complete strict semantic parent ran every reachable child after the retired
source pipeline was removed (`/tmp/ama-t193-semantic-actual-origin-notes.log`).
Its sole remaining failure was the old nominal diagnostic test's null primary
subject assumption. The migrated list leaf passed independently
(`/tmp/ama-t193-current-list-origin-evidence.log`). The corrected diagnostic leaf
passed (`/tmp/ama-t193-current-complete-causal-notes.log`), requiring both full
nominal origins, the same bad caller token and both original Type equations.
An intermediate check incorrectly searched only flat notes; actual causal evidence
was retained in DiagnosticContext, as designed in T189. The final assertion checks
both stores and original source identities rather than weakening that evidence.
All other parent children passed with the same production inputs; the parent
failure is not relabeled a green aggregate. A final aggregate rerun is still due.

All currently launched jobs are collected. Next: refresh per-file mappings for
new/removed implementation/test inputs, run broad current relocation/semantic/
editor/compiler and sanitizer consumers, update final comments/usage contracts,
then complete actual Doxygen and final archive/hash/commit gates. T193 remains
unfinished; source/API retirement is included in its eventual task commit. No
specification or conformance fingerprint changes.

## Why the unchanged source bindings preserve static meaning

The existing exact reference graph check establishes the injective source-origin
map F and verifies every final requiring interval/target with full multiplicity.
Only parsed real module-path intervals are replaced. A canonical replacement
remains a module path (one or two root dots); atomic three-dot names, binder names,
roles, source nesting and all other tokens remain unchanged. Thus the original
and final syntax structures correspond through the exact edit coordinate map.

Static local identity is source owner plus original binder, not a dotted spelling
interpreted as a file. The coordinate/origin correspondence maps those binders
bijectively and preserves every name-based interface selection. Constructor
application and partial capture maps are structural operations on those same
original identities. Replacing each nominal module origin by F therefore commutes
with forming terms, substitution, equality and typed connections. Injectivity
prevents distinct original nominal origins from merging. Fixed intrinsic module
edges are independently protected and cannot change identity. Final source
checking recomputes guarded closure and ordinary validation from the actual
prepared bytes; no old candidate, Type pointer or constructor-state cache is reused.

This proves preservation of Ama source/static binding meaning under F, not a
textual equality of generated names. Moving only a producer changes the producer
origin while leaving an unchanged returned constructor's full Type key intact.
Moving a constructor changes its mapped nominal origins through nested captures,
without collapsing different actual inputs. Explicit output invalidation prevents
an old generated receipt from certifying a new source scope. Native C bytes remain
opaque: the tool does not infer C include dependencies, rewrite arbitrary strings
or prove arbitrary foreign effects. The compiled acceptance demonstrates the
specified concrete native consumers and a whole-root physical native move with
unchanged logical identities and exact body-byte preservation.

### Broad-run checkpoint

The final per-file/header metadata gate passed with **3007** maintained inputs
and zero debts (`/tmp/ama-t193-documentation-metadata.log`). The complete strict
editor parent also passed (`/tmp/ama-t193-editor-retirement-consumers.log`).
Current strict jobs are session **3145** (complete relocation parent,
`/tmp/ama-t193-relocation-regressions.log`) and session **78360** (complete semantic
parent, `/tmp/ama-t193-semantic-final-aggregate.log`). Both are still running;
no failure has appeared in their logs. Recovery has progressed through its
pending and terminal rollback/commit refusal partitions; semantic static leaves
continue compiling their independent explicit consumers. These launches are not
accepted results until their final exit statuses are collected.

No other acceptance job is active. Remaining final API review: a captured
source-only check must not claim the complete executable frontend certificate;
keep its source/semantic success distinct from plan/input readiness. After current
runs are collected, finish that narrowly observed readiness invariant and rerun
its component/actual relocation consumers before sanitizers and final generation.

An independent strict compiler build is additionally active: session **27725**,
`/tmp/ama-t193-compiler-build.log`. Its root objects do not overlap test leaves.

The final strict **complete semantic parent passed**
(`/tmp/ama-t193-semantic-final-aggregate.log`), including static child suites and
current specialization projections. The strict compiler build also passed
(`/tmp/ama-t193-compiler-build.log`). Session 3145 remains the only earlier active
strict job (complete relocation/recovery). The editor and metadata results above
are collected successes, not inferred from launches.

Relevant ordinary semantic and specialization consumers are now running under
ASan/UBSan with strict C11 flags, `-fsanitize=address,undefined`, frame pointers,
non-PIE compilation/linking, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`; log `/tmp/ama-t193-semantic-sanitizers.log`.
Unchanged static algorithms are covered by the just-passed complete strict parent;
this sanitizer run targets the retired pipeline's actual fixture/ordinary checker
and projection consumers. Counted exhaustive allocation teardown remains separate
from unavailable LeakSanitizer. Final captured-source readiness, actual relocation
sanitizers and final rendered documentation remain pending.

The affected ordinary semantic/projection ASan/UBSan target set passed
(`/tmp/ama-t193-semantic-sanitizers.log`, former session 86575). It includes
names/native/inference/types/calls/selected/lists/control/continuations/relations/
constants/diagnostics and all specialization consumers, with current guarded
fixture teardown and refusal injection. No sanitizer error was reinterpreted as
a pass; unavailable LSan remains separate from counted teardown.

Additional strict normal compiler consumers are active in session **78241**,
`/tmp/ama-t193-compiler-consumers.log`: source delivery, specialization diagnostics,
check/status/composition/native admission and specialization composition. The
captured-catalog readiness assertions are intentionally checked separately after
the final source-only/executable certificate distinction. Complete relocation
session **3145** is still enumerating **1066** actual application I/O checkpoints;
recovery child suites have completed. Every loop has a measured finite domain.

The normal strict compiler consumer target set passed
(`/tmp/ama-t193-compiler-consumers.log`, former session 78241), including current
source delivery, diagnostics, native admission and explicit-stage composition.
The captured-catalog component now explicitly asserts that successful source-only
checking leaves the complete executable frontend certificate false, alongside its
already-unsealed input graph and absent plan. Its baseline check is active in
session **83791**, `/tmp/ama-t193-captured-readiness-invariant.log`; collect its
expected old-prototype refusal before changing the source-only postcondition.
This keeps the existing complete-frontend precondition of native/project emission
separate from source metadata readiness, without adding a new implicit mode.
Session 3145 is still the only older strict job (complete relocation).

The **complete strict relocation parent passed**
(`/tmp/ama-t193-relocation-regressions.log`, former session 3145). Its actual
application sweep includes all **1066** measured I/O ordinals, alongside the
independent preflight/publication/recovery/rollback/replay and allocation checks.
The source-only readiness baseline failed exactly at the new assertion
(`/tmp/ama-t193-captured-readiness-invariant.log`). Snapshot checking now returns
successful source/semantic certificates while leaving the composite executable
`frontend.valid` false, with no plan/input certificate. Normal executable finish
still sets it only after planning and input sealing. No relocation consumer reads
that composite source-only flag; source metadata/graph behavior is unchanged.
The corrected strict captured component run is active in session **41597**,
`/tmp/ama-t193-captured-source-certificate.log`.

Actual compiled logical/root relocations are active under ASan/UBSan in session
**60310**, `/tmp/ama-t193-relocated-execution-sanitizers.log`. Independent original
and staged source snapshot leaves are also running under the same sanitizer flags;
their logs are `/tmp/ama-t193-original-snapshot-sanitizers.log` and
`/tmp/ama-t193-staged-snapshot-sanitizers.log`. Collect these before changing their
inputs. The full strict source/ordinary, editor, compiler and metadata evidence
above remains scoped accurately; no conformance fingerprints were refreshed.

### Final component/sanitizer and metadata acceptance

All source-only and actual execution readiness checks are collected successes:
strict captured component (`/tmp/ama-t193-captured-source-certificate.log`) and
ASan/UBSan captured component
(`/tmp/ama-t193-captured-certificate-sanitizers.log`), original snapshot
(`/tmp/ama-t193-original-snapshot-sanitizers.log`), staged snapshot
(`/tmp/ama-t193-staged-snapshot-sanitizers.log`) and actual relocated compilation/
execution (`/tmp/ama-t193-relocated-execution-sanitizers.log`). The final strict
compiler build passed (`/tmp/ama-t193-final-compiler-build.log`). The final complete
per-file/header metadata gate passed **3007** inputs with zero debt
(`/tmp/ama-t193-final-metadata.log`). All jobs from those runs are collected.

No maintained implementation input is being changed during the next rendered
Doxygen gate. The complete strict semantic/relocation/editor sets, normal compiler
consumers, planning consumers, source/ABI rebinding and Unicode integration plus
relevant sanitizer runs above establish this task's actual boundary. The one
post-aggregate source-only certificate refinement changes no relocation read
behavior: no relocation consumer reads the composite executable flag, and final
component/staged/actual-execution sanitizer consumers verify its source semantics.
The T196 independent conformance review gate remains unresolved; no current
whole-project conformance claim or fingerprint refresh is made here.


## T193 accepted result

Actual Doxygen generation passed (`/tmp/ama-t193-doxygen.log`): **2288** maintained
file pages, **8890** documented interfaces and **447/447** canonical destinations.
The 3007-input complete metadata gate, strict full semantic/relocation/editor
parents, normal compiler/planning consumers, captured source mode, actual moved
compiled programs, Unicode attribution and relevant ASan/UBSan checks above are
collected successes. No acceptance job remains active.

T193 removes the replaced concrete-only projection/event replay and sole-purpose
tests while retaining/migrating actual naming, walker, inference, storage, control,
capability and diagnostic coverage. All original/final relocation sources use the
same guarded Type/ordinary validation as compilation, without inventing an entry,
probing future native paths or exposing a fake executable/input certificate.
Atomic static names and source bytes stay intact; actual dependency origins and
full concrete captures map structurally under the exact injective relocation.

The final task commit includes current source/API comments, per-file evidence,
updated implementation contracts, the complete task archive and the readable
step/failure record. Regenerate Doxygen after this archival prose and require its
manifest hashes to match staged source bytes before committing. No specification
or conformance fingerprint change is included. T196 retains the known independent
conformance review gate; next ready task is T194 concrete compiled facades.
