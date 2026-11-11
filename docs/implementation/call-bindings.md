# Call binding syntax and positional holes

Implementation status: the CALL-07 entry-literal amendment is specified but not
implemented yet. The priority follow-up in [TODO.md](../../TODO.md) covers lexer,
AST, call parsing and replacement of literal-prohibition tests. The evidence below
covers the current implementation; it does not certify CALL-07 support.

Boundary: `src/parser/calls/`, especially `bindings/`, implementing section 7
call-mode/placeholder requirements and the fixed positional collection interface.
No callee interface or declared-name lookup is used during parsing.

An argument retains its complete line/marker spans, kind, optional declaration
label, source or receiver, independent direction position and explicit omitted
flag. Empty positional < and > create an omitted node with neither source nor
receiver. Their counters still advance. No hidden variable/default value is
invented. Actual type compatibility and callee arity are checked after resolution.

Recognition finds the first ASCII angle marker: such bytes cannot occur in a
valid name/access/copy source. Optional labels must have declaration-name syntax;
spaces separate label, marker and nonempty operand. >> must be contiguous. Empty
named bindings and empty >> are invalid. Comment-only text after < or > supplies
no operand. Input sources and output receivers use distinct grammar boundaries.

The call-list state consists of mode, one selected naming style, input/output
counts and whether an output was seen. Entry accepts inputs before outputs, never
>>. Resume accepts only >. Retained/static reads require one or more >>. Every
new binding preserves this finite-state predicate or fails before append. A
whole-list interface uses positional bindings only. Induction over the list
establishes mode/style/order consistency and positions equal to the preceding
count in the same direction, independently for inputs and outputs.

Counts cannot wrap: every argument consumes a distinct nonempty physical line,
so each count is bounded by the source byte length, which source storage requires
to be smaller than SIZE_MAX. A body-level > remains an early-return AST node;
only a line exactly one level deeper than its call is an output placeholder.

Work is linear in argument bytes (plus pinned XID searches), with one owned node
per binding and source/access nodes only for supplied operands. Failure leaves
the parent module invalid and all partial storage owned until destruction.

Evidence: `make -C tests/unit/parser/calls` passed C11/pedantic/Werror. Tests check
three-input/three-output holes and their exact positions, resume holes, whole-list
calls/reads, comments, malformed spacing, literal rejection, all mixed modes and
named placeholders. An independent finite-language oracle checks all 3280 call
forms: four modes times every sequence of length zero through three over nine
binding forms. The argument family includes empty/nonempty positional forms and
named input/output/read forms. Runtime reset and actual interface arity are not
claimed by syntax tests.
