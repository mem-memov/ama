# Exact UTF-8, identifiers and entry spelling

Boundary: `src/source/unicode/` and `src/source/path/exact.h`. Implements SRC-03;
physical module mapping and whole-project alias checks belong to the path catalog.
Pinned input, license, digest and offline regeneration are in
[Unicode provenance](../../third_party/unicode/README.md).

Decoding accepts precisely ASCII and 2/3/4-byte sequences with continuation bytes,
minimal values 0x80/0x800/0x10000, maximum 0x10ffff and no surrogates. Lead bytes
partition possible widths. Folding six continuation bits reconstructs the scalar;
minimal-value checks prove shortest form. Failure changes no decoded outputs.
Whole-buffer validation advances by a positive width or returns the first failing
sequence's lead-byte offset, including truncation. NULL is allowed for empty views.
These are byte operations, independent of locale, normalization or wchar_t.

Unicode 17.0.0 XID properties are sorted disjoint closed ranges. Binary search
removes ranges that cannot contain the scalar, preserving membership equivalence
until success or an empty interval. Single-word `unicode_identifier` recognition decodes each scalar,
uses XID_Start or underscore first and XID_Continue or underscore thereafter.
It rejects empty words and reports the failing scalar's lead offset. Comparing
identities still uses original bytes: property membership never rewrites names.

The C generator unions exactly the pinned XID ranges into finite-domain bitmaps
and emits maximal increasing runs. It verifies the input edition. No sampled
acceptance or host library substitutes for these tables. A separate test parser
constructs expected membership from the vendored data and compares every code
point through 0x10ffff, including unassigned values and surrogates. Tests also
encode/decode every scalar, truncate every valid multibyte encoding, and exercise
malformed, overlong and out-of-range sequences and international names.

`path_exact_child` requires an already-verified parent, one entry name and its
consistent full path. It enumerates exact bytes before lstat. A name resolving
through filesystem folding without an exact entry is unsupported, never accepted
as the requested spelling. Symlinks are explicitly unsupported rather than
silently followed. All directory streams close on success/failure. Filesystem
mutation during compilation is outside the stable-input assumption; observed
I/O failures are reported. Root/prefix verification is the catalog's duty.

UTF-8 recognition costs O(bytes); property lookup O(log ranges) per scalar.
Generation/test bitmaps use O(0x110000) storage; runtime tables are immutable.
Entry lookup uses O(directory entries) time and constant auxiliary storage.

Evidence: `make -C tests/unit/source/unicode` passed with C11/pedantic/Werror;
regeneration and `cmp` reproduced the committed table byte-for-byte. The suite
checks actual Unicode entry lookup, absence and symlink refusal. Whole-project
resolution is a separate boundary; no compiler frontend is claimed here.


T145 documents the decoder, range-search and offline-generator contracts in
source. The [Unicode test leaf](../../tests/unit/source/unicode/main.c) now compiles
and executes the maintained generator, compares its complete output byte-for-byte
with the committed include, rejects wrong editions and reversed/out-of-domain
ranges, and checks a short output through a readonly stdout descriptor. The latter
failed before the generator checked its final `fflush`; it now reports that IO
failure correctly instead of letting process-exit buffering hide it. This changes
no scalar/property membership: every generated range declaration is unchanged.
The generated include's purpose/rule comment is emitted by the generator rather
than manually inserted into its property ranges. No host Unicode tables or
network access participate in generation or tests.

## T015: complete-name recognition and pinned input validation

`unicode_name` validates one already-delimited complete name. It composes the
single-word predicate using exactly one ASCII space as separator. Every word
must be nonempty and obey its own XID_Start/underscore rule. Leading/trailing
spaces, repeated spaces, TAB, non-ASCII whitespace, digits or combining marks
starting a later word all fail. For example, `item step2`, `магазин товар данные`
and `中国 变量` are valid; `item 2`, `item  value` and `item ` are invalid.
The existing single-word API continues to reject spaces.

The caller supplies a bounded original byte view and writable error-offset output.
Success leaves that output unchanged. Failure reports a zero-based original-byte
position: the bad scalar/word start, second repeated space or trailing space itself.
Malformed UTF-8 reports the bad sequence's lead byte. Empty/NULL input reports
zero. Inputs are never changed, copied into normalized text or retained. Composed
and decomposed spellings, case variants and internal single spaces remain exact
identity bytes. This API validates a span; it does not guess where a token ends,
perform prefix name lookup or interpret path roots. T016 and T022 wire the exact
path/token consumers separately.

ASCII space cannot be a UTF-8 continuation byte. Splitting at that byte therefore
cannot split a well-formed scalar, while a malformed partial scalar still fails
inside its selected word. The loop invariant keeps the next word offset within
the original view. A completed word either reaches EOF or consumes exactly one
nonterminal separator; the offset strictly increases. Empty next words fail
before advancing. Error rebasing stays bounded by the original length. Work is
O(B + S log R) for B bytes, S scalars and R pinned ranges, with O(1) scratch and
no allocation, locale query, system Unicode lookup or byte rewrite.

T015's source checksum case hashes every original byte of the vendored
DerivedCoreProperties input against the independently recorded SHA-256 in
[provenance](../../third_party/unicode/README.md), and checks the public edition
constant is exactly 17.0.0. Existing independent property parsing compares both
XID properties against every code point through U+10FFFF, and offline generation
still reproduces the entire maintained table byte-for-byte. Runtime recognition
uses only that immutable committed table; there is no runtime host-library update.

Additional decoder cases check every two-byte view, exact whole-buffer failure
offsets, empty/NULL input and every noncontinuation byte at each continuation
position of scalar boundaries. They exhaust all nonminimal forms at widths two,
three and four, plus every four-byte 21-bit payload above U+10FFFF. Together with
existing enumeration of every canonical scalar/surrogate and each truncation,
these cover the complete value/width partitions. Truncation and malformed decode
preserve both scalar and width outputs. Validation advances only by a positive
accepted width bounded by remaining bytes, proving finite processing and no
out-of-bounds access for valid borrowed views.

Complete-name tests check international multiword names, edge/repeated/non-ASCII
whitespace, later-word digit/combining starts, bounded input and malformed bytes
with exact offsets. A complete-domain composition case places every nonsurrogate
scalar after `a ` and checks that the pinned start rule applies again. This test
uses the independently verified property predicate; no normalization or locale
can replace the recorded edition. Filesystem exact-spelling and generated C-name
qualification remain separate contracts rather than XID-specific C restrictions.

T015 acceptance passes the full strict `-DNDEBUG -Werror` test tree, focused
Unicode ASan/UBSan leaf (leak detection disabled), complete coverage of 767
maintained inputs and all Markdown links. Warning-free Doxygen checks 595
maintained files, 2440 interfaces and all 288 canonical/page destinations;
two complete generations produce byte-identical output.


## Shared Ama word prefix

T169 implements [SRC-03](../../spec/language/source-and-names.md#src-03)'s
isolated internal ASCII hyphens through one bounded word operation. Its contract
also supplies the descriptive components required by
[TPL-01](../../spec/language/templates.md#tpl-01); full static syntax is T170's
separate boundary, not a capability claimed by this component.

The operation unicode_word_prefix, in src/source/unicode/word.c, borrows an
original byte span and returns the length of its longest valid word prefix.
NULL/empty/invalid-start input returns zero. It never reads a terminator or a
byte beyond the supplied length, allocates, retains, changes or normalizes input.
Pinned scalar XID predicates retain their existing meaning: hyphen is not added
to XID_Continue or to C identifiers. The complete-word validator requires a
nonzero prefix covering its whole view; failure reports the first unconsumed
byte and success leaves the caller's error output unchanged.

The initial transition requires XID_Start or underscore. Each later transition
consumes either one ordinary continuation scalar, or ASCII hyphen together with
its following ordinary continuation scalar. Therefore each published prefix
ends in an ordinary character; a hyphen never appears at either end or next to
another hyphen. An underscore or combining continuation immediately after a
hyphen is permitted. A space is outside the word operation: unicode_name starts
a fresh word after exactly one space, applying the start property again.

Invariant: offset is within the borrowed length, lies after a complete ordinary
scalar, and the consumed prefix is a valid word. Each accepted transition
preserves it and advances by at least one byte. At a failed transition, no longer
valid prefix can pass the offending scalar or incomplete hyphen transition;
returning the preceding offset is maximal. No input is skipped or repaired.
Bounds follow from UTF-8 decoder width being within the remaining view; advancing
one byte over a present hyphen remains at most length, even at physical EOF.
Time is O(B + S log R), for bytes B, inspected scalars S and pinned ranges R;
scratch storage is O(1), with no allocation, recursion or locale dependence.

The lexer uses this operation for each complete-name word; its source span and
original bytes remain unchanged. The complete-name validator delegates through
the same operation, so path/catalog/literal-label consumers share acceptance.
Ten explicit test source lists include the new production file, while directory
source discovery includes it automatically for normal compiler builds. Native
ABI identifiers use their existing independent supported-C recognizer and the
standalone destructor marker retains its record role.

### T169 steps and acceptance record

Input revision: 755bc97, clean before the task. The owning TODO record supplies
scope, prerequisites and acceptance. All production edits were settled before
the final sixteen-unit/two-integration batch (session 18872); the two relocated
translation units received fresh dependency timestamps before rebuilding, so
no object from an earlier edit interval supplies final evidence.

| Step | Actual result | Verification |
|---|---|---|
| 1. Contract | One bounded borrowed word-prefix operation; unchanged scalar properties and C identifier domain | Invariant, maximality, progress, bounds and cost argument above |
| 2. Implementation | Word prefix/full validation in word.c; scalar tables/search in identifier.c; lexer shares prefix and preserves spans | No duplicate word scanner, allocation, normalization or source mutation |
| 3. Independent component tests | 97656 strings over five roles, complete Unicode post-hyphen domain/truncations, exact byte/boundary cases | source/unicode passes normally and with ASan/UBSan bounds instrumentation |
| 4. Real consumers | Paths/roots/submodule storage, compact refs/indices, literals, exact identities, frontend fields, rename and relocation | Sixteen unit and two integration leaves pass; generated C program compiles and runs |
| 5. Documentation | Purpose/contracts and per-file named-case mapping include both new code files and all changed consumers/build inputs | docs-complete passes: 2337 ready inputs, zero explicit debt; final Doxygen generation follows |

Normal commands, from repository root: make, then make -C tests/unit/NAME for
each of source/unicode, lexer/tokens, source/module, source/identity,
compiler/check, parser/abi, native/status-providers, native/namespace,
driver/options, lexer/layout, parser/module, parser/declarations, parser/calls,
editor/rename/plan, editor/rename/validation and tools/relocation/preview;
then make -C tests/integration/parser/grammar and make -C tests/integration/check.
All exited zero. Exact individual logs are /tmp/ama-t169-final-*.log.

Independent component cases are
it_matches_hyphen_words_and_names_over_a_finite_alphabet,
it_checks_every_post_hyphen_scalar_and_truncated_encoding and
it_preserves_hyphen_word_boundaries_and_exact_unicode_bytes. The finite model is
an independently enumerated three-state grammar, not a copy of UTF-8 scanning.
Post-hyphen property data are read independently from the pinned Unicode source;
scalar property/checksum and every-word start tests remain unchanged. Consumer
cases inspect actual token spans, nominal types, namespace roots and candidate
rename bytes; related named mappings are in the per-file registry.

#### Compiled source acceptance

Place empty body modules a-b and a_b in a fresh source directory P, each
containing only the body section marker. Use this extensionless main file:

```text
@!
value-name
.a-b
other_name
.a_b
item-list[]
.a-b
copy-list[]
 item-list[]
value-ref&
 & value-name
value-name !
other_name !
```

Execute ./ama --emit P .main --out O --sdk /absolute/path/to/ama/src --compile
with a fresh output O, then execute O/program. Both commands exited zero and
runtime stdout was empty. The original /tmp/ama-t169-program-xxbahye2 run used
source/output directories containing spaces; results.json retains argv/streams.
This checks actual C naming and owned/list/reference construction/calls without
analyzing generated files. It introduces no static-template execution claim.

#### Failures and actual instrumentation boundary

The expanded parser/composition suite failed before parser invocation because
its fixed canonical fence inventory is stale. An isolated source archive of
unchanged 755bc97 reproduces the same assertion at documented.c:104 (exit 2;
/tmp/ama-t169-parser-baseline.log). The fixture and counted chapters are byte
identical to that revision: declarations expects 9 text fences versus 16,
lists 3 versus 4, grammar 8 versus 20 and C-library 1 versus 3. No word-scanner
operation participates in the count. T170 owns explicit example/context
reconciliation with static syntax. No example is skipped and this broad suite
is not reported as passing; full conformance remains pending later tasks.

A new rename test initially used a nonexistent status member and failed C
compilation; correcting it to the declared validated member passed the focused
and final complete rename runs. No production behavior was changed for that fix.

The first ASan/UBSan run with detect_leaks=1 failed at LeakSanitizer shutdown
(exit 2 from make), reporting ptrace incompatibility. A separate nonleaking
one-byte malloc/free C probe reproduces that fatal error (exit 1). Thus leak
sanitizer success is not claimed. With detect_leaks=0 its normal access exits
zero; an intentional one-byte overflow exits one with an UBSan diagnostic and
AddressSanitizer heap-buffer-overflow. Probe source/logs are retained under
/tmp/ama-t169-asan-EkeFnb. New word operations allocate nothing; ownership and
counted fixture cleanup remain separate evidence.

The complete Unicode leaf passed (exit 0, session 46755) with environment
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 and UBSAN_OPTIONS=halt_on_error=1,
using make -C tests/unit/source/unicode and this single C_COMPILER_FLAGS value:

```text
-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer
```

/tmp/ama-t169-sanitizers-bounds.log records the actual successful run. All tests
ran serially within shared output owners; the baseline archive and compiled
program used independent temporary trees. No semantic limits, fallback scanner
or disabled assertion were introduced. Unicode tables, native identifier policy,
canonical specification and reviewed conformance fingerprints remain unchanged.


T169 documentation acceptance: canonical/local link checks and git diff --check
pass. make docs-complete validates 2337 ready inputs with zero explicit debts;
make docs validates 1751 maintained file pages, 6711 documented interfaces and
417 rule/page destinations (5532 generated pages), using Doxygen 1.11.0,
Python 3.14.4 and markdown-it-py 3.0.0. Archive/queue updates are followed by a
final generation before committing T169; source/API and test evidence above
remain the acceptance boundary. Full canonical-example/parser and template
implementation conformance remain with the subsequent tasks.
