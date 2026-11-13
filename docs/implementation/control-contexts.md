# Exact control contexts

T038 completes static control metadata under
[FLOW-01](../../spec/language/control-flow.md#flow-01),
[FLOW-02](../../spec/language/control-flow.md#flow-02),
[FLOW-03](../../spec/language/control-flow.md#flow-03),
[FLOW-04](../../spec/language/control-flow.md#flow-04) and
[FLOW-05](../../spec/language/lists.md#flow-05).
New command-header loops and foreach execution still require later IR/runtime
frames. They are explicitly rejected in planning after successful static analysis.

## Carry the actual lexical loop

The [structural walker](../../src/semantic/walk/collect.c) carries both loop depth
and the exact nearest loop/foreach AST identity. It changes that context only when
entering an actual body. Header operations and their refusal bodies keep the
incoming context; a selected call itself introduces no loop. Branch bodies and
alternatives preserve their incoming loop context.

Sorting visits by physical source position preserves this already determined
context. Source order does not guess which loop is active. The loop wrapper is
visited before its same-site command, so all references to an enclosing loop find
an already retained semantic statement. Each statement records `enclosing_loop`;
for `^` that is the exact exit target. A handler belonging to a new loop's header
can exit an outer loop but cannot exit that new loop whose body has not begun.
Parser and semantic checks reject exits outside an actual enclosing body.

For example, assume typed `outer`, `root`, and `root`'s declared child/result
interface:

```text
outer
{
 root !
  child !
   condition
   {
    ^
   }
  :
   ^
}
```

The body exit belongs to the inner command-header loop. The handler exit belongs
to `outer`. Its colon aligns with `child !`; the handler is one level deeper.
The compiler records these identities even before execution lowering exists.

## Resolve the whole header before its terminal predicate

A loop wrapper starts at its first root line, physically before the selected
operations inside its own header. [Statement resolution](../../src/semantic/resolve/statements.c)
first checks the complete source-ordered operation/binding inventory and links
output actions. [Deferred control resolution](../../src/semantic/control/resolve.c)
then resolves predicates against those checked operation records. This does not
run a call twice or execute a native decoder: both passes construct metadata only.

[Result selection](../../src/semantic/selection/result.c) is shared by direct
operation targets and terminal conditions. It selects only the immediate parent's
result interface. Index operands stay in the initiating caller. Ordinary result
conditions retain their original field/access; an intrinsic length result retains
its exact integer type and result ordinal without a fabricated field or access.
A caller variable named `condition` cannot replace a parent's declared result.

[Predicate resolution](../../src/semantic/control/condition.c) records both the
terminal `condition_owner` and the complete `loop_root`. The latter points to the
first header operation, not the nearest operation before `{`. A simple named/indexed
loop has itself as its condition root. Every nested loop retains its own root and
lexical parent independently. These are typed source descriptors, not cached
runtime targets, indices or condition values.

[Conversion checking](../../src/semantic/values/conversion.c) requires an owned
scalar and its own native condition/index ABI capability. A reference does not
implicitly dereference and an enclosing Ama value does not inherit its child's
native capability. Failures retain the real source/declaration when present and
the complete nominal type, including the standard root for intrinsic values.

## Traverse by a reference role, not an index

[Traversal completion](../../src/semantic/control/traversal.c) reuses T031's
exact syntax-to-collection/binder role. The collection is mutable list storage;
the iterator is a compatible scalar reference, either an existing visible cell
or the introduced foreach-local cell. It does not become a persistent field.
Nested compatible binders reuse the enclosing cell; disjoint scopes retain their
own identities. A compatible explicit field declared later is still reused.

The body is fully checked even when its source list starts empty. The role is
compile-time metadata, not a runtime allocation of a new cell for an empty list.
All body references and escapes retain the selected cell identity and existing
ordinary transfer rules. A later field inferred outside the scope cannot change
earlier local bindings. An indexed truth loop such as `items[index]` retains an
indexed condition and has no traversal role; `items[item&]` is foreach structurally.

## Emptiness and branch polarity stay syntactic

A negative-only `:` branch has an alternative marker and no positive marker.
Its `negative_branch` metadata records that exact parsed form. Ordinary positive
and two-body branches retain normal polarity. Empty bodies remain legal and
carry their parser warnings; native predicate capability is still checked.

`empty_body` comes from the corresponding AST block. Comments/blank lines do
not add statements; a declaration does, even if it emits no body instruction.
For ordinary/command-header loops, this information implements FLOW-02's single
successful test on entry to an empty body. Header inputs and refusal-body work
cannot make the loop body nonempty. Foreach has its separate FLOW-05 traversal
algorithm; its empty-body flag is not an instruction to apply FLOW-02 to a scan.
No literal-byte truth guess, current list length or emitted-code size skips checks.

## Publication, bounds and execution boundary

Successful statement resolution requires complete typed operations, ordered
actions and all predicate/traversal roles. Control records retain exact AST/symbol
identities borrowing the loaded project; semantic arena ownership covers every
partial allocation. Only successful completion publishes `resolved`.

Structural collection/sorting costs O(N log N) time and O(N) storage for N nodes.
Exact statement-identity lookup adds O(N²) in the general case, plus interface/
access resolution. Iterator role lookup is finite over the module's actual
foreach roles. No source nesting uses the C call stack and no depth threshold,
previous-line heuristic or dynamic caller-chain lookup chooses a root or exit.

The [pre-storage control guard](../../src/planning/validation/control.c) rejects
new header-loop/foreach execution until T042/T044 and the runtime tasks provide
explicit frames, restart/protection edges and live traversal boundaries. Existing
ordinary branch/loop lowering remains usable, including T027's empty-loop rule.
All valid control syntax is typed before the new execution guard; unsupported
execution cannot masquerade as successful partial emission.

## Evidence

Run `make -C tests/unit/semantic/control` and `make -C tests/unit/semantic/names`.
The suites inspect exact root/terminal/outer-loop identities, all nine header mode
pairs, caller-owned terminal indices and intrinsic terminal values. Handler exits
keep outer targets, including around nested foreach bodies. They distinguish
indexing from traversal, check nested/disjoint/reused iterator cells and body
reference escapes, preserve negative polarity and four exact empty-body warnings,
and reject bad names/types even inside initially empty/default-zero paths.

Every semantic allocation in a complete nested header/handler/traversal project is
failed independently. The walker tests independently verify its carried loop
identity/depth through stable sorting and callback early stop. Frontend tests
require successful semantic/constant analysis followed by rejection before storage
publication. Full strict and sanitizer suites cover composition; these checks do
not certify the pending execution of header loops or changing-list traversals.
