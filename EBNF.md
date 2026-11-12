# Ama — Grammar

[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md) defines the language's
semantics and implementation requirements. This document defines its source
forms and the static distinctions needed to read them. Requirement identifiers
refer to that specification; implementation progress does not change the grammar.

Ama composes module-defined values and operations. Native C modules provide the
primitive contracts; Ama declarations and control flow compose them without
adding arithmetic expressions, implicit conversions, or English operation names.
Checking that composition does not prove arbitrary C implementations correct.

## Contents

- [1. Notation and physical lines](#1-notation-and-physical-lines)
- [2. Names and storage forms](#2-names-and-storage-forms)
- [3. Literal data](#3-literal-data)
- [4. Module files and declarations](#4-module-files-and-declarations)
- [5. Executable statements](#5-executable-statements)
- [6. Invocation and reading](#6-invocation-and-reading)
- [7. Branches, loops, return, and suspension](#7-branches-loops-return-and-suspension)
- [8. Native protocol companions](#8-native-protocol-companions)
- [9. Semantic and implementation boundaries](#9-semantic-and-implementation-boundaries)

## 1. Notation and physical lines

| Notation | Meaning |
|---|---|
| `::=` | Definition |
| `"x"` | Exact terminal text |
| `{ x }` | Zero or more repetitions |
| `[ x ]` | Optional part |
| `x \| y` | Alternative |
| `? ... ?` | Explicit lexical or contextual constraint |

`<statement(n)>` starts at indentation `n`; `<indent(n)>` is exactly `n` ASCII
spaces. The parameter `n` is notation, not source text. Every additional nesting
level adds exactly one space. Suffixes such as `&`, `[]`, `!`, and `%` are adjacent
to their operand; their productions do not permit intervening whitespace.

Source is strictly decoded UTF-8. Malformed sequences are errors, including in
drafts, comments, and literals. LF and CRLF terminate physical lines. A lone CR
is invalid. For parsing only, a final physical line with bytes but no line ending
has one virtual LF boundary. It adds no source byte, payload byte, or extra blank
line. An empty file has no physical line. Existing terminal LF/CRLF receives no
additional boundary. `<newline>` below includes that single virtual boundary.
Recognizing a complete file consumes all its physical lines; no unparsed suffix
is ignored. Layout may have more than one grammatical attachment but contributes
no declarations, operands or control nodes.

```ebnf
<newline>      ::= ? physical LF, physical CRLF, or the final virtual LF boundary ?
<space>        ::= " "
<separator>    ::= <space> { <space> }
<indent(n)>    ::= ? exactly n ASCII spaces ?
<comment>      ::= "#" { ? any Unicode scalar except CR or LF ? }
<line-end>     ::= { <space> } [ <comment> ] <newline>
<blank-line>   ::= { <space> } <newline>
<comment-line> ::= { <space> } <comment> <newline>
<layout>       ::= { <blank-line> | <comment-line> }
<comment-gap>  ::= { <comment-line> }
```

Tabs cannot occur in indentation or ordinary code whitespace. They may occur in
Ama comment text or literal payload. Outside literal payload, the first `#`
starts a comment; no preceding space is required. Blank lines and comments are
formatting, never section delimiters. `<comment-gap>` permits comments but no
blank line between a condition and its `?` or `{` marker. See SYN-01/SYN-02.

## 2. Names and storage forms

### 2.1 Identifiers and module paths

```ebnf
<identifier-start>    ::= "_" | ? Unicode scalar with property XID_Start ?
<identifier-continue> ::= "_" | ? Unicode scalar with property XID_Continue ?
<identifier>          ::= <identifier-start> { <identifier-continue> }
<module-path>         ::= "." <identifier> { "." <identifier> }
```

Use Unicode 17.0.0 identifier properties, pinned under SRC-03.
Identifiers are case-sensitive and compared byte-for-byte, without normalization,
case folding, transliteration, or host-locale rules. Chinese and other Unicode
alphabets are allowed. A Unicode name need not be a C identifier. See SRC-03.

A path names a module type or its shared constants namespace. It never selects
an implicit executable object. `.shop.item!` and `.shop.item%` are invalid.
Executable operations select declared instances; a path with `>>` instead reads
shared constants. Constants-only modules have no executable instance or pool.

Physical source `shop/item` has logical path `.shop.item`. Its submodule source
`shop/item+/validate` has path `.shop.item.validate`; `+` belongs only to physical
storage. Ordinary directories contribute normal path components. Companions are
derived from the physical filename: `item.c`, `item.h`, and `item.abi`. Exact
lookup, duplicate rejection, draft dependencies, and platform aliases follow
SRC-01 through SRC-05. Generated C identities follow NAME-01 through NAME-05.

### 2.2 Declaration names

```ebnf
<owned-name>          ::= <identifier>
<reference-name>      ::= <identifier> "&"
<owned-list-name>     ::= <identifier> "[]"
<reference-list-name> ::= <identifier> "&[]"
<declaration-name>    ::= <owned-name> | <reference-name>
                        | <owned-list-name> | <reference-list-name>
```

| Name form | Storage |
|---|---|
| `item` | One owned instance/value |
| `target&` | One non-owning typed reference |
| `items[]` | A dynamic list owning its elements |
| `targets&[]` | A dynamic list owning reference slots, not their targets |

The type line contains only a module path and trailing layout. Storage markers
belong to the name line. A reference list is not a reference to a whole list.

One module has one declaration namespace, covering constants, parameters,
results, and all body declarations, including declarations inside control flow.
Body declarations include result-inferred fields under TYPE-06.
`item`, `item&`, `item[]`, and `item&[]` share the base name `item` and cannot be
separate declarations there. Blocks introduce no shadowing. Different modules
may reuse names. Uses must match declared storage forms. See NAME-04.

### 2.3 Accesses and sources

```ebnf
<index>                 ::= <owned-access>
<owned-access>          ::= <identifier>
                          | <identifier> "[" <index> "]"
<reference-access>      ::= <identifier> "&"
                          | <identifier> "&[" <index> "]"
<reference-acquisition> ::= "&" <owned-access>
<constant-access>       ::= <module-path> "." ( <owned-name> | <owned-list-name> )
<whole-list-access>     ::= <owned-list-name> | <reference-list-name>
<continuation-copy>     ::= "%" ( <owned-access> | <reference-access> )
<value-source>          ::= <owned-access> | <reference-access>
                          | <reference-acquisition> | <constant-access>
<copy-source>           ::= <value-source> | <whole-list-access>
                          | <continuation-copy>
<indexed-receiver>      ::= <identifier> "[" <index> "]"
                          | <identifier> "&[" <index> "]"
<receiver>              ::= <declaration-name> | <indexed-receiver>
<instance-target>       ::= <owned-access> | <reference-access>
<call-target>           ::= <instance-target> | <whole-list-access>
```

`&item` forms a checked reference to owned storage. `target&` reads a reference
variable. `&target&` is invalid: there are no references to references. Reference
formation does not execute the target, transfer ownership, or extend lifetime;
it differs from runtime acquisition of protected access for an operation.
References are checked identities, not exposed C addresses. Empty/stale handles
may be copied as values; using one to select an instance requires validation.
See REF-01/REF-02 and REF-04 through REF-08.

`items[position]` selects an owned element; `targets&[position]` selects a reference
slot. An index is a named owned value, possibly another indexed value, whose type
provides the read-only native `[]` conversion. It is not a numeric expression.
Positions start at zero. Access/replacement/removal require `position < length`;
insertion permits `position == length`. Conversion and bounds are checked
separately. See LIST-02 and PROTO-03 through PROTO-05.

For `.std.list.append`, the last component is the constant declaration name and
all preceding components are the exact module path. Do not search longest
prefixes or reinterpret failed lookups. A type-line path names a type; a
copy-source path selects a constant. `[]` on a constant member denotes its whole
collection. Qualified constants are never receivers, mutable-reference sources,
instance targets, or continuation-copy operands. See CONST-03/CONST-05.

### 2.4 Storage compatibility

The productions recognize forms before binding. Static checking then requires
the same declared module identity and the following storage relationship.
There is no implicit dereference, reference acquisition, list splicing, or
conversion between different module types.

| Destination | Ordinary compatible source |
|---|---|
| Owned scalar | Owned scalar access or scalar constant |
| Reference scalar | Reference access or explicit `&` formation |
| Whole owning list assignment | Whole owning list or owned constant list |
| Whole reference-list assignment | Whole reference list |
| One owning-list initializer | One owned scalar, scalar constant, or supported literal |
| One reference-list initializer | One reference access or explicit `&` formation |

An owned scalar declaration also permits a supported literal. A reference
declaration never accepts a literal. Each declaration initializer constructs one
value/element; a whole-list initializer is not an implicit list copy.

Explicit `%owned` or `%reference&` sources instead clone an instance and its saved
execution position into a distinct owned scalar destination. `%` does not produce
a reference or copy a whole list. Ordinary reference copying copies the handle;
it does not clone the referent. Copy capability, ownership overlap, source-tree
availability, internal-reference remapping, and failure behavior follow TYPE-05,
REF-03, and COPY-01 through COPY-07. These are semantic checks, not parser guesses.

## 3. Literal data

```ebnf
<literal-payload> ::= { ? any Unicode scalar except CR or LF ? }
<literal-line(n)> ::= <indent(n)> "|" <literal-payload> <newline>
```

There are exactly two literal positions: a declaration initializer whose first
non-indentation character is `|`, and an entry-call input operand beginning with
`|` after `<` and its required separator. Everything after that `|` through the
physical line boundary is payload, including trailing spaces, tabs, `#`, quotes,
backslashes, `@`, and further bars. The marker and line boundary are excluded.
No trailing comment is recognized in payload.

```text
message
.std.text
 |He said: "Hello!" # this is data
```

`|` supplies empty bytes; `||text` starts with a literal bar. There are no quotes,
escapes, multiline literals, interpolation, or integer-literal expressions.
Several lines of data are represented by a list of separate string values.
`255` is not a source; `|255` is bytes interpreted by the receiving type.

The native interface receives immutable UTF-8 bytes plus explicit length, not
`wchar_t` or a required NUL-terminated string. Literal capability requires the
native `|` binding. Select its constructor over fresh payload storage without
first default-constructing that payload. Constructor failure is technical, not
an acquisition refusal; arbitrary constructors are not run by the compiler to
guess validity. See LIT-01 through LIT-04, CALL-07, and LIFE-01/LIFE-02.

## 4. Module files and declarations

```ebnf
<module-file>            ::= <draft-file> | <complete-module-file>
<draft-file>             ::= ? well-formed UTF-8 source without a genuine
                              zero-indent @=, @!, or @C activation marker ?
<complete-module-file>   ::= <executable-module-file> | <constants-only-file>
<executable-module-file> ::= <layout>
                             [ <constants-section> ]
                             [ <parameters-section> ]
                             [ <results-section> ]
                             <implementation-section>
<constants-only-file>    ::= <layout> <constants-section>
<constants-section>      ::= "@=" <line-end> <declarations(0)>
<parameters-section>     ::= "@<" <line-end> <declarations(0)>
<results-section>        ::= "@>" <line-end> <declarations(0)>
<implementation-section> ::= "@!" <line-end> <body(0)>
                            | "@C" <line-end> <layout>
<declarations(n)>        ::= <layout> { <declaration(n)> <layout> }
<body(n)>                ::= <layout> { <statement(n)> <layout> }
<declaration(n)>         ::= <indent(n)> <declaration-name> <line-end>
                             <layout>
                             <indent(n)> <module-path> <line-end>
                             <initializers(n+1)>
<initializers(n)>        ::= <layout> { <initializer(n)> <layout> }
<initializer(n)>         ::= <literal-line(n)>
                            | <indent(n)> <copy-source> <line-end>
```

| Annotation | Meaning |
|---|---|
| `@=` | Shared immutable constants |
| `@<` | Parameters |
| `@>` | Results |
| `@!` | Ama implementation |
| `@C` | Companion C implementation |

Markers start at indentation zero with only trailing spaces/comment. Sections
occur in the order shown, at most once each. Only layout follows `@C`; C code
stays in its companion `.c` file. `@!` may have an empty body. `@=` alone may
form a complete, empty constants-only module. Parameters/results require an
implementation. Unknown/repeated/out-of-order annotations in active files fail.

A draft is excluded from compilation and need not have complete declarations.
Only genuine annotation lines activate it: marker text in comments/literals and
companion files do not. A referenced draft is an error. See SYN-03 through SYN-05.

Scalars permit at most one initializer; lists permit any number, one per element.
The grammar admits the sequence and static checking enforces cardinality and
storage compatibility. Sources must already be initialized. Initialization follows
declaration order; visibility does not permit forward/cyclic initialization.

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

References without initializers are empty; lists without initializers are empty.
Owned values use their declared type's construction contract. Body declarations initialize
once at instance creation, including declarations written in branches/loops;
they are not executable assignment statements. Parameters/results initialize at
creation and reset on entry-start, with the selected input constructors described
below. Resume preserves them. See TYPE-01 through TYPE-06 and STATE-01/STATE-02.

A first-occurring simple output receiver is also a declaration under TYPE-06,
with no separate module-path line or initializer. Its type comes from the selected
result and its explicit name suffixes must match that result's storage form.
It contributes a persistent field at its receiver's lexical position and follows
the same creation-time default construction and declaration-order rules, even
inside unexecuted branches/loops. The call/read later transfers a value into it.

Shared `@=` values are immutable, independently copyable, and initialized once
before the root. Reference/reference-list declarations are forbidden in `@=`;
constant values cannot contain mutable Ama references or native resource identities.
They have no pool identity, busy state, caller link, or continuation.
Their dependency order, constructor purity, copying, and cleanup follow STATE-03
and CONST-01 through CONST-05. Enumerations are library-defined value types
exposed as shared constants; there is no separate enumeration keyword.

## 5. Executable statements

```ebnf
<statement(n)> ::= <declaration(n)> | <assignment(n)> | <call(n)>
                  | <constant-read(n)> | <branch(n)> | <loop(n)>
                  | <break(n)> | <return(n)> | <suspend(n)>
<assignment(n)> ::= <indent(n)> <receiver> <line-end>
                    <layout>
                    <indent(n+1)> <copy-source> <line-end>
```

An assignment has exactly one source one indentation level below its receiver.
Both refer to declared compatible storage. Unknown standalone assignment receivers
do not create variables; TYPE-06 inference applies only to output bindings.
Literals occur in declarations or entry inputs, never assignments.

```text
destination
 source

target&
 &item

continuation
 %original
```

Recognition uses structure, not declared types or guessed names:

- A declaration name followed by a module path at the same indentation declares
  storage. Layout may separate those lines.
- A receiver followed by an indented copy source is assignment.
- An adjacent suffix `!` starts execution; `%` resumes execution.
- An unmarked instance/list target with indented `>>` bindings reads retained
  results; a module-path target with `>>` reads shared constants.
- Simple output receivers may declare storage under TYPE-06 after parsing; this
  classification does not change the syntactic recognition of a call/read.
- An owned access followed at the same indentation by `?` or `{` selects control
  flow. Only comment lines may intervene.
- A standalone `^`, `>`, or `%` is a control statement.

A bare target is invalid. There are no implicit calls, expression statements,
arithmetic operators, or mandatory `*` name prefixes. Binding/type/ownership
checks follow syntax recognition. See COMP-01 and REF-03.

## 6. Invocation and reading

### 6.1 Productions

```ebnf
<call(n)> ::= ( <execution-call(n)> | <resume-call(n)> | <read-call(n)> )
              [ <indent(n)> ":" <line-end> <body(n+1)> ]
<execution-call(n)> ::= <indent(n)> <call-target> "!" <line-end>
                        <layout> <execution-arguments(n+1)>
<resume-call(n)> ::= <indent(n)> <instance-target> "%" <line-end>
                     <layout> <resume-arguments(n+1)>
<read-call(n)> ::= <indent(n)> <call-target> <line-end>
                   <layout> <read-arguments(n+1)>
<constant-read(n)> ::= <indent(n)> <module-path> <line-end>
                       <layout> <read-arguments(n+1)>
<execution-arguments(n)> ::= { <parameter-line(n)> <layout> }
                             { <indent(n)> <result> <line-end> <layout> }
<resume-arguments(n)> ::= { <indent(n)> <result> <line-end> <layout> }
<read-arguments(n)> ::= <indent(n)> <saved-result> <line-end> <layout>
                        { <indent(n)> <saved-result> <line-end> <layout> }
<parameter-line(n)> ::= <indent(n)> <parameter> <line-end>
                       | <indent(n)> <literal-parameter> <newline>
<parameter> ::= "<"
               | "<" <separator> <copy-source>
               | <declaration-name> <separator> "<" <separator> <copy-source>
<literal-parameter> ::= "<" <separator> "|" <literal-payload>
                       | <declaration-name> <separator> "<" <separator>
                         "|" <literal-payload>
<result> ::= ">"
            | ">" <separator> <output-receiver>
            | <declaration-name> <separator> ">" <separator> <output-receiver>
<saved-result> ::= ">>" <separator> <output-receiver>
                  | <declaration-name> <separator> ">>" <separator> <output-receiver>
<output-receiver> ::= <receiver>
```

Every binding component uses the explicit separators above. `>>` is contiguous;
`> >` is invalid. A literal input consumes its line ending directly, so `#` in
its payload cannot become a trailing comment.

### 6.2 Mode and binding rules

| Form | Body execution | Input bindings | Output bindings | Interface reset |
|---|---|---|---|---|
| `worker!` | From entry | `<` | `>` | Yes |
| `worker%` | From saved position | None | `>` | No |
| `worker` | None | None | At least one `>>` | No |
| `.namespace` | None; shared constants | None | At least one `>>` | None |

`!` and `%` follow the complete target immediately. They cannot combine with each
other or with `>>`. Zero-argument execution still requires `!`; zero-output resume
still requires `%`. Module paths cannot be execution/resume targets. A constant
read has no `:` alternative because it does not acquire an executable instance.

An operation is entirely named or entirely positional, across inputs and outputs.
Named labels match declared names including storage suffixes. Each input label
and each output label may occur at most once. Inputs precede outputs. Positional
input/output counters are independent and follow their interface declaration
orders. Whole-list operations use only the fixed positional interface.

```text
worker!
 input < source
 output > destination

consumer!
 target& < &item

sensor
 measurement >> latest
 error >> last_error
```

An empty positional `<` consumes an input position and requests its declared
default, not its old retained value. An empty positional `>` consumes an output
position without copying it. Trailing positions may be absent. Every explicit
placeholder counts against arity. Named bindings and `>>` require operands.
There are no empty-input placeholders on resume. See CALL-01 through CALL-06.

```text
worker!
 < first
 <
 < third
 >
 > selected
 >
```

This supplies inputs one/three and receives only result two. Spaces/comments
after an otherwise empty marker do not supply an operand.

### 6.3 Result-inferred declarations

A simple `<output-receiver>` (`name`, `name&`, `name[]`, or `name&[]`) declares
a persistent caller field if it is the first local variable-name occurrence of
that base identifier in the module's source text. Otherwise it uses existing
storage. This rule applies to named/positional outputs from `!`, `%`, instance/list
`>>`, and constant `>>` reads, subject to the existing mode rules: whole-list
operations remain positional and cannot resume. Empty `>` declares nothing;
indexed receivers cannot declare their collection or index.

After parsing, scan local variable-name occurrences in source-byte order,
including explicit declaration names, operands, call targets, receivers, and names
inside indices; visit all branches/loops in written order. Track encountered
base names independently of suffixes. Module-path components, qualified constant
members, callee-side binding labels, comments, and literal payloads do not count.
A local constant used by its unqualified name does count. A simple output receiver
declares a field exactly when its base name is absent from the encountered set;
then add the base name of every occurrence, including unresolved uses. The set
therefore equals the local base names in each scanned prefix. Neither execution
nor later explicit declarations change this classification. See TYPE-06.

Resolve the selected result by its interface position or label and infer its
exact module type, using LIST-04's typed interface for whole-list results and
the selected `@=` declaration for constant reads. Require the receiver's explicit
storage form to match exactly: references require `&`, owning lists `[]`, and
reference lists `&[]`. Missing/mismatching suffixes are errors. Target types come
from explicit declarations or earlier inferred receivers; inference dependencies
strictly decrease in source order. Do not infer from runtime values or callee bodies.

An earlier local use prevents inference at a later output; unresolved names remain
errors unless supplied by a valid explicit declaration under the existing rules.
A later explicit declaration of an inferred base name is a duplicate. In
`result > result`, the left label is not a local occurrence, so the right receiver
may declare a field. Repeated receiver occurrences update the same field; blocks
introduce no scope or conditional construction. Inferred fields participate in
normal initialization-order, eager-cycle, default-construction and clone checks.

For a declared `worker` returning, in order, an owned scalar, reference scalar,
owning list, and reference list, this declares four previously unseen base names:

```text
worker!
 > value
 > target&
 > values[]
 > targets&[]
```

The receiving fields exist with their default/empty values before execution.
A binding transfers the complete returned value by ordinary copy semantics:
owned payload/interface/body fields and descendants are independently copied,
with internal-reference remapping; whole owning lists use one copy boundary;
references/reference lists copy handles without cloning their targets. COPY-03
clears copied saved positions, subject to COPY-06's self-assignment no-op.
Preserving a continuation still requires a separate `%source` operation allowed
by CONT-04; `%source` is not an output receiver. Inference does not alter result-transfer
or lifecycle semantics. See CALL-02 and COPY-01 through COPY-07.

### 6.4 Typed literal inputs and evaluation order

```text
writer!
 < |Hello # this is data

writer!
 message < |Hello
```

The bound parameter supplies the type. It must be a single owned value whose
type supports `|`; references and whole-list parameters require ordinary sources.
`< |` supplies an empty literal; `<` requests the default. Each literal consumes
one input position. No hidden source variable, numeric inference, list splicing,
or reference to a temporary is introduced. See CALL-07.

Validate the complete operation statically before acquisition or mutation.
For entry-start only, resolve/preserve ordinary input sources in binding source
order before resetting target interface storage. Install parameters in declaration order, then initialize
results in declaration order. Use exactly the selected fresh-payload constructor
for each value: default, literal, or independent clone; do not default-construct
a supplied value first. Enter the body only after all inputs and results are ready.
Resume skips this preparation; retained and constant reads skip preparation and
body execution and create no invocation return link.

Requested outputs transfer in binding source order. Resolve each receiver and
its index when that transfer is reached: an earlier output may change the index
used by a later one, and a later output may overwrite the same receiver. Repeated
callee result labels remain forbidden. Each replacement follows its own
preparation/commit contract; there is no simultaneous assignment or whole-call
rollback guarantee. See CALL-08 and LIFE-01 through LIFE-05.

### 6.5 Acquisition and retained values

Entry-start resets interface storage but preserves permanent state. Resume
preserves the suspended interface and permanent state. Completion and yield retain
interface values; `>>` reads declared results only, without resetting storage or
executing a getter. Before first execution results have their initial values.
Result copies still write caller receivers; a read is not universal immutability.

Entry, resume, and retained reads acquire their selected target through result
transfer. Missing/stale/busy targets, invalid indexed selection, and resume without
a saved position are acquisition refusals. Attached `:` handles only that refusal,
not constructor/copy/body errors. No results are assigned on acquisition refusal.
An inferred receiver remains at its default/empty or previously stored value.
Without `:`, refusal ends execution with a diagnostic. Its saved status is readable
through an ordinary library module, not a magic field. See CALL-04/CALL-05,
ERROR-05/ERROR-06, and RT-03/RT-04.
An unrelated technical index-conversion failure is fatal even before body entry;
it does not select `:`. Static constant reads have no acquisition refusal.

```text
target&!
 input < source
:
 unavailable_handler!
```

### 6.6 Whole-list operations

`items[]!` and `targets&[]!` operate on the collection, not on every element body.
Their fixed positional inputs are action, primary position, source reference,
and secondary position. Their outputs are current length, completion condition,
and outcome. Omitted inputs use Inspect, zero, empty reference, and zero.

The six actions are Inspect, Append, Insert, Remove, Clear, and Swap. Those names
explain behavior; they are not reserved words. Typed action/outcome constants and
index/condition types belong to the library. No raw action-string parser is added
to the collection interface. See LIST-04 and LIST-08 through LIST-12.

Owned insertion default-creates or independently clones an element. Supplying
input three on an owning list requires element clone capability statically, even
if its handle is empty or the runtime action ignores that input. Omit/skip that
input to default-create noncopyable elements. Reference-list insertion stores a
handle. Removal/Clear preserve remaining order and check the complete owned
subtrees before destruction. Expected post-entry refusals report ordinary results,
not the call's `:`. Technical failures follow the common cleanup contract.
Retained `>>` reads stored collection results; no whole-list resume exists.

## 7. Branches, loops, return, and suspension

```ebnf
<branch(n)> ::= <indent(n)> <owned-access> <line-end> <comment-gap>
                <indent(n)> "?" <line-end> <body(n+1)>
                [ <indent(n)> ":" <line-end> <body(n+1)> ]
<loop(n)> ::= <indent(n)> <owned-access> <line-end> <comment-gap>
              <indent(n)> "{" <line-end> <body(n+1)>
              <indent(n)> "}" <line-end>
<break(n)>   ::= <indent(n)> "^" <line-end>
<return(n)>  ::= <indent(n)> ">" <line-end>
<suspend(n)> ::= <indent(n)> "%" <line-end>
```

Conditions are named owned values using the read-only native `?` conversion.
There is no built-in nonzero-integer truth rule. A loop reevaluates its condition
before each iteration. `^` exits the innermost loop and is invalid outside one.
Standalone `>` completes the current invocation; reaching the body end also
completes it. At call-argument indentation, `>` is instead an output binding.
Standalone `!` is invalid. See FLOW-01 through FLOW-03.

Standalone `%` saves the position immediately following it, yields to the caller,
and releases that return connection. `task%` resumes from that position; `task!`
discards it and starts at entry. A completed instance has no saved position.
Suspension inside branches/loops retains the needed control location. Waiting
for a nested invocation is busy, not yielded. See CONT-01 through CONT-03.

```text
prepare!
%
process!
%
finish!
```

Prefix `%` copies owned data and all included saved positions into a distinct
owned destination. It inherits no pool identity, busy flag, caller link, or pending
invocation. Ordinary copying clears saved positions instead. Neither copy executes
the source. Native activations cannot suspend or become Ama continuations.
See CONT-04 through CONT-06 and COPY-01 through COPY-07.

## 8. Native protocol companions

A native module's physical source filename plus `.abi` names a binding table,
not an Ama module. Active `@C` modules require `.h`, `.c`, and `.abi` companions
under SRC-01. The table declares six fixed protocol slots:

| Symbol | Operation | Required capability |
|---|---|---|
| `+` | Default construction | Required |
| `-` | Destruction | Required |
| `\|` | Literal construction | Literal support |
| `=` | Independent native-payload clone | Owned copying |
| `?` | Read-only conversion to C `bool` | Conditions |
| `[]` | Read-only conversion to C `size_t` | Indices |

```ebnf
<abi-file>         ::= { <abi-record> | <abi-blank> | <abi-comment-line> }
<abi-symbol>       ::= "+" | "-" | "|" | "=" | "?" | "[]"
<abi-function>     ::= ? one module-local C function identifier accepted by
                        the supported C toolchain ?
<abi-record>       ::= <abi-symbol> <separator> <abi-function> <line-end>
<abi-blank>        ::= { <space> } <newline>
<abi-comment-line> ::= { <space> } <comment> <newline>
```

Records start at column one, in any order. Leading spaces are allowed only on
blank/comment-only lines. Tabs are invalid throughout `.abi`, including comments.
The physical-line/EOF rules from section 1 still apply. `[]` is contiguous.
There are no sections, nesting, includes, macros, expressions, or executable
bodies. Duplicate slots, unknown symbols, missing `+`/`-`, and invalid signatures
are errors. An optional slot is required only when its operation is used.

```text
+ initialize
- destroy
| from_literal
= clone
? to_condition
[] to_index
```

Binding identifiers are local to the native module. The generated module prefix
qualifies them in the project C assembly. Companion C uses `AMA_NATIVE(name)` to
define/refer to such project-owned native symbols; the binding itself contains
the local C identifier. This facility does not parse/rewrite arbitrary C or rename
third-party symbols. See NAME-05 and PROTO-01/PROTO-02/PROTO-11.

Native hooks use the common status and lifecycle contracts. Native payloads may
own native resources but may not hide persistent Ama references, storage addresses,
or runtime control links. Declared Ama fields remain visible to the compiler and
runtime. See CABI-01 through CABI-05, PROTO-03 through PROTO-10, and LIFE-01
through LIFE-05. Missing capability is not inferred from function names.

## 9. Semantic and implementation boundaries

The following constraints complete this grammar without adding source syntax:

| Contract | Specification |
|---|---|
| Exact namespaces, physical submodules, source identity, native qualification | SRC-01–SRC-05, NAME-01–NAME-05 |
| Declared/inferred types, initialization order, persistent/interface state, constants | TYPE-01–TYPE-06, STATE-01–STATE-03, CONST-01–CONST-05 |
| Checked references, ownership, independent copying, deletion safety | REF-01–REF-08, COPY-01–COPY-07 |
| Call modes, binding order, literals, refusals | CALL-01–CALL-08 |
| Truth conversion, loops, suspension, continuation copies | FLOW-01–FLOW-03, CONT-01–CONT-06 |
| Collection order, operations, refusal outcomes | LIST-01–LIST-12 |
| Stable pools, acquisition, return chains, host outcomes | RT-01–RT-16 |
| One C assembly, native ABI, lifecycle, label transfers, permitted native stack use | GEN-01–GEN-04, CABI-01–CABI-05, PROTO-01–PROTO-11, LIFE-01–LIFE-05, STACK-01–STACK-04 |
| Technical status, fatal cleanup, active-path diagnostics | ERROR-01–ERROR-06, DEBUG-01–DEBUG-04 |
| Compiler decomposition, exact algorithms, invariants and acceptance evidence | COMP-01–COMP-05, ARCH-01–ARCH-06, TEST-01–TEST-08 |

Ama module execution lowers to labels and explicit runtime continuations in one
project C assembly; it does not use C calls for Ama-to-Ama control flow. Native
entry bodies inline, while ordinary calls inside native C may use the C stack.
Instances execute directly in stable pool storage. There is no implicit default
instance, global working-state mirror, automatic recursion stack, or exception
syntax. These are semantic requirements, not optional backend interpretations.
