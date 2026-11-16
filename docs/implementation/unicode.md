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
