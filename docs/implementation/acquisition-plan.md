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

## T235 structural source-and-test delivery

T235 starts after accepted T239 commit `60e95c94`, with T234's coherent
snapshot/version source contracts, T221's canonical paths and T226's declaration
expressions already accepted. The initial worktree was clean. Scope is
[PKG-07](../../spec/project/packages.md#pkg-07),
[PKG-08](../../spec/project/packages.md#pkg-08),
[PKG-09](../../spec/project/packages.md#pkg-09),
[SRC-05](../../spec/language/source-and-names.md#src-05),
[DEP-03](../../spec/implementation/compiler.md#dep-03) and
[PROJECT-01](../../spec/implementation/project-tools.md#project-01).
No new specification amendment is authorized by this implementation task.

### Intended contract and step order

The delivery closure is distinct from T239's emitted native build closure.
Selecting a source unit preserves its complete physical subtree, even when a
particular executable does not use every child. A retained ancestor may instead
be context-only: main file and companions keep the child's physical topology
valid, but do not activate sibling subtrees, semantic dependencies or tests.
Only an explicit selection or actual resolved need promotes that ancestor.

1. Establish exact identity and role transitions (absent, context, active),
   explicit membership reasons, promotion and one-expansion scheduling. Use a
   finite monotone work queue; no probable-use or filename heuristics.
2. Capture physical module units and context ancestors through the common
   catalog/byte snapshots. Retain every same-basename companion, entire owned
   subtree, version configuration and exact optional guidance. Delivery rejects
   invalid/unreadable supplied README under PKG-07, unlike generation under
   BUILD-05. Keep all captured ownership and failed scratch explicit.
3. Resolve source/test association from logical src suffixes. Preserve each
   complete mirrored test directory, require directly associated microprograms
   and avoid tests-of-tests for helper definitions. Retain original test types
   and source-origin dependency restrictions.
4. Expand actual ordinary/static/constant/native/configuration dependencies
   through the shared compiler resolver, in original scopes. Reuse T234's source
   contracts; transport supplies immutable snapshots under T236. Do not reparse
   paths, scan C expressions, execute CMake or mutate source/configuration.
5. Compose the least closed membership with recorded reasons and snapshots.
   Test child-only/context promotion, complete subtrees, cycles/shared inputs,
   mirrored tests, missing tests, native/static/template dependencies and test
   substitutions. Register every file, run component/real parent/failure checks,
   generate Doxygen, archive T235 and commit only after complete acceptance.

### Initial findings and continuation

`ProjectModule` and `ProjectDependency` already retain canonical module owners,
original spans, scoped aliases/selectors and explicit dependency roles. Their
loading registry terminates source cycles and supports entry-free sets through
`project_load_catalog_set`; do not replace this with textual reference search.
The catalog retains physical/logical identities and supports snapshot input.
T234 routes/binds one exact source contract per version outside the compiler.
Existing project testing receives explicit jobs; it does not yet discover the
PKG-08 association closure. Existing relocation discovery is tied to projected
placement; reuse neutral source/catalog primitives rather than invoking a
move-specific orchestrator for acquisition.

Current step: define the role/work-queue and physical-unit boundaries before
implementation. No T235 code, tests or jobs have started and no implementation
support is claimed. Resolve any genuine missing observable rule in AMBIGUITIES
before choosing its behavior. The accepted T239 generated reference remains
historical once these new Markdown task records change; final T235 acceptance
must regenerate its own inputs.

### T235 blocked classification boundary

Read-only review identified [AMB-020](../../AMBIGUITIES.md#amb-020). An ordinary
open helper directly in an associated test directory has `@!`, but cannot be an
independent concrete root until another module supplies its type. PKG-08's direct
placement requirement and PROJECT-03's independently runnable selection do not
explicitly determine its role. The mandatory-test result and selected root set
therefore differ between semantic closed-root classification and a strict
all-direct-code-files placement rule. Do not infer an underscore/name convention,
catch-and-skip compiler failures or silently impose a new directory restriction.

Current task is stopped before role/physical-unit implementation; no T235 code,
tests or active jobs exist. The issue contains the example, exact rules and
alternatives. Await an explicit decision and any needed spec authorization before
resuming this task; no other TODO task is selected to bypass the blocker.

### T235 resumed after AMB-020

The user explicitly authorized semantic concrete-root classification in response
to the specification-change question (2026-11-22). The answer and source are
recorded in AMB-020; PKG-08 now defines concrete executable ordinary/native roots,
retains open/non-executable helpers, and refuses a directory with no concrete
direct test. Real syntax/type errors are not swallowed to classify helpers.
The earlier stopped checkpoint above is historical. Current step resumes the
finite role/work-queue foundation; no implementation support is claimed yet.

### T235 role queue and association foundation

Implemented `tools/project/acquisition/closure/roles/`: the operation owns copied
canonical identities and distinct original causal edges. Insertion prepares every
allocation before publishing records, reasons, promotion or queue links. A record
starts retained as context, may promote once to active, and is claimed once.
Adding a cycle edge to an expanded record records the cause without requeueing.
For N retained identities and E distinct causes, storage is O(total identity
bytes + N + E). Linear membership/edge lookup makes a single request O(N + the
record's causes + identity bytes); the FIFO operations are constant time. The
finite measure is the number of identities not yet promoted/claimed, independent
of dependency cycles. The consumer must abort a failed enclosing expansion;
claiming work does not promise transactional retry after arbitrary I/O failure.

Implemented `closure/tests/association`: the common canonical path descriptor
establishes the exact version/src boundary. Copy its logical suffix below tests
in the same selected version. Physical '+' storage never enters this calculation.
Refuse non-src identities and mismatched version ownership. Both public output
paths publish together after complete allocation. Work/storage are linear in path
bytes, with no I/O, alias expansion, fallback root selection or filename guesses.

Checks collected:

- Initial role leaf failed compilation because the new header named a nonexistent
  `source/text/span.h`; changed it to the existing authoritative `source.h`. No
  test passed in that failed run (`/tmp/ama-t235-roles.log`, exit 2).
- Corrected role leaf passed (`make -j2 -C tests/unit/tools/project/acquisition/roles`,
  `/tmp/ama-t235-roles-corrected.log`, exit 0). Added FIFO/late-discovery checks
  afterward, then ran the real acquisition parent successfully
  (`/tmp/ama-t235-role-parent.log`, session 46617 collected exit 0). This includes
  all 7776 five-event words over three identities and two requested roles, exact
  cycle causes, FIFO/promotion, failed insertion allocation and failed promotion.
- Association leaf passed (`make -j2 -C tests/unit/tools/project/acquisition/association`,
  `/tmp/ama-t235-association.log`, exit 0): three nested levels, Unicode/multiword
  names, trailing root separators, original input lifetime independence, wrong
  area/version/path rejection and every result allocation failure.
- Every added maintained code/header/test/build input is registered to named real
  cases in `documentation/ready.json`. Whole-task generated documentation and
  final source/test/dependency compositions remain pending.

Current step: implement physical unit capture and concrete direct test admission,
then compose actual resolved compiler dependency expansion. T235 remains active
and incomplete; these foundations are not an acquisition/publication command.
No jobs are running at this checkpoint. No task commit is due before acceptance.

### T235 structural bytes and direct root projection

Added `closure/unit/`: parent selection, individual exact child capture and
iterative subtree traversal are separate responsibilities. A context captures
only main/all nonempty-extension companions; active capture also traverses the
whole optional plus subtree. It retains directory records, including empty
directories, and raw bytes including zeros/invalid UTF-8 in companion assets.
No special companion extension is discarded. Optional subtree absence is healthy;
a missing main, companion directory, link or technical enumeration/read failure
refuses capture. Exact device/inode directory registration rejects repeated
physical directories before scheduling. Each admitted directory is enumerated
once. For R structural records, insertion ordering costs O(R squared) comparisons
and bytes/path storage is proportional to retained input. No source compilation
or CMake execution occurs; the semantic consumer will classify source records.

Added `closure/tests/roots`: consume plain checked exports and collect immediate
ordinary/native candidates in complete-name order. Original parameterized
sources remain candidates because internal typed evidence can close their
formals. Static/constant support cannot be an executable candidate. The controller
performs exact independent entry classification afterward; candidate collection
alone is not admission. Final normal executable checking and typed pending-root
classification are described below. No filename, guessed type or diagnostic-text
filter selects helper roles.

Checks collected:

- Unit leaf `make -j2 -C tests/unit/tools/project/acquisition/unit` passed,
  `/tmp/ama-t235-unit.log`, session 19354 collected exit 0. Independent full
  membership list verifies all companions and unused/hidden/empty nested entries;
  binary bytes and empty files remain distinct. Context capture ignores a link
  inside its unselected subtree, while active capture refuses it. Missing main,
  missing optional subtree, non-file companion and every measured allocation
  failure are checked; counted teardown reports no live allocations.
- Initial root composition passed `/tmp/ama-t235-test-roots.log` (session 27875,
  exit 0). Expanded native/helper-only composition then passed
  `/tmp/ama-t235-test-roots-native.log` (session 72110, exit 0), using
  `make -j2 -C tests/unit/tools/project/acquisition/test-roots`. The real frontend
  checks closed ordinary/native tests, an open helper, a static marker, descendants
  and similarly prefixed neighboring directories. Native C contains intentional
  `#error` directives, demonstrating discovery does not compile or execute it.
  A helper-only directory fails mandatory-test selection. An actually invalid
  open helper fails normal definition checking rather than disappearing from the
  tests. Each projection allocation failure preserves public output.

Current step: compose role scheduling with captured unit/source membership,
associated directory capture, actual compiler-resolved dependencies and version
evidence. Independent executable test admission, full closure composition,
sanitizers and final documentation/acceptance remain pending. No jobs running.

### T235 resolved dependency projection

Added `closure/dependencies/expand`: consume a valid compiler Project and exact
member/role handles, then submit each retained direct compiler edge to the active
role queue with its original SourceSpan and requesting record. The compiler, not
this component, owns alias normalization, static/constant operand classification,
selector admission and PKG-09 checks. No path is reparsed and no edge kind is
filtered as supposedly runtime-unreachable. Registry membership checks precede
foreign handle dereference. Cycles terminate through the previously tested role
join/one-time queue, while distinct original causes remain recorded. Failure may
retain earlier successfully inserted edges; the enclosing operation must abort
instead of publishing a partial closure. Source owners outlive causal evidence.

Checks collected:

- Actual alias-resolved cycle composition and every edge allocation refusal
  passed `/tmp/ama-t235-dependencies.log` (session 20482, exit 0).
- Expanded static/constant case initially failed a normal compiler storage-form
  check: the fixture copied a list into unsuffixed `first`. The compiler correctly
  reported `Qualified constant transfer storage form differs from its destination`.
  Failed logs `/tmp/ama-t235-dependencies-static.log` (session 44909, exit 2) and
  `/tmp/ama-t235-dependencies-static-diagnostic.log` (exit 2) are retained. Corrected
  the fixture destination to `first[]`; no compiler rule/implementation changed.
  `/tmp/ama-t235-dependencies-static-corrected.log` passed (exit 0).
- The corrected case checks the actual static selector, producer arguments, shared
  concrete element and constant namespace, with an independently enumerated set
  of five required identities. An unrelated catalog member is never queued.
- Concrete test discovery now also checks a test that actually instantiates/calls
  the open helper with a concrete source type;
  `/tmp/ama-t235-test-roots-instantiation.log` (session 28749, exit 0).

Foundation parent acceptance is running with inputs frozen:
`make -j2 -C tests/unit/tools/project/acquisition`, log
`/tmp/ama-t235-foundations-parent.log`; collect the returned session before further
code/test edits. This is component-scale progress, not complete T235 acceptance.

### T235 captured membership joins

Foundation parent session 11966 completed successfully (exit 0,
`/tmp/ama-t235-foundations-parent.log`), including roles, source/test association,
structural capture, direct test roots and actual resolved dependency projection.

Added `closure/members`: a byte-sorted exact physical-path set accepts captured
PRESENT files/directories only. Overlapping captures compare bytes and storage
kind; contradictory observations refuse instead of overwriting an earlier
snapshot. Independent SourceText owners with equal bytes may merge. Every
distinct requesting role, bound source contract and structural/test/configuration/
guidance cause is retained; equal cause insertion is idempotent. Ordinary source
contracts borrow T234 operation records, while explicit standard roots carry no
ordinary package contract. Content remains borrowed from the capture owner and
member paths are copied. Prepare record/cause/path allocations before publication,
so a failed new cause cannot change an old membership. Linear sorted lookup and
byte comparison implement exact set union; storage is one retained path/record
per physical member and one node per distinct causal triple.

`make -j2 -C tests/unit/tools/project/acquisition/members` passed (exit 0,
`/tmp/ama-t235-members.log`). Its independent bitset oracle checks all 243
five-request words over three paths, with ordering/repetition independence.
Additional checks cover original source handles, multiple causes, empty files
versus directories, changed-byte/kind conflict, absent input rejection and every
measured member insertion allocation plus failed cause insertion on an old member.
All owner destructors report zero live allocations. Per-file cases and independent
header checks are registered. This is still a child component; whole-closure
controller/source-version/test-directory composition remains pending.

The first complete documentation coverage/header/interface gate passed:
`make docs-check`, `/tmp/ama-t235-foundations-docs-check.log`, session 38478
collected exit 0. It validates registered cases and independent headers, not
execution of the cases or final generated HTML. Subsequent fixture changes need
a final gate after maintained inputs settle. Read-only process probes during
that run could not observe the other sandbox's process namespace and supplied
no completion evidence; the collected session result is authoritative.

Review of successful compilation logs found unused parsing/routing fixture helper
warnings in four new leaves. Narrowed their shared support to
`tests/unit/tools/project/acquisition/closure/common.c`, which owns only byte
fixtures and result arenas. Existing T234 routing/parser support remains with its
real consumers. Recheck these changed leaves with warnings treated as errors;
no warning suppression or production behavior changes are used.

### T235 compiler/project-tool boundary review

The narrowed closure fixtures passed with `-Wall -Wextra -Werror`:
`make -j2 -C tests/unit/tools/project/acquisition roles association unit members
C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L'`,
`/tmp/ama-t235-narrow-fixtures.log`, session 34180 collected exit 0.

Boundary review found that the initial root/dependency child APIs borrowed live
Project/StaticInventory records. Replaced those interfaces before accepting T235:
`src/delivery/source/export` alone consumes checked compiler owners and exports
plain copied names, source categories and original resolved dependency causes.
`records.h` includes immutable source-span data only, with no AST, semantic graph,
plan or frontend. Project-tool root selection and dependency scheduling consume
these plain records. Original source-span byte owners outlive consumption; a
future serialization adapter recreates byte ownership rather than passing live
compiler pointers across processes. No old API implementation is retained.

The exporter preserves all original dependency occurrences and determines source
categories from the actual checked inventory. Copied names/records publish only
after complete success; all measured export allocations are tested. A sorted
definition list and original edge order give deterministic metadata. Work is
O(N squared + E + copied-name bytes), storage O(N+E+copied-name bytes).

The independent project executable links neutral exact source catalog/directory
and raw-byte capture primitives through a separate `PROJECT_TOOL_CAPTURE_SOURCES`
list. It never links the exporter producer, parser, semantics, planner or compiler
dispatcher. Its existing narrow shared-source list remains suitable for small
standalone planners. Public acquisition/build commands are still later tasks.

Checks collected:

- First export/root build caught a missing explicit SourceArena include in the
  tool header; `/tmp/ama-t235-export-test-roots.log` exited 2. Added the owning
  arena header; `/tmp/ama-t235-export-test-roots-corrected.log` passed (session
  17780, exit 0). This failed run is not positive evidence.
- Plain-export dependency composition passed
  `/tmp/ama-t235-export-dependencies.log` (session 95374, exit 0).
- Actual standalone tool build `make -j2 -C tools/project` passed
  `/tmp/ama-t235-project-tool-building.log` (session 69728, exit 0).
- Expanded export allocation/invalid-inventory and static/open/constants category
  assertions passed `make -j2 -C tests/unit/tools/project/acquisition test-roots dependencies`,
  `/tmp/ama-t235-source-export-composition.log` (session 4378, exit 0).

Current T235 work remains the complete controller: unit/context promotion,
associated-directory retention, per-version configuration/guidance evidence and
independent test admission must compose these children before task acceptance.
The transport/publication backend remains T236. T235 is not complete and has no
completion commit. No jobs remain active at this checkpoint.

Additional collected checks: normal `make -j2` passed
`/tmp/ama-t235-source-export-compiler-building.log` (exit 0). The real standalone
tool acceptance leaf passed `make -j2 -C tests/unit/tools/project/building`,
`/tmp/ama-t235-standalone-tool-acceptance.log` (session 90892, exit 0), checking
private builds, incremental stability, deliberately removed link membership,
invalid flags, recovery and actual init command behavior with the new neutral
source capture dependencies.

Current active check (freeze production/test inputs until collection):

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
make -j2 -C tests/unit/tools/project/acquisition \
  roles association unit members test-roots dependencies \
  C_COMPILER_FLAGS='-O0 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie' \
  LINKER_FLAGS='-no-pie'
```

Log `/tmp/ama-t235-foundations-address-undefined.log`, session **28701**. This
checks affected foundations with ASan/UBSan; leak instrumentation is disabled
because its ptrace limitation was established in T239. Counted exhaustive
allocation teardown remains independent evidence, not a LeakSanitizer claim.
The source-export changes and narrow fixture revision still require a fresh
documentation coverage/header/interface gate and final complete-task Doxygen
generation. Do not reuse the earlier pre-export 3406-input gate as current.

### T235 complete prepared-fact controller and semantic entry proof

Implemented `closure/plan/` as separate orchestration, structure, test admission,
version selection, file/request joins and lifetime operations. It consumes exact
prepared facts through an explicit provider. This provider boundary is the T236
transport/compiler-protocol adapter's responsibility; it is not a hidden network,
CMake, native hook or source mutation inside the compiler. Production controller
code consumes neutral source records and captured inputs, not frontend owners.

The controller accepts module and whole-version seeds, joins topology-only
ancestors, schedules complete submodules, retains all associated test bytes and
follows every resolved original compiler dependency. Whole-version seeds retain
source-tree directories/configuration/guidance even when there is no module to
instantiate. Source versions must have their bound T234 contract and captured
configuration. Standard units have no invented package version or test scope.
Explicit separately declared input groups carry their own source contract, or
unversioned byte origin, with a distinct membership reason. A nearby file is not
a declared input merely because it exists.

**Correctness argument.** For a fixed prepared snapshot, let each active identity's
fact contain its structural members, topology contexts, source descendants,
associated test/support definitions and original resolved dependency targets.
Each successful insertion is a set union, and roles only increase from absent to
context to active. Thus no rule removes a required cause, member or identity. The
queue claims every newly active identity once, including promotions, so every
closure obligation is eventually submitted. Conversely every insertion comes
from a seed or one exact structural/test/dependency/explicit-input fact, proving
minimality by induction against any set closed under those same facts. Retaining
whole selected units/directories is an explicit closure rule, not guessed
reachability. Cyclic source edges add reasons without requeuing expanded keys.

For finitely many reachable source/version identities N and finite fact entries,
there are at most N active expansions. Context promotions cannot add a second
active expansion. Candidate test classification is independent per concrete
source entry, on its own normal compiler closure, so another test cannot supply
missing type evidence. Provider completion and normal compiler termination remain
explicit assumptions; no depth/time/retry heuristic supplies a proof. Storage is
O(unique identity bytes + members + distinct causes + selected tests), besides
provider-owned snapshots/compiler facts. Linear set lookup and sorted insertion
give quadratic comparison bounds in retained identities/members, plus equality
byte checks and the actual provider/compiler costs. No subquadratic performance
claim is made. Failure stops with `valid == false`; partial scratch remains owned
and is never a publishable delivery.

**Entry inference correction before acceptance.** The original zero-formal
shortcut was insufficient under TPL-03. A read-only `./ama --check` probe in
`/tmp/ama-t235-grounded-root` passed (exit 0, log
`/tmp/ama-t235-grounded-root.log`) for an output `.T` determined by a typed
constant default. Added maintained regression cases rather than using that
probe as sole evidence. Compiler `delivery/source/entry` now observes normal
independent checking: completed plan means executable; typed unresolved closure
with an exact pending original generic root key means open; all other failures
remain invalid. It checks the selected root's provenance/key, not an unrelated
pending callee or unknown body field, and reads no diagnostic strings.

Source export categories now preserve original concrete/parameterized definitions;
they do not pretend that original formal count proves executable eligibility.
The controller classifies every immediate runtime candidate independently, skips
only the explicit healthy-open result and rejects actual failures. If none are
executable it reports missing concrete tests. PKG-08 links the existing TPL-03
inference rule to make this accepted AMB-020 behavior explicit; no new type
inference rule or syntax was introduced.

Checks collected before final frozen acceptance:

- ASan/UBSan affected foundations passed session 28701, exit 0,
  `/tmp/ama-t235-foundations-address-undefined.log`, with the exact command above.
  Later controller/entry changes require the final sanitizer run; that earlier
  result covers only its then-current input scope.
- Required-directory capture passed session 80667, exit 0,
  `/tmp/ama-t235-required-directory.log`, including every measured allocation.
- Real prepared source/context/cycle/test composition passed session 94484,
  `/tmp/ama-t235-plan-composition.log`; mandatory helper-only association passed
  session 17152, `/tmp/ama-t235-plan-test-association.log` (both exit 0).
- Controller allocation and empty-version compositions passed session 22804,
  `/tmp/ama-t235-plan-failure-composition.log`, exit 0. Allocator fault injection
  targets every measured controller allocation independently of healthy provider
  filesystem/compiler preparation, with zero live metadata afterward.
- Injected test-type composition passed session 83465,
  `/tmp/ama-t235-plan-injected-type.log`; granular fixture split passed session
  85487, `/tmp/ama-t235-plan-granular.log` (exit 0). Native standard-root capture
  passed session 14075, `/tmp/ama-t235-plan-standard-native.log`, exit 0. Native
  C and CMake contain intentional errors proving those bodies do not execute.
- Exact grounded/open/error entry classification and complete controller use
  passed session 54021, `/tmp/ama-t235-semantic-entry-classification.log`, exit 0.
  Typed conflict and closed-root unknown body checks passed session 5274,
  `/tmp/ama-t235-entry-error-boundaries.log`, exit 0. A root with an internally
  grounded own parameter but a separately unknown body/callee still refuses,
  session 34373, `/tmp/ama-t235-grounded-root-unresolved-body.log`, exit 0.
- Explicit declared-input retention passed session 85909,
  `/tmp/ama-t235-plan-declared-inputs.log`; nonempty whole-version enumeration
  and independent tests passed session 99130,
  `/tmp/ama-t235-plan-whole-version.log` (both exit 0).

Current step: freeze final maintained inputs, run the real acquisition parent,
final source-export/controller sanitizers, standalone tool/build consumers and
documentation gates. T235 remains incomplete until those results are collected,
its full record is archived and the task commit is made. No jobs are active at
this checkpoint. T236 owns transport/protocol preparation and publication;
T224 owns copy/delete/move consumption. No public acquisition command, automatic
CMake discovery, test execution or source publication is claimed here.

### T235 frozen acceptance checkpoint

Final normal acquisition parent passed: `make -j2 -C
tests/unit/tools/project/acquisition` with `-Wall -Wextra -Werror` and the normal
C11/POSIX flags above, `/tmp/ama-t235-final-acquisition-parent.log`, session 40765
collected exit 0. Compiler build `make -j2` passed
`/tmp/ama-t235-final-compiler-build.log` (session 53381, exit 0). Independent tool
build/incremental/failure/recovery/init acceptance passed
`/tmp/ama-t235-final-tool-building.log` (session 6212, exit 0).

Current jobs, with production/test inputs frozen:

- `make docs-check`: `/tmp/ama-t235-final-docs-check.log`, session **37042**;
  awaiting completion. This is the final current source/header/case gate.
- Final affected ASan/UBSan parent: the prior sanitizer command with targets
  `roles association unit members test-roots dependencies plan`, log
  `/tmp/ama-t235-final-address-undefined.log`, session **1550**.
  It includes the final exact entry classifier and whole prepared-fact controller.

Next: collect both jobs, then run Doxygen and inspect final staged task evidence.
Only after those gates pass may T235 move to DONE and receive its task commit.

Final frozen checks collected: documentation coverage/header/case gate session
37042 passed (exit 0), **3430 ready inputs, zero debt**. Final ASan/UBSan affected
parent session 1550 passed (exit 0), with `-Werror`, exact flags above and no
sanitizer/runtime-error diagnostics. Both include the final prepared-fact
controller, typed entry classifier, whole/empty versions and declared inputs.
LeakSanitizer remains disabled for the already documented environment limitation;
exhaustive counted controller/child allocation teardown is separate evidence.

Final required Doxygen generation is next. Maintained source/test inputs remain
frozen; only task/acceptance Markdown will change for archival after that gate.

### T235 generated-reference corrections

The first full `make docs` passed its 3430-input coverage/header/case gate but
failed the actual originating-page link check (session 71464, exit 2,
`/tmp/ama-t235-final-doxygen.log`). Interface comments cited requirements absent
from their file-purpose comments, first reported at `closure/members/members.h`.
Added purpose explanations and the exact canonical rule links to eleven file
comments. Audited real C comments with the existing documentation scanner: each
file-purpose rule set now covers its originating interface/helper references.

A temporary proof script initially removed an original standalone rule link as
well as newly added links and correctly rejected the comparison. Replaced that
script with exact added-rule sets per file; reconstruction then matched every
frozen accepted file hash. `/tmp/ama-t235-file-purpose-only.json` records the
eleven affected files, and `/tmp/ama-t235-frozen-inputs.json` retains all 57
non-Markdown fingerprints. The reconstructed old bytes match exactly, the
changes lie only in `@file` comments, and affected files contain no `__LINE__`
or `#line`; behavioral source and interface tokens are unchanged. This is not
a new behavioral/sanitizer run or a conformance-fingerprint refresh.

Supported Doxygen regeneration then caught one undocumented forward typedef
(session 12993, exit 1, `/tmp/ama-t235-doxygen-purpose-corrected.log`): documented
`AcquisitionRoleRecord`'s stable identity purpose/cycle lifetime. Its exact inserted
comment was likewise removed in a hash comparison to the accepted prior bytes,
then the frozen input record was updated. No implementation branch changed.

Current step: regenerate/check Doxygen after those documentation-only corrections,
then archive/commit T235 only after the generated reference passes. No failed
documentation run is treated as successful acceptance.

### T235 accepted result and continuation

Supported Doxygen generation passed session 54907 (exit 0),
`/tmp/ama-t235-doxygen-forward-purpose.log`: **2597 maintained file pages,
9981 documented interfaces, 493 canonical/reference destinations, 8209 pages**.
Tools: Doxygen 1.11.0, markdown-it-py 3.0.0, Python 3.14.4. The generated manifest
matched the supported build report, and every rendered-input fingerprint plus
all 57 frozen non-Markdown inputs matched before Markdown archival.

T235 is accepted and its complete task moved to DONE; dependent navigation is
updated. Its prepared-fact controller has seven new independent leaves and 40
named cases, original compiler dependency/category export and exact independent
entry classification. Only the twelve described documentation comments changed
after the final behavioral/sanitizer gate; reconstructing their prior bytes
proved every other accepted byte unchanged. Failed documentation/proof runs
remain recorded above and are not counted as passes.

The next ready task is **T224**, consuming this closure for resolved copy/delete/
move and safe replacement. T236 owns actual Git/process snapshot preparation,
compiler protocol integration and publication/recovery. The T235 provider
interface is an explicit preparation boundary, not a newly supported public
acquisition command. It does not execute test bodies, native code or CMake.
T237 still owns current complete release/conformance checks; no source hashes in
old conformance claims are refreshed to manufacture acceptance.

Current finalization step: regenerate Doxygen for these Markdown-only archival
changes, recheck rendered/frozen hashes, inspect explicit staged paths and commit
T235. No implementation/test inputs are being changed; no task is skipped around
a blocker.

The first archival regeneration passed session 55611 (exit 0),
`/tmp/ama-t235-doxygen-archived.log`. Review then corrected only a TODO overview
comma, the newly ready T224 status sentence and an extra blank line at DONE EOF
reported by `git diff --check`; no implementation/test file changed. Final
regeneration after those Markdown corrections owns delivered provenance.
