# Structural branches, loops and traversal headers

The parser implements the source forms in
[FLOW-01–FLOW-04](../../spec/language/control-flow.md),
[FLOW-05](../../spec/language/lists.md#flow-05) and
[SYN-01/SYN-02](../../spec/language/source-and-names.md#syn-01).
Names, nominal types, truth capabilities and iterator visibility remain semantic
checks. No existing declaration determines which grammar production is selected.

## Bodies and markers

A branch retains separate positive and negative blocks and their actual marker
spans. A negative-only `:` has no invented `?` marker. Empty blocks have null
statement endpoints. Loop and traversal nodes retain opening and closing spans.
Only comments at the condition's depth may separate it from its first marker;
a blank line or another-depth comment does not form that gap. Parent comments
close implicit bodies; they cannot reopen them. A loop close has no parent-comment
gap. Break checks walk the active body-frame chain, so only loop bodies supply
break scope. Return and suspension remain explicit standalone AST tags.

An owned indexed header `items[position]` remains a truth-controlled loop.
`items[item&]` is always a traversal binder, and `items&[item&]` marks a collection
of references. Collection and binder names are retained separately without a
fake indexed access. Arbitrary expressions, nested collection indices and
reference-list binders are rejected.

## One loop, one repeat root

```text
a
 b !
  condition
  {
   work !
  }
```

One loop node occupies the ordinary statement slot originally occupied by `a`.
Its `command` owns the original operation tree; `condition_owner` identifies `b`
and `condition` retains the terminal access. The parser checks every active link
for one successor, no ordinary outputs, and no module-path target. Entry inputs
remain on their link. Every operation frame is marked as part of this completed
header, preventing later sibling actions. Its ordinary acquisition handler still
belongs to that operation.

The terminal body uses ordinary body frames. Once it finishes, header handlers
are parsed in the incoming body context. Consequently, `^` in a header handler
cannot break the loop whose condition was refused. A nested loop in a handler
supplies its own body scope normally.

Folding copies the root's call payload into one arena-owned command node and
replaces the original statement with the loop. Direct child parent links are
updated to that command. Active frames continue to reference the same nested
operations; the root frame is redirected to the command. No temporary variable,
copy operation or recursively parsed loop is introduced.

## Empty bodies and execution boundary

Closing each written body records one `empty-body` warning if it has no parsed
statements. Declarations count, including declarations that lower to no runtime
instructions. Warning records borrow source spans and are arena-owned. They are
inserted in physical marker order, including nested empty handlers; no optional
unwritten body or empty module implementation warns. Allocation failure while
recording a required warning fails parsing instead of silently losing it.

The driver renders warnings from complete parsed modules before destroying
sources. Warnings do not change successful compilation to failure, but failure
to write a warning prevents successful output acknowledgement. The diagnostic
code fixes its severity independently of message prose.

Existing owned-condition execution now directs both successful truth outcomes
to the exit for a parsed-empty loop. It still executes the checked access and
native conversion. A declaration-only body retains the ordinary backedge.
Negative-only branches use the existing negative successor. New condition-chain
and traversal forms now have complete [T044 control IR](structured-lowering.md).
Their C execution remains guarded before artifact staging until the corresponding
runtime adapters are implemented; a parsed tree alone is not runtime support.

## Exactness, ownership and checks

Body/operation frames are arena-owned and iterated, not recursive C activations.
Each successful step consumes physical input, changes a body phase once or pops
a finite frame. Folding visits a finite ancestor chain once per loop; nested
ordinary loop bodies have independent chains. Break-scope checks take frame-depth
time; sorted warning insertion takes O(w²) total in the worst case for w warnings.
Storage is O(source size), with all partial nodes released by module destruction.

The independent leaf `make -C tests/unit/parser/control` checks branch forms,
written/omitted markers, warning locations/order, declaration emptiness, nested
repeat roots, header handlers, traversal/index distinctions, malformed nearby
forms, 256-level bodies and every allocation failure. Compiled execution tests
in `tests/unit/codegen/execution` observe native truth-conversion effects: empty
true and false loops check once; declaration-only bodies repeat. Driver tests
exercise nonfatal warning output and `/dev/full` output failure. Per-file evidence
is registered in `documentation/ready.json`.
