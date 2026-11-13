# Exact lexical tokens and structural regions

T022 implements [SYN-06](../../spec/language/source-and-names.md#syn-06) through
[`lexer/tokens/tokens.h`](../../src/lexer/tokens/tokens.h). The scanner consumes
physical views from the [layout reader](layout.md), produces original spans and
never queries a declaration inventory. Its cursor and diagnostics allocate no
memory. Source/layout owners outlive scanning and all returned token views.

The caller supplies the indentation of its actual grammar region. Empty physical
lines ignore depth; code, literals and comment-only lines must match it exactly.
A comment cannot create a deeper region. Spaces-only lines and nonliteral trailing
code spaces fail. Arbitrary spaces immediately before a trailing `#` belong to
the comment suffix and remain legal. The caller's structural parser determines
which region may continue, so a lexical cursor does not guess a block from names,
preceding indentation or an unfinished declaration.

Names are maximal sequences of pinned XID words with exactly one internal ASCII
space. Punctuation terminates that sequence; a space before punctuation remains
a code separator rather than part of the name. At punctuation positions, an
explicit longest-first fixed inventory recognizes `&[]` before `&[`/`&`, `>>`
before `>`, `..` before `.`, and the fixed section annotations. Unknown characters
fail without transliteration or guessed repair.

A finite boundary relation distinguishes compact joins from required single
spaces. Paths, reference prefixes/suffixes and index/list storage joins adjoin
their operands; binding and execution/copy markers use one separator. Ampersand
role is determined by the existing token context, without binding a name. Its
standalone form remains available for a self-reference source. A double-dot root
cannot become an internal component separator. Repeated code separators reject.
Literal tokens expose the exact marker and raw payload spans independently.

Each successful token consumes at least one original code byte, so scanning a
finite line terminates. XID scanning consumes one complete scalar at a time;
joining another name word consumes its separator and nonempty word. Fixed-token
alternatives are ordered by decreasing spelling length, proving longest match.
The boundary relation checks only the immediately preceding token and explicit
reference-prefix state. Output is published only after recognition and joining
checks succeed; an error records an original byte site and leaves the caller's
token unchanged. Work is linear in code bytes up to pinned XID interval searches;
auxiliary storage is constant, excluding borrowed source/layout owners.

`make -C tests/unit/lexer/tokens` exercises Unicode multiword names, nested index
accesses, compound tokens, compact references/lists, execution/copy/binding spaces,
all three physical ending variants, comments and empty lines, raw named-entry
literals, invalid Unicode spacing/name starts and global TAB/CR rejection. Token
publication is checked byte-for-byte after failure. An allocator set to fail its
next request observes no further allocation while the cursor scans valid tokens.

This is a lexical component, not a complete source parser. It does not establish
balanced accesses, statement mode/arity, section order or legal control flow.
T023/T024 integrate checked regions/tokens with section/declaration/access ASTs;
T025–T027 apply the same contracts to calls and structural bodies, and T028 composes
the complete parser/ABI reader. Historical parser acceptance is not evidence that
these remaining current-source forms are already implemented. In particular,
region attachment and dedented-comment/body closure are grammar decisions.
