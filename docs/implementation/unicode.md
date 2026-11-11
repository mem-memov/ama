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
until success or an empty interval. Identifier recognition decodes each scalar,
uses XID_Start or underscore first and XID_Continue or underscore thereafter.
It rejects empty names and reports the failing scalar's lead offset. Comparing
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
