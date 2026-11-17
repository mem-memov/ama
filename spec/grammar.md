# Ama source grammar

[Specification index](README.md)

<!-- chapter-contents -->
In this chapter:

- [Reading this grammar](#reading-this-grammar)
- [Physical lines, comments, and spacing](#physical-lines-comments-and-spacing)
- [Names, paths, and storage forms](#names-paths-and-storage-forms)
- [Literal data](#literal-data)
- [Module files and declarations](#module-files-and-declarations)
- [Static template syntax and design boundaries](#static-template-syntax-and-design-boundaries)
- [Executable statements and structural recognition](#executable-statements-and-structural-recognition)
- [Invocation, retained reads, and result selection](#invocation-retained-reads-and-result-selection)
- [Branches, loops, completion, and suspension](#branches-loops-completion-and-suspension)
- [Native protocol companion syntax](#native-protocol-companion-syntax)

<!-- /chapter-contents -->

This reference defines the source forms and the contextual distinctions needed
to parse them. The [language chapters](README.md) define what a valid program
does. Parsing precedes name binding and type checking: a recognizable form can
still fail because its operand has the wrong type, storage form, or capability.

Ama composes module-defined operations without a general expression evaluator.
Most operation names belong to module interfaces. The six built-in list methods
are an explicit fixed interface described in [Working with lists](language/lists.md).

## Reading this grammar

This document uses a **project-specific, indentation-parameterized EBNF
(Extended Backus–Naur Form)**. Its grouping, optionality, and repetition operators
follow the Wirth-style convention. Angle-bracketed names, `::=`, special
sequences, and indentation parameters are the conventions defined below; this
is not a claim of conformance to ISO/IEC 14977, and this notation is not ABNF.

| Notation | Meaning |
|---|---|
| `<name>` | Nonterminal defined by another production |
| `::=` | Defines the nonterminal on the left |
| `"x"` | Exact terminal text, including any spaces inside the quotes |
| `x y` | Concatenation: recognize `x`, then `y` |
| `x \| y` | Alternative: recognize either `x` or `y` |
| `( x )` | Grouping |
| `[ x ]` | Optional part: zero or one occurrence |
| `{ x }` | Repetition: zero or more occurrences |
| `? ... ?` | Special sequence whose lexical or contextual predicate is stated in words |
| `<rule(n)>` | A family of nonterminals parameterized by indentation depth |

Repetition and optionality apply to their enclosed groups. Concatenation binds
more tightly than alternatives; parentheses override that grouping. Metasyntax
outside quoted terminals contributes no source characters. In particular, the
spaces used to align these productions are document formatting, whereas `" "`
and `<space>` explicitly recognize a source space. All definition operators
share one column; continuation lines align with the right-hand side, with
additional indentation for nested groups.

The indentation parameter `n` is a nonnegative integer. `<statement(n)>` starts
at indentation `n`; `<indent(n)>` means exactly `n` ASCII spaces. `n+1` denotes
the next nesting level, and `0` denotes the outermost level. Parameters are
metanotation, not Ama source text: for any fixed depth they expand to ordinary
productions with that exact indentation. They do not infer names, types, or
nesting from declarations.

Source whitespace is explicit grammar and is never freely skipped. Recognition
consumes the whole file. Layout can have more than one grammatical attachment
but contributes no declarations, operands, or control nodes. The constraints
stated after production blocks are part of this grammar; name binding, type
compatibility, and execution behavior remain defined in the language chapters.

## Physical lines, comments, and spacing

```ebnf
<newline>                    ::= ? physical LF, physical CRLF, or the final virtual LF boundary ?
<space>                      ::= " "
<separator>                  ::= <space>
<indent(n)>                  ::= ? exactly n ASCII spaces ?
<comment>                    ::= "#" { ? any Unicode scalar except TAB, CR, or LF ? }
<comment-suffix>             ::= { <space> } <comment>
<line-end>                   ::= [ <comment-suffix> ] <newline>
<blank-line>                 ::= <newline>
<comment-line(n)>            ::= <indent(n)> <comment> <newline>
<layout(n)>                  ::= { <blank-line> | <comment-line(n)> }
<comment-gap(n)>             ::= { <comment-line(n)> }
```

The lexical rules are specified in
[Source files, names, and layout](language/source-and-names.md). The following
constraints also apply to every production below:

- Decode strict UTF-8. Reject TAB everywhere, including drafts, literal payloads,
  comments, and `.abi` companions. LF and CRLF are line endings; a lone CR is invalid.
- A nonempty final physical line without an ending has one virtual LF boundary
  for recognition only. It adds no source/payload bytes. An empty file stays empty.
- Code separators are exactly one U+0020. Repeated spaces are permitted only in
  structural indentation, literal payloads, comment text, and trailing-comment
  prefixes. A blank line contains no spaces. Code has no trailing spaces unless
  a trailing comment follows them.
- Recognize a literal in its permitted position before interpreting `#`.
  Otherwise the first `#` and the maximal preceding run of spaces form a trailing
  comment suffix. A comment-only line instead requires exactly its containing
  region's indentation; it cannot invent a deeper region.
- Recognize the longest fixed compound token: `@=`, `@<`, `@>`, `@!`, `@C`, `>>`,
  `[]`, `&[]`, `&[`, `...`, and `..`. Their characters cannot be separated by
  spaces. At a type start recognize `...` before `..` and `.`.

`<comment-gap(n)>` permits comments at the condition/marker indentation but no
blank lines. `<layout(n)>` permits both blank lines and comments at that level.
Neither a blank line nor a comment delimits a section.

### Compact and separated punctuation

| Direct joins, without spaces | Separate tokens, with one space |
|---|---|
| `&item`, `target&` | `target& !`, `target& %` |
| `items[]`, `targets&[]` | `% target&` |
| `items[current position]` | `input& < &item` |
| `targets&[positions[next position]]` | `output > receiving value` |
| `.shop tools.item state`, `..integer` | `< ..palette.red` |

Structural indentation and an operand separator before a leading `.` or `&`
remain outside that path/access. Storage suffixes, prefix `&`, path dots, and
bracket joins do not admit optional whitespace. Standalone markers have no
trailing separator. `|` immediately precedes its raw payload: a space after it
is data.

## Names, paths, and storage forms

### Complete names and paths

```ebnf
<identifier-start>           ::= "_" | ? Unicode scalar with property XID_Start ?
<identifier-continue>        ::= "_" | ? Unicode scalar with property XID_Continue ?
<name-word>                  ::= <identifier-start> { <identifier-continue> }
<identifier>                 ::= <name-word> { <space> <name-word> }
<module-root>                ::= "." | ".."
<module-path>                ::= <module-root> <identifier> { "." <identifier> }
<placeholder-type>           ::= "..." <identifier> { "." <identifier> }
<type-name>                  ::= <module-path> | <placeholder-type>
```

Use Unicode 17.0.0 identifier properties. Names are case-sensitive and compared
byte-for-byte without normalization or transliteration. A complete name cannot
begin/end with a space and contains exactly one space between nonempty words.
Every word obeys its own start rule: `item step2` is valid; `item 2` is invalid.

In a name position, consume the maximal complete name up to punctuation or the
code-part boundary. A space alone cannot separate two operands. Never consult
declarations to split a name or repair its spelling.

Each path component is one complete name. The leading `.` selects project/import
roots, while `..` selects the standard-library root. These are absolute logical
root markers, not filesystem traversal. Internal separators are single adjoining
dots: `.shop tools.item state` contains two components. Empty components, bare
roots, trailing dots, internal doubled dots, and three leading dots are invalid
in a concrete module path. A `<placeholder-type>` instead starts with exactly
three dots and uses the same component spelling. Its whole name denotes one
scoped static parameter, result or alias under
[TPL-01/TPL-02](language/templates.md#tpl-01) and
[TPL-12](language/static-results.md#tpl-12), not a third filesystem root. It has
no trailing dot and does not select a member. Bare `...` is recognized only as
the positional static-binding marker below, never as a placeholder name or path.
Physical `name+/` submodule directories add no source syntax; their exact mapping
is defined by SRC-05 in [Source files and paths](language/source-and-names.md).

### Declaration names

```ebnf
<owned-name>                 ::= <identifier>
<reference-name>             ::= <identifier> "&"
<owned-list-name>            ::= <identifier> "[]"
<reference-list-name>        ::= <identifier> "&[]"
<declaration-name>           ::= <owned-name> | <reference-name>
                                 | <owned-list-name> | <reference-list-name>
```

| Form | Declared storage |
|---|---|
| `item` | One owned instance/value |
| `target&` | One typed non-owning reference |
| `items[]` | A list owning its elements |
| `targets&[]` | A list owning reference slots |

The type line carries no storage suffix. The four forms share their
base-name namespace; adding a suffix cannot declare a second `item` beside an
existing `item`. [Declarations and scope](language/declarations.md) define
visibility, receiver inference, and the non-shadowing rule for foreach locals.

### Accesses and sources

```ebnf
<index>                      ::= <owned-access>
<owned-access>               ::= <identifier>
                                 | <identifier> "[" <index> "]"
<reference-access>           ::= <identifier> "&"
                                 | <identifier> "&[" <index> "]"
<reference-acquisition>      ::= "&" <owned-access>
<self-reference>             ::= "&"
<constant-access>            ::= <module-path> "."
                                 ( <owned-name> | <owned-list-name> )
<whole-list-access>          ::= <owned-list-name> | <reference-list-name>
<continuation-copy>          ::= "%" <space> ( <owned-access> | <reference-access> )
<value-source>               ::= <owned-access> | <reference-access>
                                 | <reference-acquisition> | <self-reference>
                                 | <constant-access>
<copy-source>                ::= <value-source> | <whole-list-access>
                                 | <continuation-copy>
<indexed-receiver>           ::= <identifier> "[" <index> "]"
                                 | <identifier> "&[" <index> "]"
<receiver>                   ::= <declaration-name> | <indexed-receiver>
<instance-target>            ::= <owned-access> | <reference-access>
<call-target>                ::= <instance-target> | <whole-list-access>
```

`&item` forms a reference to owned storage; `target&` reads reference storage.
`&target&` is invalid. A lone `&` is a source for the instance lexically containing
it. It is not a receiver, operation target, condition, index, or `%` operand.
Shared constant initialization cannot use it. See
[Values and references](language/values-and-references.md).

An index is a named owned scalar access, possibly indexed recursively, whose
type supplies native `[]`. It is not a literal or arithmetic expression.
`items[position]` selects an owned element; `targets&[position]` selects a
reference slot. Indexing does not itself execute the selected target.

In a qualified constant source, the final path component is the constant name;
the preceding components name its module. At least one module component and one
constant-name component must follow the root marker. `.catalog.item states[]`
is a constant list source. No longest-prefix lookup or alternate path split is
permitted. A path in a type line names a type instead. This direct source form
does not accept a placeholder or a static binding block; use the explicit
constant-read namespace form below to select a specialized type. In particular,
`...type.member` remains one complete placeholder, not qualified constant access.
Constants cannot be receivers, mutable-reference sources, execution targets, or
continuation-copy operands. See [Constants](language/declarations.md).

`% owned` and `% target&` are explicit continuation-copy sources for an owned
scalar destination. Suffix `%` on an operation and standalone `%` are different
forms. Storage compatibility, copy capabilities, ownership overlap, lifetime,
and internal-reference remapping are specified in
[Values and references](language/values-and-references.md) and
[Continuations](language/continuations.md); they are checked after recognition.

## Literal data

```ebnf
<literal-payload>            ::= { ? any Unicode scalar except TAB, CR, or LF ? }
<literal-line(n)>            ::= <indent(n)> "|" <literal-payload> <newline>
```

There are two literal positions: a declaration initializer beginning with `|`
and an entry-call input beginning with `|` after `<` and its separator.
Everything after `|` through the physical line boundary is payload, including
repeated/trailing spaces, `#`, quotes, backslashes, and further bars. The marker
and line boundary are excluded; no trailing comment is recognized in payload.
TAB remains globally invalid.

```text
message
..text
 |He said: "Hello!" # this is data
```

`|` supplies empty bytes; `||text` begins with a literal bar. There are no quotes,
escapes, multiline literals, interpolation, or numeric-literal expressions.
`255` cannot replace a source; `|255` supplies bytes to the receiving type's
literal constructor. The receiving type's capability and construction behavior
are defined in [Declarations and scope](language/declarations.md).

## Module files and declarations

```ebnf
<module-file>                ::= <draft-file> | <complete-module-file>
<draft-file>                 ::= ? well-formed UTF-8 source without a genuine
                                   zero-indent @=, @!, or @C activation marker ?
<complete-module-file>       ::= <executable-module-file> | <constants-only-file>
<executable-module-file>     ::= <layout(0)>
                                 [ <constants-section> ]
                                 [ <parameters-section> ]
                                 [ <results-section> ]
                                 <implementation-section>
<constants-only-file>        ::= <layout(0)> <constants-section>
<constants-section>          ::= "@=" <line-end> <declarations(0)>
<parameters-section>         ::= "@<" <line-end> <input-declarations(0)>
<results-section>            ::= "@>" <line-end> <result-declarations(0)>
<implementation-section>     ::= "@!" <line-end> <body(0)>
                                 | "@C" <line-end> <layout(0)>
<declarations(n)>            ::= <layout(n)> { <declaration(n)> <layout(n)> }
<body(n)>                    ::= <layout(n)> { <statement(n)> <layout(n)> }
<declaration(n)>             ::= <indent(n)> <declaration-name> <line-end>
                                 <layout(n)>
                                 <indent(n)> <static-expression(n)>
                                 <initializers(n+1)>
<input-declarations(n)>      ::= <layout(n)>
                                 { ( <input-declaration(n)> | <static-formal(n)> )
                                   <layout(n)> }
<input-declaration(n)>       ::= <indent(n)> <declaration-name> <line-end>
                                 <layout(n)>
                                 <indent(n)> <static-expression(n)>
                                 <initializers(n+1)>
<static-formal(n)>           ::= <indent(n)> <placeholder-type> <line-end>
                                 [ <template-signature(n+1)> ]
<template-signature(n)>      ::= <layout(n)> <static-formal(n)> <layout(n)>
                                 { <static-formal(n)> <layout(n)> }
<result-declarations(n)>     ::= <layout(n)>
                                 { ( <declaration(n)> | <static-result(n)> ) <layout(n)> }
<static-result(n)>           ::= <indent(n)> <placeholder-type> <line-end>
                                 <layout(n)> <indent(n)> <static-expression(n)>
<static-expression(n)>       ::= <type-name> <line-end> <static-inputs(n+1)>
                                 [ <static-outputs(n+1)> ]
<static-inputs(n)>           ::= <layout(n)> { <static-input(n)> <layout(n)> }
<static-outputs(n)>          ::= <static-output(n)> <layout(n)>
                                 { <static-output(n)> <layout(n)> }
<static-input(n)>            ::= <indent(n)> <placeholder-type> <space> "<" <space>
                                 <static-expression(n)>
                                 | <indent(n)> "..." <space> "<" <line-end>
                                 | <indent(n)> "..." <space> "<" <space>
                                   <static-expression(n)>
<static-output(n)>           ::= <indent(n)> <placeholder-type> <space> ">" <space>
                                 <placeholder-type> <line-end>
                                 | <indent(n)> "..." <space> ">"
                                   [ <space> <placeholder-type> ] <line-end>
<initializers(n)>            ::= <layout(n)> { <initializer(n)> <layout(n)> }
<initializer(n)>             ::= <literal-line(n)>
                                 | <indent(n)> <copy-source> <line-end>
```

| Annotation | Section |
|---|---|
| `@=` | Shared immutable constants |
| `@<` | Runtime inputs and static Type/Template formals |
| `@>` | Runtime results and static Type/Template result definitions |
| `@!` | Ama implementation |
| `@C` | Companion C implementation |

Annotations occupy zero-indent lines in the displayed order, at most once each.
Only layout follows `@C`; native code lives in companion files. `@!` may be empty.
A constants-only file may contain an empty `@=` section. Parameter/result sections
require an implementation. Unknown, repeated, or out-of-order annotations fail
in active files.

A file with no genuine zero-indent `@=`, `@!`, or `@C` marker is a draft.
Shared encoding, TAB, and lexical spacing checks still apply, but incomplete
declarations need not parse. Marker text in comments/literals and companion
files does not activate a draft. See SYN-03–SYN-05 in
[Module sections and unfinished files](language/source-and-names.md#module-sections-and-unfinished-files).

The grammar admits an initializer sequence; static checking permits at most one
initializer for a scalar and any number for a list, one value per element.
Whole-list assignment is not implicitly a declaration initializer. Sources must
be initialized in the specified declaration order. Explicit/inferred fields,
creation-time construction, interface reset, and constant restrictions are
defined in [Declarations and scope](language/declarations.md). The separate
`<input-declaration>` production gives TPL-02 its input-pattern context without
changing ordinary field recognition. A `<static-expression(n)>` starts after its
caller has written the indentation or operand separator; `n` is the indentation
of that expression's head line, and its bindings are at `n+1`.

An optional static binding block immediately follows a declaration's type line;
ordinary runtime initializers follow at the same depth as the bindings. Named
inputs use a full foreign formal name; positional inputs use bare `...`. Each
nonempty input has exactly one space on each side of `<` and a static expression
on its right. A positional input may instead end immediately after `<`. Static
outputs follow all inputs at their level and use `>` with a placeholder receiver;
only a positional output may omit that receiver. Nested expression bindings are
one level deeper. Comments and blank lines follow the displayed `<layout>`
regions; they neither supply a binding nor change the kind of an input pattern.
Once an ordinary initializer has begun, no later static binding can belong to
that declaration. Static bindings initialize no runtime storage. Binding modes,
positions, skips and output receiver scope follow
[TPL-12](language/static-results.md#tpl-12).

The `<static-formal>` alternative belongs only to `@<` or to a signature already
nested within it. A header with no signature members declares Type; a header with
one or more indented members declares Template. The same structural distinction
applies recursively. There is no trailing `<` on a header. A comment or blank
line cannot substitute for a signature member. Signature names belong to the
expected constructor interface, not the surrounding module's local parameter
namespace. Static formals consume no runtime input position. A `<static-result>`
in `@>` has a name line and a same-level expression line; its static bindings
are one level deeper, and it has no runtime initializer. The expression determines
its Type or Template result under TPL-12. Placeholder introductions, categories
and uses are checked after parsing under
[TPL-02](language/templates.md#tpl-02), [TPL-07](language/templates.md#tpl-07) and
TPL-12; the parser never consults a parameter table to select a production.

```text
count
..integer
 |3

lines[]
..text
 |First line
 |
 |Third line
```

## Static template syntax and design boundaries

The source shape distinguishes interface type expressions, static bindings,
static result definitions and explicit static formals. Collect local Type
introductions from both `@<` and `@>` before checking uses under
[TPL-02](language/templates.md#tpl-02). Collect static result names separately
under [TPL-12](language/static-results.md#tpl-12): a result name is not another
free parameter. A bare placeholder in an interface type expression introduces
Type unless that name already denotes a declared Template or static result.
An already visible received alias is likewise a use, not a parameter introduction;
TPL-12 classifies receiver aliases before finalizing implicit interface parameters.
Bare placeholders on the right-hand side of static input bindings may likewise
introduce Type parameters in that interface context. A known Template retains
its category wherever it is used; a runtime value never determines which category
its source spelling declares.

Without a static output phase, a known Template application at the root of a
runtime input type follows TPL-07's complete root-input coverage rule. A
placeholder head with one or more named static bindings in that input position
can introduce a Template-application pattern. Its head names the local Template
parameter; binding labels name the applied constructor's foreign formals, and
right-hand expressions use the input pattern's local scope. A concrete
template-path head has the same root-input coverage requirement and permits the
same local right-hand Type introductions, but fixes the constructor origin
without introducing a local Template head parameter.
Positional bindings cannot introduce an unknown constructor's named signature;
they require an explicitly declared signature or a known concrete constructor
origin. Their ordinary recognition never depends on choosing an actual type.

An explicit static formal uses `...name` in `@<`. With no nested signature it
declares Type; with a nonempty named signature it declares Template. Signature
members use the same recursive form. Results can introduce new Type parameters
and use Template parameters already introduced in `@<`; an unknown applied
placeholder head in `@>` does not introduce a Template parameter. A static
result's required second line distinguishes it from an input formal. The body
may reuse interface parameters, apply or forward known constructors, and receive
immutable static aliases under TPL-12; it cannot introduce a new free Type or
Template parameter. Without a static output phase, shared/body/runtime-result
declarations give omitted constructor residuals fresh field inference variables
under TPL-08, unlike complete root-input patterns or residuals preserved inside
a partial static argument or static result definition. With a static output
phase, the expression follows TPL-12's concrete-producer rule instead: omitted
producer inputs receive expression-local inference terms, including in a static
argument or result definition. Shared `@=` declarations may reuse interface
parameters in their type expressions under
[CONST-06](language/declarations.md#const-06); they cannot introduce parameters.
Their resolved types and constant construction must meet the ordinary constant
restrictions.

Static typing uses all specified typed connections under
[TPL-03](language/templates.md#tpl-03): inputs, explicitly typed output receivers,
defaults, declaration initializers, assignments, reference bindings and aliases.
They constrain one fixed specialization, independently of execution order. A
literal or an otherwise unconstrained receiver contributes no concrete type
identity. Static propagation neither executes a later statement early nor
relaxes ordinary initialization, ownership or storage-form rules.

Ordinary arguments still use the existing `<parameter>` and `<result>` productions.
Static bindings attach to static expressions in declarations, result definitions,
constant namespaces or standalone static projections, never to an entry or
resume operation. There is no separate list of type arguments at an ordinary
runtime call site.

```text
@<
source&
...data.element type

@>
result&
...data.element type

@!
result&
 source&
```

This is a complete template module returning the supplied reference.
The whole `...data.element type` is one type parameter. Storage suffixes still
belong to the variable names, not the type line. A Type parameter can instead
first appear in a result declaration:

```text
@>
result
...result type

@!
```

This complete module has no runtime inputs. Assuming its path is `.default result`
and the selected result type supports default construction, this caller determines
the result Type through an explicitly typed receiving field:

```text
@!
worker
.default result

number
..integer

worker !
 > number
```

`...result type` is a named parameter of the callee, introduced in `@>`.
Receiving into a new unconstrained name instead would not supply this evidence.
The parser recognizes the same result declaration and output-binding shapes in
both cases; solving their type constraints is a later semantic operation.

A pattern introducing a Template parameter still belongs only to an input type
and has a binding block:

```text
@<
sample&
...container
 ...element < ...item type
@!
```

The pattern relates an ordinary typed reference input to the explicitly named
constructor application; it does not pass a constructor as a runtime reference.
Its nominal decomposition and uniqueness checks follow
[TPL-07](language/templates.md#tpl-07). An explicit static-formal declaration
states an expected signature without consuming a runtime argument. This fragment
declares one Type and one Template:

```text
@<
...item type

...family
 ...element
 ...nested family
  ...content
@!
```

Static applications can be nested on a binding's right-hand side. The following
is a declaration fragment. Assume `.outer` expects a Template formal
`...factory` with one residual Type formal `...value type`, plus a Type formal
`...element`. Assume `.dictionary` has Type formals `...key type` and
`...value type`, and `...item type` is already introduced in the enclosing
template's input section:

```text
worker
.outer
 ...factory < .dictionary
  ...key type < ..text
 ...element < ...item type
```

The dictionary's unbound value formal remains part of that partial constructor;
its existing key binding and its residual signature are checked against
`.outer`'s `...factory` formal under [TPL-07](language/templates.md#tpl-07) and
[TPL-08](language/templates.md#tpl-08). The less-indented `...element` binding
belongs to `.outer`, so indentation alone determines each binding's owner.

Static results use the ordinary declaration's two-line shape with a placeholder
result name. The following `@>` fragment assumes `...family` has the dictionary's
two-slot signature and `...key type` is a local Type parameter:

```text
@>
...dictionaries for key
...family
 ...key type < ...key type

...integer dictionary
...family
 ...key type < ...key type
 ...value type < ..integer
```

The first expression leaves a residual constructor; the second is fully applied.
These declarations define no runtime result fields or initializer sources.
Their meaning, independent static output positions and projection rules belong
to [TPL-12](language/static-results.md#tpl-12).

The explicit constant-read namespace accepts a static type expression under
[CONST-03](language/declarations.md#const-03) and
[CONST-06](language/declarations.md#const-06). Direct qualified constant sources,
runtime value sources, call targets and native protocol records do not acquire
placeholder or static-application alternatives. A bare placeholder in a static
expression is parsed alike whether its declared category is Type or Template;
later scope/kind checks determine valid use. Bare `...` never supplies a type
expression. Omission of a runtime `<` binding is not partial static application.
A compound placeholder never splits into a constructor and member. The
[remaining design gates](../AMBIGUITIES.md#open-template-design-questions) are
semantic questions, not permission for type-dependent parsing. These source forms
do not claim implementation support.

**Explanation: finite syntax and specialization creation.** Nested static
applications use the productions above even when the same template path appears
at several levels. Whether instantiated definitions recursively create
specializations is checked after resolution under
[TPL-11](language/templates.md#tpl-11), using constructor origins and concrete
bindings. The parser does not reject repeated spellings, inspect a definition's
dependencies, or impose a template-depth limit. Reusing an already supplied Type
and applying its original constructor again have different resolved meanings;
that distinction changes no EBNF production.

## Executable statements and structural recognition

```ebnf
<statement(n)>               ::= <declaration(n)> | <assignment(n)> | <call(n)>
                                 | <constant-read(n)> | <static-projection(n)>
                                 | <branch(n)> | <loop(n)> | <foreach(n)>
                                 | <break(n)> | <return(n)> | <suspend(n)>
<assignment(n)>              ::= <indent(n)> <receiver> <line-end>
                                 <layout(n+1)>
                                 <indent(n+1)> <copy-source> <line-end>
<static-projection(n)>       ::= <indent(n)> <type-name> <line-end>
                                 <static-inputs(n+1)> <static-outputs(n+1)>
```

An assignment has one source exactly one level below its receiver. It cannot
use a literal source. A first-occurring simple receiver may declare a persistent
field after parsing; an indexed receiver never declares its collection or index.
Lexical receiver classification does not wait for its type to become concrete.
A field may keep a symbolic type term until the interface-wide constraints are
solved; later evidence never changes its original declaration/use classification.
[TYPE-06](language/declarations.md#type-06) defines that exact source-order
classification, and [TPL-03](language/templates.md#tpl-03) defines the admissible
typed connections.

```text
destination
 source

target&
 &item

continuation
 % original
```

A standalone static projection requires at least one static output action,
including an explicit positional skip. Its head has no `!` or `%`, and the
statement has no refusal handler. A static expression embedded in a declaration,
binding or result definition may also contain static output actions. Any such
output phase requests a concrete checked producer under
[TPL-12](language/static-results.md#tpl-12). Omitted producer inputs receive
deterministic expression-local inference terms, and static output constraints
may help determine them. All producer inputs must be uniquely determined before
any result is published; skips or unselected outputs do not waive that check.
The expression itself still denotes the concrete producer Type; receiving a
static result does not replace that Type. A returned Template may retain its
own residual parameters. Alias visibility and publication follow TPL-12;
these compile-time operations execute no runtime statement.

**Example: static projection fragment.** Assume hypothetical `.prepare types`
has one static Type input and two explicit static outputs. This reads the second
output and skips the first:

```text
.prepare types
 ... < ..text
 ... >
 ... > ...prepared family
```

The bare `...` selects positional mode; it is not a placeholder name. The
receiver is a complete placeholder spelling. Static input and output positions
are independent of ordinary runtime positions and of each other under TPL-12.

**Example: later static evidence, with ordinary execution order.** Assume
`.forward` is the template defined in [TPL-04](language/templates.md#tpl-04):

```text
@!
known
..integer

worker
.forward

worker !
 < |0
 > inferred

inferred
 known
```

The output receiver first introduces `inferred` under TYPE-06, initially with
the result's symbolic type. The later assignment from `known` constrains that
same field to `..integer`, which also fixes the worker's input/result type.
The literal is then checked against that determined type; its bytes did not
select a type. At runtime the call still precedes the assignment, and the fields
are constructed under the ordinary declaration-order rules.

Recognize the shape before name/type lookup:

- A declaration name followed by a same-level type expression is a declaration;
  its static bindings precede any ordinary initializers.
- Only `@<` admits a static-formal header `...name`, with an optional nonempty
  indented signature. A static result in `@>` instead requires a same-level
  expression line. Binding lines have `<` or `>`; runtime arguments remain
  inside their operation's argument block.
- A receiver followed by an indented copy source is an assignment.
- Suffix `!` starts entry execution; suffix `%` resumes.
- An unmarked instance target with output actions reads retained results. A
  static type expression with `>>` reads shared constants; one with static `>`
  actions alone is a standalone static projection. A whole-list parent selects
  intrinsic methods. These forms are distinguished by source tokens and
  indentation before lookup, including for a placeholder expression head.
- An owned access followed by same-level `?`, `:`, or `{` is control flow. A `:`
  after an eligible operation is instead that operation's refusal handler.
- A bracketed reference-name binder before `{` is foreach. A bracketed owned
  index before `{` is an indexed-condition loop.
- A single nested path of operation links ending in an owned condition and `{`
  is one condition-chain loop.
- Standalone `^`, `>`, and `%` are control statements.

A bare target is invalid. There are no implicit calls, general expression
statements, arithmetic operators, or mandatory `*` name prefixes. A static
projection requires its explicit output actions. A missing declaration type line
cannot turn a second unindented name into an assignment source.

## Invocation, retained reads, and result selection

```ebnf
<call(n)>                    ::= ( <execution-call(n)> | <resume-call(n)> | <read-call(n)> )
                                 [ <layout(n)> <indent(n)> ":" <line-end> <body(n+1)> ]
<execution-call(n)>          ::= <indent(n)> <call-target> <space> "!" <line-end>
                                 <layout(n+1)> <execution-arguments(n+1)>
<resume-call(n)>             ::= <indent(n)> <instance-target> <space> "%" <line-end>
                                 <layout(n+1)> <resume-arguments(n+1)>
<read-call(n)>               ::= <indent(n)> <call-target> <line-end>
                                 <layout(n+1)> <read-arguments(n+1)>
<constant-read(n)>           ::= <indent(n)> <static-expression(n)>
                                 <constant-read-arguments(n+1)>
<execution-arguments(n)>     ::= { <parameter-line(n)> <layout(n)> }
                                 { <execution-output-action(n)> }
<resume-arguments(n)>        ::= { <execution-output-action(n)> }
<execution-output-action(n)> ::= <indent(n)> <result> <line-end> <layout(n)>
                                 | <selected-result-call(n)> <layout(n)>
<read-arguments(n)>          ::= <read-action(n)> { <read-action(n)> }
<read-action(n)>             ::= <indent(n)> <saved-result> <line-end> <layout(n)>
                                 | <selected-result-call(n)> <layout(n)>
<constant-read-arguments(n)> ::= <indent(n)> <saved-result> <line-end> <layout(n)>
                                 { <indent(n)> <saved-result> <line-end> <layout(n)> }
<selected-result-call(n)>    ::= ( <selected-execution-call(n)>
                                 | <selected-resume-call(n)>
                                 | <selected-read-call(n)> )
                                 [ <layout(n)> <indent(n)> ":" <line-end> <body(n+1)> ]
<selected-execution-call(n)> ::= <indent(n)> <result-target> <space> "!" <line-end>
                                 <layout(n+1)> <execution-arguments(n+1)>
<selected-resume-call(n)>    ::= <indent(n)> <result-target> <space> "%" <line-end>
                                 <layout(n+1)> <resume-arguments(n+1)>
<selected-read-call(n)>      ::= <indent(n)> <result-target> <line-end>
                                 <layout(n+1)> <read-arguments(n+1)>
<result-target>              ::= <call-target>
<parameter-line(n)>          ::= <indent(n)> <parameter> <line-end>
                                 | <indent(n)> <literal-parameter> <newline>
<parameter>                  ::= "<"
                                 | "<" <separator> <copy-source>
                                 | <declaration-name> <separator> "<" <separator> <copy-source>
<literal-parameter>          ::= "<" <separator> "|" <literal-payload>
                                 | <declaration-name> <separator> "<" <separator>
                                 "|" <literal-payload>
<result>                     ::= ">"
                                 | ">" <separator> <output-receiver>
                                 | <declaration-name> <separator> ">" <separator> <output-receiver>
<saved-result>               ::= ">>" <separator> <output-receiver>
                                 | <declaration-name> <separator> ">>" <separator> <output-receiver>
<output-receiver>            ::= <receiver>
```

Every separator above is explicit. `>>` is one token; `> >` is invalid. A literal
input consumes its newline directly, so `#` in payload cannot become a comment.

### Modes

| Form | Runtime inputs | Output actions |
|---|---|---|
| `worker !` | `<` bindings | `>` bindings and selected operations |
| `worker %` | None | `>` bindings and selected operations |
| `worker` | None | At least one `>>` binding or selected operation |
| `items[]` / `targets&[]` | None | At least one selected list method `!` |
| Selected list method `!` | Its declared `<` bindings | Its declared results |
| `.namespace` / `...type` constant read | None | At least one `>>` binding |
| `.producer` / `...producer` static projection | None | At least one static `>` action |

Execution modes cannot combine at one operation level. Module paths cannot be
entry/resume targets. A constant-read namespace likewise cannot acquire `!`, `%`,
a reference suffix or acquisition prefix, and constant reads have no refusal
handler. A whole-list parent has neither `!`/`%` nor ordinary bindings. Its methods
accept only `!`;
they are intrinsic interface entries, not first-class values with retained reads
or resume positions. The productions intentionally recognize a broader target
set; the list interface restricts that set during static checking.

### Constant namespace shape

The namespace head is a module path or a locally declared placeholder, followed
by the same optional static binding block as a declaration's type expression.
A known Template head may be applied through that block; a Type head is already
a type and cannot receive constructor arguments. Static input labels belong to
the selected constructor's formals, and static output labels to its declared
static results; source expressions use the surrounding template's established
scope. Static output receivers may introduce immutable aliases under
[TPL-12](language/static-results.md#tpl-12); they introduce no free parameter.

The namespace expression must resolve to one concrete Type under
[CONST-06](language/declarations.md#const-06) before constant-member lookup.
Without a static output phase, explicit bindings or already determined static
bindings must supply that identity; omitted namespace inputs do not receive
inference terms. When static output actions are present, their producer follows
TPL-12: expression-local inference terms and static output constraints may
determine omitted producer inputs, and the complete producer must be concrete
and checked before publishing outputs or selecting constants. Requested constant
names and ordinary constant receiver types cannot complete an unresolved
namespace. A residual constructor is not a constant namespace. This is a static
namespace selection, not a runtime value source or an instance operation.

Static bindings and saved-result bindings are both at depth `n+1`. All static
inputs and any static output actions precede the first `>>` binding. A nested
application on a static input's right-hand side owns its own, deeper binding
block as usual. Once the first saved-result binding begins, no later static
binding can attach to that constant read. No runtime input bindings, ordinary
`>` transfers, selected calls or refusal handler are admitted in a constant-read
block. Static `...result > ...receiver` and `... > ...receiver` have distinct
source forms from those ordinary transfers. The static application's mode follows
TPL-12; named and positional constant requests independently retain CONST-03's
ordinary saved-read rules.

**Examples: specification fragments, not implementation claims.** Assume the
enclosing interface declares Type parameter `...element type`, its substitution
is already concrete, and that selected type declares constant `zero`:

```text
...element type
 zero >> initial value
```

Assume `.container` declares Type formal `...element type` and shared constant
`empty list[]`. Its concrete namespace can be selected without an instance:

```text
.container
 ...element type < ..integer
 empty list[] >> values[]
```

The following fragment is invalid because a static binding follows a constant
request:

```text
.container
 empty list[] >> values[]
 ...element type < ..integer
```

The direct copy-source spelling `.palette.red` retains its separate production.
There is no inline static application inside such a source, and
`...element type.zero` does not mean the `zero` constant of `...element type`.

### Binding form and order

Static input/output bindings use the separate mode and interface positions in
[TPL-12](language/static-results.md#tpl-12): one application uses named labels
throughout or bare `...` throughout, inputs precede outputs, and input/output
counters are independent. Named labels always require operands; only positional
static bindings admit a skip. The two counters follow the actual constructor's
residual input order and explicit static-output declaration order. A nested
static expression has its own mode and counters. Resolving those interfaces is
a semantic operation, not a different parse for different constructor bindings.

One operation uses either named or positional ordinary bindings across both
inputs and outputs. Input/output counters are independent and follow interface
declaration order. Inputs precede every output action. Each named input or output
label occurs at most once and includes its declared storage suffix. A selected
result operation has its own binding mode; its selector consumes no ordinary
result position and does not change its parent's mode.

Empty positional `<` consumes an input position and requests its declared
default. Empty positional `>` consumes an output position without transferring
it. Trailing bindings may be omitted; explicit placeholders count against arity.
Named bindings and `>>` always require operands. Resume has no input placeholders.

```text
worker !
 < first
 <
 < third
 >
 > selected
```

This fragment supplies input positions one and three, requests input two's
default, discards result one, and transfers result two.

### Direct operations on results

A selected operation is nested one level beneath its parent and selects a
declared public result, or a list service's declared intrinsic interface entry.
Root operands, input sources, indices, ordinary bodies, and handlers use the
initiating caller's namespace. Result selectors use the preceding operation's
interface. Private fields are not selectors. Several sibling actions are allowed
and complete in source order, including their nested output actions.

```text
factory
 method !
  > value
 another method !
  > other value
```

This is a body fragment with compatible declared interfaces. Both calls select
the actual retained method results of `factory`; no intermediate value is copied.
The exact execution, protection, input/default construction, output-transfer,
inference, and refusal contracts are in
[Calls and results](language/calls-and-results.md). List method arguments and
results are in [Working with lists](language/lists.md#calling-list-methods).

## Branches, loops, completion, and suspension

```ebnf
<branch(n)>                  ::= <indent(n)> <owned-access> <line-end> <comment-gap(n)>
                                 ( <indent(n)> "?" <line-end> <body(n+1)>
                                   [ <layout(n)> <indent(n)> ":" <line-end> <body(n+1)> ]
                                 | <indent(n)> ":" <line-end> <body(n+1)> )
<loop(n)>                    ::= <owned-condition-loop(n)> | <condition-chain-loop(n)>
<owned-condition-loop(n)>    ::= <indent(n)> <owned-access> <line-end> <comment-gap(n)>
                                 <indent(n)> "{" <line-end> <body(n+1)>
                                 <indent(n)> "}" <line-end>
<condition-chain-loop(n)>    ::= ( <condition-entry-link(n)>
                                 | <condition-resume-link(n)>
                                 | <condition-read-link(n)> )
                                 [ <layout(n)> <indent(n)> ":" <line-end> <body(n+1)> ]
<condition-entry-link(n)>    ::= <indent(n)> <call-target> <space> "!" <line-end>
                                 <layout(n+1)>
                                 { <parameter-line(n+1)> <layout(n+1)> }
                                 <loop(n+1)>
<condition-resume-link(n)>   ::= <indent(n)> <instance-target> <space> "%" <line-end>
                                 <layout(n+1)> <loop(n+1)>
<condition-read-link(n)>     ::= <indent(n)> <call-target> <line-end>
                                 <layout(n+1)> <loop(n+1)>
<foreach(n)>                 ::= <indent(n)> <foreach-header> <line-end> <comment-gap(n)>
                                 <indent(n)> "{" <line-end> <body(n+1)>
                                 <indent(n)> "}" <line-end>
<foreach-header>             ::= <identifier> "[" <reference-name> "]"
                                 | <identifier> "&[" <reference-name> "]"
<break(n)>                   ::= <indent(n)> "^" <line-end>
<return(n)>                  ::= <indent(n)> ">" <line-end>
<suspend(n)>                 ::= <indent(n)> "%" <line-end>
```

### Marker layout and contextual distinctions

Conditions and their first `?`, `:`, or `{` marker align, with only same-level
comments between them. Bodies begin on following lines exactly one level deeper;
there are no inline bodies or required trailing spaces. `}` aligns with `{`.
Every written control body may be empty; the empty-body warning and execution
rules are defined in [Conditions and loops](language/control-flow.md).

A dedent ends an implicit body. A parent-level comment can precede an optional
`:` alternative or refusal handler, as allowed by `<layout(n)>`. A loop's closing
`}` has no parent-comment gap: comments before it belong to the loop body or a
valid nested region. Dedented comments cannot be followed by lines reopening
the closed body. Comment-only lines cannot introduce deeper blocks.

`?` selects a positive branch, optional `:` its negative branch. `:` alone after
a condition selects only the negative branch and cannot then acquire another
marker. A standalone `:` without a preceding condition/eligible operation is
invalid. Its role is structural, never chosen by type lookup. At call-argument
indentation, `>` is an output binding; as an ordinary statement it completes the
invocation. Standalone `!` is invalid. `^` requires an enclosing loop body.

### Linear loop-condition commands

The recursive `<loop(n+1)>` in a header continues the same command. Only a
`<loop>` encountered as an ordinary statement introduces a separate loop node.
Each link has exactly one child; only entry links accept inputs before it.
Ordinary output bindings, sibling selected operations, module-path targets, and
terminal `?`/`:` branches are excluded from this header.

The terminal condition is an owned scalar access, possibly indexed, with its
own native truth conversion. A reference or whole list is not a terminal
condition. The loop's static repeat entry is the first link in its containing
ordinary body, or the condition itself for a zero-link loop. Header handlers
are outside that loop's body, so their `^` applies to an enclosing loop, if any.

```text
factory
 next !
  has item
  {
   process item !
  }
```

The example assumes the declared result chain shown. On each test the command
starts at `factory`; it does not restart an enclosing branch or caller. Full
binding, repetition, cleanup, empty-body, and refusal rules are in
[Computing a loop condition through calls](language/control-flow.md#computing-a-loop-condition-through-calls).

### List traversal

```text
items[item&]
{
 item& !
}
```

The header's bracketed reference name distinguishes a binding from an index.
`items[position]` before `{` remains an indexed-condition loop, regardless of
which names exist or what types they have. A foreach source must be a mutable
list field of the current instance, and its element type must be statically
known. The loop is an ordinary statement, not a link or terminal condition in a
condition-chain loop.

Reference binding scope, live boundary corrections, removed-element guards,
nested traversal, and continuation behavior have one authoritative definition
in [Traversing a changing list](language/lists.md#traversing-a-changing-list).

### The three uses of `%`

| Source form | Syntactic role |
|---|---|
| Standalone `%` | Suspend the current invocation |
| `task %` | Resume the selected instance |
| `% task` / `% task&` as a copy source | Copy state including saved positions |

These forms add no automatic scheduling or parallel execution. Their state and
lifetime rules are specified in [Continuations](language/continuations.md).

## Native protocol companion syntax

A physical source filename followed by `.abi` names a flat binding table, not
an Ama module. Active `@C` modules require their native companions under SRC-01.

```ebnf
<abi-file>                   ::= { <abi-record> | <abi-blank> | <abi-comment-line> }
<abi-symbol>                 ::= "+" | "-" | "|" | "=" | "?" | "[]"
<abi-function>               ::= ? one module-local C function identifier accepted by
                                   the supported C toolchain ?
<abi-record>                 ::= <abi-symbol> <separator> <abi-function> <line-end>
<abi-blank>                  ::= <newline>
<abi-comment-line>           ::= { <space> } <comment> <newline>
```

Records start in column one, in any order, with exactly one separator between
symbol and one module-local C identifier accepted by the supported toolchain.
C identifiers are not multiword Ama names. Leading spaces are permitted only
before comments. Blank lines contain no spaces, and trailing spaces require a
comment. The shared physical-line, UTF-8, and TAB rules still apply.

There are no sections, nesting, includes, macros, expressions, or executable
bodies. Duplicate/unknown slots and missing required lifecycle slots are errors.
`+` and `-` are mandatory; other capabilities are required when used.

```text
+ initialize
- destroy
| from_literal
= clone
? to_condition
[] to_index
```

The [native-interface chapter](implementation/native-interface.md) defines
signatures, status values, module-local symbol qualification, payload restrictions,
and lifecycle obligations. These are semantic/ABI contracts; the grammar never
infers them from function names.
