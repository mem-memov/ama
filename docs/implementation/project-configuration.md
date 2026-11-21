# Project configuration and path resolution

## Ownership and current task boundary

[T232](../../DONE.md#t232) owns shared configuration, normalization and the
compiler/tool integration. Its independently verifiable foundations are
[T260](../../DONE.md#t260) (strict INI documents),
[T261](../../DONE.md#t261) (paths and alias scopes),
[T263](../../DONE.md#t263) (selector matching),
[T262](../../DONE.md#t262) (selected-unit coverage and ordinary edge policy), and
[T265](../../DONE.md#t265) (standard-origin isolation).
T260 implements source-only INI parsing and T261 implements path normalization
and alias scopes. T262 implements selection coverage and edge policy. T232 now integrates these
foundations with the required-project frontend; its accepted boundary and
remaining tool owners are stated in the final acceptance record below.
The prerequisites are committed: T231 `036d3d1`, T222 `ddd055d`, and diagnostic
follow-up T259 `56e8708`. The earlier source foundations required no specification amendment; T264
subsequently records the explicitly authorized standard-isolation clarification.

Canonical contracts are [PKG-03](../../spec/project/packages.md#pkg-03),
[PKG-04](../../spec/project/packages.md#pkg-04),
[PKG-05](../../spec/project/packages.md#pkg-05),
[PKG-06](../../spec/project/packages.md#pkg-06),
[PKG-09](../../spec/project/packages.md#pkg-09),
[SRC-06](../../spec/language/source-and-names.md#src-06),
[CONST-05](../../spec/language/declarations.md#const-05), and
[DEP-01/02](../../spec/implementation/compiler.md#dep-01).
The [configuration EBNF](../../spec/grammar.md#project-configuration-and-dependency-selectors)
resolves syntax separately from later alias/coverage and filesystem checks.

## Planned component boundaries

1. Configuration encoding/physical lines preserve immutable original bytes,
   reject BOM/TAB/lone CR/malformed UTF-8, and recognize empty physical lines,
   LF/CRLF and a nonempty final unterminated line. INI comments and value data
   follow the INI profile, not Ama source literal/comment rules.
2. Record recognition separates exact headers, assignments and selector records.
   Typed package/module parsing owns section/key uniqueness, legal value forms,
   required repository fields and exact original spans. Module records remain
   sectionless. No interpolation, shell expansion, downloading or module lookup
   belongs here. A later equals sign and semicolon in a value remain data.
3. Pure normalization consumes complete raw path components, a defining directory
   and an explicit namespace kind. A finite stack implements dot/parent reduction;
   underflow refuses before any filesystem request. Keep each surviving component's
   source provenance and never rewrite immutable AST/source bytes to canonical text.
4. Scoped alias tables use exact names and distinct package/module relative bases.
   Collect before resolution so declaration order does not affect meaning.
   Package targets have no alias; local targets may use only package aliases.
   Expansion is bounded by these levels, not a cycle-detection heuristic.
5. Selector matching uses finite scalar/component dynamic programming against
   valid canonical identities. Normalize real alias bases before mask suffixes;
   wildcard cancellation and search-order-based preference are forbidden.
6. Catalog/project integration retains selected package/configuration ownership,
   resolves every written path in its defining source's environment, and checks
   explicit cross-version coverage plus src-to-tests isolation at that edge.
   Preserve caller-origin test substitutions and unnormalized occurrence spans.
   Qualified constants expand/normalize the complete path before final-member
   selection; the current eager raw split cannot remain the resolver authority.
7. Consumers use resolved identities rather than reparsing raw source spellings.
   Include ordinary/static declarations, constant sources/read namespaces, native
   metadata, editor/source indexes and dependency input capture. Record selected
   package.ini and used module.ini presence/absence/effective scopes in caches.
   Relocation's full new-path rewrite belongs to T224; its shared read-only model
   must nevertheless remain capable of consuming prepared catalog snapshots.

## T260 contract and correctness obligations

The parser borrows one immutable SourceText and allocator authority. All returned
names, paths, values and diagnostic spans identify original bounded byte ranges;
no borrowed view survives source destruction. The document owner retains every
complete/failed allocation until one explicit destructor call. Only a complete
valid document is consumable; invalid or allocation-refused parses cannot publish
partial success or mutate the original bytes. Configuration parsing performs no
filesystem/network calls and creates no Ama instance or nominal identity.

Each physical-line transition consumes positive input bytes or reaches EOF.
Record checks operate on bounded spans. Exact header/key comparison, never a
shorter prefix or locale comparison, establishes uniqueness. Required fields are
checked for each repository before document success. The implementation must
state its actual lookup complexity rather than assume that small files make
quadratic work linear. Allocation arithmetic is checked before construction;
all refusal paths retain the same teardown owner.

Planned partitions: empty/comment-only and empty optional sections; every ending
form; Unicode/multiword names; raw later equals/semicolon data; exact assignment
spaces; whitespace-only/padded lines; malformed encoding/BOM/TAB/CR; module/package
profile differences; duplicate/reordered sections and keys; required/unknown
repository keys and directory containment; alias/path/static distinctions;
selector traversal/mask syntax; deterministic original error sites; complete
owner cleanup after every measured allocation failure. Component tests cover
physical-line/record operations, and complete-document tests exercise their real
handoffs. Per-file test ownership is registered before acceptance.

## Continuation checkpoint

2026-11-21: T260's allocation-free line/encoding boundary is implemented under
`src/source/configuration/lines/` and independently tested by
`tests/unit/source/configuration/lines`. The shared source parent registers its
configuration subtree. `make -C tests/unit/source/configuration/lines` passed
(log `/tmp/ama-t260-configuration-lines.log`, exit 0). It checks exact source
spans and output preservation, LF/CRLF/unterminated endings, internal NUL and
noninitial BOM data, malformed encoding/TAB/lone CR, comment/line boundary padding,
and no allocations while scanning under a fail-next allocator. The whitespace
predicate was exhaustively checked over 0 through 0x10FFFF against the 25
independently listed White_Space code points from Unicode 17's official
[PropList](https://www.unicode.org/Public/17.0.0/ucd/PropList.txt); UINT32_MAX is
also refused as a whitespace scalar. This pins boundary classification instead
of inheriting the host locale. First/last-scalar checks preserve all internal
value data; a backward trailing-scalar scan is bounded by validated UTF-8 width.
The modified source parent still needs its complete acceptance run after typed
records are connected. Source/header/test/build inputs have named per-file
coverage entries; final documentation gates remain pending.

T260's record and complete-document layers are now implemented. Independent
record/shape tests passed (`make -C tests/unit/source/configuration/records`,
`/tmp/ama-t260-configuration-records.log`, exit 0). The configuration parent
passed (`/tmp/ama-t260-configuration-parent.log`, session 38656, exit 0), followed
by the enhanced document leaf (`/tmp/ama-t260-document-long-record.log`, exit 0).
The latter includes all 36 section/field order permutations, exact duplicate and
required-field refusals, original CRLF/Unicode issue sites, raw value bytes,
case/prefix/canonically-equivalent Unicode alias distinctions, all six allocation
refusals across a mixed document, every byte prefix of that document, and a
2048-component raw path without a fixed buffer or recursive path scan.

The actual document uniqueness implementation scans earlier complete exact keys.
For B input bytes, R retained records and maximum compared key/pattern length K,
time is O(B + R squared times K) and owned auxiliary storage is O(R), excluding
the borrowed SourceText. Each fixed-size arena allocation is checked by its
existing overflow-safe allocator. Alias/selector/repository records are linked
only after initialization; every refusal leaves valid false and the same arena
destructor owns all partial state. Each line is consumed once, each raw shape
scan is bounded by its span, and duplicate loops traverse finite earlier lists.
Repository-directory checks establish lexical relative containment only; actual
checkout filesystem containment/representability belongs to acquisition.
No source module, remote repository or alias target is looked up by T260.

T260 acceptance includes trusted full source-parent/sanitizer runs and final
per-file/Doxygen validation before its task commit. All new maintained source,
header, fixture and Make inputs have named case mappings.
Root `make` passed (`/tmp/ama-t260-build.log`, exit 0). The first source-parent
and sanitizer launches both exited zero, but were incorrectly launched together
against an overlapping leaf output owner. Those logs are not trusted sanitizer
or composite acceptance. Both jobs have been collected and stopped normally;
rerun the source parent then the sanitizer leaf serially before claiming either.
Original logs: `/tmp/ama-t260-source-parent.log` and
`/tmp/ama-t260-document-sanitized.log`.
Serial rerun logs: `/tmp/ama-t260-source-parent-serial.log` and
`/tmp/ama-t260-document-sanitized-serial.log`.
The serial rerun completed successfully: source parent then ASan/UBSan document
leaf both exited zero (session 61500). The previously established LSan environment
restriction is unchanged; no leak-sanitizer pass is claimed. Counted fixture
ownership covers all document allocation refusals separately.

T260 is accepted as a source-only foundation. Final delivery regenerates Doxygen
and verifies original/staged fingerprints before its commit. There are no active
code-test jobs. T261 is next, then T262 and T232 integration. No filesystem alias
resolution, dependency authorization or compiler support is claimed by T260.

Documentation preacceptance initially refused a missing canonical rule link on a
new enum contract (`/tmp/ama-t260-documentation-preacceptance.log`). Reviewed all
new compiler Doxygen blocks, added requirement-specific explanations/links, and
verified every block through the actual rule-contract validator. An independent
before/after check established that production bytes outside Doxygen blocks did
not change. Added the remaining duplicate-directory-field case to the complete
schema refusal table; the final document leaf passed
(`/tmp/ama-t260-document-final-schema.log`, exit 0). Final acceptance reruns the
current root build and sanitized document leaf; source-parent's prior serial
acceptance remains distinct from these targeted final checks.

Final targeted checks passed: root `make` (`/tmp/ama-t260-build-final.log`) and
ASan/UBSan complete-document tests (`/tmp/ama-t260-document-sanitized-final.log`),
both exit 0. No code-test job remains active. Compiler comments explicitly cite
the canonical rule for every Doxygen contract, including private helpers/types;
no documentation requirement or warning policy was weakened. Next gate is the
corrected full documentation preacceptance run, with log
`/tmp/ama-t260-documentation-preacceptance-corrected.log`.

The corrected documentation preflight then found missing standalone-header command
metadata (`/tmp/ama-t260-documentation-preacceptance-corrected.log`). Registered
strict independent C11 commands for every new header, matching the existing
source-header contract. Before another complete run, invoked the actual rule,
standalone-header and included-test-signature validators for all new configuration
inputs; all passed. The next complete run is recorded at
`/tmp/ama-t260-documentation-preacceptance-headers.log`.

The full registry/header/case check passed with 3150 ready inputs and zero debts.
Rendered validation then refused a canonical link on a header's originating file
page (`/tmp/ama-t260-documentation-preacceptance-headers.log`). Doxygen moves
some type contracts to separate type pages; file-purpose comments now explicitly
route all related canonical rules as well. Original type/function explanations
remain intact. Production bytes outside Doxygen blocks are unchanged. Verify the
actual generated links with the supported builder before repeating the complete
acceptance command; no generated page or validator is edited by hand.

The supported builder then passed actual file/type/interface/link validation:
`python3 tools/documentation/build.py --doxygen doxygen`, log
`/tmp/ama-t260-documentation-origin-links.log`, exit 0. Observed 2358 maintained
file pages, 9151 documented interfaces and 488/488 canonical/referenced
destinations. This generation does not substitute for the pending complete
`make docs` preacceptance run. That complete command now runs at
`/tmp/ama-t260-documentation-complete-preacceptance.log` before task archival.
After archival-only narrative edits, regenerate with the supported builder and
verify final maintained/staged fingerprints; code/registry/header evidence is
reusable only if those inputs remain unchanged.

Complete `make docs` preacceptance passed, exit 0, with 3150 ready inputs and zero
debts, 2358 maintained file pages, 9151 documented interfaces and 488/488 checked
destinations (`/tmp/ama-t260-documentation-complete-preacceptance.log`). T260 has
moved from TODO to DONE. Only task/narrative Markdown changes follow that gate;
code, per-file registry and header-check inputs are frozen against a saved exact
fingerprint set. Final supported-builder regeneration validates the archived
narrative and all actual destinations (`/tmp/ama-t260-documentation-delivery.log`)
before staged/worktree/manifest comparison and commit. No acceptance job is
silently treated as complete before its status is collected.

## T261 path and scope contract

Status: active after T260 `f62e448`; repository was clean at task start.
Canonical authority remains SRC-06/PKG-04/CONST-05; no spec edit is authorized
or needed. This section is a design contract until its acceptance is recorded.

Planned boundaries:

1. Extract the existing authoritative ModuleRootKind into a narrow path header;
   retain its values and include it from the catalog instead of duplicating root
   enums. Extend the existing canonical validator through one shared structured
   scan that distinguishes namespace/version/area/module endpoints. Existing
   complete-module acceptance must remain identical. Package scopes and later
   selector tools need a real version boundary, not a synthetic probe module.
2. Represent a normalized path as an arena-owned canonical spelling and bounded
   component views with original SourceSpan provenance. Generated base components
   have no invented source location. Inputs remain immutable; failure leaves the
   output pointer unchanged and all scratch remains under the supplied arena.
3. Normalize with a finite component stack. Dot is a no-op; parent removes one
   component and underflow refuses immediately. The initial parent of ../ is
   a real reduction, not a discarded prefix. Relative inputs use the explicit
   defining directory/root; absolute paths select their written root. No physical
   intermediate path is loaded. Validate resulting namespace shape separately
   from requiring a complete module or splitting a qualified constant.
4. Alias use accepts only a read-only lookup of already normalized targets.
   Lookup the entire leading alias name once and normalize its appended suffix;
   no remaining component can trigger another alias lookup. Preserve both original
   use occurrence and surviving component origins.
5. Construct package aliases from direct targets against the version directory.
   Construct module aliases against its containing directory, permitting only
   package aliases on their right side. Collect/check all keys first, with exact
   shared/local collision refusal; no source-order binding or module-alias chain.
   A normalized target denotes a namespace/module prefix without importing it.
   Validate that the supplied package environment belongs to the actual module
   version; a foreign version's table cannot leak in through caller metadata.
6. Select constants only after normalization. The final name is the member and
   the remaining prefix must itself be a complete module identity. Never try
   alternate splits, defaults, filesystem candidates or implicit instances.

Invariants: roots never change under relative reduction; each live stack entry
is a whole validated component; every reduction consumes input or removes one
entry; normalized results contain no dot/parent/alias markers. Canonical identity
is idempotent under repeated normalization, while source provenance remains tied
precisely to the current written occurrence. Alias expansion is finite by scope
construction, not a recursion-depth heuristic. Code must check all combined
allocation/count/text arithmetic, retain explicit ownership and report original
spans on syntax, scope, underflow, endpoint or allocation refusal.

Acceptance partitions: unchanged bounded canonical-module models plus partial
namespace/version/area shapes; ordinary/standard and relative roots; empty/base/
underflow and first ../ behavior; Unicode/multiword/deep paths; cancelled missing
intermediates without I/O; exact component origins; alias whole-name lookup,
local/shared collisions, unknown/self/local chains and reordered declarations;
foreign-version scope refusal; constant selection including aliases that supply
the complete qualified path; idempotence; every measured allocation refusal.
Use independently stated finite models and actual T260 document compositions.
Register every maintained input and standalone header command from creation.

Checkpoint: task planning is recorded, but T261 code is not yet implemented and
no acceptance run is active. Next operation is the narrow root/structured-path
foundation, with independent tests before normalization and scope consumers.

T261 step 1: moved the unchanged ModuleRootKind values to a narrow root header,
reexported by the catalog. The canonical validator now shares a structured scan
that describes roots, component counts, version byte/component boundaries and
structural source areas. Complete-module acceptance retains its original public
predicate and algorithm domain. Existing bounded version/path/canonical models
passed (`/tmp/ama-t261-existing-path-models.log`); new exact descriptor/refusal and
bounded shape tests passed (`/tmp/ama-t261-path-shape.log`). No filesystem lookup
or synthetic probe module is involved. Standard components named src/tests never
become ordinary package areas.

Extracted the original SourceSpan/message/expected issue shape into SourceIssue
in the source-text layer, removing the obsolete INI-specific type instead of
retaining compatibility aliases. This prevents the pure normalizer from depending
on INI line parsing. All configuration component leaves passed after the type
extraction (`/tmp/ama-t261-shared-source-issue.log`, exit 0). The existing status,
source spans, diagnostic text, parsing behavior and ownership remain unchanged.
New inputs have explicit per-file cases and standalone header commands.

Normalization and alias-scope construction are still unimplemented. A normalized
prefix is a component-stack normal form, not necessarily a complete or extendable
module identity before a later suffix is reduced. Do not apply endpoint checks
early: actual module/constant use supplies that context under SRC-06/CONST-05.
The structured canonical descriptor is for known canonical metadata and actual
endpoint checks, not permission to load an intermediate alias component.
Checkpoint: no active job; next is arena-owned normalized path storage and the
pure stack reducer, followed by typed INI/alias-scope compositions and acceptance.

T261 step 2: the pure normalization component now admits raw syntax, obtains at
most one already-normalized alias target, reduces complete components on a sized
arena stack and materializes independent output bytes in one aligned result
block. A copied result retains original SourceSpan origins without retaining the
metadata/provider arena. Canonical metadata import and containing-directory/
qualified-constant selection are separate operations. Constant member names use
the ordinary name rule after normalization; an unremoved dotted version token
cannot become a constant name. Root-only normal forms remain internal prefixes,
not newly admitted bare source operands.

Normalization tests passed (`/tmp/ama-t261-path-normalization-corrected.log` and
`/tmp/ama-t261-normalization-storage-boundaries.log`, exit 0). The initial test
compile missed stdint.h for SIZE_MAX, then passed after that explicit include;
no production behavior or expectation was weakened. Independent reverse
cancellation and prefix-balance models cover 21840 operands: both roots, both
./ and ../ starts, every 4-ary word of lengths 1..6 over a/b/dot/parent. Observe
exact final names, first underflow sites and surviving component provenance.
Additional cases cover cancelled nonexistent intermediates without I/O, deferred
endpoint validity across alias suffix reduction, whole multiword alias names,
Unicode origins, constant paths supplied entirely by an alias, metadata/provider
arena retirement, all measured allocation refusals and representability guards.
All new inputs have case mappings and headers have standalone commands.

Only stack normal form is required for an intermediate alias target. Its suffix
may remove a temporarily invalid module-area component before the actual source
use requires a complete identity. This follows normalize-before-use under SRC-06;
no intermediate module is queried. The canonical shape descriptor is used for
known metadata and final role checks, never as a premature alias-target probe.

Checkpoint: T261 normalization is independently tested; alias scope construction
is still unimplemented. No test job is active. Next: exact package/local alias
scope preparation, version ownership checks and real T260 document compositions,
then current-source parent/root/sanitizer and documentation acceptance.

T261 step 3: immutable alias scopes now prepare one sorted own table and borrow
at most one package table. Package relative targets use the exact version base;
local targets use the containing module directory and only package aliases as
their expansion provider. All key collisions are rejected before target
normalization. Version matching compares the complete structural version prefix,
not a character-prefix approximation. Parsed document nodes are not retained:
only original SourceText spans, normalized owned bytes and the explicit package
owner survive construction. Normalization still does not load any module.

Scope construction uses finite insertion sorting and exact binary lookup. For
n own entries and maximum compared-key length K, own ordering and recovery of
original definitions take O(n² K); collisions add O(n log(p+1) K) for p package
entries, plus the actual path reduction/copy cost. Lookup examines at most two
tables in O((log(n+1)+log(p+1)) K), without recursion or module inheritance.
Storage is proportional to own entries and copied normalized targets, not the
entire shared package table per consumer. Each reduction consumes one raw
component; each insertion-sort step decreases its cursor. These finite measures
also rule out implicit alias cycles independently of any depth limit.

Acceptance so far: configuration parent `/tmp/ama-t261-configuration-parent.log`,
full source parent `/tmp/ama-t261-source-parent.log`, and root compiler build
`/tmp/ama-t261-compiler-build.log` exited zero. Changed compiler Doxygen contracts,
standalone headers and registered real test interfaces passed their scoped
validators. New scope tests cover source-order independence, original spans,
parsed-node retirement, exact names, local isolation, shared-key collision before
RHS resolution, foreign scopes and every measured package/module allocation
refusal. The initial sanitizer invocation reached LSan's established ptrace
restriction (`/tmp/ama-t261-alias-sanitizers.log`); this is not a sanitizer pass.
ASan/UBSan is being rerun with leak detection disabled; counted owner teardown
remains the independent leak/failure evidence.

Checkpoint: T261 is still active. Collect alias ASan/UBSan, then run normalized
paths under the same instrumentation serially; complete current documentation
acceptance before archiving. No specification or compiler integration claim is
made by these source-only foundations.

T261 acceptance: alias and normalization ASan/UBSan leaves both exited zero,
serially, with ASAN_OPTIONS=detect_leaks=0, strict C11 flags plus
-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie, and LINKER_FLAGS=-no-pie.
Logs: `/tmp/ama-t261-alias-asan-ubsan.log` and
`/tmp/ama-t261-normalization-asan-ubsan.log`; neither contains sanitizer findings.
The full documentation gate `/tmp/ama-t261-documentation.log` passed with 3181
ready inputs, zero debts, 2387 file pages, 9247 interfaces and 488/488 destinations.
Only archival Markdown changes follow this accepted source/registry snapshot.
Final supported regeneration and manifest/staged agreement are required before
commit. T261 is accepted; no active test job remains. Next ready task is T262.

Final diff review removed one trailing blank line from each of the two alias test
support files; comparison against the accepted staged snapshot proves these are
only EOF-newline normalization. The complete alias leaf was rerun successfully
(`/tmp/ama-t261-alias-final-format.log`). Production, registry, public headers and
all other test bytes remain identical to the full documentation-gate snapshot.
Final regenerated delivery is `/tmp/ama-t261-documentation-commit.log`.

## T263 selector matching contract

Status: accepted after T261 `161448f`; clean worktree at entry. This independent
foundation is extracted from T262 before selection coverage and edge admission.
Canonical authority is [PKG-05](../../spec/project/packages.md#pkg-05), with
[PKG-04](../../spec/project/packages.md#pkg-04) package-only alias expansion and
[SRC-06](../../spec/language/source-and-names.md#src-06) normalized real bases.
No specification amendment is authorized or needed.

Compile a validated selector's component literals into Unicode scalar arrays.
Copy normalized alias-base components before appending the untouched wildcard
suffix. Never normalize a wildcard or accept dot/parent traversal. Only package
scopes may supply selector aliases and only the ordinary root is eligible.
Compiled scalars are arena-owned; SourceSpan origins borrow original SourceText,
not document nodes or alias-target storage. Publish only after full preparation;
failed scratch stays owned by the supplied arena.

Matching examines a complete ordinary version or module identity. Ineligible
identities, including standard paths and mere vendor/package/area prefixes,
produce false rather than an invented candidate. Identity existence remains the
prepared-view consumer's responsibility; matching does no I/O or enumeration.
Exact word DP tracks prefixes of pattern scalars and candidate scalars: literal
extends two matching prefixes; star either consumes no character or extends its
current matched prefix. Exact path DP tracks prefixes of component sequences:
a whole-component star consumes at least one component, then optionally more.
Embedded stars never consume a separator. Boolean reachability unions all
partitions, so duplicate decompositions cannot change a result or choose a
preferred version. Independent tests will enumerate possible star expansions
rather than reproduce these DP recurrences.

Matcher scratch is local and destroyed on every success/failure. Output changes
only after complete success; original inputs never change. Size arithmetic for
component tables and two-row workspaces is checked. Both DP traversals are
finite increasing index domains; no recursive search, semantic depth bound,
backtracking heuristic, filesystem probe or latest-version selection is used.
For pattern scalar count P, candidate byte length B and component count C,
word comparisons across all component pairs cost O(P B); path reachability adds
O(pattern-components times C). Compilation is linear in copied spelling and
scalar counts, matching scratch is O(B+C), and compiled storage is linear in
pattern size. Actual byte/character distinctions are explicit under UTF-8.

Checkpoint: contract and prerequisite order recorded; no T263 code or acceptance
exists yet. Next implement narrow pattern preparation and two-row match kernels,
then independent finite models, real INI/alias composition, allocation injection,
standalone headers and generated documentation before the task commit.

T263 steps 1–2: preparation now separates raw admission/package-only expansion,
component streaming, checked measurement, immutable scalar storage and final
publication. A shared public selector-spelling header removes the consumer's
need to import private record-parser declarations. Candidate workspace ownership,
scalar DP, component DP and cleanup have distinct source owners. Alias lookup
adds its existing O(log A times K) exact-name cost to otherwise linear compilation
for A package aliases and compared key width K.

Initial selector leaf `/tmp/ama-t263-selector-models.log` and configuration parent
`/tmp/ama-t263-configuration-parent.log` passed. The complete current selector
leaf passed ASan/UBSan with fail-fast errors and leak detection disabled
(`/tmp/ama-t263-selector-asan-ubsan.log`); counted fixture owners independently
prove teardown on every measured compile/match allocation refusal. There are
17,640 exhaustive model pairs: 120 scalar patterns times 62 words, and 340
component patterns times 30 paths. The independent models enumerate substitutions
and partitions recursively only inside these bounded tests, never in production.
Additional checks cover package alias origins/retirement, absent/local/standard
alias refusal, uncancellable traversal, empty/root results, exact Unicode and
version identities, ineligible candidate shapes, 1024 components and 4096-byte
names. Compilation scratch remains arena-owned and matcher scratch retires
immediately on every exit. Scoped canonical-contract, actual test-signature and
strict standalone-header validators passed.

Checkpoint: no test job active. Next run current root/source composition and full
documentation gate while T263 remains TODO, then archive with final fingerprint
agreement and commit. T262 remains selected-unit/edge authorization owner; the
matcher does not enumerate files, assert existence or implement dependency
coverage by itself.

T263 acceptance: current root `make` and `make -C tests/unit/source` both exited
zero (`/tmp/ama-t263-compiler-build.log`, `/tmp/ama-t263-source-parent.log`).
Complete `make docs` passed (`/tmp/ama-t263-documentation.log`): 3198 ready inputs,
zero debts, 2403 file pages, 9289 interfaces and 488/488 canonical destinations.
ASan/UBSan used ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 and
UBSAN_OPTIONS=halt_on_error=1, strict C11 flags plus address/undefined sanitizers,
frame pointers and non-PIE instrumentation. No LSan pass is claimed.
T263 is archived; only narrative Markdown is changed after the accepted gate.
Final generation is `/tmp/ama-t263-documentation-delivery.log`, with unchanged
compiler/registry/header/test fingerprints and exact final staged agreement
required before commit. No test job remains active. T262 is next.

## T262 selection and explicit-edge contract

Status: accepted after T263 `9617370`; worktree clean at entry. Authority remains
[PKG-05](../../spec/project/packages.md#pkg-05),
[PKG-07](../../spec/project/packages.md#pkg-07),
[PKG-08](../../spec/project/packages.md#pkg-08) and
[PKG-09](../../spec/project/packages.md#pkg-09). No spec change is authorized.

1. Build one shared immutable finite view of actual prepared ordinary versions
   and module identities. Validate complete canonical endpoint kinds, uniqueness
   and the presence of each module's real version record. Borrow stable normalized
   ModulePathValue owners; copy the caller's pointer array into view-owned sorted
   records. A version record remains explicit even when its areas are empty.
   The source/catalog consumer supplies actual availability after PKG-02 selection;
   this pure component never infers a version from a guessed probe module or
   reads a filesystem. T232 owns the lasting compiler/catalog adapter.
2. For each declaring version, evaluate its compiled selectors against every
   actual view record using T263. Retain a unique selected-root entry per matching
   identity, not per matching decomposition or selector. A sparse arena-owned
   root list borrows the shared view; it does not copy the universe into every
   package. Compiled patterns and aliases need survive construction only.
   Refusal retains partial arena storage and leaves the output unpublished.
3. Coverage is the union of selected roots' units. A selected module covers its
   exact main identity and complete logical-component subtree. A selected source
   module additionally covers proper descendants of its mirrored tests directory;
   a test file equal to the directory spelling is not inside that directory.
   A selected whole version covers all its source identities and test directories
   associated with actual source modules in the shared view. It does not invent
   orphan experiments or activate unrelated topology-only ancestor units.
   All prefix comparisons require complete separator boundaries. The validated
   source layout makes logical module descendants the SRC-05 structural subtree.
4. Check only explicitly authored ordinary-definition edges. Verify that the
   origin belongs to the declaring selection's exact version. Apply src-to-tests
   refusal before same-version privilege. Same-version references and standard
   targets need no selector; other ordinary targets require unit coverage.
   Type substitutions are checked at their originating explicit definition;
   receiving a caller-supplied Type is not a second authored dependency. T232
   must preserve that provenance at real frontend handoffs. Availability and
   permission remain separate: an allowed identity can still be missing and
   must fail normal module lookup. Standard-origin policy is not broadened by
   this ordinary-version component.

All operations traverse finite views, selector lists and component sequences.
View construction uses exact byte sorting/duplicate checks and actual version
membership. Selection is a finite union over N view records and S selector
patterns, using T263's proven matcher. Source/test coverage uses explicit version,
area and whole-component boundaries; no C-expression inspection, likely-use
estimate, filesystem fallback or recursion-depth limit is involved. A context
ancestor never becomes a selected root solely because its file is present.
SourceSpan origins and normalized metadata lifetimes remain explicit.

Acceptance will compare complete bounded selected sets and coverage decisions
against independently enumerated structural-unit/test relations; include mask
order/duplicate decompositions, empty/missing wildcard members, namespace/version
boundaries, source versus test origins, topology-only ancestors, absent modules,
Unicode names and every measured allocation refusal. Include a real prepared
package-selector/catalog handoff where practical, while retaining T232's pipeline
integration responsibility. Register code/header/build/test inputs from creation.

Checkpoint: contract and ordering recorded; no T262 implementation or acceptance
exists yet, and no job is active. Next implement the shared view, sparse selected
root union, exact unit/test coverage and explicit-edge decision boundary, then
component/composition models, ownership/failure tests and required documentation.

T262 design correction during source-layout review: the initial logical-prefix
association was insufficient. SRC-05 erases structural plus suffixes, so
`tests/a/case` can physically mean `tests/a/case` or `tests/a+/case`.
Only the former is inside the PKG-08 associated directory of source `src/a`.
This is specified behavior, not an open semantic question or permission to guess.

The shared view therefore also receives verified physical provenance: for each
test module, the number t of leading ordinary directory components after tests/,
stopping before the first structural plus directory or the main filename.
For a source suffix of k components, association requires the exact logical
suffix prefix, a proper descendant, and k <= t. This statistic is sufficient:
logical comparison proves component-name equality and k <= t proves all required
associated-directory ancestors were ordinary. Later physical components can have
plus suffixes because the complete associated test directory is retained.
Non-test records require zero; the numeric bound excludes the main filename.
The caller derives t from verified physical layout, never from logical spelling.
T232 must preserve this proof at its catalog adapter; actual-layout compositions
will test this boundary. Coverage is queried only for actual view members, so
no physical provenance is invented for missing targets. Same-version/standard
edge privilege still does not prove normal module availability.

Checkpoint: provisional T262 implementation exists under
src/source/configuration/selection/ (view, ordering/validation, sparse union,
association, coverage and explicit-edge decision). It is not accepted or tested;
no test job is active. Next add independent set/physical-association models,
resource refusals and real prepared-layout composition, register every new input,
then compile/test these components before any integration or completion claim.

T262 step 1 component evidence: the new selection leaf compiled and passed
(`/tmp/ama-t262-selection-model.log`, exit 0). Its independent finite relation
covers all 64 subsets of six selector entries against 24 prepared identities,
including duplicate/reversed patterns, absent wildcard members, version variants,
selected-child versus topology ancestor, orphan tests and physical plus-subtree
counterexamples. Policy vectors apply src-to-tests refusal before internal
privilege, preserve ordinary test origins, distinguish foreign declaring owners,
and admit standard targets without pretending they exist. Every measured sparse
union allocation refusal is cleaned and leaves output unchanged; view metadata
checks cover duplicates, absent version records, invalid physical-prefix counts
and preflight SIZE_MAX refusal. New source/header/test/build inputs are registered
with explicit cases. These are component witnesses, not final acceptance.

Current checkpoint: T262 active; no process or test job remains active. Production
files are under src/source/configuration/selection/, tests under the matching
unit leaf. Next strengthen root-identity assertions and empty/malformed metadata
partitions, add real package/catalog physical-layout handoffs and Unicode/order
cases, verify ownership lifetime and all remaining refusal paths; then run scoped
header/contracts, full source/root, ASan/UBSan and generated documentation before
archival/commit. T232 remains actual compiler integration owner. Nothing in spec/
was edited. Next unused task ID remains T264.

T262 step 2: a separate no-I/O provenance projector now validates each verified
area-relative physical module path against its normalized logical suffix,
stripping one plus only from directory ancestors. It computes the sufficient
ordinary-prefix statistic instead of expecting adapters to guess it from names.
The unit model now composes that projector into view construction and checks
all projected counts against explicit physical fixtures. Exact selected-root
identity sets are asserted in addition to counts and coverage. Malformed physical
paths preserve outputs and refuse. `/tmp/ama-t262-provenance-model.log` exited zero.

Checkpoint update: no job remains active. New projector is registered, and all
current T262 unit/model cases pass. Remaining work is real filesystem/catalog
composition, further empty/order/Unicode/lifetime failure partitions, scoped
header/comment checks, root/source/sanitizer acceptance and generated documentation.
T262 remains incomplete and uncommitted. Spec files are unchanged; T232 remains
pipeline owner, with the physical provenance contract now explicit.

T262 step 3: real package selection and module indexing now feed physical source
paths through the provenance projector into the shared view. The fixture includes
valid ordinary test directories, structural test plus directories and an ignored
external version with a locally present replacement. Expected coverage is checked
after catalog destruction because normalized metadata owns its copied bytes.
The real handoff passed (`/tmp/ama-t262-real-catalog.log`). Unicode/multiword view
records, reversed input order, caller-array retirement, compiled-pattern owner
retirement, empty views and invalid declaring versions are additionally covered.
Complete current selection leaf passed fail-fast ASan/UBSan with leak detection
disabled (`/tmp/ama-t262-selection-asan-ubsan.log`). Scoped canonical contract,
strict standalone-header and actual included case-signature checks passed.

Complexity: view insertion sorting is O(N² K), with O(N log(N+1) K) version
membership checks for N records and compared byte width K. Selection evaluates
at most N times S finite matcher decisions and allocates O(R) sparse root nodes
for R matched identities, borrowing O(N) shared view storage. A query performs
one binary lookup and at most R unit-prefix checks. For a selected whole-version
test query it scans N actual source records at most once: only one unique version
root can equal that candidate's version. Thus query time is bounded by
O((R+N+log(N+1)) K), with O(1) query scratch. The provenance projection consumes
each physical/logical component once and uses O(1) scratch. Version comparisons,
proper test-directory descent and the ordinary-prefix count jointly preserve
association; no logical string prefix substitutes for erased physical topology.

Checkpoint: selection ASan/UBSan and scoped validators collected with exit zero;
no active job. Next current root/source composition and full documentation gate,
then archival-only Markdown, final manifest/staged agreement and task commit.
No specification file has changed. T232 still owns complete pipeline integration.

T262 acceptance: root `make` and complete source parent both exited zero
(`/tmp/ama-t262-compiler-build.log`, `/tmp/ama-t262-source-parent.log`). Complete
`make docs` passed (`/tmp/ama-t262-documentation.log`): 3216 ready inputs, zero
debts, 2420 file pages, 9339 interfaces and 488/488 canonical destinations.
Current selection leaf ASan/UBSan used fail-fast options with leak detection
disabled, strict C11 plus address/undefined sanitizers, frame pointers and non-PIE
instrumentation. It exited zero; no LSan pass is claimed. All new maintained
inputs have explicit named cases and standalone public/private header commands.
Only archival/current-status Markdown changes follow the accepted snapshot.
Final generation is `/tmp/ama-t262-documentation-delivery.log`; require unchanged
accepted compiler/registry/header/test inputs and final manifest/staged agreement.
T262 is archived; no test job remains active. T232 integration is next.

## T232 integration checkpoint

T262 committed as `6348092`; repository was clean before integration review.
Read-only tracing of compiler/project loading and canonical SRC-01/SRC-06,
PKG-03/04/05/09 exposed [AMB-016](../../AMBIGUITIES.md#amb-016): an explicit
ordinary path in a standard module has no specified selector/configuration owner.
Standard modules have no package version, and local companions cannot declare
selectors/repositories. Neither a silent exemption nor borrowing the consumer's
package scope follows from current requirements. The ordinary-version-only T262
policy must not become an implicit answer for this different origin category.

T232 is blocked before implementation. Only TODO, ambiguity registry and this
checkpoint changed; no product code or spec changed. No job is active. User
clarification is required before choosing behavior. Resume by recording the
answer and its source, obtaining specific spec authorization if the answer does
not itself authorize the necessary amendment, then aligning canonical rules
before implementation. Do not switch to another TODO task to bypass AMB-016.

T232 AMB-016 decision checkpoint: the user explicitly selected option 1 in the
clarification reply: forbid authored standard-to-ordinary dependencies and permit
ordinary types through template parameters. The registry records the exact answer
and source. This resolves behavior, but the reply does not explicitly authorize
editing spec/. AGENTS.md requires that separate authorization before the canonical
amendment and implementation. T232 remains blocked pending that authorization;
no product/spec code changed, no job is active, and no other task is substituted.

## T264 authorized standard-isolation amendment

The user explicitly authorized the requested canonical amendment with “да, вноси
и продолжай делать задачи по плану”. AMB-016 records this source separately from
the preceding behavioral choice. T264 adds the normative restriction once in
PKG-05, with motivation and forbidden/allowed standard-interface fragments.
The grammar links this semantic constraint without changing path productions;
the acceptance table adds direct/alias/static/native and provenance witnesses.
Unused local aliases remain nonimports, supplied ordinary Types/Templates remain
usable, and C system-library/build inputs retain their existing contracts.
No new standard package configuration or caller-scope borrowing is introduced.

Consistency review checked SRC-01/SRC-06 and PKG-03/04/05/09: the standard root
still has no package wrapper, relative paths retain their namespace, ordinary
alias targets remain legal declarations without imports, actual use is checked
at the lexical definition, and test substitutions remain authored by their caller.
T264 changes specification/narrative inputs only; no compiler acceptance claim
or conformance fingerprint refresh follows. Next run supported generated-reference
validation, then archive/commit T264 and resume T232 implementation.

T264 acceptance: supported documentation builder exited zero, including 488/488
canonical destinations. Only archived/current-status narrative Markdown changes
follow this snapshot; canonical spec and compiler/registry/header/test inputs
remain unchanged. Final delivery is `/tmp/ama-t264-documentation-delivery.log`.
Verify its complete input manifest against final staged bytes before committing.
AMB-016 is resolved and T232 may resume after this specification commit. No
compiler implementation, parser/runtime test acceptance or conformance renewal
is claimed by T264. No test job remains active.

## T265 standard-origin policy contract

Status: accepted after T264 `55c13d5`, clean worktree at entry. Canonical authority
is the amended PKG-05, with SRC-01 endpoint validation and SRC-06 normalization.
Add a separate dependency_standard_edge_check API that takes normalized defining
and target module values, with no package selection parameter. This preserves
T262's ordinary-definition API and makes caller selector borrowing impossible
at the standard boundary. Return a distinct standard-to-ordinary refusal after
validating complete endpoints; standard-to-standard is allowed. Invalid/null or
wrong-origin metadata refuses rather than selecting another owner's policy.
No allocation, mutation, lookup or I/O occurs. Endpoint validation is finite
O(source bytes + target bytes) with O(1) scratch. Namespace kinds partition the
entire valid domain, independently of component words such as src/tests.

Tests will exercise positive/negative/invalid root partitions, Unicode names,
ordinary src/tests targets, normalized local-alias targets, original input
preservation and a fail-next-allocation control. Caller Type substitution is
not submitted as an authored standard edge; real provenance handoffs remain
T232's responsibility. No runtime or frontend support claim follows this helper.
Checkpoint: task and contract recorded; next implement policy and real component
composition tests before required acceptance and a separate commit.

T265 step evidence: standard.c exposes a separate standard-origin decision
without accepting a package table; the ordinary edge API remains ordinary-only.
A distinct refusal distinguishes forbidden standard-to-ordinary references from
invalid metadata. Component/alias-composition leaf passed
(`/tmp/ama-t265-standard-policy.log`), as did the complete source parent
(`/tmp/ama-t265-source-parent.log`). Cases cover Unicode/multiword standard names,
ordinary src/tests targets, misleading standard component words, null/incomplete
endpoints, wrong-origin calls, unchanged inputs and a fail-next-allocation
control. Local aliases are legally prepared without importing; actual normalized
uses carry their original span and choose policy by the final namespace.
The complete current selection leaf passed fail-fast ASan/UBSan with leak
checking disabled (`/tmp/ama-t265-standard-asan-ubsan.log`). Strict standalone
public-header, canonical-contract and actual case-signature checks passed.

Checkpoint: scoped tests and sanitizer job collected with exit zero. Root build
is next/being collected, followed by the full documentation gate while T265 is
still TODO. After acceptance archive, regenerate final source manifest, inspect
staged bytes and commit. T232 remains real frontend/loader provenance owner;
these component tests are not a claim that source compilation yet invokes the API.

T265 acceptance: root make exited zero (`/tmp/ama-t265-compiler-build.log`). Full
make docs exited zero (`/tmp/ama-t265-documentation.log`): 3218 ready inputs,
zero debts, 2422 file pages, 9343 interfaces and 488/488 canonical destinations.
Sanitizer invocation used ASAN_OPTIONS=detect_leaks=0:halt_on_error=1,
UBSAN_OPTIONS=halt_on_error=1 and strict C11 flags plus address/undefined
sanitizers, frame pointers and non-PIE instrumentation. Only current/archived
Markdown changes follow the accepted implementation snapshot. Final generation
is `/tmp/ama-t265-documentation-delivery.log`; require accepted code/registry/
header/test byte identity and final manifest/staged agreement before commit.
T265 is archived; T232 integration remains the next work. No test job is active.

## T266 retained configuration ownership

The real catalog currently checks package.ini shape but discards its bytes and
selected-version context. Source snapshots similarly retain only Ama text and
native companions. Before alias preparation, retain each selected version's exact
configuration SourceText once and connect indexed modules to that owner. This is
an independently accepted T232 prerequisite, not a new language requirement.
The record must own its logical/physical names independently of borrowed root
descriptors. Standard modules retain no package context. Empty versions still
retain their configuration, so permission/view construction does not invent
probe modules. Ignored external versions contribute neither records nor bytes.

A source-only snapshot attachment must copy configuration bytes, validate the
complete version identity, refuse duplicate owners and connect only matching
ordinary module identities. It performs no filesystem lookup and publishes no
partial module/context connections on allocation or encoding failure. Configuration
syntax remains the strict document parser's responsibility; storage preserves
exact bytes and original physical locations. The finite version/module scans
use exact canonical version boundaries, not textual prefix guesses.

Acceptance for this step: actual selected filesystem copies, empty versions,
standard isolation, detached caller/root storage, changed-on-disk inputs, exact
UTF-8 diagnostics and fail-at-every-allocation checks; copied source snapshots
with explicit configuration attachment and no physical reread. T232 remains
uncompleted until scope preparation, dependency/semantic handoffs and input-graph
capture consume these records. No jobs are active at this checkpoint.

T266 implementation: catalog-owned ModuleVersion records preserve configuration
SourceText, complete logical version and physical directory. Filesystem indexing
loads only the already selected package configuration and links each source to
that version. Explicit snapshot attachment copies bytes/paths before publication,
validates UTF-8 with owned original diagnostics and reconnects only exact version
identities. No root pointer survives build. Source-only snapshots without attached
configuration remain explicitly incomplete for future T232 scope preparation;
this API does not invent an empty package configuration. No INI interpretation,
implicit download, local companion discovery or permission exemption is introduced.

Version lookup validates the finite canonical path once and compares exact
version lengths/bytes against the retained list. Publication validates a complete
version, refuses duplicate ownership, allocates/copies all owner fields, then
performs one finite source scan. Every matching source receives the same owner;
standard/different-version sources remain unchanged. With V versions and M
modules, publication uses O(V + M) bounded identity comparisons plus examined
path bytes, and one record/two names beyond retained configuration text. Failure
before publication can leave only catalog-owned scratch, never changed module
links. Catalog destruction releases records/text after all borrowing consumers.

The real project-input capture links each active module to its version's shared
CONFIGURATION node keyed by the original physical package.ini path. It borrows
retained bytes from the Project, matching existing source/ABI lifetime contracts;
FILE and CONFIGURATION key domains remain distinct. Disk changes after acquisition
cannot alter the captured graph. A subsequent project acquisition observes changed
configuration and invalidates all dependent modules through the existing exact
fixed-point algorithm. Inactive/ignored versions do not gain consumer edges.

Scoped catalog and project-input runs passed with explicit assertion cases. Next:
register per-file evidence, run parent/source and project closure composition,
fail-fast ASan/UBSan, root build and documentation gates before archival/commit.
No other task is marked complete.

T266 checks collected with exit zero: catalog leaf
(`/tmp/ama-t232-config-storage.log`), real project-input capture
(`/tmp/ama-t232-config-inputs.log`), complete source parent
(`/tmp/ama-t266-source-parent.log`) and real project closure
(`/tmp/ama-t266-project-closure.log`). Scoped canonical-comment, standalone-header
and compiled actual-case-signature checks also passed. Allocation tests cover
every measured configuration copy/publication failure with unchanged module links
and balanced owner teardown; the existing full catalog/capture failure sweeps
include the new filesystem and graph allocations.

Active acceptance jobs: catalog ASan/UBSan session 10911
(`/tmp/ama-t266-catalog-sanitizers.log`), project-input ASan/UBSan session 57200
(`/tmp/ama-t266-inputs-sanitizers.log`), root make session 90358
(`/tmp/ama-t266-build.log`). Both sanitizers use detect_leaks=0:halt_on_error=1,
UBSAN halt_on_error=1, strict C11, frame pointers and non-PIE instrumentation;
no LeakSanitizer claim is made. No shared output trees overlap these jobs.
Next collect all exits before running the full documentation gate.

T266 sanitizer jobs 10911 and 57200 completed with exit zero. Root build 90358
is still being collected. Full documentation acceptance is next; implementation,
registry and fixture bytes remain frozen during that run.

T266 acceptance collected: all sanitizer, source/catalog, project-input and closure
jobs exited zero, as did root make. Full make docs exited zero
(`/tmp/ama-t266-documentation.log`), with 3221 ready inputs and zero debts.
The accepted manifest is preserved at `/tmp/ama-t266-prearchive-manifest.json`.
Only narrative task archival/current-contract Markdown changes follow; implementation,
registry, headers and tests remain byte-identical. Final supported Doxygen delivery
will be `/tmp/ama-t266-documentation-delivery.log`; verify final manifest and staged
bytes before committing. T266 is archived; T232 remains open and is next.
No test job remains active.

## T267 module configuration activation

T266 committed as 96e8413. T232's next real integration step retains the optional
module alias companion when a required definition activates. The physical source
filename plus .ini is authoritative under PKG-03; physical submodules inherit no
local alias file under PKG-04. Reuse one exact optional-companion reader for native
and configuration activation. Reader output contains the anticipated owned path,
presence and optional immutable UTF-8 SourceText; it publishes only after complete
success, including observed absence. Module activation caches successful results
and leaves fields unchanged on failure. An unused module's companion is not read.

Extend authoritative ModuleSnapshotEntry with an explicit optional configuration
source: NULL means captured absence, not permission to probe the filesystem.
Snapshot construction deep-copies present text/locations and seals absent/present
metadata. Existing source-only snapshot callers must explicitly supply absence;
relative/configuration-aware relocation transport remains T224/T232's pending
handoff, without a claim that these tools already support local aliases.

Input capture uses a configuration node keyed by the original companion path. Its
payload is one presence byte followed by exact original bytes when present. Thus
absent and present-empty configurations differ without timestamp/hash assumptions.
Only active modules receive edges. No source manager reread or INI interpretation
is performed during capture. Every allocation/UTF-8 failure remains managed,
owned and unpublished. Next implement, run actual activation/capture compositions
and measured failure checks, then documentation acceptance before a task commit.

T267 implementation checkpoint: shared exact companion probing now reports an
owned anticipated path and explicit presence, with unchanged native missing-file
semantics. Required-module parsing captures the local alias file before dependency
discovery; valid unused definitions do not activate their companions. Snapshot
entries explicitly include the optional captured source; every existing caller
now supplies NULL for its current source-only domain. Snapshot construction seals
absence or copies present text/locations without filesystem fallback. The pending
configuration-aware relocation work must transport actual local companions; the
current explicit NULL call sites do not establish that feature.

Real input capture adds the tagged local-configuration node and links its actual
module consumer. Temporary encoding storage is released on all outcomes; original
source remains Project-owned. The encoding is injective: its first byte determines
presence, and present observations retain the entire remaining byte sequence.
Absent encoding has length one and tag zero, so it cannot equal present-empty.
Capture performs O(B) byte work/scratch plus graph admission; exact filesystem
activation scans catalog membership once, probes one finite path and loads at most
one companion, with all partial allocations retained under catalog destruction.
Snapshot source copying completes before record publication, so encoding/allocation
refusal leaves no newly visible source. Successful repeat activation allocates
nothing and never reopens the file.

Passed: catalog initial/scoped runs, local graph activation/invalidation tests
(`/tmp/ama-t267-inputs.log`), source parent (`/tmp/ama-t267-source-parent.log`),
project closure (`/tmp/ama-t267-project-closure.log`) and strict scoped canonical
comment/header/actual-case-signature checks. New cases cover parent/submodule
isolation, explicit absent/present snapshot ownership, copied original failure
locations, directory/symlink refusal, creation of a physically empty file, captured
disk changes, removal, invalid unused/used companions and all measured physical
load/snapshot-copy failures. Existing full closure/capture failure sweeps exercise
the actual new activation and invalidation allocations. Next: real captured
compiler/tool compositions, sanitizers, root build and full documentation gate.

T267 real captured compiler and relocation reference/staging compositions passed
with exit zero: `/tmp/ama-t267-captured-compiler.log`,
`/tmp/ama-t267-relocation-references.log`,
`/tmp/ama-t267-relocation-staging.log`. These verify the changed snapshot ownership
boundary and existing absolute-path tool behavior, not future local-alias rewriting.
Next run fail-fast catalog/input sanitizers and root make, collect all statuses,
then freeze code/registry/tests for full documentation acceptance.

Active T267 acceptance jobs: catalog ASan/UBSan session 72235
(`/tmp/ama-t267-catalog-sanitizers.log`), input capture ASan/UBSan session 86253
(`/tmp/ama-t267-inputs-sanitizers.log`), root make session 4750
(`/tmp/ama-t267-build.log`). Sanitizers use strict C11, address/undefined checks,
frame pointers, non-PIE and halt_on_error=1 with detect_leaks=0. Counted allocator
zero-balance checks are separate evidence; no LeakSanitizer claim is made.
Jobs own separate build trees. No completion is inferred before collecting exits.

T267 sanitizer sessions 72235 and 86253 and root make 4750 collected with exit
zero. No sanitizer warning/error occurred; all explicit assertions remained enabled.
Root build, original/captured catalog, source parent, project closure, input capture
and captured compiler/tool compositions have passed. Final scoped comment/header
checks passed after comment review. Next full make docs freezes implementation,
registry, headers and fixtures; archive only after that required gate succeeds.

T267 full make docs passed (`/tmp/ama-t267-documentation.log`): 3227 ready inputs,
zero debts, 2431 maintained file pages, 9373 interfaces and 488/488 canonical
destinations. The accepted manifest is preserved at
`/tmp/ama-t267-prearchive-manifest.json`. T267 is archived; T232 remains open.
Only narrative archival/current-status Markdown changes follow this accepted code
snapshot. Final supported Doxygen generation is
`/tmp/ama-t267-documentation-delivery.log`; require unchanged accepted code,
registry, header and test hashes plus final manifest/staged agreement before
committing. No test job is active.

## T232 active compiler scope integration

After T267 2017df2, resume T232 itself rather than introducing another independent
source-only foundation. Prepare actual required-module alias scopes from retained
version/module inputs, cache one package scope per exact retained version owner,
then normalize written dependency operands in that defining scope before registry
lookup. Keep original occurrence spans and never substitute canonical text into
AST sources. Standard scopes have no package parent and authored standard-to-ordinary
edges use T265. Ordinary snapshot modules must supply explicit version configuration;
missing context is an error, not an invented empty file or caller-scope fallback.

Steps remaining in this task: transport package/local captured configuration
through compiler/tool snapshot consumers; prepare exact shared selector views and
ordinary edge checks; normalize full qualified constant operands before splitting
member identity; align all semantic/static consumers with resolved occurrence
metadata; exercise actual generated ordinary/static/native/constant compositions,
all failure points, sanitizers and documentation before marking T232 complete.
The first scope/normalization step may temporarily expose old fixture/snapshot
assumptions; migrate those inputs to the specified explicit context rather than
relaxing rules. No completion or commit is authorized for an unfinished T232.

T232 continuation checkpoint — AMB-017 decision and AMB-018

Actual required-module scope preparation, package-scope sharing, source-preserving
relative/alias path admission, standard isolation, shared actual availability views
and ordinary selector/src-test checks are implemented in the working tree.
Scoped loader/policy/isolation assertions and every measured real alias-loading
allocation failure passed (`/tmp/ama-t232-scopes.log`,
`/tmp/ama-t232-scope-policy.log`, `/tmp/ama-t232-scope-isolation.log`). Captured
compiler contexts passed after explicit retained version attachment and linking
the version capture source (`/tmp/ama-t232-captured-scopes-rerun.log`); the initial
run failed to link the newly consumed version-capture function, then the explicit
project source list was corrected. Version capture now supports preparation as
well as complete source snapshots; its only current API is module_catalog_capture_version.
Catalog ownership checks passed (`/tmp/ama-t232-captured-version.log`).

No T232 task completion, sanitizer/full-doc gate or commit is claimed. Tool
reference/staging runs failed assertions in older cross-package fixtures that omit
initial dependency declarations. During their migration review, AMB-017 exposed a
separate final-state requirement gap: moving a formerly local dependency across
versions creates a required external permission that no original explicit selector
represented. The then-current TOOL-02's exact selected-set preservation did not
state whether new compensating selectors could be added. The user accepted separate copy/delete/
move commands, opt-in dependency clones or package.ini extension and safe replacement,
with explicit authorization for the T268 canonical amendment. Read-only consistency
review isolated AMB-018 and the user accepted original src-relative suffixes for
additional dependencies. T268 updates the canonical source-operation rules and
acceptance; no clarification remains pending. Do not reinterpret the initial
invalid fixture failures as evidence for the valid minimal example. No code edit
implements the new source-operation policy. All T232 jobs have completed and no
T232 job is active.

Remaining work after T268: finish exact package/local configuration transport
through tool snapshots,
normalize qualified constants before member splitting, update static/semantic
occurrence consumers, migrate genuinely obsolete fixture permissions, register
all new compiler granules and run component/composition/failure/sanitizer/build/
documentation gates. T232's working changes stay uncommitted until accepted;
T224 owns full current-policy source-operation implementation after closure
foundations; no old importer-rewrite behavior is accepted by these design decisions.

T232 qualified-constant integration step (active)

Purpose: a qualified value source is one raw path expression, not a parser-owned
module/member pair. CONST-05 and SRC-06 require the complete defining-scope
normal form first; a whole alias can supply its final member, and reductions
can expose a component inherited from a canonical containing directory.

The AST now retains qualified_path without the optional list suffix. Discovery
marks that exact occurrence PROJECT_CONSTANT_PATH; normalization selects a
ModuleConstantPath in the project arena before applying the same module endpoint
and defining-package policy as other dependencies. The edge retains its original
full operand span, normalized namespace and copied member bytes/provenance.
Constant namespace source-kind checks still require an ordinary definition.
Static collection binds the exact explicit edge, and selection compares complete
normalized member bytes against own declarations. Ordinary semantic checking
consumes the already selected declaration proof rather than splitting the raw
text again. No generated SourceText/SourceSpan or receiver-based namespace
choice is introduced. The optional member origin is genuine or absent.

Invariant: for every admitted qualified occurrence, its namespace is exactly the
normal form without its last component and its member is exactly that final
component. The pure path component establishes terminating stack reduction and
one final split; the project layer only carries that result through ownership and
permission checks. All namespace/member bytes are project-owned, source origins
borrow catalog/config owners, and every failed partial allocation is released by
the project destructor. Static/runtime consumers select the same own declaration
identity. Editor indexing records a member token only where that actual token is
written inside the operand; an alias key or inherited component is not fabricated
into a variable-name occurrence. Rename validation must still prove the complete
candidate graph and cannot silently leave a broken alias consumer.

Planned checks: whole aliases including list storage, raw relative paths with
cancelled nonexistent components, changed final members, inherited members with
no source origin, invalid complete prefixes, src-to-tests refusal, and every
measured descriptor/member allocation failure. Compose static collection/member
selection, actual closed runtime namespaces and editor consumers before acceptance.
The source-operation implementation remains T224; these checks do not accept
superseded external-importer rewriting.

Observed before adding the new qualified scenarios: parser sources passed
(/tmp/ama-t232-qualified-parser.log), and the existing project input leaf passed
(/tmp/ama-t232-qualified-inputs-initial.log). Current new-scenario jobs are
project inputs session 69406 (/tmp/ama-t232-qualified-inputs.log), parser sources
session 52214 (/tmp/ama-t232-qualified-parser-rerun.log), and static constant
catalog session 66114 (/tmp/ama-t232-qualified-inventory.log). Their exits must be
collected; no T232 completion, commit or documentation acceptance is claimed.

Collected qualified-step results and corrections

The initial qualified parser, input graph and static catalog jobs all exited
zero. Static member selection and the ordinary closed-namespace leaf also
passed (/tmp/ama-t232-qualified-selection.log and
/tmp/ama-t232-qualified-runtime.log). The full static-constant parent passed
(/tmp/ama-t232-qualified-constants-parent.log). Source-only captured contexts
passed both retained/absent local configuration and a standard template receiving
an ordinary Type through its actual call arguments
(/tmp/ama-t232-captured-standard-template.log). Explicit authored standard-to-
ordinary type and qualified-constant paths both refuse at the original operand
(/tmp/ama-t232-qualified-standard-boundaries.log).

The expanded actual-catalog test covers an external src unit selector, its
mirrored test program and structural check+/nested helper, while refusing an
unrelated test directory (/tmp/ama-t232-test-ancestry.log). Whole-operand member
selection, inherited origin absence and every measured descriptor allocation
failure pass (/tmp/ama-t232-qualified-inputs-complete.log).

Observed failures were retained and corrected, not reclassified as passes:

- Tree-parser rejection fixtures treated a bare path as already split. The
  grammar parses one raw module-path operand, so incomplete normalized constant
  namespaces are now rejected by the project boundary. The tree rerun passed
  (/tmp/ama-t232-qualified-tree-rerun.log); malformed path syntax remains rejected.
- The new namespace-alias test initially lacked the POSIX unlink declaration;
  its explicit include fixed compilation and the full leaf passed
  (/tmp/ama-t232-qualified-runtime-alias-rerun.log).
- A full raw module-path index occurrence initially covered its member token.
  The index now ends that namespace occurrence before the genuine normalized
  source-local member; aliases supply no invented token. The actual navigation
  and alias case passed (/tmp/ama-t232-qualified-editor-alias-rerun.log).
- Alias-owned optional member provenance is not an edited module-source span.
  A newly added excessive identity comparison refused an otherwise unchanged
  constant binding. Written operands, selected declaration identity and indexed
  source occurrences provide the required proof; the foreign-origin comparison
  was removed. Unrelated-field rename with a retained constant alias and the
  complete validation leaf pass (/tmp/ama-t232-qualified-rename-alias-rerun.log).
- The generated-program fixture first used a whole list where each declaration
  initializer supplies an element. It now declares the list then copies the
  constant list by assignment. Verified C compilation/execution passed with
  exact output length=7, length=3, length=0, proving the aliased payload copy and
  existing integer/list cleanup (/tmp/ama-t232-qualified-generated-rerun.log).
- The project parent exposed an old required-draft diagnostic expectation that
  omitted the member from its raw qualified operand. The exact original full
  operand is now expected; status and requester semantics are unchanged.

Ready coverage now explicitly registers the new scope/normalization/admission
and qualified-binding granules, their real leaf cases, the additional test
support files and both maintained integration inputs. This is coverage metadata;
full documentation acceptance is still pending. The current active jobs are
project parent rerun session 31429 (/tmp/ama-t232-project-parent-rerun.log) and
make docs-check session 77714 (/tmp/ama-t232-documentation-check.log). Collect
both exits before further implementation changes. No T232 completion or commit
is claimed; T224 still owns complete current-policy source operations.

T232 boundary after T268's source-operation redesign

The accepted plan puts complete source-operation commands/adapters in T224,
after T235's exact source/test closure. The former global importer-renaming
mover is superseded by explicit cloning plus safe deletion. T232 must not claim
its old graph-rewrite acceptance as evidence for that model or grow a partial
new source-operation implementation before its prerequisites.

The provisional uncommitted old-mover changes in discovery/package-areas.c,
references/domain.c and staging/validation/domain.c were therefore withdrawn;
they are not new retained code or accepted configuration adapters. Their exact
pre-withdrawal diff is /tmp/ama-t232-superseded-mover-adapters.patch for audit.
T224 owns migration/removal of the superseded operation orchestration, real
configuration transport and source-operation acceptance. The generic captured
compiler interface is tested here with exact deep-copied version/module scopes,
future physical paths, explicitly captured absence, qualified constant aliases
and a standard template receiving an ordinary Type. These are reusable current
read-only foundations with a real accepted consumer, not a source-operation
support claim.

The regular project parent passed from clean owned fixtures
(/tmp/ama-t232-project-parent-fresh.log). Earlier preserved-failure reruns had
stale generated absent-entry inputs; leaf clean resets those owned files before
retry, without changing the test's specified expectation. The genuine fixture
migration added the declaring application's exact external selector in the
multi-root case, retaining its draft-parent/nonimport property. The expanded
requester test now expects the whole original qualified operand.

Editor parent and complete frontend composition passed
(/tmp/ama-t232-editor-parent.log and /tmp/ama-t232-frontend-composition.log).
AddressSanitizer/UBSan project-input and closed-namespace leaves passed with
halt_on_error=1, detect_leaks=0, frame pointers and non-PIE flags
(/tmp/ama-t232-inputs-sanitizers.log and /tmp/ama-t232-namespaces-sanitizers.log).
Counted allocation teardown is independent of disabled LeakSanitizer; no LSan
pass is claimed. Actual verified C integration passed again
(/tmp/ama-t232-qualified-generated-final.log). make docs-check passed 3241 ready
inputs and zero debts before the final unused parser-expectation cleanup;
final documentation gates must include that cleanup and the withdrawn adapters.

## T232 final acceptance and continuation

T232 is accepted on 2026-11-21. Earlier checkpoints above are historical; no
T232 test or documentation job remains active. The current record is
[DONE T232](../../DONE.md#t232), and T221 is the next ready task.

Final `make docs` exited zero (session 35894,
`/tmp/ama-t232-documentation-confirmed.log`): 3241 ready inputs, zero explicit
debts, 2443 maintained file pages, 9414 documented interfaces and all 488
canonical destinations validated. This includes standalone headers, actual
registered test signatures, rule contracts and rendered links. Accepted source
fingerprints are retained in `/tmp/ama-t232-accepted-manifest.json`.
Final independent parser errors (session 69068) and source/module (70136)
checks exited zero; logs are `/tmp/ama-t232-parser-errors-final.log` and
`/tmp/ama-t232-source-module-final.log`. Verified generated C integration
(session 19143) exited zero at `/tmp/ama-t232-emission-accepted.log` after
withdrawal of the provisional superseded-mover adapters.

The sanitizer commands use `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`,
`UBSAN_OPTIONS=halt_on_error=1`, C flags `-O0 -g -Wall -Wextra -std=c11
-pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined
-fno-omit-frame-pointer -fno-pie`, and `LINKER_FLAGS=-no-pie`, against
`tests/unit/project/inputs` and `tests/unit/semantic/specializations/namespaces`.
Their exits and independent counted-allocation evidence are recorded above.
No full `make test`/conformance-fingerprint renewal or complete source-operation
acceptance is claimed: those remain T237 and T224 after their prerequisites.

Only narrative archival follows the accepted implementation. Regenerate via
`python3 tools/documentation/build.py --doxygen doxygen`, verify unchanged
accepted non-Markdown inputs and final staged documentation fingerprints, then
commit T232 independently before starting T221.

## Accepted acquisition-planning consumer

[T234](../../DONE.md#t234) reuses the exact selector/finite-view and parsed
configuration APIs described here. Its independent project-tool
[acquisition contract](acquisition-plan.md) owns requester-scoped repository
routing, contained physical mapping and coherent snapshot/version source
records. It adds no compiler-side acquisition, global repository table or
alternate wildcard semantics. T235/T236 retain delivery/transport ownership.
