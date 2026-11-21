# Source AST and structural parser

Boundary: `src/ast/` and `src/parser/{access,declarations,body,calls,module,static,types}/`.
Retains structural machinery for COMP-01/02, TYPE-01/02, ordinary source/access
syntax, sections and call/control syntax. Full type, declaration-order, capability and ownership
checks belong to semantic resolution. In particular multiple scalar initializers, constant
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
never implicit instances. Literal payload is accepted in declaration initializers and entry input
bindings, never in assignment/output/condition/index operands. It remains an
uninterpreted borrowed byte range, including embedded zero and physical EOF.

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


T146 documents every retained AST type, parser helper and iterative frame boundary.
The ordinary parser now represents complete multiword/two-root accesses,
standalone self sources, direct-result operation trees, negative-only
conditionals, foreach bindings and lexical condition-loop repeat roots.
T168 rechecks the declaration/call/result/control components and their real
frontend consumers; [the audit](audit/template-sprint.md) records exact evidence.
The current source representation is described below and in the
[T206 evidence and caller review](static-source.md). Earlier task-specific
acceptance remains in [DONE](../../DONE.md#t170); it does not establish current
static-module support.

## Separate static syntax

The parser implements structural recognition under
[TPL-01](../../spec/language/templates.md#tpl-01),
[TPL-12](../../spec/language/static-results.md#tpl-12) and
[COMP-01](../../spec/implementation/compiler.md#comp-01). It does not perform
scope lookup, infer substitutions, determine a producer, or generate a Type.

`@...` selects one exclusive module graph stored in `AstModule.graph`. Ordinary
sections and the runtime body remain empty. A public static pair has a complete
single-dot name and a same-indent value head. A producer use has a concrete path,
zero or more named inputs and at least one named output. All inputs precede
outputs. Graph values nest through named input edges only. There are no static
skips, positions or standalone formal/signature declarations. Runtime calls
retain their own positional forms. An empty fully valid static graph is the
closed marker classified by [T221](../../DONE.md#t221).

An ordinary declaration keeps a variable name and a complete structural Type
expression under [TPL-15](../../spec/language/templates.md#tpl-15). Named input
bindings form original child expressions. Each level can finish with one bare
foreign export selector stored in `selected_export`; it introduces no local
name and has no children. Ordinary initializers follow the complete outer
expression at their prescribed depth. Graph output bindings are not declaration
selectors. The explicit-stack parser retains separate graph-value,
producer-statement and declaration-Type modes.

Constant namespaces use their own simple head/optional-selector reader under
[CONST-03](../../spec/language/declarations.md#const-03). They retain ordinary
`>>` requests and acquire no declaration constructor bindings. Shared selector
recognition preserves the same original foreign-name span without module lookup.
[T269](../../DONE.md#t269) records this source foundation; T226 owns complete
semantic/executable declaration-expression admission.

Every syntax span borrows original bytes. Arena frames retain the exact physical
indentation, owning input and exact source-expression mode. Each step consumes a source
line or pops a finite frame; deep source nesting does not consume the C stack.
Partial nodes, frames and diagnostics have the same destruction contract as
ordinary parsing. A head opens only its prescribed child region: comments cannot
invent deeper nesting or reopen a region after dedentation. LF, CRLF and a final
virtual newline preserve the same code structure without changing literal data.

Dependency discovery follows graph and declaration-expression owners and every
concrete nested head. It excludes local placeholder names, foreign labels and
export selectors as independent dependencies, while retaining a selected head's
producer role. Source acceptance does not itself certify a complete ordinary
Type or a closed producer. The existing simple-Type/export semantic boundary
and T226's remaining integration are distinguished in the
[declaration-expression contract](declaration-expressions.md).
