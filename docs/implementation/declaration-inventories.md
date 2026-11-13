# Complete explicit declarations and interface lookup

This is the T030 boundary for [NAME-04/NAME-06](../../spec/language/declarations.md#name-04),
[TYPE-01](../../spec/language/declarations.md#type-01) and declared interface
metadata under [CALL-08](../../spec/language/calls-and-results.md#call-08).
It supplies the explicit inventory consumed by [source-order receiver inference
and foreach classification](source-order-inference.md). It does not infer variables or claim new
list/result-chain execution support.

## One inventory, independent of execution

`semantic_names` borrows a complete loaded project and creates one semantic type
per exact module identity. Every explicit declaration in constants, inputs,
results and persistent body storage participates, including declarations inside
branches, loops, selected-operation handlers and traversal bodies. A foreach
binder is not an explicit module declaration and is not added by this stage.
Branch outcomes, list contents and runtime call stacks cannot change this set.

A symbol retains its original declaration, owner, role, full module ordinal,
direction-specific ordinal, reference/list flags and later-linked nominal type.
The complete base name is the key: its words are indivisible and its storage
suffix does not create another namespace. Exact byte comparison preserves case
and Unicode spelling distinctions. Equal names in different modules remain
independent. A duplicate error names the later declaration and retains the first
one as related context, including their original storage forms.

After all module inventories exist, nominal linking resolves complete Project or
Standard paths to those exact semantic types. `.value` and `..value` remain
distinct even when their component spellings agree. Constants-only namespaces
cannot be used as executable value types. Looking up a type creates no instance.

## Source order across structural owners

[Collection](../../src/semantic/walk/collect.c) follows only owning AST edges.
It includes selected actions, operation handlers, folded loop commands and all
ordinary/traversal bodies; parent-operation, terminal-owner and ordinary-binding
alias links do not schedule extra visits. Every event retains its incoming loop
depth. A loop/foreach increments depth only for its body. Header commands and
their handlers retain the containing body's depth.

[Stable sorting](../../src/semantic/walk/sort.c) orders collected events by
original byte position before [callbacks](../../src/semantic/walk/blocks.c) run.
This matters for a declaration in a condition-loop body followed physically by a
header handler: the AST owns that handler through the command tree, but its
position comes after the body. Collection order alone must not assign the handler
an earlier field ordinal. Stable equal-position order keeps a loop owner before
its same-site command node.

Each event is arena-owned. Collection or visitor failure leaves one destructible
semantic owner. A callback failure stops further callbacks. The walker performs
no recursive C descent, native execution or source mutation. These are node-start
positions, not a serialized stream of every operand. The inference pass creates
its own binding/operand events at their written positions, rather than scanning
a parent call's entire binding list before selected children.

## Shared interface metadata

`semantic_interface_name` and `semantic_interface_position` are the shared
allocation-free lookup boundary. Both select only the requested constant/input/
result role. Named lookup uses complete base-name bytes; positional lookup uses
that role's declaration ordinal. State fields and wrong roles do not match.
After nominal linking, both return the same field with its exact type and
reference/list flags. Binding labels still undergo storage-form checks separately.
Omitted bindings consume their normal declared positions; selectors do not add
new declared result ordinals.

Call checking and source-order inference consume these functions. Later result
checking shares that metadata rather than inventing another label/position map.
Named binding diagnostics still retain a known wrong-role declaration as context
without accepting it as an interface field.

## Visibility and initialization remain different checks

Explicit names are visible throughout their module, including earlier executable
text and other bodies. The complete inventory enables an ordinary assignment to
use a later explicit source field, and an ordinary call can use a later explicit
target. This does not infer a new variable or permit forward
initialization: a creation-time initializer still requires an earlier initialized
source under the existing initializer-order check. A constant initializer still
has its own constant-only restrictions. Tests distinguish successful earlier
body uses from a failing initializer naming that same later explicit declaration.

## Exactness and bounds

Every collected node is reached through exactly one owning edge. Finite explicit
work frames visit it once; alias edges are excluded. Induction over the structural
tree therefore gives the complete event multiset with correct lexical depth.
Bottom-up merge passes preserve that multiset and stable equal-position order,
and double the bound on sorted runs until one covers the list. The checked event
count and final half-count width test prevent counter wrap.

Induction over source-ordered declarations then establishes that the symbol table
contains exactly the visited declaration prefix with unique full base names and
correct module/role ordinals. Exact lookup either finds the unique earlier name
and diagnoses a duplicate, or proves adding the new name preserves uniqueness.
Nominal lookup uses the already unique module registry, so type equality is
identity equality rather than spelling-based conversion.

For N structural nodes, D declarations and maximum name length L, inventory work
is O(N log N + D² L), with O(N + D) semantic storage. Interface lookup is O(D L)
by name and O(D) by ordinal, with no allocation. Faster indexes may preserve these
same contracts; no normalization, fallback namespace or guessed field is used.

## Evidence

`make -C tests/unit/semantic/names` checks all 256 section/storage duplicate
pairings, the body-before-header-handler diagnostic, complete inventories in both
root families, exact reference/list metadata, case/Unicode byte distinctions,
source-ordered visits and header-handler loop depths. Stable-sort tests cover
empty/singleton, equal-position and power-of-two boundaries through 17 events,
with multiset/tail/stability laws. Every actual expanded inventory allocation is
failed once and counted owners return to zero.

`make -C tests/unit/semantic/types` distinguishes later explicit body uses from
forward creation-time initialization. Existing nominal/storage and binding suites
exercise the metadata through real typed call checking. Per-file named commands,
including standalone header checks, are in `documentation/ready.json`.
