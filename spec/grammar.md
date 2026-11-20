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
- [Project configuration and dependency selectors](#project-configuration-and-dependency-selectors)
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
- Recognize the longest fixed compound token: `@...`, `@=`, `@<`, `@>`, `@!`,
  `@C`, `>>`, `[]`, `&[]`, `&[`, `//`, `./` and `../`. Their characters cannot
  be separated by spaces. At a type start, classify absolute, relative, alias
  and single-dot static forms before reading their components.

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
| `/acme/shop tools/v2.4.14/src/item state`, `//integer` | `< //palette/red` |

Structural indentation and an operand separator before a path, static name or `&`
remain outside that path/access. Storage suffixes, prefix `&`, path slashes,
static-name dots and bracket joins do not admit optional whitespace. Standalone markers have no
trailing separator. `|` immediately precedes its raw payload: a space after it
is data.

## Names, paths, and storage forms

### Complete names and paths

```ebnf
<identifier-start>           ::= "_" | ? Unicode scalar with property XID_Start ?
<identifier-continue>        ::= "_" | ? Unicode scalar with property XID_Continue ?
<name-word>                  ::= <identifier-start>
                                 { <identifier-continue> | "-" <identifier-continue> }
<identifier>                 ::= <name-word> { <space> <name-word> }
<decimal-digit>              ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
<nonzero-digit>              ::= "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
<version-number>             ::= "0" | <nonzero-digit> { <decimal-digit> }
<version-component>          ::= "v" <version-number> "." <version-number> "."
                                 <version-number> [ "-" <name-word> ]
<path-component>             ::= <identifier> | <version-component> | "." | ".."
<path-components>            ::= <path-component> { "/" <path-component> }
<ordinary-absolute-path>     ::= "/" <path-components>
<standard-absolute-path>     ::= "//" <path-components>
<relative-path>              ::= ( "./" | "../" ) <path-components>
<direct-path>                ::= <ordinary-absolute-path> | <standard-absolute-path>
                                 | <relative-path>
<alias-path>                 ::= "~" <identifier> [ "/" <path-components> ]
<module-path>                ::= <direct-path> | <alias-path>
<placeholder-type>           ::= "." <identifier> { "." <identifier> }
<type-name>                  ::= <module-path> | <placeholder-type>
```

Use Unicode 17.0.0 identifier properties. Names are case-sensitive and compared
byte-for-byte without normalization or transliteration. A complete name cannot
begin/end with a space and contains exactly one space between nonempty words.
Every word obeys its own start rule: `item step2` is valid; `item 2` is invalid.

ASCII hyphen-minus U+002D may occur only inside a name word, immediately between
ordinary identifier characters. `<identifier-continue>` itself excludes `-`;
the separate alternative requires a non-hyphen continuation after every hyphen.
Thus `item-2`, `item-_value` and `ключ-2` are valid, while `-item`, `item-`,
`item--name`, `item- name` and `item -name` are invalid. A hyphen does not start
a new word, so a digit or another permitted `XID_Continue` scalar, including a
combining character, may follow it. Other dash/minus characters are not alternate
spellings of U+002D. The exact profile follows
[SRC-03](language/source-and-names.md#src-03).

In a name position, consume the maximal complete name up to reserved punctuation
or the code-part boundary; a valid internal hyphen is part of that name.
`a-b` is one name, not subtraction. A malformed hyphen sequence is invalid and
cannot be repaired by splitting it into operands. A space alone cannot separate
two operands. Never consult declarations to split a name or repair its spelling.

The absolute prefixes `/` and `//` select the ordinary and standard logical
namespaces respectively. Neither addresses the host filesystem root. Path
separators are adjoining single slashes. A whole component is one complete
name, a version spelling, or the structural `.` or `..`. The latter are
normalized with the exact stack algorithm in
[SRC-06](language/source-and-names.md#src-06), before module lookup. A relative
path starts with `./` or `../` and uses the defining source's containing logical
directory and namespace. Empty components, trailing slashes, three leading
slashes and a normalized endpoint above the namespace root are invalid. The
normalized ordinary module identity must have vendor, one or more package
components, version, `src` or `tests`, and one or more module components under
[SRC-01](language/source-and-names.md#src-01); standard modules need no package
wrapper. The version production's numeric fields have no leading zero except
`0`; its optional variant is one nonempty name word with no spaces, dots or
slashes. Version components are permitted only at that package-version
position in a canonical ordinary identity. They are not ordinary dotted names.

An `<alias-path>` looks up exactly the complete name after `~` and before its
first slash, expands it under the [package alias rules](project/packages.md#pkg-04),
then normalizes the supplied suffix. No alternate lookup or partial-name match
is permitted. Source module paths contain no dependency wildcard `*`.

A `<placeholder-type>` starts with exactly one dot and has no slashes. Its
whole descriptive name denotes one scoped static parameter, result or alias
under [TPL-01/TPL-02](language/templates.md#tpl-01) and
[TPL-12](language/static-results.md#tpl-12), with no filesystem lookup or member
selection. `.data.element type` is one name, not a path. Bare `.`, `..name`,
`...name`, empty components and a trailing dot are invalid static names.
The slash in `./module` and second dot in `../module` distinguish them from a
static name before lookup. `@...` remains an independent module annotation.
Static bindings always use complete named labels and operands. Physical
`name+/` submodule directories add no source syntax; their exact mapping follows
[SRC-05](language/source-and-names.md#src-05).

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
<constant-access>            ::= <module-path> [ "[]" ]
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

In a qualified constant source, expand any alias and normalize the entire path
first. Exactly its final component is the constant name, and all preceding
components must form the module's complete identity. The final name must obey
the ordinary constant-name rule; optional `[]` selects the owning-list form.
For example, `/example/demo/v1.0.0/src/catalog/item states[]` is a constant list
source. A whole alias may supply all components, including the final constant;
there need not be a slash written after the alias. No longest-prefix lookup or
alternate path split is permitted. A path in a type line names a type instead.
This direct source form
does not accept a placeholder, a static application or an export selector; use
the explicit constant-read namespace form below to select a prepared type.
In particular, `.type.member` remains one complete placeholder, not qualified
constant access.
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
//text
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
                                   zero-indent @=, @!, @C, or @... activation marker ?
<complete-module-file>       ::= <executable-module-file> | <constants-only-file>
                                 | <static-module-file>
<executable-module-file>     ::= <layout(0)>
                                 [ <constants-section> ]
                                 [ <parameters-section> ]
                                 [ <results-section> ]
                                 <implementation-section>
<constants-only-file>        ::= <layout(0)> <constants-section>
<static-module-file>         ::= <static-marker-file> | <static-producer-file>
<static-marker-file>         ::= <layout(0)> "@..." <line-end> <layout(0)>
<static-producer-file>       ::= <layout(0)> "@..." <line-end> <static-body(0)>
<constants-section>          ::= "@=" <line-end> <declarations(0)>
<parameters-section>         ::= "@<" <line-end> <declarations(0)>
<results-section>            ::= "@>" <line-end> <declarations(0)>
<implementation-section>     ::= "@!" <line-end> <body(0)>
                                 | "@C" <line-end> <layout(0)>
<declarations(n)>            ::= <layout(n)> { <declaration(n)> <layout(n)> }
<body(n)>                    ::= <layout(n)> { <statement(n)> <layout(n)> }
<declaration(n)>             ::= <indent(n)> <declaration-name> <line-end>
                                 <layout(n)> <indent(n)> <declaration-type(n)>
                                 <initializers(n+1)>
<declaration-type(n)>        ::= <type-name> <line-end> <type-arguments(n+1)>
                                 [ <layout(n+1)> <export-selector(n+1)> ]
<type-arguments(n)>          ::= <layout(n)> { <type-argument(n)> <layout(n)> }
<type-argument(n)>           ::= <indent(n)> <placeholder-type> <space> "<" <space>
                                 <declaration-type(n)>
<export-selector(n)>         ::= <indent(n)> <placeholder-type> <line-end>
<initializers(n)>            ::= <layout(n)> { <initializer(n)> <layout(n)> }
<initializer(n)>             ::= <literal-line(n)>
                                 | <indent(n)> <copy-source> <line-end>
```

| Annotation | Section or module kind |
|---|---|
| `@=` | Shared immutable constants |
| `@<` | Runtime input declarations |
| `@>` | Runtime result declarations |
| `@!` | Ama implementation |
| `@C` | Companion C implementation |
| `@...` | Entire static marker or producer, with no other annotation sections |

Ordinary annotations occupy zero-indent lines in the displayed order, at most
once each. Only layout follows `@C`; native code lives in companion files.
`@!` may be empty. A constants-only file may contain an empty `@=` section.
Parameter/result sections require an implementation. A static file has only its
`@...` marker and static graph; it has no `@=`, `@<`, `@>`, `@!`, or `@C` section.
An empty graph is the `<static-marker-file>` form under
[TPL-14](language/static-results.md#tpl-14); a `<static-producer-file>` has at
least one graph statement. This distinction depends on the whole valid source,
not on how another module uses its path. Unknown, repeated, combined-kind, or
out-of-order annotations fail in active files.

The native boundary follows [TPL-13](language/templates.md#tpl-13): an `@C`
definition may have no free template parameters. A selected closed static export
is a use of a prepared ordinary type and introduces no free parameter. The
parser therefore recognizes its ordinary declaration shape without deciding
whether the selected module is native, an Ama template, or a static producer.
Those module-kind and closure checks follow name resolution.

A file with no genuine zero-indent `@=`, `@!`, `@C`, or `@...` marker is a draft.
Shared encoding, TAB, and lexical spacing checks still apply, but incomplete
declarations need not parse. Marker text in comments/literals and companion
files does not activate a draft. See SYN-03–SYN-05 in
[Module sections and unfinished files](language/source-and-names.md#module-sections-and-unfinished-files).

Every ordinary declaration retains a name line and a type-expression head at
the same indentation. Named `<type-argument>` lines supply structural static
substitutions one level deeper; each right-hand expression can have further
named arguments. An optional bare `<export-selector>` follows all arguments
at that same child level. It selects one named output of the head's static
producer. The selector is childless and introduces no alias or parameter;
there is no ordinary `>` static output-binding form. Distinguish the named
argument, selector and ordinary initializer by their source shapes without
loading a definition or consulting declared names. Initializers follow the
complete type expression and do not belong to a nested argument expression.

Free placeholder operands in ordinary interface type expressions introduce
Type parameters under [TPL-02](language/templates.md#tpl-02); labels select the
callee's formals/exports and never introduce parameters. Shared/body declarations
may only reuse the collected interface parameters. Standalone static formals,
result pairs and graph statements remain exclusive to `@...`.

[TPL-15](language/templates.md#tpl-15) requires one closed final Type for each
used concrete specialization. Nested operands may carry Types or Templates as
required by the selected constructor's contract. A static producer needs the
explicit named selector even if it exports only one result; it must have all
inputs determined before publishing that result under TPL-12. Ordinary
constructors produce their own Type and take no export selector. There is no
first-output fallback or implicit runtime type generation. The declaration
type and every required dependency must pass the existing finite-composition
and admission checks; syntactic nesting does not waive those checks.

Every ordinary name/type declaration denotes runtime storage. A direct marker
remains invalid as a declaration Type under TPL-14;
references/lists/constants also require ordinary runtime-capable Types.

```text
handler
/example/demo/v1.0.0/src/application types
 .handler type
```

This declaration names the static output `.handler type` from
`/example/demo/v1.0.0/src/application types`. It creates an ordinary `handler`
instance of that returned Type, not an instance of the static module. The selector is one level below the
type path. Any ordinary initializer follows at that same depth:

```text
number
/example/demo/v1.0.0/src/application types
 .integer type
 |3
```

The grammar admits an initializer sequence; static checking permits at most one
initializer for a scalar and any number for a list, one value per element.
Whole-list assignment is not implicitly a declaration initializer. Sources must
be initialized in the specified declaration order. Explicit/inferred fields,
creation-time construction, interface reset, and constant restrictions are
defined in [Declarations and scope](language/declarations.md).

```text
lines[]
//text
 |First line
 |
 |Third line
```

## Static template syntax and design boundaries

```ebnf
<static-body(n)>             ::= <layout(n)> <static-statement(n)> <layout(n)>
                                 { <static-statement(n)> <layout(n)> }
<static-statement(n)>        ::= <static-definition(n)> | <static-producer-use(n)>
<static-definition(n)>       ::= <indent(n)> <placeholder-type> <line-end>
                                 <layout(n)> <indent(n)> <static-value(n)>
<static-value(n)>            ::= <type-name> <line-end> <static-inputs(n+1)>
<static-inputs(n)>           ::= <layout(n)> { <static-input(n)> <layout(n)> }
<static-input(n)>            ::= <indent(n)> <placeholder-type> <space> "<" <space>
                                 <static-value(n)>
<static-producer-use(n)>     ::= <indent(n)> <module-path> <line-end>
                                 <static-inputs(n+1)> <static-outputs(n+1)>
<static-outputs(n)>          ::= <static-output(n)> <layout(n)>
                                 { <static-output(n)> <layout(n)> }
<static-output(n)>           ::= <indent(n)> <placeholder-type> <space> ">" <space>
                                 <placeholder-type> <line-end>
```

An `@...` file with no graph statements denotes a static marker under
[TPL-14](language/static-results.md#tpl-14). A nonempty one consists of static
equations. Both forms have no ordinary variables,
runtime values, mutable state, initializers, calls, conditions, loops, native
bodies, or runtime instances. Every static input and output binding is named:
its label and operand are complete placeholder/type expressions as shown above.
There are no positional static bindings, bare `.` markers, operandless
bindings, skips, or a trailing capture of the expression's own Type. Ordinary
runtime positional arguments and results enumerate their respective interface
declarations.

A `<static-definition>` is a pair: a public receiver name followed by a
same-level value expression. Its RHS denotes an ordinary Type, a static marker
or a partial constructor (Template), never a nonempty static producer itself.
Named inputs may apply the constructor, and their right-hand expressions may nest further
ordinary constructor applications. Its head can also be a supplied Type or
Template name. Without inputs, a supplied name forwards its value unchanged.
The expression's caller writes the leading indentation or operand separator;
`n` is the indentation of its head line and its input bindings occur at `n+1`.

```text
@...
.dictionary type
/example/demo/v1.0.0/src/dictionary
 .key type < .key
 .value type < .value

.forwarded family
.received family

.nested type
/example/demo/v1.0.0/src/outer
 .element < /example/demo/v1.0.0/src/inner
  .value < .value
```

Assume the displayed ordinary constructors have the named parameters used in
these fragments. The definition pairs expose their resulting Types or
Templates under `.dictionary type`, `.forwarded family`, and `.nested type`.
The indented names on the left of `<` belong to the selected constructor;
right-hand names use the supplying static module's scope. A name containing dots
still denotes one complete scoped name, not selection through another name.

A `<static-producer-use>` selects at least one explicitly named result of an
`@...` module. The head is a concrete module path. All input bindings precede
all output bindings at that level. The statement is a set of result equations,
not an effectful call and not an ordinary constructor's own-Type capture.
Its producer's kind and output labels are checked after resolution. A static
producer cannot be used as a `<static-value>` head; obtain its named outputs
and then use those aliases as values. A marker path is a value-expression head,
has no outputs and accepts no input bindings. Its path shares `<module-path>`
syntax; marker versus producer versus ordinary constructor is resolved after
parsing, without alternate tokenization.

```text
@...
/example/demo/v1.0.0/src/prepare types
 .source type < .source
 .prepared family > .family
 .prepared value > .value

.container type
.family
 .element < .value
```

This graph obtains two named outputs and uses them in an ordinary constructor
application. Receiving another output adds another public alias; unselected
producer outputs still undergo the required validation under
[TPL-12](language/static-results.md#tpl-12). No first-output selection is implicit.

Collect all pair receivers and named output receivers throughout the static
module before determining its free input names. Every receiver is public; a
forward reference to a receiver does not introduce a separate input. Remaining
free source names form the named input interface. Foreign formal/output labels
never introduce local parameters. Definitions and aliases have the immutable,
scoped equality semantics in [TPL-02](language/templates.md#tpl-02) and
[TPL-12](language/static-results.md#tpl-12).

There is no explicit signature-declaration tree in this syntax. An incoming
Template's uses contribute named parameter, category and residual-signature constraints
under [TPL-07](language/templates.md#tpl-07) and
[TPL-10](language/templates.md#tpl-10). An occurrence does not invent a complete
signature, forbid unused additional parameters, or choose a constructor from
its capabilities. An application keeps the actual constructor's unapplied
residual parameters; separate applications do not mutate a shared binding map.

In ordinary templates, only interface declaration type expressions introduce
free parameters. Their Type equations come from explicit bindings and typed connections under
[TPL-03](language/templates.md#tpl-03): inputs, explicitly typed output receivers,
defaults, declaration initializers, assignments, reference bindings and aliases.
A literal or an otherwise unconstrained receiver supplies no concrete type
identity. Static propagation neither executes a later statement early nor
relaxes initialization, ownership or storage-form rules.

```text
@<
source&
.data.element type

@>
result&
.data.element type

@!
result&
 source&
```

This ordinary template returns the supplied reference. Its input and result
share one Type parameter; no standalone static declaration is present. A Type
parameter may instead first appear as an ordinary result type:

```text
@>
result
.result type

@!
```

Assuming the selected Type supports default construction, a caller's explicitly
typed receiver can determine that result Type. Receiving into a new unconstrained
name alone supplies no such evidence. Static modules instead declare their
interfaces through graph sources and receivers, without `@<` or `@>` sections.
Neither source form transmits a Type or Template as an ordinary runtime value.

**Explanation: finite syntax and specialization creation.** Nested static
applications use the productions above even when the same template path appears
at several levels. Whether instantiated definitions recursively create
specializations is checked after resolution under
[TPL-11](language/templates.md#tpl-11), using constructor origins and complete
bindings. The parser does not reject repeated spellings, inspect definition
dependencies, impose a template-depth limit, or choose a production by module
lookup. Cycles of static result definitions and unknown-key readiness are
separate semantic checks under TPL-12/TPL-11. These productions claim source
forms, not current implementation support.

## Executable statements and structural recognition

```ebnf
<statement(n)>               ::= <declaration(n)> | <assignment(n)> | <call(n)>
                                 | <constant-read(n)>
                                 | <branch(n)> | <loop(n)> | <foreach(n)>
                                 | <break(n)> | <return(n)> | <suspend(n)>
<assignment(n)>              ::= <indent(n)> <receiver> <line-end>
                                 <layout(n+1)>
                                 <indent(n+1)> <copy-source> <line-end>
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

Standalone static equation graphs belong only to `@...` modules. An ordinary
body has no static alias-definition pair or public static output binding.
Its declarations may contain the structural input bindings and single named
selection of TPL-15; these contribute declaration type constraints, not graph
statements or runtime operations.

**Example: later static evidence, with ordinary execution order.** Assume
`/example/demo/v1.0.0/src/forward` is the template defined in [TPL-04](language/templates.md#tpl-04):

```text
@!
known
//integer

worker
/example/demo/v1.0.0/src/forward

worker !
 < |0
 > inferred

inferred
 known
```

The output receiver first introduces `inferred` under TYPE-06, initially with
the result's symbolic type. The later assignment from `known` constrains that
same field to `//integer`, which also fixes the worker's input/result type.
The literal is then checked against that determined type; its bytes did not
select a type. At runtime the call still precedes the assignment, and the fields
are constructed under the ordinary declaration-order rules.

Recognize the shape before name/type lookup:

- A declaration name followed by a same-level type-expression head is a
  declaration; nested named inputs and an optional single export selector
  precede its ordinary initializers.
- `@<` and `@>` contain runtime name/type declarations. A leading
  `.name` receiver and same-level value expression form a static definition
  only inside an `@...` module. Static binding lines have named `<` or `>`;
  runtime arguments remain inside their ordinary operation's argument block.
- A receiver followed by an indented copy source is an assignment.
- Suffix `!` starts entry execution; suffix `%` resumes.
- An unmarked instance target with output actions reads retained results. A
  type namespace with `>>` reads shared constants. A whole-list parent selects
  intrinsic methods. These forms are distinguished by source tokens and
  indentation before lookup, including for a placeholder namespace head.
- An owned access followed by same-level `?`, `:`, or `{` is control flow. A `:`
  after an eligible operation is instead that operation's refusal handler.
- A bracketed reference-name binder before `{` is foreach. A bracketed owned
  index before `{` is an indexed-condition loop.
- A single nested path of operation links ending in an owned condition and `{`
  is one condition-chain loop.
- Standalone `^`, `>`, and `%` are control statements.

A bare target is invalid. There are no implicit calls, general expression
statements, arithmetic operators, or mandatory `*` name prefixes. A missing
declaration type line cannot turn a second unindented name into an assignment
source. Static graph statements are not executable statement alternatives.

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
<constant-read(n)>           ::= <indent(n)> <constant-namespace(n)>
                                 <constant-read-arguments(n+1)>
<constant-namespace(n)>      ::= <type-name> <line-end>
                                 [ <layout(n+1)> <export-selector(n+1)> ]
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
| `/example/demo/v1.0.0/src/namespace` / `.type` constant read | None | At least one `>>` binding |

Execution modes cannot combine at one operation level. Module paths cannot be
entry/resume targets. A constant-read namespace likewise cannot acquire `!`, `%`,
a reference suffix or acquisition prefix, and constant reads have no refusal
handler. A whole-list parent has neither `!`/`%` nor ordinary bindings. Its methods
accept only `!`;
they are intrinsic interface entries, not first-class values with retained reads
or resume positions. The productions intentionally recognize a broader target
set; the list interface restricts that set during static checking.

### Constant namespace shape

The restricted `<constant-namespace>` form permits
an ordinary module path, a visible Type placeholder, or a static module path
with exactly one indented named export selector. It has no static argument or
result bindings. A placeholder denotes its complete Type; it cannot be applied
as a constructor inside this constant-read namespace.

The namespace must already resolve to one concrete ordinary Type under
[CONST-06](language/declarations.md#const-06) before constant-member lookup.
A selected static producer must be independently determined under
[TPL-12](language/static-results.md#tpl-12); requested constants and receiver
types do not determine its parameters. A known selected nominal Type may
participate in checking while its required definitions are still being validated;
this does not waive their checks or authorize publishing an unchecked export.
A residual Template is not a constant namespace. Namespace selection executes
no runtime operation, and it does not
create an instance. Reuse of a supplied Type and actual constructor applications
retain their distinct provenance under
[TPL-11](language/templates.md#tpl-11).

The optional selector and saved-result bindings are both at depth `n+1`.
The selector, when present, is exactly one complete placeholder line before
every `>>` binding. No later selector can attach after a constant request, and
no `<` or `>` binding is accepted. A constant read has at least one saved-result
binding, no selected operations, and no refusal handler. Named and positional
constant requests retain CONST-03's ordinary saved-read rules.

**Examples: specification fragments, not implementation claims.** Assume the
enclosing interface declares Type parameter `.element type`, its substitution
is already concrete, and that selected Type declares constant `zero`:

```text
.element type
 zero >> initial value
```

Assume `/example/demo/v1.0.0/src/application types` is a closed static producer
exporting a concrete Type as `.container type`, and that Type declares constant
`empty list[]`:

```text
/example/demo/v1.0.0/src/application types
 .container type
 empty list[] >> values[]
```

All type construction occurs in the static module. Replacing its selector by a
named static input/output binding, adding a second selector, or placing the
selector after `>>` is invalid ordinary syntax.

The direct copy-source spelling `/example/demo/v1.0.0/src/palette/red` retains its separate production.
There is no inline static application or export selection inside such a source,
and `.element type.zero` does not mean the `zero` constant of
`.element type`. A bare static module path is not an ordinary constant namespace.

### Binding form and order

All static input and output bindings are named and have operands. Static input
bindings also occur in declaration type expressions under TPL-15; named static
output bindings and result aliases belong to `@...` graphs. No static binding
has a positional counter or an empty skip. At one producer use, input bindings precede output
bindings. An alias-definition RHS and its nested constructor values have input
bindings only. Formal/result labels resolve to the selected producer's interface,
while operands resolve in the supplying lexical scope. Scope, partial
application and use-derived signature constraints follow
[TPL-07](language/templates.md#tpl-07),
[TPL-08](language/templates.md#tpl-08), and
[TPL-12](language/static-results.md#tpl-12) and
[TPL-15](language/templates.md#tpl-15).

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

## Project configuration and dependency selectors

These productions describe configuration inputs, not Ama source sections.
Their semantics are defined once in
[Projects, packages, and repeatable module delivery](project/packages.md).
The line rules of [PKG-03](project/packages.md#pkg-03) deliberately define a
small Ama INI profile; implementations must not inherit a generic parser's
quoting, interpolation, duplicate-key or whitespace behavior.

```ebnf
<ini-comment-line>           ::= ";" { ? Unicode scalar except TAB, CR or LF ? } <newline>
<ini-layout>                 ::= { <blank-line> | <ini-comment-line> }
<ini-value>                  ::= ? nonempty UTF-8 line value without TAB, CR, LF,
                                   leading whitespace or trailing whitespace ?
<alias-key>                  ::= "~" <identifier>
<package-ini>                ::= <ini-layout> { <package-ini-section> <ini-layout> }
<package-ini-section>        ::= <dependency-section> | <package-alias-section>
                                 | <repository-section>
<dependency-section>         ::= "[dependencies]" <newline> <ini-layout>
                                 { <dependency-selector> <newline> <ini-layout> }
<package-alias-section>      ::= "[aliases]" <newline> <ini-layout>
                                 { <package-alias-record> <ini-layout> }
<package-alias-record>       ::= <alias-key> " = " <direct-path> <newline>
<module-ini>                 ::= <ini-layout> { <module-alias-record> <ini-layout> }
<module-alias-record>        ::= <alias-key> " = " <module-path> <newline>
<repository-prefix>          ::= "/" <identifier> { "/" <identifier> }
<repository-section>         ::= "[repository " <repository-prefix> "]" <newline>
                                 <ini-layout> { <repository-record> <ini-layout> }
<repository-record>          ::= ( "url" | "directory" | "revision" ) " = "
                                 <ini-value> <newline>
<mask-character>             ::= <identifier-continue> | "-" | "." | <space>
<selector-component>         ::= ( <mask-character> | "*" )
                                 { <mask-character> | "*" }
<selector-components>        ::= <selector-component> { "/" <selector-component> }
<dependency-selector>        ::= "/" <selector-components>
                                 | <alias-key> [ "/" <selector-components> ]
```

The UTF-8, TAB and physical LF/CRLF rules apply; configuration has no byte-order
mark. As for source, a nonempty unterminated final line has one virtual LF for
recognition. Every nonblank line starts in column zero and has no trailing
whitespace. A comment occupies its whole line and starts with `;`; `#` does not
introduce an INI comment, and `;` inside a value remains data. Assignment joins
are exactly the written `" = "`. Alias names and path components retain their
exact single internal spaces. Values have no quoting or escape processing,
interpolation or environment expansion; a later `=` belongs to the value.

Only the displayed sections and repository keys exist. Each section, alias key
and fixed repository key may occur once in its scope. Duplicate/unknown records
are errors, and the mandatory `url` and `directory` fields are checked under
[PKG-06](project/packages.md#pkg-06). Its prefix ends before the version and
matches whole vendor/package components. The directory field is a relative
physical directory within the Git checkout, not an Ama source path. Empty
configurations and empty optional sections are permitted.

Package aliases have direct path targets. Module aliases may additionally refer
to package aliases, but never to a module alias; shared/local collisions are
errors under [PKG-04](project/packages.md#pkg-04). A module INI file has no section
headers. Package and module aliases have different relative bases under that
rule; both use [SRC-06](language/source-and-names.md#src-06) normalization.

Dependency selectors are absolute ordinary patterns or package-alias patterns,
not standard paths, static names or local module aliases. An alias can supply a
complete version/module selector without a written suffix. First normalize an
alias's real path base; append its pattern suffix afterwards. Selector
components have no leading/trailing whitespace and must not be exactly `.` or
`..`. Do not normalize pattern traversal by cancelling a wildcard component.
A standalone `*` matches a nonempty chain
of path components, whereas an embedded `*` matches zero or more scalars within
one component. Exact literal portions and the set of eligible complete
package/module identities determine all matches under
[PKG-05](project/packages.md#pkg-05). A selector may end at a complete version or
at a complete `src`/`tests` module; the storage names and missing `src`/`tests`
are never inferred. Multiple matching decompositions do not duplicate an
identity. Masks are forbidden in source `<module-path>` operands.

## Native protocol companion syntax

A physical source filename followed by `.abi` names a flat binding table, not
an Ama module. Active `@C` modules require their native companions under SRC-01.
TPL-13's native-leaf check applies to their resolved Ama definitions, not to
the companion-table grammar; native protocols gain no generic binding form.

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
The standalone `-` record symbol remains the native destructor slot; admitting
internal hyphens in Ama names changes neither this token nor the supported C
toolchain's function-identifier rules.

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
