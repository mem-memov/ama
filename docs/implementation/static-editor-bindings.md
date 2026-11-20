# Static source bindings in editor navigation and checked rename

T192 follows T206, T207, T208, T211, T212, T184 and T189. Rules are
[COMP-01](../../spec/implementation/compiler.md#comp-01),
[COMP-02](../../spec/implementation/compiler.md#comp-02),
[NAME-06](../../spec/language/declarations.md#name-06),
[TPL-02](../../spec/language/templates.md#tpl-02) and
[TPL-12](../../spec/language/static-results.md#tpl-12).
No specification change is authorized. Existing
[editor analysis](editor-analysis.md) and [checked rename](editor-rename.md)
contracts remain required.

## Planned steps and acceptance boundary

1. Establish actual original-source navigation for static module paths, graph
   definitions, aliases/equality receipts, free parameters, foreign labels and
   ordinary named export selectors. Include multiple concrete captures of one
   source template; original declaration identity must not become an arbitrary
   first concrete instance or a generated specialization filename.
2. Extend only the needed editor index/query/source identity boundaries, using
   the accepted frontend static metadata and ordinary source roles. Keep
   concrete runtime variants distinct while retaining their shared original
   declaration. Do not duplicate parsing or choose origins from spelling.
3. Extend the existing checked rename proof and actual workspace transactions.
   Preserve declaration bijection and all resolved bindings, including foreign
   labels with multiple actual origins. A request without a unique valid target
   must be refused rather than given an invented coordinated-rename meaning.
4. Exercise Unicode/hyphens/atomic names, whole-graph and source-ordered scope,
   incomplete/error snapshots, stale versions, capture rejection, allocation
   failures and byte-preserving refused transactions. Run real consumers,
   relevant sanitizers and documentation gates before a separate commit.

## Initial implementation observations

The current index consumes only concrete SemanticType/SemanticSymbol records.
Static modules do not have runtime Types. Indexing an ordinary export annotation
as its selected concrete field Type loses the distinction between the written
producer path and the returned Type. Multiple concrete captures also reuse the
same original source span but have different semantic symbol/type pointers; the
current duplicate check rejects those pointers outright. Focused tests will
establish these observable boundaries before selecting the representation.

All queried sources and accepted metadata remain borrowed from the immutable
EditorAnalysis owner. New index state must expire before its frontend; failed
construction must publish no complete index. Rename source transactions retain
the existing exact version/binding checks and synchronous atomic writer contract.
Current step: run independent original-source navigation probes. No active jobs
and no production change yet.

Current strict probe: session 40607, `/tmp/ama-t192-navigation-initial.log`.
The leaf is registered in the editor parent and per-file registry. Collect its
actual result before editing those source inputs; no acceptance is claimed.


The initial probe confirmed wrong-path navigation
(`/tmp/ama-t192-navigation-initial.log`): `.factory` incorrectly navigated to
returned `.box` source. The index now takes module heads, graph symbols,
known foreign labels and export selectors directly from the accepted static
inventory. Ordinary runtime indexing no longer invents a module target from
its substituted field Type. The one record constructor accepts an explicit
source/semantic record, retaining one allocation/validation path.

The first corrected probe passes all three static navigation assertions, then
exposes the expected next issue (`/tmp/ama-t192-static-source-index.log`): two
concrete `.box` captures reuse original spans but different SemanticSymbol/Type
pointers, causing the current duplicate check to refuse indexing. This is not
a language ambiguity; those variants must remain distinct while their original
declarations remain source identities. No task acceptance or commit is claimed.

Current checkpoint: no active jobs. Production changes are limited to the first
source-target index step; static rename, concrete-variant grouping, unused/open
source views, query/transaction proof integration and regression/resource/docs
acceptance remain unfinished. Header purpose text now names TPL-12 for correct
file-level Doxygen navigation. All new/changed current code inputs have their
focused cases registered. No specification change was made.


Both navigation probes passed after preserving source identity separately from
concrete target records (`/tmp/ama-t192-source-variants.log`). The strengthened
case also passed (`/tmp/ama-t192-retained-variant-targets.log`): one original
variable declaration retains both actual SemanticSymbol owners, while singular
symbol/Type fields stay absent when concrete views disagree. New exact target
lookup preserves output on null/missing/out-of-range queries. Source lexical
fields/uses/receivers/foreach cells come from the accepted connection plan, so
uninstantiated ordinary source does not require an implicit runtime Type.

The index groups equal original spelling spans, removes exact duplicate target
evidence and retains distinct concrete or foreign targets explicitly. A single
navigation result exists only when every target has the same original definition;
otherwise the singular query refuses, while the explicit target query retains
the alternatives. No first-context or name-based target selection is used.
Source records and all alternative nodes are arena-owned until analysis teardown.
Sorting is finite iterative merge sorting; group membership uses exact pointers
inside one immutable snapshot, never cross-snapshot pointer equivalence.

Current checkpoint: no active jobs. Next step is source-level completion and
signature queries, which currently still select the first SemanticType by module
path. They must expose original declarations with optional unique concrete
metadata, not manufacture an ordinary Type for static graphs or unused templates.
Then update actual operation/binding query ambiguity and the complete rename
bijection proof; no rename behavior is claimed complete yet. Remaining checks
include dynamic actual foreign labels, scope/Unicode/error snapshots, workspace
transactions, failure injection, existing consumers, sanitizers and Doxygen.
No specification edits or task commit have been made.

Source interface/completion probes passed
(`/tmp/ama-t192-source-interface-queries.log`): two concrete captures still expose
one original ordinary input and result; the Type parameter is not an extra
runtime argument. A static graph exposes whole-graph names and named metadata
results without ordinary variables or runtime Types. Completion callbacks now
receive ephemeral source definitions with optional unique concrete metadata.
The now-unused typed-only semantic_symbol_available helper/prototype is removed;
typed initializer diagnostics retain their still-used classifier.

The existing binding suite caught a nested-index regression
(`/tmp/ama-t192-original-visibility-regression.log`): a source use's diagnostic
extent contains its entire indexed operand, rather than only its lexical name.
Indexing that whole extent masked inner index variables. Source-use indexing now
uses the accepted AstAccess.name when present, retaining plain source-only
foreach names otherwise; no token splitting or closest-span heuristic is used.
Current corrected binding regression: `/tmp/ama-t192-lexical-use-span-regression.log`
(record its returned session). No other job is active. The driver declaration
formatter and ambiguous concrete operation queries are implemented but still
need their direct consumer checks and per-file registration. Rename integration
and the remaining acceptance partitions remain unfinished.


All six source/variant/interface/actual-foreign-label probes passed
(`/tmp/ama-t192-actual-foreign-labels.log`); the Unicode/dotted-name forward and
receipt case also passed (`/tmp/ama-t192-unicode-graph-source.log`). A formal
label of a passed-in constructor retains both actual original module formals;
the local constructor parameter remains a distinct source symbol. Single-origin
navigation refuses that genuinely plural definition while explicit targets retain
both. Concrete operation/binding APIs similarly refuse multiple runtime contexts
without modifying output, while original variable/interface navigation survives.
The corrected original visibility/nested-index regression passed
(`/tmp/ama-t192-lexical-use-span-regression.log`).

Actual driver source metadata passed
(`/tmp/ama-t192-driver-source-observations.log`), including normal existing rename
cases and all 1563 then-measured rename allocation checkpoints. A further error
snapshot exposed a real omission: JSON emitted the flat diagnostic notes but
lost T189's context chain (`/tmp/ama-t192-editor-error-attribution-initial.log`).
The JSON codec now validates the complete diagnostic before output and emits
flat notes followed by context attribution in the human renderer's same order.
Its new independent decoder/cycle/no-allocation cases passed
(`/tmp/ama-t192-diagnostic-json-contexts.log`). The first driver rerun refused its
preserved old generated fixture directory
(`/tmp/ama-t192-editor-error-contexts-fixed.log`); after collecting that failure,
only that leaf was cleaned before the current isolated rerun.

Current run: session 67216, `/tmp/ama-t192-editor-error-contexts-clean.log`.
Collect before changing driver inputs. All other jobs are collected. Every
current new/changed code/build/test input is registered; no task completion is
claimed. Still pending: complete checked rename source/Type correspondence,
static names and multi-origin transaction refusal, editor workspace regressions,
resource failures, sanitizers and final documentation acceptance.


JSON context serialization direct cases passed
(`/tmp/ama-t192-diagnostic-json-contexts.log`), including a source allocator set
to refuse its next allocation, independently decoded note ordering/Unicode paths,
cyclic/null diagnostic rejection before output and preservation of an earlier
writer error. The driver rerun initially stopped at its fixture's exclusive
mkdir because failed leaves deliberately retain generated files. Its isolated
clean rerun is session 67216, `/tmp/ama-t192-editor-error-contexts-clean.log`;
collect it before further driver changes. All other jobs are collected.

Current source-query interface now returns source definition views to signature
and completion callbacks, with concrete fields only when uniquely justified.
Existing ordinary JSON remains unchanged for a unique runtime symbol; static and
multi-capture source declarations have their own accurate source metadata. The
unused old typed-only visibility query is removed rather than retained behind
a compatibility wrapper. Source availability reuses the compiler's lexical local
lookup and persistent construction order, with no runtime value dependence.
Next affected boundary is checked rename: source target selection must replace
the old single-SemanticSymbol assumption, and full captured Type correspondence
must replace module-name-only comparison. No rename completion is claimed.

### Exact rename correspondence checkpoint

Collected the driver JSON-context rerun successfully (former session 67216,
`/tmp/ama-t192-editor-error-contexts-clean.log`). The early rename source-target
regression also passed (`/tmp/ama-t192-source-rename-plan-regression.log`).

Rename selection now borrows an original definition view. Static whole names use
the shared production recognizer from `parser/static/names.h`; the duplicate
internal declaration was removed. Source edit collection checks every target of
a spelling and refuses edits shared with another original declaration.

Replaced the old module-path-only Type comparison and noninjective sorted symbol
key. Validation builds a full concrete-owner bijection by exact nominal
capture/residual graphs with mapped original formal sites. An iterative memoized
term-pair list avoids recursive C calls or depth limits. Declaration matching uses
that owner bijection, mapped source sites and storage/role/Type metadata. All
occurrence alternatives require exactly one mate in both directions; static and
source-only bindings are included. Program comparison selects each exact paired
owner instead of the first Type with the same module path.

A strict first build exposed use of `StaticModule.module` instead of its actual
`definition` field (`/tmp/ama-t192-owner-bijection-plan.log`); corrected before
behavioral checks. The existing plan leaf then passed
(`/tmp/ama-t192-exact-correspondence-plan.log`). The new static rename leaf's
first compile required an explicit use of the shared `text_is` fixture helper;
its first four cases then passed (`/tmp/ama-t192-static-rename-initial.log`).
The six-case boundary run passed
(`/tmp/ama-t192-static-rename-boundaries.log`), including two captured owners,
whole Unicode static formal labels, named graph selection, uninstantiated source
fields and multi-origin refusal. A subsequent case exhausts every measured
candidate allocation failure; its current acceptance run is the editor parent.

Active jobs: session 52441, strict `make -C tests/unit/editor`,
`/tmp/ama-t192-editor-regressions.log`; session 98548, `make docs-check`,
`/tmp/ama-t192-metadata-current.log`. Neither is accepted until completion is
collected. Still pending: current strict editor result, affected driver/parser
and diagnostic consumers, sanitizer coverage, final per-file/header/Doxygen gates,
final input review and task commit. T192 remains unfinished; no spec edits.

The strict editor parent has completed all rename leaves, including **4080**
measured candidate-allocation refusal points for the two-capture static-formal
case. Each failure reports managed out-of-memory, preserves original source bytes
and releases every allocation under the counted owner. The parent is still
running its other editor consumers; this is a partial result, not full acceptance.
The final strict driver/editor run passed
(`/tmp/ama-t192-driver-final-regression.log`), as did parser/static
(`/tmp/ama-t192-parser-recognizer-regression.log`) and the complete tools/output
parent (`/tmp/ama-t192-source-codec-regressions.log`).

The metadata checkpoint failed because the allocation case was incorrectly
registered with its included support file as the standalone test translation
unit. Corrected the registry to its real owning `static/main.c`; no artificial
support-directory Makefile was introduced. Current metadata rerun: session 22425,
`/tmp/ama-t192-metadata-current-fixed.log`.

### Strict consumer acceptance and sanitizer checkpoint

The complete strict editor parent passed
(`/tmp/ama-t192-editor-regressions.log`), including source queries, all rename
leaves, workspace, lexical, bindings and composition consumers. The repaired
metadata gate passed: 3020 maintained inputs, zero documentation debts
(`/tmp/ama-t192-metadata-current-fixed.log`). Final strict driver/parser/output
results above were collected successfully; no job is inferred from a launch.

Added an explicit rename case for forward graph uses plus defining/equality
receipts and their ordinary selector. Updated source API comments to describe
original static targets and complete owner correspondence; removed the obsolete
qsort include and sorted-path comparison prose. No production semantic changes
followed strict acceptance.

Current active acceptance: session 12960, full editor ASan/UBSan parent with
`make -j4 -C tests/unit/editor`, strict C11 flags plus
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and linker
`-fsanitize=address,undefined -no-pie`, `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`; log `/tmp/ama-t192-editor-sanitizers.log`.
The complete tools/output parent passed under the same sanitizer configuration
(`/tmp/ama-t192-output-sanitizers.log`). LeakSanitizer is unavailable in this
sandbox; counted allocation teardown and exhaustive refusals are separate leak
ownership evidence, not a LeakSanitizer claim. Documentation complete gate is
running separately (`/tmp/ama-t192-documentation-complete.log`). Collect both
before final acceptance/archive. Actual generated Doxygen for final maintained
inputs, final diff review and task commit remain pending.

## Final representation and preservation argument

The complete checked snapshot is the input domain. Source-only declarations and
actual concrete projections borrow one immutable frontend; none owns a runtime
instance. Arena grouping retains one source interval and a finite set of exact
target tuples. Tuple duplicates merge declaration flags, while distinct source
origins and specialization owners remain alternatives. A singular definition
exists iff every alternative has the same defining source span. Concrete fields
are individually suppressed on disagreement; the original evidence remains
queryable. Missing/invalid/ambiguous queries preserve the caller's destination.

Source signatures and lexical availability consult accepted compiler inventories.
They introduce no new syntax, lookup heuristic, inferred Type or default instance.
The ordinary role mapping is explicit, static interface names have no runtime
slots, and the concrete-only operation/binding views require exactly one actual
owner. Written path identity is separate from a selected output's Type identity.

Rename preparation chooses one original declaration and collects only its exact
accepted occurrences. A shared label with a second origin is refused before a
prepared edit can publish. Candidate checking first runs the ordinary compiler,
then proves a bijection of complete concrete owner graphs and their declarations.
Original formals correspond through source ownership/role/ordinal and exact edited
source coordinates, so changing their spelling does not erase capture identity.
Each nominal vertex preserves origin, category, grounding, capture and residual
counts and every mapped child. A memoized iterative pair work list visits only a
finite product of accepted term vertices; no recursion limit decides equality.
Uniqueness plus injectivity establishes the complete owner/declaration bijections.
Every source occurrence alternative then has exactly one mate in both directions;
all resolved statement/binding/initializer/control metadata retains that mapping.
Thus successful candidate compilation alone cannot silently certify capture.

Preparation and validation write no source documents. Their scratch owners retire
on both success and refusal; allocation failures retain a managed diagnostic.
Publication retains the existing complete source/native checkpoint and synchronous
all-document writer contract. Error/draft snapshots expose lexical diagnostics,
not fabricated static bindings. JSON diagnostics preserve persistent context notes
without failure-time allocation and reject malformed context cycles before output.

The obsolete typed-only availability query and module-path-only rename proof were
removed after their production consumers migrated. Their still-relevant visibility
and transaction tests now exercise the source-based implementation; they are not
kept as tests of a dead alternate API. No static parser, legacy compatibility
wrapper or generated specialization source file was added.

### Collected acceptance before rendered documentation

The full ASan/UBSan editor parent passed with the added forward/equality receipt
case (`/tmp/ama-t192-editor-sanitizers.log`). Its final static-candidate sweep
covers **4140** allocation refusal ordinals in the complete active fixture
workspace; the earlier strict checkpoint's 4080 count predates the added graph
producer fixture and is not the final count. All original source bytes and counted
owners were preserved for each refusal. The full output parent also passed under
ASan/UBSan. Root `make -j4` passed with strict O2/Werror C11 flags
(`/tmp/ama-t192-compiler-build.log`). `make docs-complete` passed with **3020**
ready inputs and zero debt (`/tmp/ama-t192-documentation-complete.log`).

No jobs from those acceptance runs remain active. Actual Doxygen generation is
the next gate; do not change maintained inputs until its result is collected.
The known pre-existing conformance fingerprint failure remains assigned to T196;
this task does not refresh fingerprints or claim whole-project conformance green.

Actual Doxygen generation passed
(`/tmp/ama-t192-doxygen.log`): **2302** maintained file pages, **8942** documented
interfaces and **446/446** referenced canonical destinations. This is rendered
HTML/XML evidence, separate from the metadata gate. Final review clarified that
`EditorOccurrence.declaration` is the source group's aggregate flag: compare it
once per source group, not once per alternative target tuple. The target set
comparison must not depend on which actual tuple happens to head the group.
The narrow strict static rename rerun is session 92769,
`/tmp/ama-t192-final-group-flags.log`; no interface or language rule changed.
Final archival prose requires regenerating Doxygen and checking staged source
hashes against its manifest before commit.


## T192 accepted result

The final grouped-flag comparison passed the strict static rename leaf
(`/tmp/ama-t192-final-group-flags.log`) and its ASan/UBSan rerun
(`/tmp/ama-t192-final-static-sanitizers.log`), both including all **4140**
candidate-allocation refusal points. No acceptance jobs remain active.
Together with the full editor/driver/parser/output regressions, strict compiler
build, complete metadata/header gate and actual rendered documentation above,
this satisfies T192. Final Doxygen regeneration after this archive must preserve
its source hashes through staging; the commit includes the complete source,
tests, API documentation, per-file registry and task record. T196 retains the
independent existing conformance gate; T193 is the next ready relocation task.
