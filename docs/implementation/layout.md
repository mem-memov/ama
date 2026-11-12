# Physical layout and literal boundaries

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/lexer/layout/` and `src/lexer/literals/`, retaining physical
line/payload machinery needed by SYN-01/02/04, LIT-01/02 and CALL-07. This is
partial implementation evidence, not complete current lexical conformance.
`SourceLayout` owns line nodes in an arena; source ranges borrow their manager.
Read once after init and destroy after dependent parsers. Failed reads retain
only owned partial storage and an allocation-free structured diagnostic.

Reading validates all UTF-8 before splitting at LF. A CR immediately before LF
belongs to the ending, not the payload; a final newline is optional. Each node
retains the physical content range, ending kind, exact leading-space count,
code and untouched literal payload. Ama literals start at a first non-space `|`
or at the operand of a positional/named input `<` with required ASCII spaces.
The input boundary recognizes exactly a Unicode declaration name with optional
`&`/`[]`, a separated `<`, and a separated `|`; it never looks up a label. Code
ends immediately after the literal marker while payload extends to physical EOL.
Following `#`, tabs, quotes, zeros and spaces remain data.

`layout_read` reads Ama; `layout_read_abi` uses the same physical-line/UTF-8
algorithm with literal classification disabled. In ABI records `|` is an ordinary
binding symbol and `#` still begins a comment. Both retain unterminated final
lines without fabricating a source byte or extra line.

A separate active-file validator rejects lone CR everywhere and tabs in Ama code
or indentation. Comment payload is not code whitespace. Native ABI validation
rejects tabs everywhere, as PROTO-11 requires. Draft recognition can therefore
use strict UTF-8 lines without parsing unfinished active grammar or enforcing its
whitespace. The module parser will decide whether active validation is required.

Splitting maintains the invariant that completed line ranges partition the
processed source prefix, including distinct terminator ranges. Each iteration
advances past an LF or to EOF. Classification removes only the explicitly defined
indentation/comment/trailing-code-space intervals; literal payload ranges refer
directly to original bytes. No normalization, name lookup or token guess occurs.
Each physical line is classified once; candidate labels use pinned XID interval
searches. Time is O(b log r) for b bytes and r fixed Unicode ranges, storage is
O(physical lines), with constant auxiliary space.

Evidence: `make -C tests/unit/lexer/layout` passed C11/pedantic/Werror. Tests cover
CRLF/LF/no-final-newline, positional/named input boundaries, malformed separators,
literal tabs/comments/trailing spaces, ABI tab rejection, lenient draft
reading versus strict whitespace, invalid UTF-8, and every line-allocation failure.


T146 documents every line/read/classification/validation operation and its borrowed
span/owned-node lifetime. The current validator still permits TAB in some payloads
and inactive files, accepts separators/trailing code spaces that the specification
forbids, and does not enforce all comment indentation. T022 must replace these
remaining gaps before current-source acceptance is claimed. Literal preservation
means preserving legal bytes; it does not override the specified global TAB ban.
Existing positive TAB fixtures measure the retained code and must change in that
feature task. No alternate whitespace contract is offered to Ama programmers.
