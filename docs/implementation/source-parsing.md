# Sources and uniform receiver assignments

T024 implements the source and receiver syntax from
[REF-01/REF-03](../../spec/language/values-and-references.md#ref-01),
[CONT-04](../../spec/language/continuations.md#cont-04) and the
[access/source grammar](../../spec/grammar.md#accesses-and-sources).
The parser records exact syntax before type or declaration lookup.

An assignment always has one receiver and one indented source. These two fragments
have the same AST shape, regardless of whether either receiver was declared:

```text
existing target
 source value
new target
 source value
```

The AST does not label the second as a declaration by guessing its spelling.
T031 will apply sequential visibility and first-use inference to the same
assignment nodes. Indexed receivers preserve their complete access/index trees;
they do not become inferred base declarations. A source stays in its receiver's
child region. Correctly indented comments may precede it; a parent comment closes
that region and cannot be skipped to reopen it later.

## Distinct source modes

Source publication follows disjoint explicit forms:

- A permitted `|` marker retains a raw literal payload, including an empty one.
  Assignment does not permit this form; a declaration initializer or entry input
  supplies the receiving type for literal construction.
- Leading `.`/`..` selects a qualified constant and its optional compact `[]`.
  Splitting uses the final component dot after a real module component; a root dot
  never supplies the missing member. Complete names remain unsplit across spaces.
- `& item state` acquires a reference to the owned access `item state`; the
  ampersand adjoins its first word. Reference
  slots and whole lists cannot be acquired as owned scalar storage.
- `% source` retains the explicit continuation-copy tag and requires a scalar
  owned/reference operand. Whole lists and another source prefix are not operands.
- Ordinary named/reference/list/indexed accesses remain value-source nodes.
- A lone `&` is a self-reference source with its original one-byte span. It has
  no fabricated variable name, access node or implicit default module instance.

For example, `< |&` is literal data containing an ampersand, while `< &` carries a
self-reference source. The parser does not confuse those forms. A lone `&` cannot
be a receiver, invocation target, condition, index or `%` operand. Shared-constant
and nominal storage legality are semantic decisions, completed in T032.

Qualified splitting, instance operand validation and source publication are
separate small operations under `parser/access/source/`. The public source output
changes only after its entire form succeeds. Partial allocations remain AST-owned,
and diagnostics retain original spans and fixed expected constructs. Literal
assignment errors point to the original literal and explain the typed initializer
alternative; no name inventory or inferred literal type influences parsing.

## Correctness and evidence

The complete-name scanner and compact punctuation give unique access boundaries.
Each nested index adds one nonempty owned access and one closing bracket; the
iterative access parser tracks outstanding closes without recursive C calls.
Source forms use distinct leading punctuation, with the exact lone ampersand
separated before reference-acquisition parsing. Thus syntax alone selects a source
kind. Region traversal advances physical lines and cannot consume a parent comment
as child layout. Work is linear in represented bytes/nesting, with AST-owned nodes
and constant auxiliary parsing state apart from explicit body frames.

`make -C tests/unit/parser/sources` checks uniform known/unknown receiver shapes,
recursive multiword indices, qualified lists in both root families, reference and
continuation modes, self-reference source positions, literal/self distinction,
forbidden contexts, exact literal-assignment sites, parent-comment reopening and
every allocation failure of a composed self-reference assignment. The existing
access leaf independently exhausts all short `a&[]` strings against its grammar
model and checks Unicode/nested original spans.

[T032 static transfer typing](typed-transfers.md) now gives self its lexical
module identity without inventing a named access.
[Construction](ordered-construction.md), [staged replacement](staged-replacement.md)
and [entry preparation](entry-preparation.md) implement final-identity formation
and clone remapping. T044 supplies complete initializer/assignment/input plans;
current generated-project tests execute those checked self sources.
