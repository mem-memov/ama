# Declarations, sections and shared lexical recognition

T023 connects the [exact lexical boundary](lexical-tokens.md) to the source AST
under [SYN-03/SYN-04](../../spec/language/source-and-names.md#syn-03),
[TYPE-01](../../spec/language/declarations.md#type-01) and the
[canonical grammar](../../spec/grammar.md). It represents syntax and original
spans; it does not construct instances, infer missing types or bind names.

## One recognition path

`parser_module_classify` checks every line with the common token scanner before
publishing an activation decision. UTF-8, TAB and physical endings were checked
by the reader. Code spacing, compound boundaries and annotation shape apply to
drafts too. An annotation is a standalone zero-indent token. Only genuine `@=`,
`@!` and `@C` activate the source; `@<`/`@>` alone do not. Comments and accepted
literal payloads never contribute activation tokens. Invalid input changes no
classification output. The loader can reuse this same boundary in T029.

A lexically valid draft may retain unfinished structure such as `unfinished[`;
its declarations and bodies are not parsed. This does not permit unknown code
punctuation, repeated/trailing code spaces, indented annotations or TAB. Active
sources receive the fixed ordered section pass and AST parsing. Native bodies
accept only zero-indent layout after `@C`; module-level leading comments likewise
cannot invent a child region.

`lex_name` is the shared maximal-name operation used both by token recognition
and access/declaration parsing. It combines valid XID words with one internal
space and stops before punctuation or an external separator. It never tries a
shorter previously declared spelling. Project and Standard type paths consume
one or two initial dots followed by complete dot-adjoining components. Neither
root-only paths nor empty components are repaired. Qualified constants must still
have a member after the module path; `..integer` alone is not a constant source.

## Declaration and initializer regions

All four storage forms use the same syntax node: `name`, `name&`, `name[]` and
`name&[]`. The type follows on its own line at the declaration depth. Initializers
are one level deeper, and retain source order and exact literal/copy spans.
Scalar initializer limits and constructor capability checks remain semantic work.

For example, this fragment has two declarations and one explicit initializer:

```text
@!
item state
# Same declaration region before its type
..integer
 # Initializer region
 |0
# Parent comment closes that initializer region
other state
..integer
```

The region cursor skips blank lines and only comments at the region's depth.
A different-depth comment remains for the enclosing grammar or an error. Thus
putting ` |1` after the final parent comment cannot reopen the first declaration's
initializer block. A deeper comment before the type line is not an allowed type
gap, and a comment cannot invent a skipped initializer level. Body frames and
binding regions use this explicit layout boundary too.
[Result trees](result-parsing.md) and [condition/foreach bodies](control-parsing.md)
compose it through their own structural frames.

The AST owner publishes `valid` only after required stages succeed. Partial nodes
and physical lines remain owned on failure, and destruction releases them before
source storage. Parsing never executes native code. Body nesting remains an
explicit frame chain rather than C recursion.

## Why recognition and parsing terminate

The classifier advances through each finite line and token; each successful token
consumes original code bytes. Its activation flag is private until EOF succeeds.
The section pass permits strictly increasing section ranks and constructs disjoint
half-open intervals. A declaration consumes a nonempty name and type line before
advancing through a finite initializer sequence. Region skipping always advances
a physical line and never manufactures a region. Shared names have the lexical
maximality proof, so no declaration inventory or name-split search is needed.
Work is linear in visited source bytes up to pinned XID interval searches; AST and
layout storage are proportional to the represented input, with explicit heap
frames for nested bodies.

## Executable evidence

`make -C tests/unit/parser/declarations` checks all four storage forms, multiword
names, Standard type roots, constant/parameter/result/body sections, ordered
initializers, LF/CRLF/unterminated EOF, exact comment regions and forbidden
reopening. It checks lexically unfinished drafts separately from invalid spacing
and annotations, unchanged classification output on failure, and every allocation
of a composed declaration parse. Existing tree tests cover deep nested bodies.
The shared bounded-name API has an independent token-leaf output-preservation case.
`make -C tests/unit/compiler/check` carries multiword declarations through full
source/semantic/planning composition with `.value` and `..value` simultaneously;
their nominal types remain distinct and no C companion is executed.

Existing positive call/copy fixtures now use the required spaces before execution
suffixes and after `%`. A temporary syntax migration used successful AST source
positions to insert those separators, preserving literal/comment bytes; explicit
fragment fixtures and negative-prefix cases were reviewed separately. Real
compiled execution tests retain their prior observable results. Detailed new
[assignment/inference sources](source-parsing.md), [call/result trees](result-parsing.md)
and [control grammar](control-parsing.md) have their own structural composition
evidence rather than being inferred from this declaration-only test.
