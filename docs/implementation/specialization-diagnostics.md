# Original-source diagnostics through specialization

T189 follows [T184](../../DONE.md#t184), [T185](../../DONE.md#t185) and
[T188](../../DONE.md#t188). Rules are
[COMP-04](../../spec/implementation/compiler.md#comp-04),
[NAME-03](../../spec/implementation/compiler.md#name-03) and
[DEBUG-01](../../spec/implementation/diagnostics.md#debug-01).
No specification change is authorized or required.

## Steps and evidence partitions

1. Check actual frontend failures with original definition, application and related
   spans. Cover kind/signature mismatch, unknown evidence, occurs and guarded
   cycles, producer completeness, output definitions/equalities and residuals.
   Render before source owners expire, preserving allocation-free diagnostics.
2. Compile source with Unicode, spaces and internal hyphens. Observe native C
   errors and managed runtime failures through a captured ordinary caller,
   preserving original paths/byte coordinates and active call sites. Virtual
   identities are metadata, never invented filesystem source paths or runtime
   invocations of static producers.
3. Check order/cache independence, refused allocations and publication/native-
   companion preservation. Correct actual source-map/diagnostic consumers only
   when independent cases expose a defect; run relevant existing tests and
   sanitizer/documentation acceptance before archival and a separate commit.

## Boundary

Diagnostic records borrow sources and related arrays owned by frontend stages;
those owners must outlive rendering. Runtime source rows borrow immutable compiled
strings. Active caller paths stream from existing operation/return metadata before
cleanup; compiler static contexts are not runtime call frames. Error formatting
must not reinterpret a generated hash as a source name or parse a virtual identity
as a filesystem path. The first check uses a genuinely compiled captured caller
with an original Unicode/multiword/hyphen filename and an actual native failure.
The initial runtime source-map case required no production source-map change; frontend attribution corrections are recorded below.

Initial source-map run: session 45827,
`/tmp/ama-t189-runtime-source-before.log`, strict O2/Werror. Collect its completion
before editing its maintained inputs. It checks the original Unicode/multiword
captured caller path and root invocation line through managed native failure;
subsequent cases will strengthen path punctuation and check-only/C-error domains.

## First outcomes and source-provenance correction

The initial Unicode/multiword runtime case passed
(`/tmp/ama-t189-runtime-source-before.log`), then the strengthened path with an
internal hyphen and checks excluding fictitious factory/hash frames passed
(`/tmp/ama-t189-runtime-path-profile.log`). Actual runtime source maps need no
production change for that case.

The independent incomplete-producer frontend case failed as intended:
`/tmp/ama-t189-producer-provenance-before.log` reports only factory:1:1 and no
requiring ordinary source. Static export resolution now attaches an actual caller
attribution owned by its existing ordinary-type record. It does not alter creation
ancestry, infer missing inputs, replace the primary cause or discard existing
related notes. A new generic diagnostic context chain supplements those notes
without allocating on the failure path. Frame identity is idempotent only for the
same exact location/prose; rebinding is refused. Validation uses Floyd cycle
detection, then checks every note, with no recursion or arbitrary depth limit.
The renderer streams original notes followed by newest caller attribution first.

The corrected frontend provenance case passed
(`/tmp/ama-t189-producer-context-initializers.log`). Adding the internal record
field required explicit null initialization in five existing positional C
initializers; the initial strict build diagnosed the omitted field rather than
accepting indeterminate attribution (`/tmp/ama-t189-producer-context-after.log`).
Record tests passed, including 4096-node chains, self/nontrivial cycles, invalid
spans, immutable repeated notes and invalid callers
(`/tmp/ama-t189-context-records-immutable.log`). Rendering tests passed, preserving
old notes and rejecting cycles before any output (`/tmp/ama-t189-context-render.log`).

Current run: session 68514, `/tmp/ama-t189-producer-diagnostic-refusals.log`, strict
O2/Werror, every frontend allocation position through an unresolved producer and
allocation-free record rendering. Collect before editing its maintained inputs.
No task acceptance or commit is claimed: other error families, C-error mapping,
cache/order and composition/sanitizer/documentation checks remain pending.

All 1587 frontend allocation refusal positions passed strict and ASan/UBSan runs
(`/tmp/ama-t189-producer-diagnostic-refusals.log`,
`/tmp/ama-t189-producer-diagnostic-sanitizers.log`); the complete diagnostic
record/render parent also passed ASan/UBSan
(`/tmp/ama-t189-diagnostics-sanitizers.log`). Actual native C preprocessing error
checks passed, retaining the original Unicode companion path and protecting its
bytes plus existing output from failed publication
(`/tmp/ama-t189-native-c-source-and-publication.log`).

Known constructor signature errors had another missing definition context:
`/tmp/ama-t189-signature-kind-cycles-before.log` reports the bad foreign input but
not `.target`. Inventory-owned attribution now links the original known interface
without altering metadata. Both signature/category refusals and eager/shared
value cycle location cases passed
(`/tmp/ama-t189-signature-context-after.log`). Unknown ordinary input types already
reported their actual annotation; that strengthened case passed without changing
its resolution algorithm (`/tmp/ama-t189-unknown-input-site-before.log`).

A returned Template with an additional allocator residual was correctly refused
as an ordinary Type, but its export definition/residual witness were absent
(`/tmp/ama-t189-extra-residual-before.log`). The error now includes the actual
selected export definition and the first original residual in canonical formal
order. One remaining formal is a sufficient counterexample to complete Type;
metadata retains the entire residual set, and the two inline source witnesses
are not an attribution-depth budget. The corrected case passed
(`/tmp/ama-t189-extra-residual-after.log`).

Current strict expanded graph-family run: session 71949,
`/tmp/ama-t189-graph-diagnostic-families-before.log`. Collect before modifying its
inputs. It adds definition cycles, an occurs equation, changing-key recursion,
and distinct pair-definition/equality receipts. T189 remains incomplete; no
specification changes or completed-task commit have been made.

## Causal equality evidence

The expanded graph cases passed definition-cycle, occurs and changing-key
refusals but exposed lost pair-definition provenance after normalization
(`/tmp/ama-t189-graph-diagnostic-families-before.log`). Retaining only the
representative chain recovered the equality receipt but still omitted the
previous pair definition (`/tmp/ama-t189-equation-evidence-after.log`). The
corrected traversal passed all ten original frontend cases
(`/tmp/ama-t189-causal-evidence.log`).

Each successful variable binding retains its exact immutable equation. On a
conflict, an allocation-free worklist follows original variable/value links and
both original operands of each binding reason, including derivation parents.
An owned visited bit queues each variable once, so explanation cycles terminate;
no recursion or depth cutoff is involved. Equation parents predate children.
Each visited variable owns its definition/binding attribution frames. First
errors and existing concrete-type notes remain intact. These failure-only work
links cannot be reused because the solver status is sticky. Storage is linear
in variable count; equation-parent paths are finite. Context validation and
idempotent insertion can take quadratic time in the number of retained notes;
this concerns failure formatting, not inference or successful runtime execution.

The direct solver case passed both binding orders, including a cyclic graph of
explanations, with the next allocator call refused and no allocation attempted
(`/tmp/ama-t189-causal-component.log`). No obsolete independent implementation
was retained: the existing definition extractor is shared by primary and causal
notes, rather than duplicated.

Continuation checkpoint: strict full constraints and expanded frontend families
are running, respectively sessions 25932 and 24968; logs
`/tmp/ama-t189-all-constraints.log` and
`/tmp/ama-t189-all-frontend-families-fixed.log`. The initial added malformed-name
case used an opaque source field and failed strict compilation
(`/tmp/ama-t189-all-frontend-families.log`); it now uses `source_path`.
Remaining acceptance: collect these runs, current-input sanitizer runs, relevant
record/source/frontend consumers, build and documentation gates. T189 is not
complete and no specification file was changed.

Current expanded frontend checks passed strict and ASan/UBSan runs, including
1587 injected frontend allocation refusals, original malformed-name locations,
independent-bridge closure under reordered declarations/roots, and all graph
families (`/tmp/ama-t189-all-frontend-families-fixed.log`,
`/tmp/ama-t189-frontend-sanitizers-final.log`). The complete constraint parent,
current strict native-C/runtime diagnostic leaf, compiler build and source
integration also passed (`/tmp/ama-t189-all-constraints.log`,
`/tmp/ama-t189-codegen-final.log`, `/tmp/ama-t189-build.log`,
`/tmp/ama-t189-source-integration.log`).

Metadata verification caught one new field comment lacking its own canonical
rule link (`/tmp/ama-t189-docs-metadata.log`). It is corrected; this comment-only
change does not alter executable tokens. Current jobs: metadata session 6588,
publication parent 32415, causal component sanitizers 56600, native/runtime
sanitizers 33266, driver editor consumer 78502, plus the frontend composition
run in `/tmp/ama-t189-frontend-composition.log` (collect its returned session).
No current acceptance supersedes the known T196 stale conformance fingerprint
blocker recorded by T188; hashes remain untouched.

All remaining executable checks completed successfully: the complete publication
parent, direct causal-evidence solver under ASan/UBSan, codegen diagnostics under
ASan/UBSan, real driver/editor consumer and independent frontend composition
(`/tmp/ama-t189-publication-consumers.log`,
`/tmp/ama-t189-causal-component-sanitizers.log`,
`/tmp/ama-t189-codegen-sanitizers-final.log`,
`/tmp/ama-t189-driver-editor.log`, `/tmp/ama-t189-frontend-composition.log`).
The final expanded frontend runs cover **1599**, rather than the earlier 1587,
allocation positions after the retained equality evidence was added; strict and
sanitizer runs independently report the same current count. Allocation counts
are measured, not hard-coded acceptance limits. ASan/UBSan use leak detection
disabled because LSan is unavailable in this environment; counted teardown is
separate resource evidence.

For V variable records, E retained equations and N attached notes, failure-path
work takes O((V+1)E + N²) time in the worst case and O(V) owned space. Each
variable is queued once, each derivation-parent walk has at most E equations,
and validating/inserting notes costs a linear scan per insertion. The solver's
success path only retains an equation pointer at an existing binding; inference,
normalization, runtime layout and executable semantics are unchanged.

Metadata acceptance passed: 2983 ready inputs, zero documentation debts
(`/tmp/ama-t189-docs-metadata-final.log`). All jobs listed above have been
collected successfully. Actual Doxygen generation is the remaining gate, started
in `/tmp/ama-t189-doxygen-initial.log`; no executable job remains active.

Reproduction uses repository-root working directory. Strict runs set
`C_COMPILER_FLAGS='-O2 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`
on `make`, and on `make -C` with each of:
`tests/unit/compiler/specialization-diagnostics`,
`tests/unit/semantic/static/constraints`,
`tests/unit/semantic/static/publication`,
`tests/unit/codegen/specialization-diagnostics`,
`tests/unit/compiler/composition`, `tests/unit/driver/editor`, and
`tests/integration/source`.
Sanitizer runs use
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1`,
`C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie'`
and `LINKER_FLAGS='-fsanitize=address,undefined -no-pie'` on
`make -C tests/unit/compiler/specialization-diagnostics`,
`make -C tests/unit/codegen/specialization-diagnostics`, and
`make -C tests/unit/semantic/static/constraints/solve`.
Diagnostic record/render component sanitizers were checked independently at the
unchanged context implementation (`/tmp/ama-t189-diagnostics-sanitizers.log`).

Actual Doxygen validation found a source-navigation gap after generation:
`/tmp/ama-t189-doxygen-initial.log` reports a missing canonical link on the
constraint model's originating file page. Field comments rendered the rule on
struct pages, while the file purpose did not yet name that diagnostic rule.
The file purposes now also explain COMP-04 for constraint models, inventory
and ordinary publication types, so file-level navigation carries the same
contract. This changes comments only. Generation is rerun before archival;
there is no accepted rendered-documentation claim yet.

The corrected render then exposed TPL-08 on the ordinary publication struct's
field page but not its file page (`/tmp/ama-t189-doxygen-corrected.log`). Exact
comparison of canonical references against all changed originating file pages
isolated that additional link; the file purpose now explains residual metadata
under TPL-08 as well. No generator checks were weakened. Final corrected render
must pass before the task can be archived.

## Acceptance

All executable jobs and the complete documentation gate passed; no jobs remain
active. `make docs-complete` reports 2983 ready inputs and zero debts
(`/tmp/ama-t189-docs-complete.log`). Actual Doxygen validation passed with 2271
maintained file pages, 8842 documented interfaces and 443/443 canonical/page
destinations (`/tmp/ama-t189-doxygen-residual-link.log`). Tool versions are
Doxygen 1.11.0, Python 3.14.4 and markdown-it-py 3.0.0. The task is archived in
[DONE](../../DONE.md#t189). A final generation after archival must retain these
checks and match final input hashes; its receipt is
`/tmp/ama-t189-doxygen-archive.log`. Earlier session checkpoints above are
historical and collected, not pending work.

The task adds no alternate diagnostic implementation: original definition
extraction is shared, and context validation/rendering remain in their existing
components. Removed/replaced code has no retained competing version or dedicated
obsolete test. The per-file registry covers all new implementation/support/build
inputs. No specification or conformance fingerprints were changed; the known
T196 conformance review gate remains explicitly outside this task's acceptance.


The first archival generation correctly refused a maintained-input change:
`/tmp/ama-t189-doxygen-archive-before-eof.log` caught removal of a trailing blank
line in DONE while generation was running. No result from that run is accepted.
All archival prose and whitespace are now fixed before the isolated final rerun
(`/tmp/ama-t189-doxygen-archive.log`); staging and manifest comparisons must match
these final bytes before committing.
