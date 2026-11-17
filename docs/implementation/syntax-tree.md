# Source AST and structural parser

Boundary: `src/ast/` and `src/parser/{access,declarations,body,calls,module,static}/`.
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
At that audit revision a declaration type was one concrete path span. T170
replaces it with a source-owned structural expression and records static formals,
signatures and result definitions, as described below. Source classification never queries declared
names, and failed partial trees remain unusable while their arenas stay
destructible.


## Static syntax construction (T170)

Input revision: 858c7f4; the tree was clean before T170. This component parses
structural static syntax under [TPL-01](../../spec/language/templates.md#tpl-01),
[TPL-12](../../spec/language/static-results.md#tpl-12) and
[COMP-01](../../spec/implementation/compiler.md#comp-01). It neither collects
free formals nor performs substitution, kind inference or execution. Those
consumers have separate prerequisites and acceptance in T172–T184.

Static syntax records borrow original SourceSpan bytes. Expressions, binding
links, formal signatures and parser work frames belong to the containing AST
arena. The source manager outlives that arena. Failure leaves partial allocations
owned and the module unusable; destroying the arena needs no recursive tree walk.
A completed node preserves its exact head, label, marker and receiver sites;
there is no generated module file, source replacement or inferred runtime slot.

Type heads distinguish module paths and placeholder spelling only. An atomic
static name is exactly three dots and complete adjoining components; scope and
Type/Template category are never used to select a production. Bare three dots
instead precede a separated static binding marker. The lexer recognizes that
compound before the two-dot module root and preserves existing compact joins.

The static AST model records ordered inputs and outputs, positional/omitted
flags, nested input expressions and full receiver spellings. Formal signature
children remain recursively structured source records; the presence of children
is syntax, while local/foreign identities and exact signatures belong to later
resolution. An output definition has a separate name and expression line.

Expression parsing must use explicit owned frames. Frame indentation is the
physical head/binding line's indentation, not the column at which an inline
right-hand head begins. Inputs precede outputs; an expression ends at its
caller's boundary without swallowing ordinary initializer or constant-read
bindings. Declarations consume their type expression before runtime initializers;
no later static binding can reopen that type expression. The mode/arity solver
never enters parsing. Signature parsing likewise uses finite explicit frames;
comments cannot invent indentation levels or nonempty signature members.

Progress invariant: each accepted source line advances the physical cursor;
a nonconsuming transition closes a finite pending frame. Each frame owns its
expression/signature and fixed indentation/phase; all appended links preserve
source order. Nested expressions cannot consume a sibling's binding. No C
activation stack or arbitrary depth budget represents source nesting. The final
implementation record must derive its exact time/space bounds from these
operations and record allocation-failure and ordinary-consumer checks.

Current step 4 (2026-11-18): expression, signature, section, declaration,
constant-namespace and standalone-projection compositions are implemented.
The independent static leaf passes component and interface/body cases
(`/tmp/ama-t170-body-tests.log`). The canonical example inventory now records
all current text fences in its chapters, adds templates/static-results, declares
input/interface/binding contexts explicitly and expects the documented invalid
post-constant static binding to fail. Its composition passes
(`/tmp/ama-t170-canonical.log`). No specification bytes were changed.

The existing semantic inventory accepts concrete syntax only: it now rejects
unprocessed static nodes, placeholder heads and binding blocks instead of silently
ignoring them. T184 owns composing the static resolver before that boundary.
This is an implementation-stage admission check, not a restriction on Ama.
Next: run depth/composed allocation failures and ordinary consumer regressions,
register per-file evidence, complete documentation acceptance and commit T170.

Acceptance checkpoint: complete parser parent, token leaf, semantic names,
inference/composition, project closure and parser/check integrations passed
(`/tmp/ama-t170-regressions.json`). Two initially guessed editor index/query leaf
paths do not exist and were not counted as tests; use the actual editor parent.
The settled static/composition/compiler-check leaves pass their `*-settled.log`
checks, including all composed allocation-failure prefixes and 600-level formal
signatures. `make` and codegen/assembly pass. `make docs-complete` admits 2352
maintained inputs with zero debt. The first editor parent attempt failed in rename
planning: the new nullable namespace pointer was dereferenced on ordinary calls.
Indexing now tests its presence and operation queries use the explicit call mode;
existing real root/selected/constant editor cases now assert these three shapes.
Rerun the full editor parent and final documentation on corrected sources.

Correctness and cost: a completed frame owns a contiguous source-depth region.
Appending a binding changes only its owner's tail; pushing its input frame keeps
that parent suspended until the child's first dedent. Closing a child extends
only that input/parent span. Input/output phase changes are monotone. The same
induction applies to each signature's child list. Section productions select
formal/result/declaration shape solely from their marker and leading spelling;
body productions select projection/constant read from written output actions.
Consequently identical syntax is classified identically regardless of names in
any registry. Existing runtime initializer/call drivers retain their own phase
and cannot reopen the completed static binding block.

Every successful transition consumes a line or closes a previously pushed frame.
Binding/name validation scans each owning head a constant number of times. With
b admitted bytes, r pinned Unicode ranges, e expression/binding/signature records
and l layout lines, these static components cost O(b log r + e + l) time and
O(e + l) arena storage, including traversal frames retained until arena cleanup.
The live logical frame chain is bounded by source nesting; the C activation
stack does not grow with that nesting. Proper child depths are bounded by their
physical indentation bytes, so depth addition has no separate arbitrary limit.
OOM invalidates the containing AST and leaves every partial allocation with its
single arena owner; direct component outputs are published only on success.
This argument concerns structural parsing, not future specialization complexity.

Corrected acceptance: `make -C tests/unit/editor` passes every rename, workspace,
lexical, binding and composition child (`/tmp/ama-t170-editor-corrected.log`).
The static component's complete ten-case leaf passes `-Werror` with AddressSanitizer
and UndefinedBehaviorSanitizer, including all failure injections and deep trees
(`/tmp/ama-t170-static-sanitized.log`):

```sh
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
make -C tests/unit/parser/static \
  C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -fsanitize=address,undefined -fno-omit-frame-pointer'
```

LeakSanitizer remains disabled only for the independently reproduced T169
container/ptrace limitation; counted fixture teardown asserts zero live owned
allocations. No active functional jobs remain. The per-file registry includes
all new production/test/build units and standalone header commands; final
Doxygen/coverage acceptance follows on these settled maintained sources.

The first final Doxygen render correctly refused a missing originating-file link
for `src/ast/static.h`: links on separately rendered type pages do not substitute
for that file's purpose links. Its file purpose now explains and links TPL-01,
TPL-07 and TPL-08 alongside COMP-02/TPL-12, matching the complete header boundary.
No validator was weakened; regenerate and validate every rendered destination.

T170 acceptance (2026-11-18): corrected `make docs` passes with 1765 maintained
file pages, 6765 documented interfaces, 417 canonical destinations and 5573 pages
under Doxygen 1.11.0/Python 3.14.4/markdown-it-py 3.0.0. The actual rendered-file
and destination gates pass; `make docs-complete` reports 2352 ready inputs and
zero debt. Direct source-path resolution checks every static-header rule link;
`code --goto /home/u/repos/ama/spec/language/templates.md:759` returns zero for
TPL-07 in the supported VS Code navigation setup (no GUI click claim). The full
record moves to [DONE](../../DONE.md#t170); regenerate once on these final archive
inputs before committing. No spec bytes or conformance fingerprints changed.
