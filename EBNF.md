# Ama — Grammar

This document specifies the agreed target syntax of Ama. **The compiler does
not yet implement this revision.** In particular, typed declarations, `|`
literals, references, lists, `@!`, and `@=` require implementation work.
The mandatory call suffix `!` and loop exit `^` are implemented. See
`README.md` for the currently executable language and `TODO.md` for the migration
plan. Examples below describe the target language, not runnable current fixtures.

Hash-based internal naming and file/line source mappings have been implemented
independently, and the current compiler now uses parsing, AST validation, and
C emission as separate stages. That does not imply support for the target syntax
below.

This document supersedes the previous syntax specification for future work;
it does not claim that the existing parser already accepts the new grammar.

## 1. Notation, encoding, and layout

| Notation | Meaning |
|---|---|
| `::=` | Definition |
| `"x"` | Terminal text |
| `{ x }` | Zero or more repetitions |
| `[ x ]` | Optional part |
| `x \| y` | Alternative |
| `? ... ?` | Informal constraint |

The productions below use indentation parameters: `<statement(n)>` starts at
indentation `n`, and `<indent(n)>` means exactly `n` spaces. This parameterized
notation expresses layout rules; `n` is not source text. Unless stated otherwise,
lines permit trailing spaces and an optional comment. A final newline is optional.

Source is UTF-8. Identifiers are case-sensitive and compared byte-for-byte;
there is no implicit normalization, case folding, or transliteration. Unicode
names must not be restricted to fit C identifiers. The current approximation
allows bytes at or above 0x80 in names; strict UTF-8 identifier validation remains
a separate implementation decision.

LF and CRLF delimit physical lines; a lone CR is invalid. Indentation uses
spaces, exactly one per additional nesting level. Tabs cannot be indentation
or ordinary code whitespace. Literal payload is data rather than code whitespace.
Blank lines are formatting, never section separators.

```ebnf
<newline>       ::= ? LF or CRLF ?
<space>         ::= " "
<indent(n)>     ::= ? exactly n spaces ?
<comment>       ::= "#" { ? any character except a line ending ? }
<line-end>      ::= { <space> } [ <comment> ] <newline>
<blank-line>    ::= { <space> } <newline>
<comment-line>  ::= { <space> } <comment> <newline>
<layout>        ::= { <blank-line> | <comment-line> }
```

Outside a literal, `#` begins a comment. Inside a literal it is ordinary data.
Comments do not count as blank lines when enforcing adjacency of a condition
and its structural marker.

## 2. Names, types, references, and literals

### 2.1 Identifiers and paths

```ebnf
<letter>        ::= "a".."z" | "A".."Z" | ? a byte >= 0x80 ?
<digit>         ::= "0".."9"
<identifier>    ::= ( <letter> | "_" ) { <letter> | <digit> | "_" }
<module-path>   ::= "." <identifier> { "." <identifier> }
```

A module path names a type in the project/library namespace. For example,
`.shop.item` identifies module `shop/item`. Duplicate module paths across the
project and imported library are errors.

A declaration's type line contains only the module path and optional trailing
layout. Neither `&` nor `[]` belongs on that line.

### 2.2 Declaration names

```ebnf
<owned-name>           ::= <identifier>
<reference-name>       ::= <identifier> "&"
<owned-list-name>      ::= <identifier> "[]"
<reference-list-name>  ::= <identifier> "&[]"
<declaration-name>    ::= <owned-name> | <reference-name>
                       | <owned-list-name> | <reference-list-name>
```

| Form | Meaning |
|---|---|
| `item` | An owned instance/value |
| `target&` | A typed reference to an instance |
| `items[]` | An owned dynamic list of instances/values |
| `targets&[]` | An owned list of references to independent instances |

There is no mandatory `*` prefix. `&` after the identifier denotes reference
storage. `[]` denotes a collection; it does not encode a fixed size. A reference
list owns its slots, not the referenced objects. A reference to a whole list
is not introduced by `targets&[]`.

Exact name-uniqueness rules across storage forms still need finalization.
Duplicate declarations are errors; a declaration and its uses must agree on
storage form. In particular, do not silently select a different declaration
when a suffix is missing.

### 2.3 Access and reference acquisition

```ebnf
<index>             ::= <owned-access>
<owned-access>      ::= <identifier>
                      | <identifier> "[" <index> "]"
<reference-access>  ::= <identifier> "&"
                      | <identifier> "&[" <index> "]"
<reference-acquisition> ::= "&" <owned-access>
<value-source>      ::= <owned-access> | <reference-access>
                      | <reference-acquisition>
<whole-list-access> ::= <owned-list-name> | <reference-list-name>
<copy-source>       ::= <value-source> | <whole-list-access>
<receiver>          ::= <owned-access> | <reference-access>
                      | <whole-list-access>
<call-target>       ::= <owned-access> | <reference-access>
                      | <whole-list-access>
```

Examples:

```text
target&
 &item

other&
 target&
```

Prefix `&` obtains a reference to an owned object. Suffix `&` reads a reference
variable. Reference acquisition does not transfer ownership or extend lifetime.
References to references are forbidden: `&target&` is invalid. An acquisition
operand must resolve to owned instance storage, not a reference or whole list.

An index is a named value, not an integer literal. Its type and checked
conversion are supplied by the standard library through compiler mappings.
Index origin and the exact mapped interfaces remain to be specified. Invalid
positions must not expose memory. Failure handling for reference acquisition
outside calls still needs a separate rule.

General member-access spelling and collection-length access are not finalized
here. Whole-list assignment denotes a type-compatible collection copy; an indexed
access denotes one element. Parameter/result spelling for passing whole lists
needs matching collection declarations in the callee interface.

### 2.4 Line literals

```ebnf
<literal-line(n)> ::= <indent(n)> "|" <literal-payload> <newline>
<literal-payload> ::= { ? any character except a physical line ending ? }
```

The first `|` after indentation introduces data through the end of the physical
line. The marker and line ending are excluded. All following content, including
spaces, `#`, quotes, backslashes, tabs, `@`, and further `|` characters, belongs
to the payload. It is not tokenized as code or an annotation.

```text
message
.std.text
 |He said: "Hello!" # this is data
```

- `|` alone is an empty literal.
- `||text` is a literal whose content begins with `|`.
- There are no quotes, escape sequences, multiline literals, or continuations.
- Several lines are represented as a list of individual string values.
- There is no integer-literal expression category. `255` without `|` is not a
  valid numeric source, and `|255` is valid only in declaration initialization.
- The declared type interprets the payload. The language does not assume that
  `255` is a machine integer or that `-5` is unsigned subtraction.

The proposed native literal interface is immutable UTF-8 bytes plus an explicit
length, not host-dependent `wchar_t` and not a required NUL-terminated string.
The final native decoding ABI and initialization-error timing remain open.

## 3. Module sections

```ebnf
<module-file>       ::= <layout>
                        [ <parameters-section> ]
                        [ <results-section> ]
                        [ <implementation-section> ]
<parameters-section> ::= "@<" <line-end> <declarations(0)>
<results-section>    ::= "@>" <line-end> <declarations(0)>
<implementation-section> ::= "@!" <line-end> <body(0)>
                       | "@C" <line-end> <layout>
                       | "@=" <line-end> <layout>
<declarations(n)>    ::= <layout> { <declaration(n)> <layout> }
<body(n)>            ::= <layout> { <statement(n)> <layout> }
```

| Annotation | Meaning |
|---|---|
| `@<` | Parameters |
| `@>` | Results |
| `@!` | Executable Ama body; replaces `@Ama` |
| `@C` | Companion C implementation |
| `@=` | Constant module; replaces the proposed `@enum` |

Annotations start at indentation zero on their own lines. Only spaces and an
optional comment follow them. Repeated, unknown, or out-of-order annotations
are errors. An annotation-looking sequence inside a `|` payload is data.

Either declaration section can be absent. The implementation selector is last;
only one selector is allowed. `@!` can have an empty body. There is no constructor
section. `@C` selects the companion file at the module path plus `.c`; C code
is never embedded in Ama. Only comments and blank lines follow `@C` or `@=`.

A missing implementation selector denotes a draft, not an empty executable or
constant module. Drafts do not execute and cannot be entry modules or referenced
types/callees. Preserve unfinished-file tolerance without treating indented
literal payload as an annotation. The precise draft-validation migration is
implementation work.

`@Ama` and `@enum` are superseded spellings, not alternatives in this target
grammar. The current compiler still uses `@Ama` until migrated.

### 3.1 Typed declarations

```ebnf
<declaration(n)>    ::= <indent(n)> <declaration-name> <line-end>
                       <layout>
                       <indent(n)> <module-path> <line-end>
                       <initializers(n+1)>
<initializers(n)>   ::= <layout> { <initializer(n)> <layout> }
<initializer(n)>    ::= <literal-line(n)>
                     | <indent(n)> <copy-source> <line-end>
```

A scalar declaration permits zero or one initializer. A list permits zero or
more; each initializes one element. The grammar admits a sequence, with this
scalar/list cardinality enforced semantically. A whole-list source cannot be
silently interpreted as one scalar list element: nested collection types and
splicing are not defined here.

```text
count
.std.integer
 |255

other_count
.std.integer
 count

lines[]
.std.text
 |First line
 |
 |Third line

target&
.shop.item
 &item
```

Sources must be type-compatible and already initialized. Initialization follows
declaration order; forward or cyclic initialization is not implied by name
visibility. Literal bytes cannot forge a reference. A reference initializer is
a compatible reference variable or acquisition of an owned instance.

Without an initializer, use the declared type's initial-state contract. Numbers
may default to zero, text to empty, and lists to empty. Native resource types
must define whether empty creation is valid. Runtime initialization and copy
failures require cleanup without partially initialized owned state.

### 3.2 Storage duration

Declarations in parameter and result sections define per-call storage/defaults.
Declarations in an Ama body define persistent instance state. Their initialization
happens once at instance creation, not each time execution reaches the source
location. Declarations inside control-flow bodies still describe owned state;
they do not conditionally create it during each call.

Owned child states are initialized recursively without executing child bodies.
Infinite fixed-size ownership cycles are forbidden. Future recursive structures
through empty lists or references do not imply recursive eager allocation.

Assignments remain executable actions. Explicit declarations are required;
an unknown assignment receiver no longer implicitly creates a numeric local.

### 3.3 Constant modules

A module ending in `@=` has no parameters or executable body. Its result section
contains initialized constants, available without executing the module. Constants
cannot be assignment receivers or mutable references. Initially, actions can use
constants of a library integer type; other constant-capable types, evaluation
rules, and member-selection syntax still need specification.

```text
@>
none
.std.integer
 |0
append
.std.integer
 |1
@=
```

## 4. Executable statements

```ebnf
<statement(n)> ::= <declaration(n)> | <assignment(n)> | <call(n)>
                 | <branch(n)> | <loop(n)> | <break(n)> | <return(n)>
```

After a name line, a module path at the same indent identifies a declaration.
An indented source identifies assignment. A target immediately followed by `!`
identifies a call, with or without arguments; recognizing it does not depend on
looking up declared aliases. A bare target is not a call. `?` and `{` at the same
indent select control flow. No mandatory `*` or call suffix `()` is introduced.

### 4.1 Assignment

```ebnf
<assignment(n)> ::= <indent(n)> <receiver> <line-end>
                    <layout>
                    <indent(n+1)> <copy-source> <line-end>
```

The source is exactly one indentation level deeper than the receiver. Both
must resolve to declared, compatible storage. No literal is allowed here:
initialize a named value in a declaration and assign from that value instead.

```text
destination
 source

target&
 &item

other&
 target&
```

Value assignment copies owned state under its type contract. Owned children and
lists are independent; external references retain their targets. Reference
assignment copies identity without ownership. Unsupported copies must fail
explicitly rather than aliasing resources silently. Destination replacement and
failure cleanup must follow the type's lifecycle contract.

### 4.2 Calls and their alternative

```ebnf
<call(n)>       ::= <indent(n)> <call-target> "!" <line-end>
                    <layout> { <argument(n+1)> <layout> }
                    [ <indent(n)> ":" <line-end> <body(n+1)> ]
<argument(n)>   ::= <indent(n)> <parameter> <line-end>
                  | <indent(n)> <result> <line-end>
<parameter>     ::= "<" <copy-source>
                  | <declaration-name> "<" <copy-source>
<result>        ::= ">" <receiver>
                  | <declaration-name> ">" <receiver>
```

Required spaces between adjacent argument components are omitted from the
productions for readability. Parameters precede results. A call uses either
positional arguments or named arguments throughout; positional order follows
interface declaration order. Named labels include the storage suffixes from
the interface, such as `target&`.

The suffix `!` is mandatory for every call, including calls without arguments.
It immediately follows the complete target, with no intervening whitespace.
Arguments remain on separate lines one level deeper; `()` is not used. Name
resolution and target type checks occur after syntax recognition.

```text
worker!
 value < count
 result > answer

consumer!
 target& < &item

target&!
 value < count
:
 handle_unavailable!
```

The runtime validates and acquires the selected instance before executing it.
A call resets parameter/result storage to its declared defaults, copies inputs,
executes the body over the selected persistent state, and transfers requested
results. Omitted parameters keep defaults; omitted results are ignored. Ordinary
parameters have value semantics. References copy checked handles, not states.

The runtime marks the instance busy while executing, protects it from deletion
or relocation, and releases access on all supported exit paths. A second instance
of the same module is a separate target. No manual `busy` result is required.

An attached `:` handles inability to start the call, including a missing target,
busy target, or invalid indexed selection. It does not handle errors after the
body starts. Failed acquisition does not assign results. The reason-binding
syntax and exact unhandled-failure policy remain to be finalized; the proposed
default is a diagnostic and termination. Static type errors remain compile errors.

Zero-argument calls are complete statements such as `worker!` or `target&!`.
Whole-list calls use `items[]!` or `targets&[]!`; element calls use
`items[position]!` or `targets&[position]!`. Brackets without the final `!`
select storage; they do not execute its module. Collection operation signatures,
length access, traversal, and mutation restrictions are still library/runtime
contract work. Literal call arguments and hidden integer temporaries are forbidden.

### 4.3 Branches and loops

```ebnf
<branch(n)>     ::= <indent(n)> <owned-access> <line-end>
                    <indent(n)> "?" <line-end> <body(n+1)>
                    [ <indent(n)> ":" <line-end> <body(n+1)> ]
<loop(n)>       ::= <indent(n)> <owned-access> <line-end>
                    <indent(n)> "{" <line-end> <body(n+1)>
                    <indent(n)> "}" <line-end>
<break(n)>      ::= <indent(n)> "^" <line-end>
<return(n)>     ::= <indent(n)> ">" <line-end>
```

A condition and its marker have the same indent. Comments can separate them,
but a blank line cannot. True/false branch bodies use one deeper indent. A
loop re-evaluates its condition each iteration; a standalone `^` exits the
innermost loop, including when nested inside branches. `worker!` invokes
`worker`; `@!` is the module-body annotation. A standalone `!` is not a valid
statement in the target syntax.
A standalone `>` returns from the current call, leaving its result storage for
the caller. Reject `^` outside a loop during semantic validation.

Truth conversion is supplied by a library contract mapped by the compiler;
there is no universal `size_t` truth rule in the target language. Existing
compile-time restrictions on recursive calls are not removed by this grammar.
List traversal syntax is not inferred from these condition-controlled loops.

## 5. Type and implementation boundaries

Every declaration has a module-defined type. Integer representation, arithmetic,
range, and overflow belong to library types. Compiler mappings to index and
condition operations must validate signatures and check host-size conversions.
Type names such as `.std.integer` and `.std.text` in examples are illustrative,
not already implemented library paths.

Native modules use the same Ama interfaces and call syntax, selecting `@C`.
The future ABI supplies generated typed state/reference declarations and explicit
lifecycle operations. Native implementations may hold `void*` internally but
must not infer layout from parameter names or accept arbitrary casts as type
checking. Handwritten native code includes generated headers. Its layouts,
copy/cleanup hooks, status protocol, and literal parser ABI remain to be finalized.
The currently implemented `size_t` native ABI is not a specification of this
future typed interface.

Generated C identifiers use deterministic hashes of complete source identities,
with collision checks, rather than ASCII restrictions or slash substitution.
Emit correctly escaped `#line` directives containing actual source paths,
including Unicode names, for source diagnostics and debugging. These mechanisms
must preserve source identity; line mapping alone does not restore variable names.

## 6. Migration boundary

This revision requires coordinated lexer, section splitter, parser, symbol/type,
codegen, runtime, native library, documentation, and test changes. In particular:

- Replace `@Ama` with `@!` and introduce `@=` constant modules.
- Replace numeric-default declarations with explicit module types and optional
  indented `|` literals or named-value initializers.
- Remove integer literal arguments, literal assignments, implicit numeric locals,
  and the old same-indent assignment source.
- Introduce mandatory `!` on all call targets; remove bare calls and the
  parser's dependence on declared aliases for call recognition. Preserve
  argument indentation and parameter/result order.
- Replace standalone `!` breaks with `^`; reject `^` outside loops and reject
  standalone `!` statements.
- Introduce suffix-reference names, prefix acquisition, indexed and collection
  access, checked calls, and type-directed value copying.
- Preserve persistent body state while introducing typed initialization.

Do not silently treat target examples as current regression fixtures. Tests for
implemented behavior remain separate until the corresponding migration lands.
