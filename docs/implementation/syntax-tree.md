# Source AST and structural parser

Implementation status: the CALL-07 entry-literal amendment is specified but not
implemented yet. The priority follow-up in [TODO.md](../../TODO.md) covers lexer,
AST, call parsing and replacement of literal-prohibition tests. The evidence below
covers the current implementation; it does not certify CALL-07 support.

Boundary: `src/ast/` and `src/parser/{access,declarations,body,calls,module}/`.
Implements COMP-01/02, TYPE-01/02, source/access syntax, SYN-01 through SYN-05
and call/control syntax. Full type, declaration-order, capability and ownership
checks remain section 5. In particular multiple scalar initializers, constant
reference storage and break-outside-loop are represented for semantic rejection.

An AstModule owns a layout and an arena. Nodes contain original half-open source
spans, storage suffixes, paths, initializer/source kinds, indexed access chains,
call modes/bindings, and explicit child/alternative blocks. No generated C,
resolved type or mutable global symbol environment enters syntax recognition.
Text spans borrow the source manager, which must outlive the module. Parse once
from initialized storage. Only valid=true permits consuming a complete AST;
partial allocations remain owned for cleanup and error context on failure.
A structured ParseError carries a stable grammar-expectation key and Diagnostic;
see the [error contract](parser-errors.md).

Section intervals feed independent declaration parsing and an explicit-frame body
parser. A same-indent type path follows a declaration name; an indented copy
source follows an assignment receiver; !/% selects entry/resume; indented binding
markers select retained reading. An owned access with same-indent ?/{ selects
control flow, allowing comments but no blank line before the marker. No declared
name decides which production applies. Static paths select constant namespaces,
never implicit instances. Literal payload is accepted only in initializers.

Names decode pinned XID properties; access parsing iteratively builds a single
nested-index chain, then consumes its required closing brackets. Index operands
must themselves have owned-access syntax. Prefix acquisition and continuation
copy are separate source kinds; forbidden reference-to-reference acquisition and
whole-list continuation/resume have no AST interpretation. Qualified constants
split at the final dot, without namespace lookup or longest-prefix searching.

For each production, consumed lines/bytes advance monotonically or produce a
failure. Body frames track their block, expected indentation and closing rule.
Entering control pushes one arena frame; finishing a child pops it or switches
once to an alternative. Loops consume a matching same-indent closing marker.
These transitions simulate the parameterized block grammar by induction on the
explicit frame chain, without recursive C activations. Access-chain construction
likewise increases consumed bytes until its finite closing-bracket phase.

Nodes are linked only inside this owned tree; an allocation refusal sets an
allocation-free error and leaves the module invalid. Destroying both arenas frees
all partially constructed nodes/frames/lines without walking the tree recursively.
For b source bytes and n nodes, parsing costs O(b log r+n) with r XID ranges.
A layout run is inspected by local lookahead and then consumed, a constant number
of times; popping frames does not rewind the global line cursor. Memory is O(lines+n),
including explicit depth frames and index chains. No arbitrary nesting cutoff.

Evidence: `make -C tests/unit/parser/tree` passed C11/pedantic/Werror. It checks
all storage/source forms, literal byte spans, nested indices, assignments, named
calls, reads, resumes, alternatives, shared reads, branches/loops/return/break/
yield, negative syntax partitions, 512-deep control and index chains, and every
allocation refusal in a composed parse. Call binding ASTs and positional-hole invariants are described in the
[call contract](call-bindings.md).
