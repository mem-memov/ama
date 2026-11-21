# Project configuration and path resolution

## Ownership and current task boundary

[T232](../../TODO.md#t232) owns shared configuration, normalization and the
compiler/tool integration. Its independently verifiable foundations are
[T260](../../DONE.md#t260) (strict INI documents),
[T261](../../TODO.md#t261) (paths and alias scopes) and
[T262](../../TODO.md#t262) (selector matching and edge authorization).
T260 implements the source-only INI parser. T261/T262 and compiler integration
remain planned; source parsing alone is not a claim of complete compiler support.
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
