# Project configuration and path resolution

## Ownership and current task boundary

[T232](../../TODO.md#t232) owns shared configuration, normalization and the
compiler/tool integration. Its independently verifiable foundations are
[T260](../../DONE.md#t260) (strict INI documents),
[T261](../../DONE.md#t261) (paths and alias scopes) and
[T262](../../TODO.md#t262) (selector matching and edge authorization).
T260 implements source-only INI parsing and T261 implements path normalization
and alias scopes. T262 and compiler integration remain planned; these foundations
alone are not a claim of complete compiler support.
The prerequisites are committed: T231 `036d3d1`, T222 `ddd055d`, and diagnostic
follow-up T259 `56e8708`. No specification amendment is requested or made.

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
