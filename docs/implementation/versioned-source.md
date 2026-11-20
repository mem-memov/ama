# Versioned source identity and package selection

[Integration task T231](../../DONE.md#t231) and
[foundation task T258](../../DONE.md#t258). Status: T258 foundation accepted;
T231 scoped integration acceptance is complete after foundation commit `8361282`; final delivery regeneration and commit are recorded below. Whole-sprint acceptance remains T237. The user authorized implementation on
2026-11-20 after T243 planning.
The canonical rules are [PKG-01](../../spec/project/packages.md#pkg-01),
[PKG-02](../../spec/project/packages.md#pkg-02),
[SRC-01..05](../../spec/language/source-and-names.md#src-01),
[SYN-06](../../spec/language/source-and-names.md#syn-06) and
[TPL-01](../../spec/language/templates.md#tpl-01). This record does not amend them.

## Planned steps and acceptance

| Step | Purpose / handoff | Verification | State |
|---|---|---|---|
| 1 | Recognize bounded exact version components and canonical identity structure without filesystem access or numeric overflow | Independent version/path positive, negative, boundary and bounded exhaustive cases; standalone headers | T258 and canonical consumer components passed |
| 2 | Select whole package-version copies before module discovery; retain physical plus/source mapping and exact aliases/errors | Real catalog/source snapshot, missing-local-hole, duplicate, Unicode and allocation/I/O checks | Physical/projected catalog and fault-injection consumers passed |
| 3 | Recognize slash paths and single-dot atomic static names without lookup; preserve source/literal spans | Lexer/parser components and real ordinary/static/constant consumers; malformed spacing/separators | Scoped components/consumers passed; final delivery generation follows archival |
| 4 | Migrate identity/dependency/native/source-tool consumers and retained fixtures; delete obsolete dotted-root routes | Actual composed semantic/codegen/native/editor/relocation tests and compiled execution | Scoped components/consumers passed; final delivery generation follows archival |
| 5 | Reconcile correctness/ownership arguments, per-file evidence and documentation; archive and commit | Required focused/parent checks, source links, final documentation provenance | Scoped components/consumers passed; final delivery generation follows archival |

## Initial findings

At input commit `2dc75ac`, source catalog paths, identity component splitting,
lexer root tokens and parser static names still implement the superseded dotted
model. Physical plus-directory storage and byte-exact filesystem probing are
existing reusable foundations. A complete package selection must precede their
module-level duplicate checks, rather than becoming module shadowing.

Version numbers are lexical digit strings, not host integer values. Recognition
must therefore accept arbitrarily long finite decimal components without numeric
conversion, while enforcing the exact zero/leading-zero rules. Variant validation
reuses the pinned single-word Unicode predicate. All input views remain borrowed;
recognition allocates nothing and changes no bytes or filesystem state.

The original queue's authorization summary was updated to record the new user
request. T222's specification amendment remains a separate gate. No ambiguity
has been identified in the currently examined version/identity contracts.
The first implementation inventory showed that bounded recognition and physical
version selection have independent public contracts and tests. They are extracted
as T258, with T231 retained as their explicit compiler/catalog/parser integration
owner. This is a task/commit boundary refinement inside the authorized scope,
not a new language rule or a declaration that the remaining T231 work is complete.

## Evidence and continuation

Step 1 kernels are implemented in `src/source/path/version.{c,h}` and
`src/source/path/logical.{c,h}`. The existing catalog/parser still use their old
paths pending the next migration step; these kernels alone do not establish
versioned project support. T258 owns these independently tested components; T231
owns their pending integration with compiler/catalog/parser consumers.

`make -C tests/unit/source/path` passed twice as the version and endpoint
partitions were added. The expanded leaf then passed under ASan/UBSan with
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1` and
`C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'`.
Exit 0; log `/tmp/ama-t231-path-sanitizers.log`. This is memory/undefined-behavior
instrumentation, not a LeakSanitizer claim. The source parent
`make -C tests/unit/source` also exited 0
(`/tmp/ama-t231-source-parent-step1.log`), exercising the newly registered leaf
through its real parent as well as unchanged source regression consumers.

The leaf checks all 97,656 strings of length 0..7 over `v01.-` against an
independently constructed finite grammar language. At these lengths no variant
fits, and numbers have at most two digits from that alphabet; the grammar's
possible numeric strings are exactly `0`, `1`, `10`, `11`. Independent positive
and negative vectors separately cover Unicode variants, truncated/malformed
encoding, hyphen/space rules and host-range independence (4096 digits per field).
576 constructed package-shape combinations check structural endpoint membership.
Bounded unterminated/NUL views and 1024-component finite namespaces cover byte
bounds and absence of a semantic depth cap. These finite domains are not a claim
to enumerate the infinite accepted language.

Correctness: decimal scans consume precisely a nonempty digit run and reject
multi-digit leading zero; three runs have exactly two intervening dots and only
the optional complete word remains. The endpoint state records vendor, one-or-more
package names, the unique version boundary, src/tests and one-or-more module
names; the standard branch instead requires only complete ordinary names.
Every successful loop advances a bounded cursor. The components and Unicode
word scans cover disjoint spans a constant number of times: O(input bytes)
time and O(1) auxiliary space, no integer conversion/allocation or retained views.

New source/header/test/Makefile evidence is registered by exact paths and ten
named assertion cases in the existing per-file registry. The first
`make docs-check` exited 2 because the two new headers lacked their explicit
standalone compilation commands in that registry. Added those commands using
the existing strict C11 header contract. The corrected `make docs-check` exited
0 with 3034 ready inputs and zero debts
(`/tmp/ama-t231-docs-check-step1-fixed.log`), including both independent header
compilations and actual named-case interface checks.

## Whole-version selection contract and argument

`src/source/module/packages/packages.h` exposes selection for a fresh initialized
catalog, stable borrowed root descriptors and a finite prepared filesystem tree.
It does not claim a filesystem snapshot against unrelated concurrent writers.
It owns no source/parser state and reads no version's modules or configuration.
It selects candidate version directories; validation/loading of the selected
version's required files remains a later consumer's obligation, with no fallback.

The output is a sorted linked list in the catalog arena. All path strings and
nodes remain owned until catalog destruction; each node's root descriptor is
borrowed through the enclosing selection/indexing call. The output pointer
changes only on complete success. Partial queue/selection storage and exact
error context remain owned for inspection/destruction after any failure.
The operation never modifies source files or publishes a partially built module
catalog. It cannot be used on an already discovered/indexed catalog.

Root validation, optional storage discovery, candidate classification, FIFO
ownership and sorted selection publication have separate implementation files.
First process every configured ordinary root's local storage, then every external
storage. For each candidate, the logical key is the full vendor/package/version
prefix. Consequently a local key is already known when an external candidate is
considered; ignore the latter without entering its version contents. Same-priority
duplicates fail rather than merging trees or selecting by traversal order.
Different versions and complete variants remain distinct; sorted reporting is
not a highest-version choice. Standard roots do not contribute ordinary versions.

Induction over queued namespace directories preserves their exact physical,
relative and logical prefixes and vendor/package depth. A version boundary is
recorded, never enqueued; non-directory files outside versions are not module
candidates. Pinned Unicode/name and version predicates decide candidate spelling
without looking up declarations. Physical directory registration rejects aliases
before requeueing, including empty repeated project roots. Every successful scan
consumes a finite entry list, and each namespace directory is queued once, proving
termination without recursive C calls or a semantic depth bound.

Sorted insertion preserves one record per key and publication only after both
passes establishes whole-version precedence independently of root order. Exact
probes/entry enumeration own and close their streams; catalog arenas own all
scratch, so aborting a child leaves no detached allocation owner. A code review
found that storage-path allocation failure could otherwise retain a caller's
root string in the error record. Discovery now uses the already catalog-owned
verified root spelling; allocation-failure tests reject retaining the borrowed
root address. This preserves diagnostic lifetime even when another allocation
cannot succeed.

Let D count registered namespace/root directories, C version candidates, V
selected versions, B retained path/name bytes and L the largest logical-key byte
length. Directory identity checks cost O(D squared); simple sorted selection costs
O(C V L). Add B, the existing entry-sorting cost and exact physical-probe cost
(including their directory reads) for total work. Arena space is O(B + D + V + E)
for E retained enumeration records. Filesystem/tool limits remain external
failures, not invented limits on the source language.

## Selection verification and remaining delivery gates

The independent leaf `make -C tests/unit/source/module/packages` passed its initial
six scenarios (`/tmp/ama-t231-packages-initial.log`), then the expanded descriptor,
physical-failure and I/O partitions (`/tmp/ama-t258-packages-io.log`). The final
owned-error-context implementation also passed strict ASan/UBSan with the same
flags recorded above (`/tmp/ama-t258-packages-owned-context.log`, exit 0).
Earlier successful instrumented runs are retained as historical input boundaries
in `/tmp/ama-t258-packages-sanitizers.log` and
`/tmp/ama-t258-packages-io-sanitizers.log`.

Nine explicit selection cases cover empty/dependencies-only roots, hierarchical
packages, exact sorted version identities, local priority across both root orders,
local and external duplicate refusal, misplaced boundaries, symlink/UTF-8/storage
shape errors, invalid/repeated descriptors and output preservation. Each measured
allocation can fail in turn; every measured opendir/readdir/closedir/lstat call
can independently report EIO. The I/O fixture uses actual directory operations,
counts acquired streams and requires zero live streams on every exit. Simulated
close failure releases its actual resource before reporting the error, making
cleanup observable rather than claiming unspecified close behavior is universal.

`make -C tests/unit/source` passed with both new leaves registered in the real
parent (`/tmp/ama-t258-source-parent.log`, exit 0). It includes existing source
regressions; their old frontend behavior is not evidence of new project syntax.
`make` also exited 0 (`/tmp/ama-t258-build.log`), compiling/linking the actual
compiler with the new nested source files under its default O2/C11 flags. This
is neither a clean whole-sprint release nor generated Ama execution acceptance.

The per-file registry maps all eleven new production source/header files and
their test/Make/IO support to the ten path and nine selection cases. Five new
headers have independent strict C11 compilation commands. Source explains rule
links, ownership and failures; this record is routed through the component
contract registry. Initial `make docs` exited 0 (`/tmp/ama-t258-docs-initial.log`): 3045 ready
inputs, zero debts, 2312 maintained file pages, 8975 documented interfaces and
487/487 canonical destinations. Doxygen 1.11.0, Python 3.14.4 and pinned
markdown-it-py 3.0.0; the worktree provenance check verified all 2559 rendered
input fingerprints before archival.
No specification or reviewed complete-conformance fingerprints have changed.

The structural/link/task check verifies 258 unique IDs, acyclic prerequisite
closure, nineteen original build/release plan rows, all seventeen new maintained
inputs and five standalone-header commands. Its temporary parser was adjusted
to accept existing prerequisite-label capitalization/colon variants; the final
check passes. `make conformance-check` exits 2 with the expected newer-specification
refusal (`/tmp/ama-t258-conformance.log`); no complete witness is renewed.

Continuation: T258 implementation/component/parent/sanitizer and initial
documentation checks are complete. After archival, the final delivery command is
`make docs > /tmp/ama-t258-docs-final.log 2>&1`, with active-job metadata recorded
at `/tmp/ama-t258-active-job.json`. Collect its exit status and require final
source/staged provenance before committing; a lost/unconfirmed job outcome must
be checked or rerun safely, never inferred. T231 resumes actual catalog/parser/
identity/dependency/source-tool migration after that commit. T231 is now active:
first connect package selection/areas and canonical lookup to the actual catalog,
then migrate its fixture/identity consumers. No active jobs. T237 owns the eventual
whole-sprint conformance renewal; no compiler entry support is inferred from these
foundation tests.

## T231 integration checkpoint: catalog and identity consumers

After `8361282`, the actual catalog selects whole versions before walking their
required `src` and `tests` areas. A selected version must have a regular
`package.ini` and both areas; configuration contents remain T232's responsibility.
Physical exclusions operate after selection and never select an external copy.
The common region walker preserves plus-storage identities and standard-library
separation. Source lookup now validates canonical slash identities. Identity
encoding splits slashes, keeping version dots inside their component.

The catalog's existing source/override/native/enumeration/snapshot fixtures were
migrated explicitly, with four added compositions for selected local holes,
missing required local inputs, exclusions, and distinct external versions/areas.
`make -C tests/unit/source/module/packages`,
`make -C tests/unit/source/module` (including the added compositions),
`make -C tests/unit/source/identity`, and
`make -C tests/unit/native/namespace` all exited 0. Logs are respectively
`/tmp/ama-t231-selection-consumer-regression.log`,
`/tmp/ama-t231-catalog-selected-areas.log`,
`/tmp/ama-t231-identity-migration.log`, and
`/tmp/ama-t231-native-identities.log`. Frozen tuple bytes/digests were independently
constructed using counted UTF-8 frames and Python SHA-256, not copied from the
production encoder. The unchanged standard digest is a positive control.

Current step: shared bounded raw path recognition, then lexer/parser consumers.
No jobs are active. Sanitizers, broader consumers, per-file registration and
final documentation remain pending for T231; no complete compiler support claim
or specification change is made.

### T231 lexical/parser handoff

The shared bounded path-syntax recognizer now distinguishes raw `/`, `//`, `./`,
`../`, and `~` forms from canonical identities. It reuses maximal complete-name
and version-prefix operations, owns no state, and performs no lookup. Dots in
versions are not static-name separators. Lexical module paths are one operand;
static names use one dot and retain atomic descriptive components. Parser type
classification uses the leading lexical family even for malformed tails, so a
bad type is not reinterpreted as a different statement.

`make -C tests/unit/source/path` exited 0, adding complete enumeration of 21,845
strings of length 0..7 over `./a~` against an independently expanded grammar set,
plus Unicode/bounded/source-versus-canonical cases. `make -C tests/unit/lexer`
also exited 0 (`/tmp/ama-t231-path-syntax.log`, `/tmp/ama-t231-lexer.log`).
The parser fixtures were migrated through an explicit closed owner-file list;
C includes, payloads and malformed partitions were reviewed separately.

The first section run failed on an unmigrated old static spelling; after migration
`make -C tests/unit/parser` passed module/declarations/sources/tree/calls/bindings/
results/control, then failed its documented-example census: the specification has
additional fences since that historical fixed registry. This run is not a parent
pass (`/tmp/ama-t231-parser-migration.log`). The ordinary complete-constructor
examples also belong to pending T226 support, not a claim of current acceptance.
`make -C tests/unit/parser/static` passed separately. The new access-span test
initially missed its stdio include; after that fix access passed. The errors leaf
then exposed malformed type-family classification/separator handling; those were
fixed rather than changing expected diagnostics. `make -C tests/unit/parser errors
abi` passed after the fix (`/tmp/ama-t231-parser-errors-abi.log`). Root `make`
passed (`/tmp/ama-t231-compiler-build.log`); this does not establish generated
execution. Current integration continues with canonical builtin, dependency,
static-key and library-identity consumers. No active runs remain.

### T231 dependency and native-library integration checkpoint

`make -C tests/unit/project` now passes both closure and input-capture leaves
(`/tmp/ama-t231-project-migration-spans.log`, exit 0). Fixtures use explicit
package metadata/areas, preserve actual plus storage, and distinguish separate
packages across roots; a duplicate selected version is refused even without a
module-level collision. Existing graph/permutation/depth and measured allocation
failure assertions remain. The first migration run failed stale hand-authored
AST offsets. Its source was edited while that failed job was active; that run
is not acceptance evidence. The collected fresh run uses independently measured
fixture-prefix lengths and passes.

Thirty-four maintained standard Ama interfaces now name slash-rooted standard
modules; C/header/ABI companion contents are unchanged. Exact builtin references
in dependency, list, native-bridge and transport-profile consumers use those same
canonical identities. Static-key origin tagging uses the explicit slash root.
Library component helper names anchor to the bytewise least actual exported
canonical module path rather than a fabricated non-module name; every key still
contains the complete explicit ABI configuration. This finite minimum is
independent of discovery/link order, costs O(E L), borrows immutable export paths,
and allocates nothing (E exports, L maximum path length).

`make -C tests/unit/library/resolution` passed the migrated typed/source/native
profile and failure scenarios (`/tmp/ama-t231-library-resolution-final.log`).
The added explicit reversed-export test also passed
(`/tmp/ama-t231-library-anchor-acceptance.log`, exit 0), exercising the component
anchor independently of export ordering. Initial migration failures were fixture
suffixes `.h/.c/.abi` accidentally treated as module strings; those three literal
suffixes were restored, without changing production path rules. The fixture writer
now reports the failing physical path before asserting.

`make -C tests/unit/source` passed again after raw syntax and catalog integration
(`/tmp/ama-t231-source-parent.log`, exit 0). The final raw-path instrumentation
command uses `C_COMPILER_FLAGS`, not the ignored generic `CFLAGS` variable:
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 make -C
tests/unit/source/path C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11
-pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined
-fno-omit-frame-pointer'` passed (`/tmp/ama-t231-raw-path-asan-ubsan-final.log`).
An earlier run with `CFLAGS` was ordinary instrumentation-free acceptance, not an
ASan claim; it reused the older temporary log name mentioned in the historical
foundation record. LeakSanitizer fails in this ptrace environment, independently
reproduced by a malloc/free probe (`/tmp/ama-t231-lsan-probe.log`). The failed
LSan-enabled run is `/tmp/ama-t231-path-asan-ubsan.log`. ASan/UBSan with that
explicit environment restriction and counted ownership checks remain distinct
from any leak-instrumentation claim.

Current job: `make -C tests/unit/semantic/static/registry` at
`/tmp/ama-t231-static-registry-vectors-fixed.log` (session 50272). Its frozen
counted-text and binary SHA-256 vectors were constructed independently, preserving
Unicode lengths and standard/ordinary root tags. The first fixture-generation
attempt had an invalid C string from a temporary text replacement stopping at a
semicolon inside the literal; corrected before this fresh run. Collect its status
before changing its code inputs. Broader semantic/native/source-tool fixture
migration and final per-file/documentation/acceptance remain unfinished.


The complete static registry parent passed (identity vectors/laws/failures,
candidates, scopes, environments, checks and cache):
`/tmp/ama-t231-static-registry-vectors-fixed.log`, exit 0. The independent new
static descriptor vectors are retained in maintained assertion source, not only
the temporary calculator. Catalog ASan/UBSan also passed with the explicit flags
and leak restriction above (`/tmp/ama-t231-catalog-asan-ubsan.log`, exit 0).
`make docs-check` initially refused one header contract lacking its own clickable
canonical rule link (`/tmp/ama-t231-docs-check-progress.log`); that link was added.
Current read-only investigation: projected relocation discovery must use the same
whole-version selection before indexing final module areas; its former flat-root
walker cannot simply exchange dot separators. No jobs remain active.

### Next boundary: projected package discovery

Relocation observes a read-only projected filesystem, not merely renamed source
bytes. Its old final walker starts at a flat root and therefore cannot share the
new package semantics by changing a separator. The next step keeps one selection
algorithm and supplies explicit root-metadata, child-metadata and directory-entry
operations for physical versus projected views. Exact directory identity insertion
will consume already checked metadata. Reader callbacks borrow their context only
through selection; selected strings/errors remain selection-owner-owned. No global
filesystem state or hidden compatibility mode is introduced. The preview owns a
separate projected selection catalog until teardown so failure paths cannot dangle.
Acceptance must exercise the real physical selection regression plus projected
whole-version precedence/collisions, source rewrites, exclusions, and measured
allocation/I/O cleanup before claiming source-tool support.

Projected-reader continuation: shared selection now accepts a synchronous
root/child/entries reader. Physical selection regression passed after this
refactor (`/tmp/ama-t231-reader-physical-regression.log`, exit 0). Directory identity
insertion consumes checked view metadata, with one alias relation rather than a
second projected identity registry. The preview owns `after_catalog` independently
of original sources. Selected final version areas now use slash logical prefixes.
The first compiler build exposed a tool adapter calling a catalog-private error
helper; the adapter now owns its explicit issue assignment instead of importing
that private function. Current build log:
`/tmp/ama-t231-projected-reader-build-fixed.log` (session returned by the tool;
collect before claiming success). Projected fixture migration, callback boundary
cases and per-file registrations for the new reader/adapter files are pending.
The prior `make docs-check` did pass after the missing link fix (3049 ready, zero
debts); it predates these reader files and is not current complete metadata proof.

The fixed compiler build completed successfully
(`/tmp/ama-t231-projected-reader-build-fixed.log`, exit 0). Reader model acceptance
passed (`/tmp/ama-t231-reader-virtual-acceptance.log`, exit 0): one independent
finite virtual tree, no physical I/O even with the first physical call set to
fail, unchanged output at every reader callback refusal, complete reader
preconditions and counted teardown. Ignored dependency versions are listed but
have no readable model metadata, proving selection skips them before probing.
The model uses explicit stable inode identities, never a filesystem lookup as
its expected selection result.

The migrated complete preview leaf passed
(`/tmp/ama-t231-relocation-preview.log`, exit 0), including carried/added roots,
standard-family moves, collisions, aliases, plus anchors, drafts, long cwd and
every measured preparation allocation. Same-priority collision evidence now
moves one complete version into a second root that would duplicate an existing
selected version; the obsolete original-root merge scenario was removed.
Two additional compositions cover ignored invalid external bytes/local holes and
a missing projected local test area. The latter's first run compared an absolute
normalized failure path with the relative request spelling; its assertion now
uses the canonical source path established by endpoint validation, without
changing production diagnostics. Current final run:
`/tmp/ama-t231-relocation-package-final.log` (collect its tool session before
claiming success). Remaining source-tool reference/staging/transaction consumers
still need migration and acceptance. T231 is unfinished and uncommitted.

The fresh twelve-case preview run passed after cleaning the fixture tree retained
by the intentional failing earlier run
(`/tmp/ama-t231-relocation-package-clean-final.log`, exit 0). The intermediate
rerun had failed the fixture's mkdir-new assertion because preserved failure
outputs existed; it was not a production regression. The migrated real reference
analysis leaf also passed (`/tmp/ama-t231-relocation-references.log`, exit 0),
including source cycles, external importers, native constant namespace/member
separation, new analysis scopes, an initial complete-version identity conflict,
overlap refusal and every measured analysis allocation failure. Literal payloads,
comments and opaque companion C strings remain deliberately unchanged; only
structured module operands and their expected identities were migrated.
Current command: driver help/check/warning-output regression at
`/tmp/ama-t231-driver-help-and-check.log`. New source-view reader/preview inputs
still require per-file registration; staging/report/transaction/recovery and other
semantic/native/frontend fixtures remain unfinished owners of T231 integration.

Driver help/check/warning-output tests passed
(`/tmp/ama-t231-driver-help-and-check.log`, exit 0). Help now uses MODULE/ENTRY/
SOURCE placeholders rather than obsolete dotted paths and names the // standard
root. The actual new versioned warning fixture retains exact warning positions
and refusal when its stderr stream cannot be written.
A manual cross-stage smoke also passed: fresh
`/tmp/ama-t231-compiled-smoke-ia94v3sl/project` contains
`packages/acme/demo/v1.0.0/{package.ini,src/main,tests/}`, with main declaring
`number` of `//integer` initialized from `|1`. The first invocation rejected
an incorrect SDK argument (a nonexistent output directory); rerunning
`./ama --emit /tmp/ama-t231-compiled-smoke-ia94v3sl/project
/acme/demo/v1.0.0/src/main --standard-root stdlib/modules --out
/tmp/ama-t231-compiled-smoke-ia94v3sl/output --sdk src --compile` exited 0
(`/tmp/ama-t231-compiled-smoke-sdk-fixed.log`). This is actual native C compile/
object verification/link/publication evidence for that finite input, not whole
sprint coverage or CMake support. Execute its published `output/program` and
collect the result (`/tmp/ama-t231-compiled-smoke-execution.log`) before claiming
execution. No generated or binary contents are analyzed.

The published smoke program exited 0 with no stdout/stderr
(`/tmp/ama-t231-compiled-smoke-execution.log`). No execution of unverified or
merely generated C is claimed. Shared directory metadata insertion is named
`module_remember_directory_metadata`, distinguishing it from unrelated native or
file metadata. No jobs remain active at this checkpoint.

The whole relocation parent passed preview/references/staging/report and transaction
preflight/preparation/decision, then failed application's independent byte oracle
(`/tmp/ama-t231-relocation-parent.log`). Three text comparisons still passed the
old fixed 16-byte length after their expected path grew; they now measure their
explicit expected text (binary assertions retain explicit byte lengths). The
actual cross-filesystem variant uses an explicitly standard destination root,
which permits its selected module subtree directly, and independently expects
`//new/a`; same-filesystem expected identities retain the versioned project path.
This preserves the actual device-ID/copy/metadata oracle without pretending an
unpackaged ordinary destination has a module identity.

An attempted failed-leaf cleanup could not remove its retained read-only fixture
directories; the following rerun also failed fixture setup and is not acceptance.
After collecting that result, only that generated tree was made owner-writable
and cleaned successfully. Current fresh application run:
`/tmp/ama-t231-transaction-application-clean-final.log`, session 56898.

The semantic parent passed earlier children but failed inference on unchanged
one-dot fixture operands (`/tmp/ama-t231-semantic-parent.log`). Eight additional
support-only owners had been omitted from the first migration inventory because
they contained no generated root string/compound dot. Their independently useful
cases were retained and migrated. The inference leaf then passed
(`/tmp/ama-t231-inference-missing-fixtures.log`, exit 0). Other semantic children
still require collection, and no semantic-parent pass is claimed. Temporary
migration manifests record exact before/after literals; maintained tests and this
record own the resulting contract/evidence.

Current source-tool documentation now describes version selection and slash
identities, with historical T016/T206 evidence separated from current support
claims. The low-level native buffer harness's fixed category-7 context uses an
independently counted/framed canonical module path; its complete hook leaf passed
(`/tmp/ama-t231-native-buffer-context.log`, exit 0). This is C ABI harness evidence,
not discovery of an Ama file in that fixture's C-only directory.

Semantic continuation: types/calls/selected/lists/control/continuations passed in
`/tmp/ama-t231-semantic-dynamic-paths.log`, before the finite graph witness case
failed a character-offset-based test lookup. Its independent 512-graph oracle is
unchanged; diagnostic vertices now map by exact equality against the three full
canonical test identities, rather than assuming a one-character module after a
dot. The previously suffix-exempt `.c` graph label was explicitly made a complete
module identity. Current remaining semantic run is
`/tmp/ama-t231-semantic-exact-witnesses.log` (session 43692).
Current independent application run remains session 56898 at
`/tmp/ama-t231-transaction-application-clean-final.log`; do not relaunch while its
allocation-failure/publication checks execute. The metadata/header/case-interface
check is `/tmp/ama-t231-docs-check-reader-and-consumers.log` (collect its session).
No complete T231 or sprint acceptance is claimed.

The reader/consumer metadata check completed with exit 0
(`/tmp/ama-t231-docs-check-reader-and-consumers.log`); it checks per-file purpose,
real C case signatures and standalone headers, not execution of every mapped
command. Source edits after that check require the final task gate to rerun.
Active jobs are still semantic remaining components (43692) and transaction
application (56898); no duplicate run is authorized by an unconfirmed status.

Planning fixture migration has started with explicit `generated/program` and
`generated/project` source roots. The parent stopped at its specialization fixture
because that fixture uses the independent compiler common owner, which does not
yet provide the new package constructor
(`/tmp/ama-t231-planning-parent.log`, compile refusal, not a planning semantic
failure). Core planning children run independently at
`/tmp/ama-t231-planning-core.log`; specialization remains unfinished. Do not edit
compiler/project shared test owners while the active semantic/application runs
still consume them. A shared fixture-construction boundary or an explicit compiler
fixture constructor must be completed with its consumers after those jobs end.

Transaction application completed with exit 0
(`/tmp/ama-t231-transaction-application-clean-final.log`): all 2420 measured
allocation refusals, all 1066 I/O checkpoint refusals, retained incomplete inverse,
actual same/cross-device publication and stale-scope refusal. The final make's
successful cleanup completed as well. Core planning ir/validation/storage/
transfers/structured passed (`/tmp/ama-t231-planning-core.log`, exit 0).
Remaining semantic job is still 43692, now past most static registry children;
recovery started at `/tmp/ama-t231-relocation-recovery.log` (collect its session).

A small shared test-package header has been prepared and independently compiles:
`tests/unit/fixtures/package.h`. It will replace duplicated direct-root fixture
construction and supply the compiler fixture's missing package constructor only
after the active semantic job has stopped reading those shared inputs. It has
not yet been connected or registered as tested execution; T231 owns that pending
step. It uses checked size sums and temporary fixture-only storage, with explicit
test vendor/package/version names and no source/directory-content inference.

Continuation: compiler parent completed successfully
(`/tmp/ama-t231-compiler-consumers.log`, exit 0). Remaining static constants
qualified/catalog/initializer completed successfully
(`/tmp/ama-t231-static-constant-final-components.log`, exit 0). The shared explicit
fixture constructor now serves project, library and compiler consumers; native
scaffold fixtures are being migrated without rewriting embedded C payloads.
Their handwritten module companions move with the main Ama file; output SDK
folders remain separate. Current jobs: static connections/closure (11466,
`/tmp/ama-t231-static-remaining-handoffs.log`), semantic specializations (37405,
`/tmp/ama-t231-semantic-specializations.log`), planning specializations (38101,
`/tmp/ama-t231-planning-specializations.log`), and native scaffold parent
(`/tmp/ama-t231-native-scaffold.log`; collect returned handle). Recovery remains
at session 60277 until its completion is collected. No complete task acceptance
is claimed; remaining fixture families, per-file mappings and final gates still
belong to T231.

Recovery replay/interruption/failures completed successfully
(`/tmp/ama-t231-recovery-destination-kinds.log`, exit 0), including both ordinary
packaged and standard cross-device destinations. Semantic and planning
specialization parents completed successfully (their logs above, exit 0).
A first native scaffold migration run exposed an error in the temporary fixture
conversion: a second conversion had mistaken already converted static names for
old module paths. That run failed and is not acceptance. The fixture owners were
regenerated once from their tracked originals, classifying only explicit Ama
fixture strings; native C payloads and SDK outputs remain untouched. Corrected
native scaffold run is session 85306. Editor fixtures now use canonical module
identities and single-dot static names, preserving comment/literal payloads and
explicit malformed-name partitions; their parent run is
`/tmp/ama-t231-editor-consumers.log` (collect its returned handle).

The corrected entire native scaffold parent passed
(`/tmp/ama-t231-native-scaffold-corrected.log`, exit 0), including independent C
header/body consumers, exact-root integer bridge, publication failures and
specialized namespace ownership. The new ordinary cross-device transaction leaf
passed strict `-Wall -Wextra -Werror -std=c11 -pedantic` compilation and execution
(`/tmp/ama-t231-package-placement-strict.log`, exit 0). It verifies both committed
placement and named importer-publication failure, with byte/inode preservation
of the preexisting package configuration and descriptor-count restoration.
Its Makefile, test file and shared `tests/unit/fixtures/package.h` now have
explicit per-file mappings. The shared header also independently compiled.

Codegen fixture migration preserves embedded C and exact comment/literal data,
changes only explicit Ama source groups and source-identity/physical-path operands,
and moves ordinary generated module areas under their explicit package. Standard
root metadata is now explicitly Standard in the common execution fixture, rather
than relying on the former accidental root policy. The first execution run
failed on an old expected `type .value` diagnostic substring: the real operation
correctly reported the full canonical value identity. Diagnostic assertions now
expect the specified canonical identity; successful execution/output expectations
are unchanged. Current codegen execution/composition run is session 84299 at
`/tmp/ama-t231-codegen-diagnostic-consumers.log`; metadata check is session 84243
at `/tmp/ama-t231-docs-check-new-fixture-boundary.log`. Both require collection.

Editor parent and the remaining static connections/closure parent completed
successfully (`/tmp/ama-t231-editor-consumers.log` and
`/tmp/ama-t231-static-remaining-handoffs.log`, exit 0). Codegen execution plus
library composition completed successfully
(`/tmp/ama-t231-codegen-diagnostic-consumers.log`, exit 0). Library
configuration/header/transport/snapshots/lifecycle/resolution also passed
(`/tmp/ama-t231-library-new-source-consumers.log`, exit 0), including actual C
transport and refreshed shared-fixture consumers.

The nested-library copy rejection fixture previously merged an unpackaged
scratch root with the parent root. It now has its own distinct `copy` package,
retaining the actual noncopyability oracle without violating whole-version
selection. A rerun before cleaning retained outputs was correctly refused by the
existing no-overwrite packager and is not acceptance. The subsequent clean run
passed nested-library scenarios but exposed the supervision fixture's moved
relative native include; that reference must be rebased to its moved owner.
The native ABI parent exposed an independently linked integer-arithmetic leaf
missing the now-real canonical path component dependency. Both are ordinary
consumer/build fixes, not specification questions.

Canonical maintained Ama examples may start with explanatory comments. The first
fixture conversion had omitted these files, producing the reported assembly
parse failure. Their complete explicit maintained owner list has now been
converted once from tracked original bytes, preserving comments and literals.
The affected codegen fixtures need fresh final acceptance; no old run is reused
for a file changed while it was being consumed. Current template run is 24872,
core codegen 35615; their logs are named above. The metadata run failed because
its new shared header lacked the registry's independent-compilation field; the
field now uses the actual strict command, and a fresh gate is still required.

The small native ABI Makefiles already inherit `source/path/version.c` through
the parser source list; adding it again produced a duplicate-definition link
failure. Their explicit additional dependency is now only `logical.c`.
The status-provider leaf owns its separate explicit list and adds both. No
sorting/deduplication wrapper masks inconsistent component ownership. Fresh
native boundary run: `/tmp/ama-t231-native-exact-link-dependencies.log` (collect
its returned handle). Moved-fixture/header metadata gate:
`/tmp/ama-t231-docs-check-moved-fixtures.log` (collect its returned handle).

The complete native ABI/toolchain/lifecycle/metadata/namespace/status-provider
run passed (`/tmp/ama-t231-native-exact-link-dependencies.log`, exit 0).
Supervision's clean rebased-include run passed
(`/tmp/ama-t231-supervision-rebased-includes.log`, exit 0). Actual exported
library execution passed after updating its appended ordinary body fragment
(`/tmp/ama-t231-library-execution-current-body.log`, exit 0). Library package
checks passed in the earlier combined run before that execution-body refusal.
The metadata gate passed with the required metadata/test-area fixtures now
explicitly mapped (`/tmp/ama-t231-docs-check-package-areas.log`, exit 0); further
maintained-input edits require another gate.

Next is actual CLI integration with maintained versioned fixture trees, not
only in-memory fixture construction. Twelve explicit integration source roots
are being moved intact under `packages/test/fixture/v1.0.0/src`, retaining their
required package.ini and tests area. Native header hashes are independently
computed from the documented category/root/component tuple using Python
struct/hashlib; generated output is not an oracle. Source paths in test commands,
companion includes and per-file mappings move with their owner. Remaining
integration families and the example projects still need their own migration
and acceptance. This does not implement aliases, acquisition or CMake commands.

Maintained parser/canonical-catalog integration passed after moving the
Unicode-named module inputs (`/tmp/ama-t231-parser-unicode-catalog.log`, exit 0).
The first move inventory used line-delimited Git output, which quotes non-ASCII
paths; the quoted spellings were not real paths and ten files were omitted.
The corrected explicit-root inventory uses `git ls-files -z`, preserving exact
Unicode names. That failure was fixture migration, not language acceptance.
The first CLI minimal build also exposed two frozen native parameter/result
view names, additional to its native namespace prefix. Independent version-two
category-1/role tuple calculations first reproduce both old frozen hashes, then
compute the new full-path hashes. No generated output supplies expected names.

Current CLI core run: 18263,
`/tmp/ama-t231-cli-core-current-native-roles.log`. Current driver parent: 54120,
`/tmp/ama-t231-driver-current-fixture-roots.log`. The preceding driver run exposed
a double physical-prefix conversion in its already migrated run fixture; that
fixture's explicit package paths have been restored. Real project/semantic
expected diagnostic spans are derived from the exact growth of each preceding
module-prefix substitution, preserving byte columns and token semantics, rather
than copying actual diagnostics. Their current logs are
`/tmp/ama-t231-project-maintained-closure.log` and
`/tmp/ama-t231-semantic-maintained-types.log` (collect returned handles).
The remaining broad codegen core run is still 35615; template handoffs run 24872
completed successfully and must not be relaunched without a new reason.

Core codegen assembly/layouts/bindings/control/library/list values/insertion/
deletion/ordering/trees/handlers/loops/positions/restart/snapshots/continuation
cleanup/continuations completed successfully
(`/tmp/ama-t231-codegen-core-migration.log`, exit 0). The CLI core run passed
check, minimal, objects and native-definition admission, then found the same
frozen view-role issue in the standard-integer observer. All explicitly
maintained native companion roles are now recalculated from independent framed
identity tuples, retaining published native payload behavior and output oracles.
Current remaining CLI run is `/tmp/ama-t231-cli-remaining-native-roles.log`
(collect its returned handle). Source package closure and Unicode nominal-Type
diagnostics both passed (`/tmp/ama-t231-project-maintained-closure.log` and
`/tmp/ama-t231-semantic-maintained-types.log`, exit 0).
Current remaining codegen parents: async consumers 64797,
`/tmp/ama-t231-codegen-async-consumers.log`, and value/transport consumers 64520,
`/tmp/ama-t231-codegen-remaining-values.log`. Do not edit their fixture inputs.

Driver parent passed with one shared explicit package fixture constructor
(`/tmp/ama-t231-driver-shared-package-fixtures.log`, exit 0). The earlier run
fixture's manual hierarchy had already been migrated before the broader driver
conversion; reapplying a prefix had duplicated its intermediate directories.
Replacing that duplicate construction with the shared fixture boundary retains
the warning/output-refusal oracle and avoids a second hierarchy implementation.

Published async/template examples and the synchronous example integration tree
now own real canonical package source areas, moved with every native companion.
Their native namespace and view-role hashes use independently framed tuples;
source-view consumers use the new physical locations. README links and the native
host builder's explicit export identity were updated with those owners. Current
runs: published examples 93252 (`/tmp/ama-t231-published-canonical-examples.log`),
async library 68551 (`/tmp/ama-t231-async-library-source-consumer.log`), and fresh
post-move template consumer `/tmp/ama-t231-template-example-package-consumer.log`.
Native task service authoring checks run at
`/tmp/ama-t231-task-service-authoring.log`; collect their returned handles. Earlier
pre-move template execution remains historical evidence rather than a claim about
these new physical source locations. No specification files have been changed.

The moved async-library source consumer, native task phase/generation authoring,
and post-move template-algorithm consumer all passed (respectively
`/tmp/ama-t231-async-library-source-consumer.log`,
`/tmp/ama-t231-task-service-authoring.log`, and
`/tmp/ama-t231-template-example-package-consumer.log`, exit 0).
The first published synchronous example run correctly failed its existing output
oracle: its file-open success case still named the old physical `project/files`
fixture as literal data. Only that explicit file target was updated to its moved
owner; the deliberately absent path and the exact expected success/failure output
remain unchanged. Fresh example run:
`/tmp/ama-t231-published-examples-current-data-path.log` (collect its handle).

Source-tool fixtures retain raw CRLF/native/draft bytes while their explicit
ordinary root and importer root become separate complete packages. The first
source-tool run exposed its Python oracle's split `Path("project", name)` still
naming the old area. Python fixture rewrites now use Python's tokenizer, preserving
method/attribute syntax rather than scanning quote-like substrings. Current
source/debugger/workflow run is 69060 at
`/tmp/ama-t231-tools-tokenized-fixture-expectations.log`; remaining relocation
collision setup still requires a complete explicit destination package. No
successful relocation-process acceptance is claimed until that case is migrated.

The async codegen parent passed debugger/parallel supervision/channel waits/
channels/message codecs/executor transfer/parallel contexts, then a maintained
retainer companion failed its frozen old native-header include. Native context
and view-role fixtures across the explicit maintained codegen native module
inventory are now recalculated by the same independent identity tuple oracle.
The old async run is a partial pass, not whole-parent acceptance. Current
remaining async run: `/tmp/ama-t231-codegen-async-native-contexts.log` (collect
returned handle). The value parent passed failure values/scalars/traversal, then
list-frame-layout's dynamic fixture helper wrote its ordinary value directly at
the old root. Its caller now passes the explicit source area while the Standard
caller remains a Standard root; the matching traversal-layout fixture is aligned.
Current remaining value run: `/tmp/ama-t231-codegen-explicit-layout-areas.log`.

The source-tool expectation rewrite exposed why language-specific tokenizers are
necessary: an initial quote-substring conversion crossed Python string/method
boundaries. Python oracles were regenerated from tracked originals using tokenize;
C owners were regenerated using complete escaped C string literals. Their actual
command/byte expectations remain independently specified. Fresh tool run is
64308 at `/tmp/ama-t231-tools-exact-fixture-inputs.log`; collect its result before
editing inputs or relaunching. The relocation collision fixture now prepares a
second complete same-version package, testing the actual initial conflict rule.

The full lexer parent and diagnostic adapters passed
(`/tmp/ama-t231-lexer-parent-current-paths.log` and
`/tmp/ama-t231-diagnostic-adapters.log`, exit 0). Source/debugger tool process
checks passed in `/tmp/ama-t231-tools-exact-fixture-inputs.log` before workflow's
hidden `.client-stage-` test filename was mistakenly converted as a module name.
That exact filesystem filename has been restored; the subsequent complete
workflow scenario passed in `/tmp/ama-t231-tools-explicit-hidden-stage.log` before
its next relocation leaf's stale acknowledgement substring failed. Fresh
relocation process run: 17189,
`/tmp/ama-t231-tool-relocation-current-ack.log`. The host builder's remaining
Standard scalar configuration records now use canonical // paths; fresh host
run: 11164, `/tmp/ama-t231-host-canonical-schema-records.log`.
Real source diagnostic fixtures now test complete-version conflict (rather than
obsolete independent-root basename merging), canonical Unicode lookup and invalid
UTF-8 bytes inside the selected source area. Expected source byte ranges are
unchanged where contents are unchanged; only their actual physical/logical owner
paths moved. Native authoring likewise constructs an explicit selected package
before its existing Unicode create/overwrite/command-boundary tests.

Real Unicode source diagnostics/native authoring/launcher/native-toolchain
integration passed (`/tmp/ama-t231-real-source-native-authoring.log`, exit 0).
The published async host now passes with all explicit canonical export/scalar
records (`/tmp/ama-t231-host-canonical-schema-records.log`, exit 0); its intentional
missing-argument/duplicate-metadata controls print errors inside a successful
harness, not product acceptance failures.
The complete source-transaction/logical-move/whole-container/debugger/rebuild
workflow passed in `/tmp/ama-t231-tools-explicit-hidden-stage.log` before the
next relocation leaf failed its old acknowledgement. The ensuing relocation
collision case now expects the existing `collision` classification: complete
same-version conflicts are detected by preview's package catalog, whose explicit
`MODULE_AMBIGUOUS -> RELOCATION_COLLISION` adapter supplies the normal diagnostic.
The old `invalid-reference` result belonged to the abandoned late basename merge.
Current relocation-only run is 3258 at
`/tmp/ama-t231-tool-version-conflict-status.log`.

Published C-library tooling now selects canonical ordinary/Standard model keys
and explicit export records from the already moved supervision package fixtures.
Its actual standalone/nested static/shared consumer run is 37695 at
`/tmp/ama-t231-published-c-library-roots.log`. Controlled product reproducibility
fixtures now have separate root/source-area owners and complete conflicting
versions; run 65906 at
`/tmp/ama-t231-product-reproducibility-package-identities.log`. Final sanitizer
validation of the complete source/module parent has started at
`/tmp/ama-t231-source-module-final-sanitizers.log` (collect returned handle), using
strict C11/Werror with AddressSanitizer/UndefinedBehaviorSanitizer and explicitly
disabled unavailable LeakSanitizer. Other counted allocation/FD failure checks
remain separate evidence; no LeakSanitizer pass is claimed.

Complete source/module sanitizer parent passed
(`/tmp/ama-t231-source-module-final-sanitizers.log`, exit 0), with the explicit
flags/environment above. Layout/header-location consumers also completed
successfully (`/tmp/ama-t231-codegen-layout-header-locations.log`, exit 0).
The actual native application wrapper and all source-module reader callback
failure checks remain independently exercised by their registered components.
Real relocation preview/apply/repeated recovery/initial-version-conflict process
acceptance passed (`/tmp/ama-t231-tool-version-conflict-status.log`, exit 0).
Published C-library examples passed static/shared and nested native consumption
(`/tmp/ama-t231-published-c-library-roots.log`, exit 0). All four controlled
product reproducibility/input-change/diagnostic/signal-control scenarios passed
(`/tmp/ama-t231-product-reproducibility-package-identities.log`, exit 0).

Current async codegen remaining run is 9805 at
`/tmp/ama-t231-codegen-current-async-fragments.log`: the previous run passed task
boundary ownership but then found one maintained no-annotation body fragment
(`detach-idempotence`) still using old Standard paths. The explicit source-owner
inventory contains just that remaining fragment; its two Type operands are now
canonical. Its actual cancellation/output/idempotence assertions are unchanged.
Current documentation metadata/header gate is
`/tmp/ama-t231-docs-check-canonical-consumers.log` (collect returned handle).
Documentation links outside spec have been rebased to their actual moved input
owners; no root specification copies or compatibility inputs were retained.

Documentation inventory and independent documentation-tool tests passed with
INI inputs included (`/tmp/ama-t231-docs-check-including-ini.log`: 3112 ready,
zero debt; `/tmp/ama-t231-documentation-ini-inventory.log`, exit 0). The gate's
new finite inventory cases retain declared package/module INI files while
excluding generated INI files; no existing input classification is removed.
This metadata pass predates the later explicit ready-queue fixture case and
therefore is not the final source/documentation fingerprint check.

The next async run passed cancellation, detachment and reference visitors before
scheduler-rounds exposed two old dynamic path format strings in its shared
explicit typed-domain fixture renderer (`kind.%s` and `.%s.%s`). They now emit
slash-separated complete identities. A direct empty/exact/longer-name/comment/
literal/Standard-path case checks that only the finite declared whole module
lines are rewritten, alongside the existing actual two-domain queue and bounded
scheduler programs. Current remaining run is 59343,
`/tmp/ama-t231-codegen-exact-schema-paths.log`. There is no runtime compiler
text substitution: this component only constructs declared test inputs.

The complete project parent passed again after shared fixture integration
(`/tmp/ama-t231-project-shared-fixture-acceptance.log`, exit 0). Unchanged recursive
build/incremental/concurrent harness contracts passed
(`/tmp/ama-t231-unchanged-build-infrastructure.log`, exit 0); their fake C-only
project trees were deliberately not converted into Ama packages. A NUL-delimited
VCS inventory of maintained active Ama sources found no remaining joined old
Standard/static prefixes. That syntax census is not a replacement for parsing
or behavioral tests and does not inspect ignored/generated files.

The long scheduler run was interrupted explicitly (session 59343, exit 130)
while investigating its blocking scenario; that combined run is not acceptance.
Inspection was limited to maintained harness/provider sources and output-file
existence/timestamps, never generated C or binary contents. Publication metadata
showed that most of the apparent delay had been native compilation, and the
program had only just entered its executable phase when interrupted. Do not
report a proven deadlock or a successful blocking check from that run. A direct
execution of the already verified published blocking program has been started
with a TTY so its ordinary stdout is visible immediately; collect its returned
handle. This is diagnostic execution, not the complete fixture/parent gate.

Correction to that diagnostic checkpoint: direct execution identified the last
published program as the *subsequent intentional invalid-budget case* (managed
status 5 with the exact quota-refusal message), not the blocking scenario. Thus
the original harness had already completed blocking and advanced into its six
budget refusal builds plus maximum-boundary build; it simply emitted no per-build
progress there. This is not evidence of a deadlock. Interrupting that normal run
was unnecessary; its exit 130 remains recorded, not counted as a parent pass.
The owning budget test now prints its explicit field/value case and maximum
case before their expensive native builds. It changes no generated program or
expected output. The new direct schema-renderer case remains before the existing
ready-queue checks, with the run-purpose comment attached to its real owner.
A fresh full remaining parent run (including the already observed scheduler
cases) is `/tmp/ama-t231-codegen-schema-and-budget-progress.log` (collect returned
handle). This repeat is justified by the interrupted acceptance and its small
maintained test-observability update, not a weakened or skipped test.

Preacceptance `make docs` passed its 3112-input/zero-debt coverage phase but failed
the rendered-link gate for `src/source/module/packages/reader.h`. Its SRC-02
contract link was confined to the reader struct's separate type page. The file
contract now also explains SRC-02's exact spelling/stable metadata requirement,
so the originating file page can carry every required rule link. This is a
comment fix, not a relaxed destination validator.

A raw VCS-to-maintained-source audit found one normalized CRLF in the moved
Unicode parser fixture (original physical line 19). Its exact CRLF has been
restored; a fresh real parser integration is
`/tmp/ama-t231-parser-original-crlf.log` (collect handle 50382). Other moved/changed
maintained text owners retained their carriage returns. Literal/comment bytes are
not rewritten to fit expected output.

The canonical example census table had old chapter ordinals/contexts beyond
T231's lexical work: source diagrams were now wrapped as code and newly inserted
module fences were wrapped as body fragments. The explicit table is updated to
the actual current fences, with diagrams still explicitly classified and no
examples removed or marked as expected parser failures. New T221 marker/T226
constructor syntax remains required by the unchanged all-positive parser case.
Its current run is `/tmp/ama-t231-canonical-fence-contexts-current.log` (collect
returned handle); any remaining newer-feature refusal belongs to those existing
implementation owners and T237's final whole-specification gate.

The corrected documentation generation passed
(`/tmp/ama-t231-documentation-origin-links.log`, exit 0): 3112 maintained ready
inputs, zero debt, 2324 maintained file pages, 9048 documented interfaces and
all 487 canonical/page destinations validated (7376 total HTML pages). Observed
tools: GCC/cc 15.2.0, Doxygen 1.11.0, Python 3.14.4, markdown-it-py 3.0.0.
This is preclosure generation: task/archive/checkpoint edits after it require
final regeneration and fingerprint checks before commit.

The CRLF-restored real parser integration passed
(`/tmp/ama-t231-parser-original-crlf.log`, exit 0). The updated canonical fence
case now reaches `language/templates.md` fence 3 and refuses its ordinary inline
constructor (`Invalid qualified constant source`), the exact pending TPL-15
boundary owned by T226. It no longer fails on an outdated count or wrongly
wrapped source diagram. The case remains all-positive, with no skips for new
syntax; T221 also still owns static-marker admission. No complete parser-parent,
make-test or renewed whole-specification conformance pass is claimed.

The README commands were verified directly against a fresh temporary output:
check and `--emit ... --compile` of the actual template-list main, followed by
execution of its published program, all exited 0 with no stderr (the executable
also had no stdout). No generated C/binary contents were read. The Python example
builder and tool oracles recovered their original unchanged literal/docstring
spelling; complete parsed-AST equality verifies that this formatting-only cleanup
preserves their already accepted behavior, including f-string expressions.

Latest continuation checkpoint: the only active acceptance job is session 38964,
`/tmp/ama-t231-codegen-schema-and-budget-progress.log`. Its scheduler parent has
passed all quota and native-wait cases and moved to ready-queue execution. The
remaining explicit child list is ready-queues, event-bridge, enrollment and
 task-phases. Wait for completion; do not overlap their output trees or treat
parent progress as final success. After collection: reconcile final evidence,
archive T231 only after its scoped gates pass, regenerate documentation for the
final maintained inputs, verify source/staged fingerprints, and commit T231.
T222 still requires the already recorded explicit specification-amendment gate.

The complete remaining scheduler and ready-queue parents passed, followed by
event-bridge and enrollment, in
`/tmp/ama-t231-codegen-schema-and-budget-progress.log`. The combined command then
failed task-phases: three appended body fragments still used the old qualified
Standard condition constants. Those exact `..std.condition.true/false` operands
are now canonical `//std/condition/true/false`, with phase/control/output oracles
unchanged. The fresh task-phases-only run is
`/tmp/ama-t231-task-phase-constant-sources.log` (collect its handle). No stale-binary
or timing diagnosis is needed: the maintained source contains the omitted
fragments explicitly. The combined parent exit 2 remains a partial result;
completion requires the final leaf's actual successful exit.

## T231 scoped closure and final delivery

The final task-phase leaf passed with the actual canonical constant operands
(`/tmp/ama-t231-task-phase-constant-sources.log`, exit 0). Combined with the
explicit successful children of the last parent run, this completes scoped
acceptance of every codegen family; it does not turn that combined exit 2 into
a pass. No active test jobs remain. The complete task/evidence is archived in
[DONE](../../DONE.md#t231); T222 is ready at its explicit spec-amendment gate.

The delivered boundary is canonical version selection and its real source,
semantic/native/tool/runtime consumers. T232 still owns relative/alias
normalization; T221/T226 own marker/complete constructor behavior and the
all-positive canonical fence gate; T237 owns the new whole-sprint conformance
renewal. No requirement is weakened, no canonical example is hidden, and no old
fingerprint is refreshed to manufacture acceptance.

Final delivery command: `make docs` at the repository root, with log
`/tmp/ama-t231-documentation-final-delivery.log`. Collect exit 0, verify every
manifest input against current source bytes and staged Git blobs, then commit
T231 without editing maintained inputs after the final generation. The generated
site/manifest remain unversioned; the commit contains comments, configuration,
test ownership and this reproducible record.

The first final-delivery launch was deliberately interrupted during its coverage
phase to remove a trailing blank line introduced by archival (`git diff --check`);
its retained log is `/tmp/ama-t231-documentation-final-delivery-interrupted.log`,
not acceptance. The fresh final-delivery log named above owns the actual result.
