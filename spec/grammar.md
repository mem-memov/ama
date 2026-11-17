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
three dots and uses the same component spelling. Its whole name is one scoped
type parameter under [TPL-01/TPL-02](language/templates.md#tpl-01), not a third
filesystem root. It has no trailing dot and does not select a member.
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
permitted. A path in a type line names a type instead. Constants cannot be
receivers, mutable-reference sources, execution targets, or continuation-copy
operands. See [Constants](language/declarations.md).

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
<parameters-section>         ::= "@<" <line-end> <declarations(0)>
<results-section>            ::= "@>" <line-end> <declarations(0)>
<implementation-section>     ::= "@!" <line-end> <body(0)>
                                 | "@C" <line-end> <layout(0)>
<declarations(n)>            ::= <layout(n)> { <declaration(n)> <layout(n)> }
<body(n)>                    ::= <layout(n)> { <statement(n)> <layout(n)> }
<declaration(n)>             ::= <indent(n)> <declaration-name> <line-end>
                                 <layout(n)>
                                 <indent(n)> <type-name> <line-end>
                                 <initializers(n+1)>
<initializers(n)>            ::= <layout(n)> { <initializer(n)> <layout(n)> }
<initializer(n)>             ::= <literal-line(n)>
                                 | <indent(n)> <copy-source> <line-end>
```

| Annotation | Section |
|---|---|
| `@=` | Shared immutable constants |
| `@<` | Parameters |
| `@>` | Results |
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
defined in [Declarations and scope](language/declarations.md). Placeholder
introductions and uses are checked after parsing under TPL-02; the parser never
consults a parameter table to select `<type-name>`.

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

The concrete module file/section/declaration productions remain unchanged apart
from `<type-name>` above. A placeholder type line in an `@<` declaration introduces
a scoped type parameter. Reusing it in `@>` or the body is a semantic lookup;
those positions cannot introduce a new placeholder. Ordinary arguments still use
the existing `<parameter>` and `<result>` productions. There is no separate list
of type arguments at an ordinary call site.

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

This is a complete proposed template module returning the supplied reference.
The whole `...data.element type` is one type parameter. Storage suffixes still
belong to the variable names, not the type line. The grammar recognizes a
placeholder only through `<type-name>`; concrete constant paths, value sources,
call targets and native protocol records do not acquire placeholder alternatives.

[TPL-07/TPL-08](language/templates.md#tpl-07) define the semantic requirements for
template-valued parameters and partial substitution. Their category/signature,
transmission and static-application source forms remain
[AMB-003](../AMBIGUITIES.md#amb-003); this grammar deliberately establishes no
production for those unsettled forms. Do not interpret omission of a runtime
argument as partial static binding or split a compound placeholder into a
constructor and member. The
[other design gates](../AMBIGUITIES.md#open-template-design-questions) are semantic
questions, not permission for type-dependent parsing.

## Executable statements and structural recognition

```ebnf
<statement(n)>               ::= <declaration(n)> | <assignment(n)> | <call(n)>
                                 | <constant-read(n)> | <branch(n)> | <loop(n)> | <foreach(n)>
                                 | <break(n)> | <return(n)> | <suspend(n)>
<assignment(n)>              ::= <indent(n)> <receiver> <line-end>
                                 <layout(n+1)>
                                 <indent(n+1)> <copy-source> <line-end>
```

An assignment has one source exactly one level below its receiver. It cannot
use a literal source. A first-occurring simple receiver may declare a persistent
field after parsing; an indexed receiver never declares its collection or index.
TYPE-06 in [Declarations and scope](language/declarations.md) defines the exact
source-order classification and type inference.

```text
destination
 source

target&
 &item

continuation
 % original
```

Recognize the shape before name/type lookup:

- A declaration name followed by a same-level type name is a declaration.
- A receiver followed by an indented copy source is an assignment.
- Suffix `!` starts entry execution; suffix `%` resumes.
- An unmarked target with output actions reads retained results. A module path
  with `>>` reads shared constants; a whole-list parent selects intrinsic methods.
- An owned access followed by same-level `?`, `:`, or `{` is control flow. A `:`
  after an eligible operation is instead that operation's refusal handler.
- A bracketed reference-name binder before `{` is foreach. A bracketed owned
  index before `{` is an indexed-condition loop.
- A single nested path of operation links ending in an owned condition and `{`
  is one condition-chain loop.
- Standalone `^`, `>`, and `%` are control statements.

A bare target is invalid. There are no implicit calls, expression statements,
arithmetic operators, or mandatory `*` name prefixes. A missing declaration type
line cannot turn a second unindented name into an assignment source.

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
<constant-read(n)>           ::= <indent(n)> <module-path> <line-end>
                                 <layout(n+1)> <constant-read-arguments(n+1)>
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

| Form | Inputs | Output actions |
|---|---|---|
| `worker !` | `<` bindings | `>` bindings and selected operations |
| `worker %` | None | `>` bindings and selected operations |
| `worker` | None | At least one `>>` binding or selected operation |
| `items[]` / `targets&[]` | None | At least one selected list method `!` |
| Selected list method `!` | Its declared `<` bindings | Its declared results |
| `.namespace` | None | At least one `>>` binding |

Execution modes cannot combine at one operation level. Module paths cannot be
entry/resume targets and constant reads have no refusal handler. A whole-list
parent has neither `!`/`%` nor ordinary bindings. Its methods accept only `!`;
they are intrinsic interface entries, not first-class values with retained reads
or resume positions. The productions intentionally recognize a broader target
set; the list interface restricts that set during static checking.

### Binding form and order

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
