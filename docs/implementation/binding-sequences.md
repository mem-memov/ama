# Separate static and runtime binding sequences

T173 is accepted on input e4c8d92 (2026-11-18). T172 supplies accepted exact module-local
static symbols, source introduction ordinals and static result ordinals. This
task normalizes interface requests before type equations; it does not specialize
modules, evaluate defaults, acquire runtime targets or integrate the executable
frontend (T184). Authority is [TPL-12](../../spec/language/static-results.md#tpl-12)
and [CALL-06](../../spec/language/calls-and-results.md#call-06).

## Contract and boundaries

Build an immutable interface snapshot from the selected actual source module.
Its static inputs follow original producer formal order with already fixed
formals removed; its static outputs follow explicit static result order. Runtime
inputs/outputs contain only ordinary declarations from their respective sections.
Constant reads use their selected shared-constant output surface, not an invented
runtime call. A slot preserves original identity and ordinal separately from its
position in the filtered snapshot. Expected Template signature spelling/order
cannot select or reorder an actual producer.

Normalize each static expression or ordinary runtime operation independently.
Input and output counters are separate. A positional skip consumes exactly one
snapshot slot; filling one slot never mutates that snapshot. Named matching is
exact, with no local capture or search for a compatible producer. Preserve every
original operand/receiver, storage-form label and explicit omission. Selected
result operations do not consume ordinary binding positions (CALL-09); they are
separate operations. Nested static expressions and runtime operations select
their own modes.

Unknown actual origins remain explicit pending requests. Mode/order and duplicate
source-label checks that need no origin can still run; positional arity and
foreign identity are resolved only when the actual interface is available.
Pending requests never borrow an expected signature as an actual positional
interface. Application residual/category normalization remains T175/T177, output
closure remains T179, and target execution remains later frontend/runtime work.

The owner borrows immutable accepted static/source metadata, owns only interface
snapshots, normalized records and scratch, and is destructible after every
failure. It never mutates a validated T172 inventory. Publication is transactional
per interface/application, with original source diagnostics and sticky first
failure. Fixed captures are exact original formal identities supplied by later
static metadata, not names guessed from a prior source expression.

## Planned steps and evidence

1. Define independent owner/record interfaces and snapshot construction. Check
   interleaved static/runtime declarations, input formals first introduced in
   results, original ordinals, fixed capture filtering and zero-sized sequences.
2. Normalize static requests: original-order snapshots, named/positional modes,
   skips, direction order, duplicates, excess positions and explicit unknown
   actual origins. Check reversed expected-signature order and nested expressions.
3. Normalize ordinary runtime requests with the same sequence invariants, while
   preserving reference/list label contracts and existing parser call-mode rules.
   Compose actual parsed calls, resumes, retained reads and independent selectors.
4. Test every real allocation refusal, original diagnostics and immutable
   snapshots; run strict C11, relevant compositions/regressions and sanitizers.
   Register named per-file coverage and standalone headers, validate local links,
   generate and validate Doxygen, archive only accepted work and commit T173.

Initial inspection: parser ordinary arguments already retain omissions and
independent direction counters, enforce call mode/style/order, and exclude
selected operations from the ordinary binding chain. The legacy typed resolver
matches runtime labels against concrete semantic fields; it is not replaced by
this unintegrated static-analysis component. T173 will keep the raw AST intact
and produce separate descriptors consumed by later constraint generation.

Initial checkpoint (superseded): planning/contracts; no jobs running and no T173 code yet.
Next action: implement/test the snapshot owner and four source-interface sequences.
All listed verification commands are planned until their actual results are
recorded; T172 acceptance is prerequisite evidence, not T173 acceptance.

Step 1 initial result (2026-11-18): the independent BindingPlan owner and exact
source/intrinsic interface providers are implemented. Slot metadata borrows exact
byte names and real source sites or canonical intrinsic descriptors; it never
fabricates source AST for an intrinsic field. Source snapshots preserve original
formal identities/ordinals while filtering fixed identities, and expose separate
module/shared-constant runtime surfaces. The intrinsic adapter reuses LIST-04's
existing canonical provider for all six methods, avoiding a second name/arity
table. Every interface is published only after all four sequences complete;
errors leave accepted T172 metadata unchanged.

The first snapshot leaf passed strict C11 warnings-as-errors in session 78618
(`/tmp/ama-t173-snapshots.log`). Seven cases check four independent sequences,
free inputs introduced in result definitions, interleaving and foreign-signature
separation, capture-order independence, exact foreign/colliding identity refusal,
shared and empty surfaces, all six intrinsic interfaces without synthetic spans,
every actual source/intrinsic snapshot allocation refusal, checked maximum counts
and sticky failure. Counted teardown checks all partial records. These are
component results only: no request normalizer is implemented yet, and T173 is
unfinished and uncommitted. Coverage registration, further boundary partitions,
normalizers/compositions and final gates remain required.

Snapshot invariant: each sequence contains exactly the applicable original fields
in ordinal order, with source/static/intrinsic identities retained. Fixed-set
membership uses exact original formal pointers; invalid metadata is rejected
before allocation, without textual or shape matching. A dense sequence ordinal
is assigned from the new tail count, independently of original ordinals. Captures
are borrowed during construction only, and no request can renumber a published
snapshot. Construction consumes finite symbol/declaration lists. For S symbols,
F captures and name-byte bound B, validation costs O(FS + F squared), ordinal
selection costs O(S squared + SF), and duplicate slot checks cost O(q squared B)
per sequence of q fields. Storage is linear in emitted slots. Intrinsic traversal
uses the canonical finite descriptor table rather than a guessed arity.

Snapshot checkpoint (superseded): initial snapshot phase passed; no jobs running. Next: finish
snapshot boundary checks/coverage, then static and runtime request normalization.
No complete T173 acceptance or compiler integration is claimed.

Expanded snapshot partitions are running in session 29087
(`/tmp/ama-t173-snapshot-boundaries.log`): all inputs fixed, duplicate actual
runtime names without first-match selection, and every failure while building a
second interface after an existing successful publication. The caller's previous
output and existing tail/slot identities must remain unchanged. Collect this run
before further functional source edits. No request-normalization API has been
published prematurely; current public construction operations are implemented.

The expanded snapshot leaf completed successfully (session 29087,
`/tmp/ama-t173-snapshot-boundaries.log`): all ten cases pass strict C11 with
warnings as errors. Every injected later-construction refusal preserves a
previously published view and the caller's nonnull output. All-fixed residual
inputs stay empty without changing the other sequences; duplicate ordinary
names fail instead of selecting the first. Named per-file coverage and independent
header probes are registered for these accepted snapshot components. Later
request-normalizer files will receive their own actual evidence, not these
unrelated snapshot cases. No jobs running; next step is request normalization.

Request phase initial results: static normalization passed three initial cases
(`/tmp/ama-t173-static-initial.log`), and ordinary normalization passed five
(`/tmp/ama-t173-runtime-initial.log`). These compose actual source/project/static
metadata with immutable snapshots, reversed expected versus actual static order,
unknown-origin records, exact names, direction/mode/arity errors, runtime storage
labels, resumes/retained/shared reads and a selected intrinsic append action.
Applications publish only complete chains and never mutate snapshots; no type
term, default, target or native hook is evaluated. More boundary/failure/depth
partitions remain required; neither normalizer is accepted as a complete task yet.

Initial fixture failures were corrected without weakening language checks:
- A narrower consumer did not use the optional shared static-scope text variable.
  It is now an uppercase literal macro with identical source bytes, avoiding
  unused storage or warning suppression; prior scope suites require regression.
- The parser already rejects static input-after-output and standalone static
  expressions without output actions. Such normalizer-negative metadata is
  cloned from valid parsed records; actual project AST/source stay immutable.
  Standalone negative sources now include their required output phase.
- Selected list services genuinely require `..integer`. The fixture loader now
  accepts explicit root families, and that composition uses the real standard
  library root. The test asserts the actual Standard integer was loaded; no
  dependency was ignored or substituted. Narrow default fixtures still assert
  their original two-source domain. Project preparation now prints original
  diagnostics before asserting, which exposed these setup failures precisely.

The initial failed static/runtime logs are superseded by collected passing runs;
`/tmp/ama-t173-static-debug.log` preserves the parser-order failure explanation.
Request/source owners and fixtures are still under T173. Next: named/nested and
fixed-residual applications, complete skip/arity partitions, allocation failures,
then full affected-parent, sanitizer and documentation acceptance. No jobs running.

Expanded normalizer parent passed in session 81446
(`/tmp/ama-t173-normalizers-parent.log`): ten snapshot, seven static and six
runtime cases. Static cases now include named reorderings, fixed-a/residual-b
mapping to original ordinal one, refusal to reopen captures, separate nested
parent positional/child named modes, and every 0..2 skip prefix plus first excess
in both directions. Runtime covers the corresponding prefix matrix. Each matrix
has supplied/withheld actual-interface controls; withholding metadata preserves
an obligation, not a claim that the source program type-checks or closes.

Three transactional boundary cases also passed session 57060
(`/tmp/ama-t173-normalization-failures.log`): every actual normalizer allocation
failure across both kinds and supplied/pending origins, every numeric direction
counter boundary, and copied/unpublished snapshot refusal without shape matching.
A previous successful application, nonnull caller output and immutable snapshot
survive a subsequent failed attempt. No failed allocation can replace the first
diagnostic or allocate further scratch.

Known-source duplicate diagnostics now additionally retain the actual foreign
field declaration alongside the earlier caller request, without reporting
allocation. Unknown/intrinsic declarations have no fabricated span. Updated
static/runtime assertions check the appropriate one/two source contexts. The
full affected static parent, including prior T172 cases and generalized fixtures,
is running in session 84030 (`/tmp/ama-t173-affected-static-parent.log`). Collect
that result before functional edits. Remaining: final coverage/header ownership,
sanitizers/build/project regressions, local/source/generated documentation checks,
archival and the T173 commit. Whole frontend execution remains T184.

The full affected static parent passed session 84030
(`/tmp/ama-t173-affected-static-parent.log`): all 36 prior T172 cases and all 26
T173 cases pass strict C11 warnings-as-errors. The actual declaration/prior
request diagnostic contexts pass, as do default two-root-domain preservation
and genuine Standard-root extension. Every current binding file now has named
per-file evidence, including standalone header probes; existing shared fixture
coverage is retained. New maintained files also pass an explicit whitespace
review, including files not yet staged. No functional behavior changed in the
final EOF whitespace cleanup. Next gates use the immutable functional tree:
ASan/UBSan affected parent, compiler build, project regression and documentation
completeness/local links. Do not edit functional inputs until those jobs finish.

Functional acceptance passed (2026-11-18): the complete affected parent passed
all 62 cases (36 T172 and 26 T173) under strict C11, AddressSanitizer and UBSan
in session 35956 (`/tmp/ama-t173-sanitizers.log`). LeakSanitizer is disabled for
the previously established container limitation; counted owner teardown checks
all tested real allocation refusals independently. Compiler `make` passed
session 17336 (`/tmp/ama-t173-build.log`); the complete project parent passed
session 26827 (`/tmp/ama-t173-project.log`). All functional jobs are finished.

Documentation completeness passed session 75140: 2440 ready maintained inputs,
zero explicit debt (`/tmp/ama-t173-doc-complete.log`). Local audit passed 207
Markdown files, 2887 local links, 214 canonical rules and 108 unchanged EBNF
productions (`/tmp/ama-t173-links.log`). No specification content changed.
Generated Doxygen/source-link validation remains the acceptance gate before
archival. Do not infer whole-program type closure, native admission, executable
integration or complete sprint conformance from this normalization component.

Normalization invariant: an application consumes only its own finite sibling
chain. Its two counters equal the number of accepted requests per direction;
each positional request maps that preincrement ordinal into the frozen residual
snapshot. A skip has identical position consumption without an operand. Named
selection uses complete borrowed bytes, never source order of an expected
signature. Direction/style/duplicates independent of actual origin are checked
before allocation; origin-dependent checks stay pending when no interface is
supplied. Every newly allocated request is fully prepared before linking and
counting. Publication occurs only after the entire sibling list succeeds.
Thus interruption leaves prior results and views unchanged. Child owners are
not traversed recursively; their later calls have independent mode/counter state.

For K requests, snapshot width Q, name-byte bound B and V published views, one
normalization costs O(V + KQ B + K squared B) time, with O(K) new owned records.
View membership and named matching use exact pointer/byte equality; no hash
identity, module search, time/depth bound or first-actual choice is involved.
Counter overflow is refused before increment; all error reporting is allocation
free. The finite original AST and monotone sibling cursor establish termination.

Final T173 acceptance: generated Doxygen passed session 39935
(`/tmp/ama-t173-doxygen.log`): 1841 maintained file pages, 7048 documented
interfaces, all 419 referenced canonical destinations and 5798 total pages.
Actual rendered destinations and source stability were validated using Doxygen
1.11.0, Python 3.14.4 and markdown-it-py 3.0.0. The supported editor command
`code --goto /home/u/repos/ama/spec/language/static-results.md:44` returned success
at TPL-12's actual rule location (session 50915); no unobserved GUI click is claimed.
All functional, source-navigation and documentation jobs are finished.

The accepted task is archived with its original criteria and evidence. Final
archive-input documentation refresh is logged in `/tmp/ama-t173-doxygen-archive.log`;
collect an outstanding refresh before the authorized per-task commit. T174 is the
next ready task. Constraint terms/application values remain T175/T177, producer
output equations/closure remain T179, executable integration remains T184 and
concrete template/native emission remains its planned later acceptance. Those
limits are deliberate stage boundaries, not silent fallback behavior.
