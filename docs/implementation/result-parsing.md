# Direct result-operation trees and refusal ownership

T026 implements the structural tree in
[CALL-09](../../spec/language/calls-and-results.md#call-09). A selected result is
an operation target, not an ordinary output binding or a new caller variable.
Parsing records selector paths, exact modes, sibling order and handler ownership;
[T036](selected-operations.md) supplies result-interface typing; later tasks implement execution.

For example:

```text
mod1
 mod2 !
  > mod3
```

The root is a retained read of `mod1`. Its only output action selects `mod2` and
starts it from entry; that child transfers its first ordinary result to `mod3`.
There is no intermediate binding, declaration or copy-source node. A selector's
access spelling still retains reference/list/index structure. Syntax does not
choose between an owned result and a referenced target by inspecting a name.

## Ordered actions and independent binding modes

Every operation retains its ordinary input/output binding list for directional
positions, and a separate ordered output-action list. An action aliases either
one ordinary binding or one selected operation in the same AST arena. A selected
operation points to its enclosing operation. Its arguments and naming style are
independent; the selector changes neither the parent's naming mode nor its
ordinary result counter. Repeated selections remain distinct source occurrences
and may name the same stored result.

The explicit output-phase flag closes inputs after either kind of action. Thus
an input after a selection is invalid even when the ordinary output count is zero.
An unmarked retained read requires at least one output action, including a
selected child with no ordinary transfer at the parent level. Static constant
reads retain their separate `>>`-only structure and cannot own selected actions
or acquisition handlers.

An unmarked line followed by one indented copy source is an assignment. Explicit
`!`/`%`, angle bindings, or an unmarked access's deeper action region identify an
operation tree instead. A copy source has no child region; an unmarked selected
read requires one. These disjoint structural forms determine recognition without
callee lookup or trying alternative interpretations based on declared names.
Malformed structure fails rather than falling back to a different meaning.

## Exact colon ownership and caller context

A `:` belongs at its operation target's indentation, including inside a selected
tree. A colon at an ordinary output binding's level is not a handler for that
binding. The AST stores the exact refusal-marker span and the owning operation's
body, including an empty body. Parent-region comments may precede the colon;
a parent comment cannot reopen the operation's ended argument region.

Calls inside a refusal body are ordinary root operations in the initiating caller,
so their enclosing-operation pointer is null. A new tree started there creates
its own selector context. Ordinary argument sources, indices and receivers keep
the caller's lexical interpretation; only a selected target's base selects from
its enclosing operation's result interface. No synthetic local names are inserted.

## Explicit parsing frames

Body and call frames share one arena-owned parser stack. Starting an operation
advances past its head and pushes its argument frame. A frame processes one
ordinary binding or pushes one selected child. Finishing validates output
requirements, optionally pushes a normal body frame for its refusal handler,
and then returns to the enclosing frame. Completed child spans extend their
parent without recursively calling the parser. Handler-body roots never inherit
selector context from the parser stack itself.

At each iteration the physical line cursor advances, or a completed frame pops.
A push follows a consumed source head/marker. Lexicographic order on remaining
source lines and outstanding frame depth therefore proves termination on finite
input. Processing each line/action a bounded number of times gives linear work
up to XID interval searches, with arena storage proportional to syntax and frame
count. C stack depth is independent of source nesting. Back-pointers and aliased
binding/action views do not affect destruction: the arena releases allocations
once without traversing syntax ownership links.

## Evidence and remaining stages

`make -C tests/unit/parser/results` checks the shorthand, ordered mixed siblings,
independent modes/styles, repeated selectors, indexed/reference/list targets,
unmarked selected reads, whole-list method parents, exact colon depth, empty
handlers, ordinary roots inside handlers, input-phase closure and invalid static
read handlers. A 512-level selected path verifies exact parent links with no
recursive C activation. Every allocation of a mixed tree and refusal body is
failed once; partial modules stay invalid and all storage is released.

The ordinary 5856-form binding oracle and compiled execution suites still apply
to operations without selections. [T036 static selection](selected-operations.md)
now resolves the complete tree and links typed output actions. T044 supplies
[complete selected-action IR](structured-lowering.md). Static checking accepts
that complete plan; C generation explicitly rejects unavailable execution
adapters before staging artifacts. `make -C tests/unit/compiler/check` and
`make -C tests/unit/codegen/assembly` verify both boundaries. T029 supplies full
dependency traversal; T027 supplies condition-chain loops and foreach syntax.
Parser/static success remains separate from runtime adapter support.
