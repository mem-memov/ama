# Physical layout and literal boundaries

The [SYN-01/SYN-02 rules](../../spec/language/source-and-names.md#syn-01) and
[LIT-01](../../spec/language/declarations.md#lit-01) require original byte views,
strict global encoding and exact literal boundaries. `src/lexer/layout/` and
`src/lexer/literals/` provide this physical layer; the
[region-aware token scanner](lexical-tokens.md) supplies active code joins.

`SourceLayout` owns line nodes in an arena and borrows source storage. Read once
and destroy after dependent parsers. Preflight validates the entire UTF-8 input,
then rejects TAB and lone CR before allocating lines, recognizing literals or
checking activation. This applies equally to drafts, comments and ABI files.
A CR immediately preceding LF is a line-ending byte, never payload data.

Each line retains its original content range, LF/CRLF/absent ending, leading-space
count and code/payload spans. An unterminated final line acquires no source byte
or fictitious extra line. Empty files have no line nodes. Classification records
spaces-only lines as physical blank candidates; active region validation rejects
their nonempty content instead of treating spaces as blank-line syntax.

Ama literals start at the first nonindentation `|`, or after an exact positional
or named input `<` and one separator. Named labels use complete XID names,
including their single internal spaces and compact `&`, `[]` or `&[]` suffix.
The recognizer validates that complete spelling without looking up declarations.
It runs before `#` recognition, so legal payload hashes, quotes, zeros and trailing
spaces remain data. TAB and lone CR are globally invalid even in payloads.
Malformed joins do not manufacture a literal boundary or hide bad code as data.

For nonliteral code, the first `#` and its maximal preceding space run form the
comment suffix; the classifier removes that run from code, preserving all original
physical bytes. A comment-only line retains its structural indentation. The token
scanner checks that depth against the grammar region, rejects trailing code
spaces/spaces-only lines, and enforces code separators. The physical reader does
not guess whether an indentation starts a valid statement body.

`layout_read_abi` uses the same global preflight and physical splitting with
literal classification disabled. Thus ABI `|` remains a binding symbol; ABI's
flat record grammar is a separate consumer, completed in T028. There is no second
lenient TAB policy or post-activation byte validator.

Splitting partitions the processed source prefix into content and terminator
ranges; each iteration advances past LF or to EOF. Classification changes view
bounds only. Candidate-label validation uses pinned XID properties, and counted
spans never normalize or trim payload bytes. Time is O(b log r), for b input bytes
and fixed Unicode interval count r; node storage is O(physical lines), with
constant auxiliary storage. Global preflight allocates nothing and can reject
malformed input before any line node is published.

Evidence: `make -C tests/unit/lexer/layout` and
`make -C tests/unit/lexer/tokens` cover every physical ending, input label/storage
boundary, raw literal spaces/comments, global TAB/CR rejection, malformed UTF-8,
exact region indentation and every physical-line allocation failure. Existing
parser fixtures retain legal payload bytes; explicit negative cases certify that
TAB cannot be hidden in drafts, comments or literals.
