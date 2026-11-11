# Fixed native ABI table parser

Boundary: `src/parser/abi/`, implementing PROTO-11. AbiTable owns physical layout
nodes and borrows original source spans for six fixed operation slots. Parse once
and consume slots only after valid=true; destroy cleans all partial layout.

A record has exactly one supported marker, ASCII spaces and one C identifier
spelling. Slots are indexed by +, -, |, =, ?, [] in that fixed order, not by helper
names. Record order is irrelevant. Blank/comment lines and trailing comments are
layout; indented records, tabs anywhere, lone CR, unknown symbols, duplicate
symbols and additional noncomment tokens fail. UTF-8 validation runs before any
record parsing. + and - are required at EOF; optional slots remain absent.

A finite slot invariant establishes at-most-once binding. Duplicate errors retain
the earlier marker as a related source location. Missing records have a zero-width
EOF span. Other failures retain the offending physical byte/line range. No failed
table is published as usable and reporting needs no further allocation.
ParseError retains a stable grammar-expectation key alongside its Diagnostic.

Native names are not Ama identifiers. Their lexical scanner implements C11 Annex
D.1/D.2 and ASCII letters/underscore/digits with GCC's dollar extension, accepts
UTF-8 or C universal character names, and rejects C11 keywords. It preserves the
module-local spelling rather than treating it as a global C symbol. The native
namespace emitter qualifies it under NAME-05; Unicode/UCN spelling remains intact. This follows
[C11 N1570, 6.4.2/6.4.3 and Annex D](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
and [GCC identifier documentation](https://gcc.gnu.org/onlinedocs/gcc/Identifiers-implementation.html).
Toolchain-specific reserved names/macros, symbol existence, actual function
usability and typed signatures still receive C compiler verification at the
native build boundary; lexical acceptance alone does not certify a callable hook.
No C declarations, expressions, macros or function signatures are parsed here.

Record scanning advances by each physical line. Name scanning consumes a positive
UTF-8 width or a fixed-width valid UCN; scalar membership follows explicit finite
ranges. Thus recognition terminates and depends on no locale, host Unicode
library or naming convention. Work is O(source bytes), with a fixed-size range
factor and six binding slots; owned layout storage is O(lines).

Evidence: `make -C tests/unit/parser/abi` passed C11/pedantic/Werror. Cases cover
all six slots in mixed order, comments/CRLF/optional final newline, international
and UCN names, malformed/extra/unknown records, duplicates with related spans,
required bindings, invalid UTF-8, tab/lone-CR rejection and every line-allocation
failure. Signature conformance remains separately covered by native ABI tests;
[Frontend composition](parser-errors.md) compiles two module-qualified
companions and all six typed uses in one C translation unit.
