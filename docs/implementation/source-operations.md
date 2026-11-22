# Resolved source operations

## T224 contract and execution plan

T224 starts from accepted T235 commit `9506245c` and a clean worktree.
[TOOL-01](../../spec/implementation/tools-and-conformance.md#tool-01) defines
selection/modes, [TOOL-02](../../spec/implementation/tools-and-conformance.md#tool-02)
defines injective cloning and exact scopes,
[TOOL-03](../../spec/implementation/tools-and-conformance.md#tool-03) defines
original-graph deletion safety, and
[TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) defines
whole-operation validation/recoverable publication.
[PROJECT-01](../../spec/implementation/project-tools.md#project-01) keeps source
mutation outside the compiler. Specification approval is not public command
implementation evidence.

### T272 prerequisite and unchanged T224 completion boundary

T272 is an explicit acceptance/commit unit extracted from the already recorded
T224 steps 1/2. It delivers original-edge/input removal checking and the read-only
compiler boundary: full active-domain semantics, immutable exports, exact protocol
syntax/schema, captured input/companion authority and root/snapshot inspection.
All component work below was already authorized/planned under T224; this refinement
makes its completed foundation independently reviewable before the larger operation
pipeline is implemented. It neither closes T224 nor drops any of its criteria.

T273 supplies the accepted project-side response/process bridge. T224 still owns
physical capture integration,
independent test-entry probes, exact selected/removal/client roots, clone maps,
rewrites and destination scopes, final graph laws, recoverable publication,
public copy/delete/move/replace commands and removal of superseded implementation.
T236 remains dependent on accepted T224. T272 passed its final real consumers,
sanitizer and documentation gates; its accepted record is [DONE](../../DONE.md#t272); all remaining T224
steps stay open. No public source-operation support is inferred from this split.

### Ordered steps

1. Implement the original surviving-consumer deletion predicate for definition
   edges and explicitly declared non-module inputs. Keep it independently
   testable with neutral compiler export records. It is shared by delete, move
   and replacement; replacement availability never changes this original test.
2. Prepare exact whole-original analysis/selection through read-only compiler
   interfaces and the common source catalog. Include newly covered roots, unused
   active modules and drafts; record all opaque/source/configuration inputs.
   Implement the compiler protocol adapter here before consumers need it; Git
   transport and publication-specific acquisition integration remain T236.
3. Derive explicit selected/removal sets and topology contexts. Consume T235's
   source/test closure for dependency cloning, without hidden acquisition. Plain
   deletion does not demand missing tests. Validate replacement/removal/client
   roots before any planned publication.
4. Construct injective physical/logical maps for selected trees, additional src
   dependencies, deepest associated-test directories and declared inputs. Validate
   original/final whole-version authority and all collisions/mode requirements.
5. Rewrite only copied parsed operands and authorized destination configuration
   fields. Preserve originals, static/member names, opaque content and exact
   selector sets; extend only explicitly authorized concrete permissions.
6. Reparse/recheck the staged whole final graph, original surviving edges and
   copied graph laws, contexts/test topology and client entry/export selections.
7. Adapt the existing identity-checked transaction/journal foundations for one
   copy/delete/move/replacement/configuration transaction, including real
   cross-filesystem publication, interruption and idempotent recovery.
8. Expose project-tool preview/apply commands, remove superseded compiler-side
   mutation/importer-rewrite APIs and their dedicated tests, and update retained
   component documentation and per-file ownership. Trace shared consumers first.
9. Accept components and real command compositions, source diagnostics, every
   relevant allocation/I/O/staleness/refusal branch, sanitizers, Doxygen and
   supported rendered provenance. Archive/commit T224 only after required gates.

No step uses text/C-expression searches for dependencies, guessed path-prefix
substitution, first-match ownership or implicit source acquisition. Existing
relocation documentation describes previous accepted tasks; T224 must replace
superseded current contracts when its new consumers are ready.

### Original-graph safety foundation

Let R be the exact original removed definition set and F the exact removed input
path set. For every original definition u outside R, report every original
resolved edge u -> v with v in R. For each declared input use (u,p), report it
when u is outside R and p is in F. This predicate ignores proposed replacement
copies and new graph edges. It is necessary for all removal operations and
sufficient for this boundary's authored-definition/declared-input safety claim:
any surviving path entering R has a first boundary edge, which this scan reports.
Explicit client-root handling, topology, completeness of original analysis and
transaction safety belong to the following composition steps.

Inputs borrow complete checked neutral source records, exact removed identities
and paths, and declared-input ownership/spans. Output blocker records own their
list metadata and borrow source identity/span owners through consumption. On
allocation/invalid-input failure no completed report is published. Empty sets and
cycles are valid; an outside unused/static/native/test consumer remains a blocker.
Work uses exact byte comparisons and finite scans, with no runtime reachability
filter. For E edges, I input uses, R removed identities and F removed paths,
straight lookup costs O((E+I)R + IF) name/path comparisons and O(blockers) records;
borrowed byte lengths multiply comparison costs.

Current step: implement/test the original-graph predicate. No new source-operation
command, mutation, protocol adapter or completed T224 support is claimed yet.

### Step 1 progress

Implemented `tools/project/source/removal/check`: exact original boundary edges
and declared input consumers produce one report node per blocking occurrence.
Removed owners do not block one another. Static/constants/parameterized/test
owners are never filtered by entry eligibility or runtime reachability. Duplicate
removed keys are harmless, drafts absent from active exports need no invented
edges, and foreign declared-input owners refuse. Complete OK/BLOCKED outcomes
publish together; invalid metadata/allocation failures leave the report output
unchanged and retain only arena-owned scratch.

Initial `make -j2 -C tests/unit/tools/project/source/removal` with
`-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L` passed
(exit 0, `/tmp/ama-t224-original-removal.log`). The independent oracle enumerates
all 4096 graph/removal pairs using row/column bit masks; further cases cover
shared declared files, every original blocker, duplicate/draft selections,
foreign owners and each measured report allocation failure with zero live
allocations. Added an explicit static/constants-only consumer case afterward.
The six named cases and all five new code/header/build inputs are registered;
standalone header and final component/parent checks remain pending.

Current step remains original-graph safety composition. The complete original
analysis/protocol adapter, physical selection/client roots, clone map, rewrite/
permission changes, transaction and public command migration remain unfinished.
No T224 completion commit is due at this substep. T235's accepted generated
reference is historical after these new maintained inputs; T224 must regenerate
its own final documentation before acceptance.

### Step 1 evidence and command-mode clarification

Expanded original-removal parent passed: `make -j2 -C
tests/unit/tools/project/source` with `-Werror`/normal C11-POSIX flags,
`/tmp/ama-t224-removal-parent.log`, exit 0. Standalone inclusion of
`tools/project/source/removal/check.h` passed strict C11 `-fsyntax-only` with
`-Wall -Wextra -Werror -pedantic -D_POSIX_C_SOURCE=200809L -I. -Isrc`.
ASan/UBSan removal leaf passed (exit 0),
`/tmp/ama-t224-removal-address-undefined.log`, using T235's documented
`detect_leaks=0`, halt-on-error, no-PIE sanitizer flags. All six cases and every
report allocation failure passed; counted result/source teardown reports zero
live allocations. No leak-instrumentation or public-command support is claimed.

Read-only command-contract review identified
[AMB-021](../../AMBIGUITIES.md#amb-021): TOOL-01 lists bare copy/delete/move
commands and requires preview/explicit application but does not state which
mode the bare command selects. Mutation, preview-only and refusal are observably
different. T224 is stopped before public request interpretation, with no
heuristic/default chosen and no specification change made. The registry records
the exact example, rules, alternatives and requested scoped authorization.

Current checkpoint: T235 is committed as `9506245c`; T224 has an uncommitted
original-graph kernel, one six-case leaf/parent and per-file evidence, but its
remaining original-analysis/protocol, selection/client roots, cloning, edits,
transaction and command migration are not complete. No jobs are running. Wait
for the explicit AMB-021 decision before resuming; do not commit this partial
task as completed or switch to another TODO task to bypass it.


### AMB-021 decision and resumption

On 2026-11-22 the user explicitly selected mandatory `--preview`/`--apply`
in response to the question authorizing this specification clarification.
TOOL-01 now requires exactly one mode and rejects absent, duplicate and
conflicting modes before mutation; PROJECT-01 uses the same command notation.
The prior waiting checkpoint is superseded. T224 is active, with no jobs running.
Next: implement the original-source analysis adapter and its component tests
(step 2), retaining the already tested removal kernel. Public commands, final
graph validation and transaction acceptance remain unfinished; no completion
commit is due yet.


### Step 2: captured-domain admission adapter

Added `src/delivery/source/domain/`: activity selection uses the production
parser for every captured catalog member, retains drafts as catalog bytes, sorts
all active identities, invokes ordinary entry-free frontend checks, and exports
original resolved definitions/edges. The caller remains responsible for complete
root/version/config/native capture. This child performs no discovery, process
execution or source mutation; it does not certify input sealing or implement
the serialized project-tool protocol. That transport boundary remains pending.

The active-set invariant is exactly all non-draft catalog records. Iteration over
the finite catalog terminates; sorting affects only deterministic request order.
Every active definition is required independently of incoming edges, so an unused
consumer or invalid body cannot disappear behind executable reachability. Drafts
are still present if an active definition incorrectly requires them. The normal
frontend supplies parsing, scopes, static closure and operation checks. Names and
export nodes are operation-arena owned, exported spans borrow frontend source
owners, and partial preparation never publishes caller output.

Initial strict component build/check is running: session 56065,
`/tmp/ama-t224-domain.log`; command `make -j2 -C
tests/unit/tools/project/source/domain` with `-O0 -g -Wall -Wextra -Werror
-std=c11 -pedantic -D_POSIX_C_SOURCE=200809L`. Collect its exit before claiming
acceptance. Five cases cover full active categories and drafts, unused errors,
empty/draft-only catalogs, every allocation failure and owner/reuse refusals.


Domain acceptance checkpoint: the initial five-case strict leaf (session 56065,
`/tmp/ama-t224-domain.log`) exited 0. The expanded six-case real source parent
(session 25251, `/tmp/ama-t224-source-parent.log`) exited 0 with the same strict
flags, including the existing six-case removal leaf. The native case supplies
sealed ABI/body/header metadata with nonexistent C paths: success demonstrates
entry-free admission without C compilation or runtime execution. Public and
internal domain headers passed strict independent C11 compilation. ASan/UBSan
expanded domain leaf (session 74656, `/tmp/ama-t224-domain-sanitizers.log`)
exited 0 with the previously documented no-PIE/halt-on-error flags and
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`; counted complete/failed owner
teardown still independently verifies zero live allocations. No leak-sanitizer
claim is made. Per-file coverage includes all six new maintained domain inputs
and both recursive dispatchers. No acceptance inputs were edited during these
runs. The serialized protocol/capture producer and later T224 steps remain open.

The first domain `make docs-check` (session 37560,
`/tmp/ama-t224-domain-docs-check.log`) exited 2: although both headers had
passed standalone compilation, their per-file records omitted the executable
`standalone_header_command`. Added the same strict registered command as the
removal header; rerun is required. This was a metadata gate failure, not accepted
rendered documentation.

The expanded seven-case strict domain composition (session 76061,
`/tmp/ama-t224-domain-composition.log`) exited 0. It passes the compiler's
original cyclic graph to the deletion predicate: an unused outside consumer
blocks removal of the cycle, and selecting all three definitions removes the
blocker. `make -j2` compiled/linked the production compiler successfully,
`/tmp/ama-t224-domain-compiler-build.log`, exit 0.

The documentation coverage rerun (session 64622,
`/tmp/ama-t224-domain-docs-check-rerun.log`) exited 0: 3441 ready inputs and
zero debts. The seventh composition case and its registry association were
added while that scan was in progress; therefore this is not a frozen final-input
acceptance. Recheck the affected registered test interfaces against final inputs
before counting the delta, and retain the full-task documentation gate for task
completion. Expanded seven-case sanitizer acceptance is running with log
`/tmp/ama-t224-domain-composition-sanitizers.log`; collect its exit.

The final seven-case sanitizer run (session 48283,
`/tmp/ama-t224-domain-composition-sanitizers.log`) exited 0. Final affected
header and registered-case probes passed through the documentation gate's own
`check_header`/`check_test_interfaces` entry functions. An initial ad-hoc call
omitted the required `expected_failure` argument and exited 1 before checking;
rerun supplied `None` and exited 0, checking both headers and all seven actual
`void it_*(void)` signatures. Full final-task documentation remains pending.

Next protocol prerequisite: original edges must retain whether their source site
is an implicit language dependency and the normalized member of a qualified
constant. A requiring construct is not automatically a rewritable path; splitting
a member out of raw text would mis-handle aliases and relative reduction. Export
these already resolved facts rather than adding a second path parser.


### Resolved edge metadata for the protocol

`DeliverySourceDependency` now retains the original `implicit` flag and an owned
normalized constant member (NULL for ordinary module operands). The exporter
copies these compiler-resolved facts along with the original source span. No
syntax/alias/relative normalization is duplicated by a project consumer. This
prevents treating a requiring list construct as a path edit or accidentally
removing the constant name when an entire operand is replaced.

The initial eight-case domain check passed (session 67069,
`/tmp/ama-t224-domain-edge-metadata.log`, exit 0). The strengthened current case
uses a captured module alias whose complete written operand is `~chosen`, but
whose resolved constant member is `values`; those bytes cannot be recovered by
splitting the operand. An explicitly identified typed-flag copy probe marks one
checked edge implicit and reexports it; this verifies transport preservation and
does not claim discovery of a real language-generated implicit dependency.
The every-allocation case now includes constant-member allocation as well.

Strict source-parent acceptance is running: session 5888,
`/tmp/ama-t224-source-constant-metadata.log`, same strict C11/Werror flags.
Collect completion before changing source inputs. No public command/protocol or
T224 completion is claimed.

Strict eight-case source-parent acceptance (session 5888,
`/tmp/ama-t224-source-constant-metadata.log`) exited 0, including alias-normalized
member preservation and each constant-member allocation failure. Existing real
acquisition dependency/test-root consumers are now running with the same strict
flags: `/tmp/ama-t224-edge-export-consumers.log`.

Protocol adapter design: add an explicit read-only compiler domain request with
configured project/standard roots and exclusions, independent of any executable
entry. It uses the ordinary catalog and the accepted domain child. A versioned
structured response retains every catalog source (including drafts), version and
module configuration capture state, native companion metadata, original active
definitions, all original edges, normalized constant members and exact source
spans. Physical paths and contents use the existing lossless JSON byte form;
logical identities use checked UTF-8 strings. Response success requires normal
checking plus complete stream write/flush; partial output or unsuccessful child
status cannot be consumed. The response does not claim sealed native/opaque
inputs, test entry classification, mutation or publication. Project-side complete
capture/revalidation remains mandatory. The separate project executable consumes
records through its process boundary and must never link compiler/parser owners.


### Structured response producer

`src/delivery/source/report/` separates lossless physical byte/source values,
catalog snapshots, definition/edge graph values and top-level stream completion.
The versioned response schema is `ama.source-domain.v1`:

- `format` identifies the schema; `inputsSealed` is always false for this
  entry-free analysis boundary, never an executable/cache certificate.
- `versions` contains each catalog version's logical identity, physical
  directory and nullable captured configuration source.
- `sources` retains every catalog member, including drafts: logical identity,
  original source snapshot, `configurationCaptured` and nullable configuration,
  plus native `captured`, nullable body/header paths and nullable ABI source.
  An unobserved companion is distinct from observed absence. C/header contents
  and other opaque files belong to separate complete unit capture, not this
  semantic response.
- `definitions` contains byte-sorted checked original identities/categories and
  original ordered dependency edges. An edge retains target, implicit flag,
  nullable normalized constant member, original physical source path and
  half-open byte offsets as exact decimal strings.

Physical paths and contents use existing `{utf8, hex}` byte objects; malformed
UTF-8 has `utf8: null` and still retains every byte in lowercase hex. Nullable
source snapshots are `{path, content}` when present. Logical identifiers are
validated UTF-8 strings. Catalog source/version arrays retain catalog order;
consumers key by exact identity rather than treating that order as precedence.
Consumers must require complete JSON and successful producer exit. A partial
write/flush failure is never an accepted response.

The producer streams borrowed immutable owners in O(source/configuration bytes
plus definitions/edges) work and O(1) auxiliary memory, excluding host stream
buffering. It allocates no new source or graph records, opens no source/native
files and neither seals inputs nor executes code. Its caller must already have
a successful whole-domain check. No command adapter or project-side decoder is
claimed yet. The report leaf uses a real entry-free compiler domain with an
open helper, marker, constants/alias, native ABI and draft plus an independent
Python JSON decoder. A non-UTF-8 physical pathname verifies byte preservation.
Immediate read-only-stream and `/dev/full` flush errors verify caller ownership.

Initial strict report leaf is running: session 44828,
`/tmp/ama-t224-domain-report.log`, the established C11/Werror flags. Collect its
completion before editing its inputs.


The strict report leaf (session 44828, `/tmp/ama-t224-domain-report.log`)
exited 0: all three C cases and independent populated/empty Python decoding
passed. Existing acquisition dependency/test-root consumers (session 14823,
`/tmp/ama-t224-edge-export-consumers.log`) exited 0 against the augmented edge
records.

Read-only command adapter: `ama --source-domain ROOT [--root DIR]
[--standard-root DIR] [--exclude PATH]` has no executable-module operand. Exact
recognition/option assembly delegates ordinary root selection, entry-free
checking, warnings and source diagnostics, then writes/flushed the versioned
response. Unknown/mutating/emission options are refused before the frontend.
Catalog/native source owners outlive exported facts and diagnostics; exported
nodes retire first. No filesystem source mutation, network, C toolchain or Ama
execution is introduced. This is a compiler analysis interface for T224's separate
project process, not completed public project copy/delete/move support.

Request/options suites passed, `/tmp/ama-t224-domain-driver-request-options.log`,
exit 0, including exact spelling, index-three option consumption, explicit root
kinds/exclusions, forbidden flags and every option allocation failure. Strict
actual domain-command leaf is running: session 51504,
`/tmp/ama-t224-domain-command.log`; collect completion before source edits.

The strict actual command leaf (session 51504,
`/tmp/ama-t224-domain-command.log`) exited 0: complete unused/open/native/draft
observations decoded independently, unused body errors refused before response,
explicit exclusions respected, empty selected versions accepted, invalid roots
and requests rejected, and immediate/flush output errors propagated. Native
C/header `#error` fixtures were not compiled or executed. This is evidence for
the read-only compiler adapter only, not project-tool mutation support.
The production compiler rebuild is running: session 95966,
`/tmp/ama-t224-source-domain-compiler.log`; collect before using the new binary.

Production compiler rebuild (session 95966,
`/tmp/ama-t224-source-domain-compiler.log`) exited 0. Actual `./ama
--source-domain` execution against fresh `/tmp/ama-t224-domain-cli-kvu9abh6`
returned valid schema, four source rows and three checked definitions for a
reference cycle, draft and external test consumer. A forbidden `--apply` flag
returned failure with empty stdout. SHA-256 snapshots of every fixture file
before and after both invocations matched exactly. Existing driver run suite
(session 53997, `/tmp/ama-t224-domain-driver-existing-run.log`) exited 0.

Current independent sanitizer runs: report leaf session 55854,
`/tmp/ama-t224-report-sanitizers.log`; actual domain-command leaf session 34481,
`/tmp/ama-t224-command-sanitizers.log`. Both use established ASan/UBSan, no-PIE,
halt-on-error and detect_leaks=0 flags. Their output trees are disjoint and their
shared compiler inputs are frozen. Collect both exits before changing those
inputs.

### Project-side protocol decoder foundation

The next child is a strict JSON reader owned by the separate project tool. It
operates on captured response bytes without filesystem/process effects. Use the
standard JSON lexical grammar with exact UTF-8/Unicode escapes and decimal-number
spelling retained without floating-point conversion. Reject duplicate decoded
object keys so protocol fields never have first/last-match ambiguity. Retain exact
source spans; decoded strings carry explicit lengths, including embedded NUL.
The schema layer subsequently validates the exact version/key/type contracts and
lossless byte objects; syntax parsing alone never authorizes project operations.

Use an explicit arena-owned container stack rather than C recursion or an
arbitrary depth limit. Each transition consumes input or completes a container;
the finite input and finite stack bound progress. Two-pass string decoding sizes
allocations exactly. All AST/decoded-string/frame memory belongs to the caller's
operation arena; number text and diagnostics borrow the immutable response source.
Publish a root only after the entire document is accepted. Counted failures must
preserve caller output and remain safely destructible. Duplicate-key checks use
exact decoded byte lengths/content; no hash collisions, guessing or text search
can choose field meaning. This syntax component has an explicit current owner:
T224's `ama.source-domain.v1` response decoder.

Report ASan/UBSan (session 55854, `/tmp/ama-t224-report-sanitizers.log`) and
actual command ASan/UBSan (session 34481,
`/tmp/ama-t224-command-sanitizers.log`) both exited 0. Native #error files remain
unexecuted; detect_leaks=0 still means no leak-instrumentation claim.

The project-side JSON syntax child is implemented in
`tools/project/source/protocol/json/`: cursor/first failure, exact numeric token,
two-pass string decoding, value attachment, container state and complete-document
publication have separate cohesive ownership. The strict initial eight-case leaf
(`/tmp/ama-t224-protocol-json.log`) exited 0. Review then tightened diagnostic
sites for a consumed control/invalid escape/unpaired surrogate and duplicate key
to point to that original offending byte/key. Expanded nine-case strict leaf
(`/tmp/ama-t224-protocol-json-positions.log`) exited 0. Known exact UTF-8 boundary
bytes, all number grammar partitions, 41 malformed document cases, five malformed
UTF-8 sequences, decoded-key aliases/embedded NUL, each allocation failure and
20,000 nested arrays are covered. Counts are fixture domains, not universal
correctness claims.

Parser invariant: each stack frame owns one already attached container, its
append tail, and the exact grammar state after the consumed prefix. Attachment
sets its parent to separator state before a child frame is pushed. Empty-first
states accept a closing delimiter; required-after-comma states do not. Each
successful transition consumes bytes or closes a container; opening a container
already consumed its delimiter. Therefore finite input plus the stack of
previously consumed opens proves termination without a depth heuristic.
A successful root with an empty stack and only trailing JSON whitespace is
exactly one complete syntax tree. Object key uniqueness compares decoded byte
length/content within that object only, so escaped aliases cannot select a field
twice and equal keys in different nested objects remain independent.

Strings are sized/decoded twice from immutable bytes. Raw UTF-8 is validated
once; each escape emits no more bytes than it consumes. A high/low surrogate
pair becomes `0x10000 + (high - 0xd800) * 0x400 + low - 0xdc00`, lying in
`[0x10000, 0x10ffff]`; unmatched surrogates are errors. Exact allocation includes
one spare zero terminator while explicit lengths preserve escaped NUL. Numeric
tokens retain original bytes, avoiding host width/rounding/locale decisions.
Memory is linear in nodes, retained container frames and decoded string/key
bytes; time adds the exact per-object pairwise decoded-key comparisons to the
linear lexical scans. Source/number spans borrow the captured response, with
all complete/failed nodes/frames/strings owned by the operation arena.
Schema validation and process-result admission are still separate pending steps;
JSON syntax success does not certify a source-domain graph.

JSON syntax ASan/UBSan passed (`/tmp/ama-t224-protocol-json-sanitizers.log`,
exit 0); exact counted teardown also remained clean. The independent project
executable compiled/linked the neutral reader without compiler/parser ownership
(`/tmp/ama-t224-project-json-build.log`, exit 0). Scoped current canonical links,
standalone headers and actual test-case interfaces passed for 49 maintained
changed inputs (session 42954); the malformed-document corpus count was checked
as exactly 41. This is scoped evidence, not a substitute for final full gates.

The typed schema foundation now validates exact unordered field sets, nonempty
NUL-free strings, canonical size_t decimal strings and redundant lossless byte
objects in `tools/project/source/protocol/schema/`. Fields are validated before
any output-array publication. Generic JSON parsing already proves unique decoded
keys. Full-width decimal extraction checks `value <= (UINTMAX_MAX - digit) / 10`
before each step; size_t conversion additionally requires an exact unsigned
round trip. No signed/floating-point/locale parser is used. Hex decoding accepts
only even-length lowercase text and verifies the corresponding UTF-8 string or
null view; output is explicitly length-delimited and zero-terminated.

Strict six-case schema leaf passed, `/tmp/ama-t224-protocol-schema.log`, exit 0.
It covers reordered/missing/unknown/NUL-extended keys, every byte value, Unicode
redundancy, malformed/mismatched hex, empty bytes, SIZE_MAX and overflow, forbidden
integer spellings/types, string-NUL refusal, output preservation and payload
allocation/owner failure. Syntax/source/payload owners independently retire with
zero counted live allocations. Source/graph reconstruction and process-result
admission remain pending.

Refined file-purpose comments distinguish byte, graph, catalog, cursor, numeric
and schema responsibilities. Those changes are comments only; final compilation
and documentation gates will use their current bytes.


### Prepared-snapshot transport prerequisite

Whole-original analysis of newly covered destination roots cannot in general be
expressed by the root-only inspection command: already covered physical files
must retain original logical identities while new files use explicit analysis
identities. Overlapping roots can otherwise duplicate a physical source or
reidentify an existing consumer. TOOL-02 therefore requires a captured-table input
mode alongside ordinary root discovery, feeding the same frontend snapshot API.
Per-root exclusions/config/native capture must also retain their exact owners.

The generic JSON syntax/schema children will be shared as neutral
`src/source/json/` components, without compiler/Ama parser, project mutation or
process ownership. The project executable links only those neutral readers.
Compiler-side captured-input adaptation and project-side response adaptation
remain separate consumers with distinct schemas. This prevents linking the
project executor into the compiler or implementing another JSON grammar.
Current JSON/schema tests are moved to `tests/unit/source/json/{grammar,schema}`
and their per-file evidence follows the same named cases. No abandoned public
API or compatibility alias is retained. The root-only command remains useful
read-only convenience; it does not claim augmented-original snapshot support.

Schema ASan/UBSan passed, `/tmp/ama-t224-protocol-schema-sanitizers.log`, exit 0.
The forthcoming move changes neutral component ownership/names only; rerun both
real leaves, actual project build and source/header/case checks on final paths.

The shared neutral JSON grammar/schema parent passed after the ownership/name
move (`/tmp/ama-t224-neutral-json.log`, exit 0). Its APIs now use `SourceJson`
and `source_json_schema_*`; old project-prefixed files/interfaces are removed.
`tools/project/sources.mk` consumes only these neutral sources and retains its
existing actual private-build acceptance owner. Grammar tests alone are not
claimed as evidence for that build-infrastructure change.

Prepared-native authority is now a plain `DeliverySourceCompanions` table. The
domain API distinguishes normal catalog activation (NULL table) from captured
mode. Production parsing first classifies each source; only native definitions
receive exact table metadata through the existing catalog snapshot operation.
Missing/duplicate/already-bound authority is refused before ordinary checking,
with no row selection or physical-path fallback. Ordinary/draft companions stay
opaque even when captured ABI bytes would be invalid as an active ABI. Catalog
ownership retains successful/failed copied metadata and original diagnostics.
This avoids a second source classifier or guessing activation from companions.

The original eight-case domain suite still passed the interface adaptation
(session 90790, `/tmp/ama-t224-domain-captured-interface.log`, exit 0). Expanded
eleven-case strict domain acceptance ran as session 44700 with log
`/tmp/ama-t224-domain-native-capture.log`; collect/record its result. New cases
cover exact native-only installation, missing/duplicate/null/prebound rows and
each classifier/companion/ABI/ordinary-check allocation failure.

Next: decode the explicit captured input schema, construct version/module catalog
snapshots using captured identities/configuration/device/inode metadata, and feed
the authoritative companion table to the same whole-domain checker. The compiler
input path and project response decoder remain distinct adapters around neutral
JSON grammar/schema helpers. No snapshot command, copy/delete/move operation or
T224 completion is claimed yet.


Captured-input data decoder: `src/delivery/source/snapshot/` accepts the closed
`ama.source-snapshot.v1` schema with `format`, `versions` and `sources`. A version
contains canonical identity, physical directory bytes and required package.ini
snapshot. A source contains canonical identity, source snapshot, decimal device
and inode strings, nullable module configuration and unconditional `companions`
with nullable body/header paths and ABI snapshot. Null fields are captured
absence, never permission to reread physical paths. Descriptor validity certifies
only data/schema/coherence: normal catalog/frontend stages still validate
physical aliases, UTF-8 roles, Ama syntax, types and operations.

Decoded source/configuration/ABI blobs preserve arbitrary bytes. Only the normal
catalog/native classifier selects role-specific encoding/syntax validation; a
draft/ordinary ABI companion may legitimately be opaque binary data. Equal raw
physical paths must carry equal bytes; coherent repeated captures share one
owned SourceText. Canonical module/version keys, source key/path uniqueness and
ordinary selected-version presence are checked without filesystem inference.
Temporary JSON trees/buffers are freed after decoding; public arrays, names,
paths and SourceTexts remain independently owned. Failure diagnostics retain the
original request source until consumed.

The first strict snapshot-data leaf `/tmp/ama-t224-snapshot-input.log` exited 2
at linking: its fixture loader needed `source_manager_load`, but the explicit
test source list omitted `source/io` (production decode itself performs no I/O).
Added the existing narrow source/io units to that leaf and rerun before any
acceptance claim. The versioned independent Python fixture model defines valid
source/native/draft/opaque inputs, deferred frontend-error cases and 27 one-error
schema/coherence mutations.


Expanded prepared-native domain acceptance (session 44700,
`/tmp/ama-t224-domain-native-capture.log`) exited 0. The captured-input data
leaf rerun (`/tmp/ama-t224-snapshot-input-rerun.log`) exited 0 after adding its
fixture I/O units: five C cases, 27 independent one-property malformed models
and every transient/result allocation passed, including successful result
independence from freed original JSON and opaque binary draft ABI bytes.
The source/configuration roles are validated later by the normal catalog;
only production-classified native ABIs are validated as native companions.

Neutral grammar/schema parent passed with the new exact uintmax_t identity
extractor (`/tmp/ama-t224-neutral-json-identities.log`, exit 0), then again after
using one full unsigned parser plus an exact size_t conversion adapter
(`/tmp/ama-t224-neutral-json-identity-adapters.log`, exit 0). The size adapter
refuses `(uintmax_t)(size_t)value != value`; filesystem identity conversions
likewise refuse loss or negative converted identities.

`delivery_source_domain_snapshot` now constructs selected versions and the exact
module registry from the decoded table, then uses the same captured-native/domain
checker. Preparation failure consumes the frontend and retains original capture
diagnostics. Snapshot data outlives failed diagnostics; successful frontend and
exported facts own their needed bytes independently. No physical root or native
path lookup is introduced.

Initial strict frontend-composition build (`/tmp/ama-t224-snapshot-frontend.log`)
exited 2 because its broad compiler fixture declared an unused teardown helper.
Replaced it with the narrower counted-source fixture and explicit owner ordering;
the next build (`/tmp/ama-t224-snapshot-frontend-rerun.log`) exited 2 because that
removed a transitive stdio include. Added the actual explicit `<stdio.h>`
dependency. Strict rerun session 74897,
`/tmp/ama-t224-snapshot-frontend-explicit-fixture.log`, is running and must be
collected before acceptance. The model's positive INI fixture uses the specified
`;` comment profile when composed with the real frontend.


Prepared frontend composition (session 74897,
`/tmp/ama-t224-snapshot-frontend-explicit-fixture.log`) exited 0: four real
frontend cases include nonexistent future paths, open/helper/native/draft rows,
ordinary source/encoding/native ABI/alias refusals, every preparation/check/export
allocation failure, empty captured input and invalid/reused owners. Successful
frontend facts remain usable after both decoded snapshot and original JSON
owners are destroyed. No physical source/native path was published or probed.

The read-only command now has an explicit second form,
`ama --source-domain --snapshot FILE`, with no root/exclusion/emission options.
It reads only that request file, decodes the closed capture model and uses the
same ordinary prepared frontend, retaining original JSON/model owners through
diagnostic reporting. Root inspection remains its distinct original form.
Request/options/domain command acceptance is running as session 29404,
`/tmp/ama-t224-source-snapshot-driver.log`; collect its result. This work is now
owned for acceptance/commit by prerequisite T272; T224 itself remains unfinished.


The strict request/options/snapshot-command parent (session 29404,
`/tmp/ama-t224-source-snapshot-driver.log`) exited 0; its result was collected
before the plan-refinement discussion. A later redundant poll correctly returned
unknown process ID because the completed session was already collected; no run
was lost or relaunched on that basis. Six real domain-command cases now include
both input forms, nonexistent future paths and request-file/schema/source/ABI/alias
refusals with empty success output. Exact captured option tests prove no implicit
root or option allocation and refuse mixed/missing input modes.

T272 final acceptance is pending current consumer rebuilds/sanitizers, scoped/full
documentation coverage, generated Doxygen and review. No active job remains at
this checkpoint. T224 remains open with all operation-pipeline criteria intact.


### T272 final frozen acceptance

No production or test inputs may change while these independent runs execute.
All use `-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic
-D_POSIX_C_SOURCE=200809L` and `make -j2`:

| Scope | Command directory/targets | Session | Log |
|---|---|---|---|
| Neutral JSON | `tests/unit/source/json` | 62363 | `/tmp/ama-t272-source-json.log` |
| Captured delivery | `tests/unit/delivery/source` | 78346 | `/tmp/ama-t272-delivery-source.log` |
| Graph/removal/report | `tests/unit/tools/project/source` | 57231 | `/tmp/ama-t272-source-components.log` |
| Driver consumers | `tests/unit/driver request options run source-domain` | 63160 | `/tmp/ama-t272-driver.log` |
| Private independent project build | `tests/unit/tools/project/building` | 86920 | `/tmp/ama-t272-project-building.log` |
| Production compiler | repository root | 74694 | `/tmp/ama-t272-compiler.log` |

Each leaf owns a distinct generated tree; private-build acceptance uses its
private copy. No code/test input edits or shared-output builds are permitted
until completion is collected. Next: collect all six results, actual compiler
root/snapshot process checks, isolated final sanitizers and documentation gates.

All six frozen strict/build runs completed with exit 0: sessions 62363, 78346,
57231, 63160, 86920 and 74694. Logs are listed in the table above. The 81 changed
maintained inputs were byte-checked against `/tmp/ama-t272-frozen-inputs.json`
after collection; none changed during those runs. Private project-build tests
exercise actual independent linking, flag/incremental inputs and exact removed
link membership with the shared neutral JSON reader.

Actual rebuilt compiler processes passed in fresh
`/tmp/ama-t272-cli-jafatikr`: root inspection retained an unused consumer; captured
inspection returned four source rows/three definitions with unpublished paths;
invalid captured code and mixed root/snapshot modes failed with empty stdout.
SHA-256 comparisons preserved request and root source/configuration bytes, and
no `not-published` directory appeared. No executable entry/native body ran.

Final ASan/UBSan jobs (same frozen code/test inputs, disjoint leaf outputs):

| Scope | Session | Log |
|---|---|---|
| Neutral JSON grammar/schema | 81172 | `/tmp/ama-t272-json-sanitizers.log` |
| Captured data/frontend | 32785 | `/tmp/ama-t272-delivery-sanitizers.log` |
| Original graph/domain/report | 33486 | `/tmp/ama-t272-source-sanitizers.log` |
| Both actual command input forms | 33513 | `/tmp/ama-t272-command-sanitizers.log` |

Flags add `-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and
`LINKER_FLAGS=-no-pie`; environment uses `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`
and `UBSAN_OPTIONS=halt_on_error=1`. Leak instrumentation remains unavailable
under the recorded ptrace restriction; exact counted allocation/teardown checks
are separate evidence. Collect all four before running shared Make-context
documentation probes or editing maintained code/tests.

All four final sanitizer jobs completed with exit 0: 81172, 32785, 33486 and
33513. The 81 frozen maintained input hashes still match. No sanitizer finding
or managed error escaped its expected test outcome; counted complete/failed
owners additionally retired to zero live allocations. The final full
documentation check/generation and source review remain before T272 archival.


### T272 accepted result and T224 continuation

Full `make docs` completed with exit 0 (session 40374,
`/tmp/ama-t272-documentation.log`): 3490 ready inputs, zero debts, 2646 maintained
file pages, 10198 documented interfaces, 494 canonical/referenced destinations
and 8354 total pages. Tools: Doxygen 1.11.0
(`9b424b03c9833626cd435af22a444888fbbb192d`), markdown-it-py 3.0.0,
Python 3.14.4 (GCC 15.2.0). The supported generated manifest equals
`build/documentation/report.json`; all 81 frozen maintained input hashes still
match after behavior/sanitizer/documentation collection. No conformance
fingerprint was refreshed; full release/conformance renewal remains T237.

The full T272 task/result is moved to DONE, and T224 depends on that accepted
foundation. Only Markdown status/plan notes changed after this documentation
run, so regenerate the reference using the supported documentation builder
before staging/commit. No code/test/build-input change is authorized by archival.

Next T224 action after the T272 commit: implement project-side response decoding
and process admission against complete locally prepared source snapshots, then
independent test-entry probing and exact original selected/removal/client-root
preparation. Clone laws, rewrites, final checks, transactional publication and
public source commands remain open. No background acceptance job remains.

Archived Markdown regeneration passed with exit 0 (session 49242,
`/tmp/ama-t272-documentation-archived.log`) through the supported command
`python3 tools/documentation/build.py --doxygen doxygen`; all file/interface/link
counts and tool versions remain as reported above. This final evidence note is
Markdown only. Refresh its reference provenance once more using the same builder
and verify every current input fingerprint before staging; no behavioral check
needs repeating because all 81 accepted code/build/test input hashes are unchanged.


### T273 accepted bridge and remaining T224 work

The [prepared source protocol](source-protocol.md) delivers the separately
accepted project-side response/process prerequisite, with complete immutable
source/version/configuration equality, compiler-classified native capture equality,
normal-success admission, raw diagnostic ownership and real compiler evidence.
Private duplicate blob/stream/wire implementations are removed. Its complete
record is [T273](../../DONE.md#t273).

Next T224 step: capture complete explicitly selected local roots and companions
through the common catalog/file primitives, then compose the bridge and independent
executable-test probes. Preserve original/newly covered identity authority;
no code-text searches, parameter-count guesses or live frontend state may replace
compiler facts. Selection, clone maps, rewriting, whole-final graph checks,
transactions, commands and removal of superseded implementations remain open.
