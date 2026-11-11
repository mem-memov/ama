# Physical layout and literal boundaries

Boundary: `src/lexer/layout/`, implementing SYN-01/02/04 and LIT-01/02.
`SourceLayout` owns line nodes in an arena; source ranges borrow their manager.
Read once after init and destroy after dependent parsers. Failed reads retain
only owned partial storage and an allocation-free structured diagnostic.

Reading validates all UTF-8 before splitting at LF. A CR immediately before LF
belongs to the ending, not the payload; a final newline is optional. Each node
retains the physical content range, ending kind, exact leading-space count,
comment-trimmed code and untouched literal payload. Only a first non-space `|`
selects a literal; following `#`, tabs, quotes and spaces remain data.

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
Time is O(source bytes), storage O(physical lines), with constant auxiliary space.

Evidence: `make -C tests/unit/lexer/layout` passed C11/pedantic/Werror. Tests cover
CRLF/LF/no-final-newline, literal tabs/comments/trailing spaces, lenient draft
reading versus strict whitespace, invalid UTF-8, and every line-allocation failure.
