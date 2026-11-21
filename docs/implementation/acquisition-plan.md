# Scoped acquisition planning

## T234 task and scope

T234 follows accepted T232 and T233 (`09468e8`). Its canonical contracts are
[PKG-05](../../spec/project/packages.md#pkg-05),
[PKG-06](../../spec/project/packages.md#pkg-06),
[PKG-02](../../spec/project/packages.md#pkg-02) and
[PROJECT-01](../../spec/implementation/project-tools.md#project-01).
No specification changes are authorized or needed for this implementation.

The shared exact selector compiler/matcher and finite prepared selection view
already exist under `src/source/configuration/selectors/` and `selection/`.
T263's independent bounded expansion models and T262/T232's real scoped
configuration consumers remain their implementation owners. Reuse these APIs;
do not introduce another wildcard syntax, greedy search or global selector table.
See [the configuration contract](project-configuration.md).

The missing boundary is project-tool acquisition planning: scoped repository
selection, relative physical mapping, operation-local revision snapshot identity
and one coherent source contract per logical version. It lives under
`tools/project/acquisition/`, outside the compiler pipeline, borrowing parsed
read-only configuration. No Git, network, source mutation or module execution
belongs to these planners. T235 owns structural source/test closure; T236 owns
transport, actual Git resolution and publication.

## Planned steps and acceptance

1. Select the longest full-component prefix from one validated package-profile
   document. Preserve that requesting document's scope and original mapping.
   Request a complete canonical ordinary version/module. Refuse no-route and
   invalid requests without consulting another scope or shorter availability
   fallback. Exhaust bounded overlapping/reordered routing tables independently.
2. Normalize the selected contained relative repository directory with an exact
   component stack, append the logical suffix through its version boundary and
   expose an owned physical version-directory plan. Actual filesystem containment
   and submodule inspection remain transport/delivery obligations. Check empty,
   dot, parent, repeated separators, Unicode and overflow/allocation refusal.
3. Intern operation-local snapshot requests by exact URL and optional revision
   bytes. Missing revision is an explicit default-branch request, not an invented
   branch name. Publish one externally resolved commit per such request; refuse
   contradictory resolutions. No planner executes Git or guesses URL equivalence.
4. Associate each logical version with exactly one URL/resolved-commit/normalized
   physical-directory tuple. Equivalent requests share one record; different
   revision names resolving to the same commit may agree. Conflicts retain both
   request origins and cannot mix snapshots. Locally selected whole versions
   bypass remote routing and cannot be filled from a repository.
5. Compose finite selector matches with actual scoped routes and snapshot/source
   planning. Empty matches and missing members remain empty; all matches survive
   deduplication and ordering. Reuse actual parsed documents and selected local
   version metadata. Run focused/parent/model/allocation/sanitizer checks, register
   every maintained input, generate Doxygen, archive T234 and commit.

## Initial invariants and proof obligations

A route is chosen only from one immutable validated document. Matching compares
exact prefix bytes followed by '/' at a logical component boundary. Among those
prefixes, greatest component count is the unique route; document uniqueness
prevents conflicting same-prefix records. Selection does not know remote
availability and therefore cannot fall back after transport failure. Neither a
target's own document nor a sibling request can modify this choice.

Physical normalization consumes each input component once. Each pushed byte is
popped at most once, giving linear amortized work and space in the directory
spelling. The existing parser already refuses absolute or escaping directory
expressions; the plan retains this invariant rather than using filesystem
heuristics. Version identity comes from the common canonical path shape, not
a substring search for a v-like word.

Snapshot and source-contract tables are operation-owned finite intern sets.
Exact equality is an equivalence relation; first insertion creates a record and
all equivalent requests find that record. A failed unequal insertion publishes
no new version association. Resolution changes only a previously unresolved
snapshot's commit field, once; actual Git effects belong to the explicit adapter.
All allocation refusal leaves partial storage owned by the operation arena,
with one destructor and unchanged public outputs. State actual table lookup
complexities with the implementation; do not assume constant-time strings.

## Continuation

Current step: define the narrow scoped-route interface and component tests.
No T234 checks have run yet. Prerequisite matcher and finite-view APIs were
reviewed read-only. Next: implement that route child, then physical mapping and
operation-local snapshot/source contracts. T234 remains active and uncommitted.

## Scoped route child: implementation and first evidence

The route borrows one common `ModulePathValue` rather than interpreting a
SourceSpan as normalized text. Its canonical bytes and original expression
provenance remain separate, so catalog candidates can carry no source location
and normalized aliases can still diagnose their actual authored occurrence.
No route allocation, I/O, availability check or target-scope callback exists.

`tools/project/acquisition/routing/select.c` scans only the supplied validated
package-profile document. All matching whole-component prefixes of one identity
form an inclusion chain: a proper prefix has fewer complete components and
fewer bytes. Maximizing matching prefix byte length therefore implements the
required component order exactly, including UTF-8 names. Time is O(Q + P),
where Q is request validation bytes and P the total examined prefix bytes;
auxiliary storage is O(1). Repeated lookups do not create a global table or change
the scope, key, source bytes or allocation count.

The first harness reached its invalid-input partition and failed because it
attempted to treat `/` as a source expression. The canonical catalog-root value
is valid metadata but is not a valid source path expression. The corrected case
uses the real `module_path_from_canonical` API and verifies that no source
location is fabricated. Production routing behavior did not change to fit that
fixture. Failed logs: `/tmp/ama-t234-routing.log` (3583) and
`/tmp/ama-t234-routing-diagnostic.log`.

`make -C tests/unit/tools/project/acquisition/routing` then exited zero at
`/tmp/ama-t234-routing-accepted.log`: all 326 ordered subsets against six
independent explicit relation rows (1956 route decisions), requester-vs-target
scope separation, an unavailable specific route retained without fallback,
module/test/version endpoints and invalid profiles/prefix/catalog-root cases.
Every lookup verifies that it allocates nothing; all borrowed owners are
destroyed with counted zero-live-allocation checks. Header/registry/sanitizer
and full task acceptance remain pending.

Current step: implement the contained physical version-directory mapping from
this route, then operation-local revision snapshots and source contracts.
No active jobs remain. T234 is not complete or committed.

## Physical version-directory child

`location/directory.c` normalizes the selected repository's physical relative
directory independently of logical module syntax. Its byte stack discards empty
and dot components and refuses root-underflow; every pushed byte is popped at
most once. `location/assemble.c` appends only the logical suffix through the
common version boundary, producing one owned logical-version/physical-directory
pair. Module and test suffixes therefore cannot split one version's origin.
This does not assert Git-tree membership or filesystem availability.

The result and failed scratch belong to an explicit arena. Returned strings
are immutable and terminated; the route, normalized identity and original INI
spans remain borrowed through consumption. Two allocations suffice: reduction
scratch and one aligned record containing both final strings. Checked sums
include every separator/terminator/header; public output changes only after
complete assembly. Work and storage are linear in original directory bytes and
normalized version/suffix bytes, with no fixed path-length/depth bound.

`make -j2 -C tests/unit/tools/project/acquisition` exited zero at
`/tmp/ama-t234-location-composition.log` (38105). It covered 780 independently
encoded word-state normal forms (one to four components over a,b,dot,parent,
empty), Unicode/dot reductions, module/test/bare-version agreement, both exact
allocation refusals and recovery with the same borrowed route, zero-byte/absolute
and absent metadata. It also reran the 1956 scoped-route decisions after fixture
sharing. The compiler exposed one unused comparison helper in the location
fixture; the composition now explicitly checks the expected retained origin URL,
which also exercises that helper. Rerun the parent to require a warning-free
current result. No specification change or transport effect occurred.

Current step: finish operation-local revision/snapshot and per-version source
contract interning. All listed jobs are collected; T234 remains uncommitted.

The warning-free parent rerun passed at
`/tmp/ama-t234-mapping-accepted.log` (52065), including route and location leaves.
The next component uses an operation-owned arena for exact URL/revision snapshot
requests and URL/commit/physical-version source contracts. It will schedule no
transport itself: a new-request flag permits one explicit resolver job, and a
separate publication operation records its already-resolved commit. Equivalent
revision names may bind one version after resolving to the same commit.
Prepared-version gating must consume the common PKG-02 selection before package
indexing, so even an incomplete selected local copy cannot cause remote hole
filling. Its borrowed `ModulePackage` records, rather than only successfully
indexed configurations, are the relevant evidence.

## Operation-local source planning: implementation and first composed checks

The source owner borrows the common pre-index PKG-02 selection. A request first
checks that complete version list, so an existing selected version needs neither
a repository mapping nor successfully indexed package contents. Otherwise it
uses the exact scoped route and contained mapping children, then interns only
URL/optional-revision bytes. First insertion returns a scheduling flag; equal
requests share a stable pending snapshot. An explicit publication call copies
an externally validated commit and refuses an unequal second assignment.

Binding remains Pending until that request's own snapshot is resolved, even
when another revision already supplied a matching version. Complete remote
contracts compare exact URL, resolved commit and normalized physical directory.
Logical version keys are separate: different versions retain distinct records
while sharing a transport snapshot. Equivalent revision names may share one
version record after equal commits are observed. Conflicts preserve all prior
contracts and expose the previous record as a diagnostic witness.

`make -C tools/project` passed at `/tmp/ama-t234-source-build.log`.
`make -C tests/unit/tools/project/acquisition/sources` passed at
`/tmp/ama-t234-source-contracts.log` (76464): exact/default/revision keys, healthy
Pending, one scheduling flag, immutable resolution, each unequal contract field,
foreign handles, all nine measured request/resolution/binding allocation refusals
and recovery. It also consumes actual `module_packages_select` output: a local
version beats the dependency copy before indexing; a deliberately incomplete
selected copy remains authoritative and its later index refusal cannot create
a remote snapshot. No Git/network/process operation is available to this code.

That first run exposed an unused static comparison in the shared test fixture;
it remains a live helper in routing/location and is now inline for leaves that
do not consume it. Snapshot handle membership and failure construction were
placed in their own cohesive child files. Next: warning-free acquisition parent,
finite selector/router composition and per-file registration/sanitizers/docs.
T234 remains active; no listed job is still running.

The warning-free parent passed at `/tmp/ama-t234-sources-parent.log` (14315).
Finite selection composition passed at `/tmp/ama-t234-finite-composition.log`
(30120): actual package aliases and overlapping masks retain both matching
variant modules across package depths, exclude the unsuffixed version, skip an
empty version missing mod1, deduplicate the overlap, and route both candidates
in either inventory order. Their snapshots and physical version directories
match independently listed expected sources; an unmatched mask yields an empty
selection without error.

The latest acquisition parent passed at `/tmp/ama-t234-composition-final.log`
(99303), including Unicode aliases with a bounded nonzero original expression
offset, missing/invalid scopes, and actual foreign-operation snapshot binding
refusal. No warnings or assertions remained.

### Source-table invariants, progress and complexity

Each successful insertion links a complete node after all owned strings exist.
Snapshot count equals exact URL/optional-revision equivalence classes; version
count equals registered logical version identities. Commit publication is a
single NULL-to-owned-text transition; equal repeats allocate nothing. Every
version with a remote snapshot has a known immutable commit. Binding an unknown
revision stays Pending and cannot infer it from another version/source record.
A conflict publishes no new version association and returns the previous
immutable contract; a failed preparation's scratch remains operation-owned.

All searches are finite linear scans. With S snapshot records, P prepared
versions and V bound versions, snapshot lookup costs the sum of compared key
byte lengths; commit membership lookup is O(S), plus commit comparison/copy.
Prepared gating costs the compared logical-version bytes across at most P
records. Binding scans at most S snapshots, P prepared records and V versions,
plus the complete compared/copied URL, commit, version and directory bytes.
No constant-time string/table assumption is made. Storage consists of unique
snapshot keys/commits, unique version contracts and per-request mapped-location
records, plus retained failed scratch until operation destruction.

Each routine consumes a finite record list or bounded byte sequence and creates
no recursive transport request itself. The invoking closure/transport owns its
work queue and actual remote effects in T235/T236. New-key scheduling flags
ensure one resolver job per exact URL/revision within that operation; they do
not guess equivalence between different URLs or unresolved revision names.

Current step: final per-file registry and header checks, ASan/UBSan and the real
project-tool parent/build composition. No jobs are active. T234 is not complete
or committed. Do not claim acquisition/transport command support from these
pure planning interfaces.

## Final T234 behavior checks before documentation

The complete project-tool parent passed at
`/tmp/ama-t234-project-parent.log` (36880), preserving the independent init,
real compiled-test and standalone incremental-build consumers. The acquisition
parent passed ASan/UBSan at `/tmp/ama-t234-sanitized-acquisition.log` (67924).
Flags/environment were `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`, compilation
`-O0 -g -Wall -Wextra -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L
-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie`, and
`LINKER_FLAGS=-no-pie`. LeakSanitizer is not claimed; counted allocation
refusal and zero-live-owner assertions provide separate ownership evidence.

The final prepared-view cases cover both storage outcomes: a maintained
packages version suppresses the dependency copy, and a dependency-only selected
version is equally authoritative. Their incomplete inputs never schedule remote
repair. Those checks passed under sanitizers at
`/tmp/ama-t234-sanitized-prepared-final.log` (19187). Reusing earlier snapshot
and version handles after unrelated insertions was then added; the final
source-contract sanitizer run passed at
`/tmp/ama-t234-sanitized-source-final.log` (39194).

A final complete normal project-tool parent exited zero at
`/tmp/ama-t234-final-project.log` (51484), covering the final maintained inputs.
All jobs are collected. Thirty new maintained code/test/build inputs plus the
changed shared-source/test dispatch files have named per-file cases in
`documentation/ready.json`; public/private headers have independent compile
commands. Current implementation baseline is `09468e8` plus the T234 working
changes; freeze those bytes for the acceptance snapshot after documentation.

Current step: `make docs`, log `/tmp/ama-t234-documentation.log`. Verify inventory,
header/interface contracts and actual Doxygen destinations before archival.
No specification files or conformance fingerprints were changed. T234 does not
claim Git acquisition, checkout validation, delivered files or publication;
those remain T235/T236 responsibilities.

## T234 final acceptance

The complete documentation gate (68874) exited zero at
`/tmp/ama-t234-documentation.log`: 3344 ready inputs, zero explicit debts,
2522 maintained file pages, 9698 documented interfaces and 492 checked
destinations. All behavior/header/component/composition/sanitizer jobs are
collected; no ambiguity or deferred documentation owner blocks this task.
The full task and evidence are archived in DONE.

The accepted boundary is pure scoped planning. The compiler still reads prepared
local sources. Git effects, actual checkout validation, delivered membership and
publication remain with T235/T236. No language rule or conformance fingerprint
changed. A final Markdown-only archival regeneration must preserve every
accepted non-Markdown input and match the staged commit bytes.

Next ready task is T244: T235 still needs T239.
