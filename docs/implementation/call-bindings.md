# Call bindings, typed literals and positional holes

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/parser/calls/`, especially `bindings/`, records call modes and
placeholders in the currently tested parser subset.
No callee interface or declared-name lookup is used during parsing.

An argument retains its complete line/marker spans, kind, optional declaration
label, source or receiver, independent direction position and explicit omitted
flag. Empty positional < and > create an omitted node with neither source nor
receiver. Their counters still advance. No hidden variable/default value is
invented. Actual type compatibility and callee arity are checked after resolution.

Recognition finds the first ASCII angle marker: such bytes cannot occur in a
valid name/access/copy source. Literal payload is excluded from that code view. Optional labels must have declaration-name syntax;
spaces separate label, marker and nonempty operand. >> must be contiguous. Empty
named bindings and empty >> are invalid. Comment-only text after < or > supplies
no operand. Input sources and output receivers use distinct grammar boundaries. A recognized
input `|` produces AST_LITERAL_SOURCE with a payload view and a source span from
`|` to physical EOL. The argument span also includes the complete payload. Empty
literal and omitted input are distinct nodes. Literal type/storage/capability
checks belong to resolution; no inferred numeric value or hidden source is built.

The call-list state consists of mode, one selected naming style, input/output
counts and whether an output was seen. Entry accepts inputs before outputs, never
>>. Resume accepts only >. Retained/static reads require one or more >>. Every
new binding preserves this finite-state predicate or fails before append. A
binding sequence has independent direction counters. Induction over that sequence
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
three-input/three-output holes and their exact positions, scalar resume holes,
comments, malformed spacing, exact literal bytes including embedded zero/EOF,
empty literals versus holes, forbidden literal positions/modes, all mixed modes and
named placeholders. An independent finite-language oracle checks all 5856 call
forms: four modes times every sequence of length zero through three over eleven
binding forms, including positional/named literals. The argument family includes empty/nonempty positional forms and
named input/output/read forms. Runtime reset and actual interface arity are not
claimed by syntax tests.

CALL-08 evaluation order is retained in the AST's binding order and distinct
interface blocks. Duplicate callee labels, interface arity, types, constructor
selection, source protection and ordered output transfer are semantic/lowering
contracts; this parser does not execute or certify those later operations.
