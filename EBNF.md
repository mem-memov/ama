# Ama — Grammar

This grammar derives from [TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md),
the source of truth for Ama. It specifies one minimal, stable language whose
domain capabilities are extended through libraries. [TODO.md](TODO.md) tracks
implementation work; grammar examples express the contract, not implementation
completion. Native details are explained in [VALUE_ABI.md](VALUE_ABI.md).

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

Source is strictly decoded UTF-8; malformed sequences are errors, not name
characters. Identifiers are case-sensitive and compared byte-for-byte, with no
normalization, case folding, or transliteration. Identifier character properties
are `XID_Start` and `XID_Continue` from [Unicode UAX #31](https://unicode.org/reports/tr31/),
with `_` allowed initially and subsequently. Use pinned property tables recorded
with the compiler, independent of the host locale or system-library updates.
Unicode names need not fit C identifiers.

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
<identifier-start>    ::= "_" | ? Unicode scalar with property XID_Start ?
<identifier-continue> ::= "_" | ? Unicode scalar with property XID_Continue ?
<identifier>          ::= <identifier-start> { <identifier-continue> }
<module-path>         ::= "." <identifier> { "." <identifier> }
```

A module path names a type in the project/library namespace. For example,
`.shop.item` identifies module `shop/item`. Duplicate module paths across the
project and imported library are errors.

Submodules of an extensionless module file `shop/item` are stored in its sibling
`shop/item+/` directory: `shop/item+/validate` has logical path
`.shop.item.validate`. Further nesting uses `validate+/details` in the same way.
Ordinary grouping directories contribute normal path components. A structural
`name+/` directory requires its sibling `name` module file; the parent's draft
or executable status does not implicitly activate/inherit into its children.
Strip the structural ASCII `+` when forming logical paths, never from actual
filesystem/include/debug paths. Detect duplicate logical identities and derive
`.c`/`.h`/`.abi` companions from the physical source filename. This convention adds
no `+` to source module-path grammar or runtime ownership rules.

Each path component is an identifier under the same Unicode rules. The host
filesystem must map path spelling unambiguously to source files. Diagnose distinct
project paths that alias under platform case/normalization behavior or cannot be
represented unambiguously; do not normalize, merge, or silently rename identities.

A module path identifies a type or constants namespace, not a callable object.
Executable module types have no implicit/default instances. Shared `@=` storage
is not an executable instance. A pool for a module type does not create an object.
Entry-start, resumption, and retained-result reading select explicit instances
by owned name, reference, or indexed element; whole-list operations select an
explicitly declared collection. Direct path forms such as `.shop.item!`,
`.shop.item%` are invalid. A path followed by `>>` is instead a static read of
its shared `@=` declarations, never an instance call or a read of instance results.
The call-target productions exclude `<module-path>`; a separate constant-read
production admits the shared read.

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

Each module has one declaration namespace for shared constants, parameters,
results, and persistent body declarations, including declarations inside branches/loops. A base name may
be declared only once: `item`, `item&`, `item[]`, and `item&[]` cannot be separate
declarations in the same module. Blocks do not introduce shadowing. Different
modules may reuse names. Uses must match the declared storage form; a missing
suffix must not silently select another declaration. Name visibility does not
relax declaration-order initialization or permit an uninitialized source.
Duplicate-name diagnostics identify both declarations.

### 2.3 Access and reference acquisition

```ebnf
<index>             ::= <owned-access>
<owned-access>      ::= <identifier>
                      | <identifier> "[" <index> "]"
<reference-access>  ::= <identifier> "&"
                      | <identifier> "&[" <index> "]"
<reference-acquisition> ::= "&" <owned-access>
<constant-access>   ::= <module-path> "." ( <owned-name> | <owned-list-name> )
<value-source>      ::= <owned-access> | <reference-access>
                      | <reference-acquisition> | <constant-access>
<whole-list-access> ::= <owned-list-name> | <reference-list-name>
<continuation-copy> ::= "%" ( <owned-access> | <reference-access> )
<copy-source>       ::= <value-source> | <whole-list-access>
                      | <continuation-copy>
<receiver>          ::= <owned-access> | <reference-access>
                      | <whole-list-access>
<instance-target>   ::= <owned-access> | <reference-access>
<call-target>       ::= <instance-target> | <whole-list-access>
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

An index is a named value, not an integer literal. Its native type explicitly
binds the `[]` conversion through its `.abi` companion. The conversion reads
current state without executing or modifying the module and reports `size_t`
or failure. Positions are zero-based: access, replacement, and removal require
`0 <= position < length`; insertion permits `0 <= position <= length`, where
`length` appends. An empty list permits insertion at zero but has no accessible
element. Conversion applies no offset. Negative or unrepresentable values must
be rejected, and list bounds are checked separately. The public library index
type identities are library declarations; the fixed collection interface is
specified below and in `TECHNICAL_SPECIFICATION.md`.

Invalid indexed invocation/read is an acquisition refusal eligible for its `:`.
An unrelated technical conversion failure is fatal, not false or a usable index.
Invalid acquisition/access outside an invocation has no `:` alternative and
follows the technical-failure protocol with source context.

Constants of another module are selected through a qualified source such as
`.std.list.append` or the explicit static path-plus-`>>` read in section 3.3. Collection
length uses the stored first result of the fixed whole-list interface; traversal
uses existing indexed access and loops. No general private-field access syntax is
implied here. Whole-list assignment denotes a type-compatible collection copy; an indexed
access denotes one element. Parameter/result spelling for passing whole lists
needs matching collection declarations in the callee interface.

Prefix `%` requests an independent copy of an instance including its saved Ama
execution position. It immediately precedes the complete owned or reference
access: `%original`, `%target&`, `%items[position]`, or `%targets&[position]`.
It is not reference acquisition and cannot produce a reference or act on a whole
list. See section 4.4 for continuation-copy semantics.

References use checked slot/generation identities, not raw C addresses. Generation
zero denotes an empty handle; all-zero storage is safely empty. Live generations
start at one. Validation checks type, live slot, and matching nonzero generation.
Generation counters never wrap. A deleted slot that has exhausted its generations
is permanently retired. Reclaiming and regrowing pool blocks must preserve
identity history or retire released identities so stale references never revive.
A reference neither owns its target nor keeps it alive. Busy/acquired instances
cannot be deleted. Pool growth does not move live instance states.

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

The native literal interface is immutable UTF-8 bytes plus explicit length,
not host-dependent `wchar_t` or a required NUL-terminated string. A native type
must bind `|` in its `.abi` file to support literal construction. Construction
reports status when attempted; invalid literal initialization reports declaration
context and follows the technical-failure protocol. Arbitrary native constructors
are not executed by the compiler merely to validate source. Equivalent earlier
validation requires an explicitly documented validation mechanism.

## 3. Module sections

```ebnf
<module-file>       ::= <draft-file> | <complete-module-file>
<draft-file>        ::= ? well-formed UTF-8 source without @=, @!, or @C;
                         incomplete declarations need not satisfy active grammar ?
<complete-module-file> ::= <executable-module-file> | <constants-only-file>
<executable-module-file> ::= <layout>
                            [ <constants-section> ]
                            [ <parameters-section> ]
                            [ <results-section> ]
                            <implementation-section>
<constants-only-file> ::= <layout> <constants-section>
<constants-section> ::= "@=" <line-end> <declarations(0)>
<parameters-section> ::= "@<" <line-end> <declarations(0)>
<results-section>    ::= "@>" <line-end> <declarations(0)>
<implementation-section> ::= "@!" <line-end> <body(0)>
                          | "@C" <line-end> <layout>
<declarations(n)>    ::= <layout> { <declaration(n)> <layout> }
<body(n)>            ::= <layout> { <statement(n)> <layout> }
```

| Annotation | Meaning |
|---|---|
| `@=` | Shared immutable constant declarations |
| `@<` | Parameters |
| `@>` | Results |
| `@!` | Executable Ama body |
| `@C` | Companion C implementation |

Annotations start at indentation zero with only trailing spaces/comment.
The order is optional `@=`, optional `@<`, optional `@>`, then `@!` or `@C`.
The constants section may accompany either implementation; it is not an
implementation selector. Repeated, unknown, and out-of-order annotations in an
active file are errors. Literal payload containing annotation text is data.
Only layout follows `@C`; declarations follow `@=`. `@!` may have an empty body.
There is no constructor section, and C code remains in the companion `.c` file.

A file with only `@=` and its declarations/layout is a complete constants-only
module, not a draft. An empty `@=` section is allowed. If parameter or result
sections occur, an implementation is required; `@=` cannot make an unfinished
interface valid. Constants-only files have no executable instance or pool and
cannot be entry, invocation, or resumption targets.

A file with none of `@=`, `@!`, or `@C` is a draft excluded from compilation.
Unfinished declarations require no full parsing/binding/type checking; strict
UTF-8 still applies. Genuine zero-indent markers activate files, not marker text
inside comments/literals or the presence of companion `.c`/`.h`/`.abi` files.
Using a draft as a type or program entry fails with file/reference locations.
Adding `@!`/`@C`, or `@=` for a constants-only file, activates full validation;
incomplete declarations then fail. Optional editor checks cannot make drafts
build inputs. Empty executable modules explicitly use `@!`.

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
may default to zero and text to empty; lists without initializers are empty,
and references without initializers are empty handles. Native resource types
must define whether empty creation is valid. Runtime initialization and copy
failures require cleanup without partially initialized owned state.

### 3.2 Storage duration

Declarations in parameter and result sections define interface storage with
per-execution defaults. Starting with `!` resets that storage before running
the body from its entry. Resuming with `%` preserves the suspended computation
and does not imply this reset. At the end of the call, parameters and results retain their resulting
values in the instance until the next execution resets them. Reading retained
results through `>>` neither resets the interface nor executes the body; it does
not expose saved parameters. Before the first execution, retained results have
their declared initial values.

Declarations in an Ama body define persistent instance state. Their initialization
happens once at instance creation, not each time execution reaches the source
location. Declarations inside control-flow bodies still describe owned state;
they do not conditionally create it during each call.

Owned child states are initialized recursively without executing child bodies.
Infinite eager ownership cycles are forbidden. Recursive type structures through
empty lists or references do not imply recursive eager allocation.

For example, a declared dependency is invoked by its local instance name:

```text
@!
item
.shop.item

item!
```

The declaration initializes owned state once when the enclosing instance is
created; it does not execute the child's body. There is no default `.shop.item`
object selected by a path-based call.

Assignments remain executable actions. Explicit declarations are required;
an unknown assignment receiver does not implicitly create a numeric local.

### 3.3 Shared constants and library enumerations

`@=` declares one immutable shared part before the module's interface and
implementation, or in a complete constants-only module. Its declarations use
normal typed-declaration syntax and the module's one base-name namespace.

Constants are independently copyable immutable values, including library numbers,
text, and enumeration values. They may not contain mutable-object references or
open external-resource identities. Reference/reference-list declarations are not
allowed in `@=`; native payloads cannot hide such references/resources. Constructors
for constants must not execute module bodies, perform external I/O, or depend on
mutable external state. Allocating private value storage is permitted.

Initialize shared values once before root creation, following declaration order
and initialized dependencies. Visible cycles and use-before-initialization are
errors. Initializers allow literals, earlier local constant sources and qualified
constant sources. Cross-module dependencies initialize first while preserving
declaration order in each module; qualification cannot bypass local forward-use
checks or permit cycles. Native constructors must not hide
initialization dependencies and are not executed by the compiler for arbitrary
validation. Initialization failure diagnoses the declaration and prevents root
startup. Shared storage is finalized once after executable instances, in reverse
initialization order, under the common cleanup/error contract.

Shared constants are not pool-slot state, interface defaults to reset, or part of
instance ownership/copy boundaries. They have no busy flag, execution position,
or caller link. They cannot be mutated, invoked/resumed, or acquired as mutable
references. Explicitly copying a constant value into variable storage is allowed
and does not copy an executable namespace or expose the shared storage itself.
Local constants are available by name in subsequent initializers and statements.

To read another module's constants, use its path and `>>` bindings:

```ebnf
<constant-read(n)> ::= <indent(n)> <module-path> <line-end>
                       <layout> <read-arguments(n+1)>
```

For example:

```text
.std.list
 append >> action
```

Named bindings select constant declaration names; positional bindings follow
`@=` declaration order. At least one `>>` receiver is required. The existing
read-mode rules prohibit `<`, `>`, `!`, `%`, mixed named/positional mode, or an
empty `>>`. Resolution is static: missing module/member, draft namespace, or type
mismatch is a compile-time error. No executable instance is acquired or created;
copy failure is a technical error, not an acquisition-refusal alternative.
An instance target with `>>` still reads only stored `@>` results, not constants.

A qualified constant source combines its logical module path and member name:

```text
items[]!
 < .std.list.append
 <
 < &sample

action
.std.list.action
 .std.list.append

action
 .std.list.remove
```

The final path component is always the `@=` declaration name; all preceding
components form the exact module path. Existing `[]` on a constant collection
member is retained. Do not search longest prefixes, import names implicitly,
or reinterpret a failed lookup as an executable module. In a declaration's type
position, a dotted path still names its type; in copy-source position it selects
a constant. Missing/draft namespaces, unknown members, incompatible types and
wrong storage forms are static errors with source locations.

`<constant-access>` is admitted through `<copy-source>` in declaration initializers,
assignment sources and entry-call arguments. It copies a compatible value into
existing destination storage without hidden Ama variables/instances or module
execution. It cannot be a receiver, `!`/`%` target, mutable reference acquisition,
or `%`-copy operand. Literal argument prohibition and output-only resume remain
unchanged. It does not add general expressions, direct condition/index forms,
or whole-list splicing. In `@=` it adds an explicit initialization dependency.
Explicit path-plus-`>>` reads and named local constants retain their meanings.

An enumeration is a library-defined value type with a finite accepted set, exposed
as named shared constants. Its literal constructor interprets symbols/bytes and
rejects unknown values; its clone contract supplies value copying. Different
module types remain incompatible even when their native numeric tags coincide.
No new `enum` keyword, automatic integer numbering, or integer-literal syntax is
introduced. The immutable constant set has no instance pool.

For example, the list library can expose these action values:

```text
@=
append
.std.list.action
 |+

remove
.std.list.action
 |-

@C
```

The member names and type paths are illustrative library identifiers, not reserved
English words. Symbols inside literals are data interpreted by the library.
A caller can obtain a named action through the constant read above and pass its
variable positionally to the fixed list interface. This is target syntax, not a
claim of current execution support.

## 4. Executable statements

```ebnf
<statement(n)> ::= <declaration(n)> | <assignment(n)> | <call(n)>
                 | <branch(n)> | <loop(n)> | <break(n)> | <return(n)>
                 | <suspend(n)> | <constant-read(n)>
```

After a name line, a module path at the same indent identifies a declaration.
An indented copy source identifies assignment. A target immediately followed by
`!` identifies execution from the entry, with or without arguments. An instance
target immediately followed by `%` identifies resumption from a saved position.
An unmarked instance target followed by indented `>>` requests identifies a
retained-result read; a module-path target followed by `>>` identifies a shared
constant read. Recognizing these forms does not require looking up declared
aliases. A bare target without `>>` requests is not a call. `?` and `{` at the
same indent select control flow. No mandatory `*` or call suffix `()` is introduced.

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

Value assignment independently clones owned children and lists. References
stored inside the copy that target any instance in that source ownership tree
are redirected to the corresponding new instance, including references to the
root, ancestors, and other copied list elements. References to external objects
retain their targets. Reference-only assignment copies handle identity without
cloning the target. No references held outside the copy are rewritten.

Shared `@=` constants are outside every instance copy boundary and are never
cloned or reset. A whole owning-list copy has all elements and their descendants
in one copy boundary; copying one element excludes its siblings. A reference-list copy does
not clone its targets. The compiler/runtime performs internal-reference fixups;
opaque native data may not hide persistent Ama handles or pointers.

Ordinary copying clears saved execution positions throughout the new ownership
tree and inherits no outstanding invocation. It preserves retained data according
to the type contract; it does not execute any body. Reference assignment does not
change target positions. Ordinary assignment to exactly the same instance or
collection is a no-op, including its saved position. `%` self-copy is invalid.

Copying between an owner and its owned descendant is forbidden in either direction.
Copying distinct siblings is permitted; non-owning links do not imply ownership
overlap. Reject statically provable violations at compile time, otherwise check
identity/ancestry before mutation. No implicit snapshot makes overlapping copies
valid. A failed standalone copy has no assignment `:` alternative.

For either owned-copy form, every instance in the source ownership tree must
be available, not running/waiting. Protect the validated tree through copying.
External reference targets are not cloned or availability-checked for this copy.
Native copy capability must be explicitly bound with `=`; known unsupported
owned copying is a compile-time error, including nested types and owned lists.
Copying references to such types remains allowed.

Validate that replacement cannot destroy a busy/acquired old descendant, and
prepare the full new tree and reference fixups before replacing the old value.
Preparation failure cleans partial resources and preserves source and destination.
Destruction of the old value begins a nonrollback commit phase; an error there
terminates execution and continues cleanup without exposing partially live state.
The replacement root preserves destination identity. The detailed lifecycle
protocol is specified in `TECHNICAL_SPECIFICATION.md`.

Prefix `%` explicitly requests copying saved execution positions as well:

```text
copy
 %original

other_copy
 %target&
```

The receiver must be distinct owned instance storage of the same module type. This form
is also admitted as a typed declaration initializer. Its allocation, cloning,
and failure rules are described in section 4.4. It does not execute either
instance, acquire a reference, or transfer ownership from the source.

### 4.2 Calls and their alternative

```ebnf
<call(n)>       ::= ( <execution-call(n)> | <resume-call(n)> | <read-call(n)> )
                    [ <indent(n)> ":" <line-end> <body(n+1)> ]
<execution-call(n)>
                ::= <indent(n)> <call-target> "!" <line-end>
                    <layout> <execution-arguments(n+1)>
<resume-call(n)> ::= <indent(n)> <instance-target> "%" <line-end>
                     <layout> <resume-arguments(n+1)>
<read-call(n)>  ::= <indent(n)> <call-target> <line-end>
                    <layout> <read-arguments(n+1)>
<execution-arguments(n)>
                ::= { <indent(n)> <parameter> <line-end> <layout> }
                    { <indent(n)> <result> <line-end> <layout> }
<resume-arguments(n)>
                ::= { <indent(n)> <result> <line-end> <layout> }
<read-arguments(n)>
                ::= <indent(n)> <saved-result> <line-end> <layout>
                    { <indent(n)> <saved-result> <line-end> <layout> }
<parameter>     ::= "<" [ <copy-source> ]
                  | <declaration-name> "<" <copy-source>
<result>        ::= ">" [ <receiver> ]
                  | <declaration-name> ">" <receiver>
<saved-result>  ::= ">>" <receiver>
                  | <declaration-name> ">>" <receiver>
```

Required spaces between adjacent argument components are omitted from the
productions for readability. In execution mode, parameters precede results.
Each form uses positional arguments or named arguments throughout; positional
order follows interface declaration order. Named labels include the storage suffixes from
the interface, such as `target&`.

Positional `<` and `>` lines may omit their source/receiver. Each line still
consumes one declared position; parameter and result counters are independent.
An empty `<` leaves that entry-call parameter at its declared default after reset,
not its old retained value. An empty `>` skips transfer of that result without
creating hidden receiver storage; the target retains the result normally.

For an interface with three parameters and three results:

```text
worker!
 < first
 <
 < third
 >
 > selected
 >
```

Only parameters one and three are supplied, and only result two is received.
Trailing positions may be omitted entirely or explicitly skipped within the
interface's arity. Too many positions are errors even if the extra lines are
empty placeholders. Input lines must precede outputs. Whitespace/comment text
after an otherwise empty marker does not supply an operand. Named argument
forms require a source/receiver; positional placeholders cannot mix with named
arguments. Validate all such rules before target acquisition/reset/mutation.

Resume `%` permits output placeholders but no input lines, even empty `<`.
Retained reads require `>>` with explicit receivers; neither `>` nor an empty
`>>` is admitted in that mode. An empty `>` at argument indentation skips a
result; at the body's statement indentation standalone `>` is early return.

The suffix `!` is mandatory for entry-start calls, including zero-argument calls.
Suffix `%` instead resumes a suspended instance; neither suffix is present on a
retained-result read. Both entry-start and resume operations can change the
target's state. Each suffix follows the complete target immediately, without
intervening whitespace. Combining `!` and `%`, or combining either suffix with
`>>`, is invalid. Arguments remain on separate lines one level deeper; `()` is not used. Name resolution and target
type checks occur after syntax recognition.

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

#### Execution mode: `<` and `>`

A target with `!` performs ordinary execution and cannot contain `>>`. It resets parameter/result
storage to declared defaults, copies supplied inputs, executes the body over
persistent state, and transfers requested results through `>`. Omitted parameters
keep defaults; omitted results remain stored but are not copied to the caller.
A call with no arguments also executes the body from its entry. Starting an
available yielded instance with `!` discards its old saved execution position;
it preserves permanent data and starts at the entry.

Ordinary parameters have value semantics. References copy checked handles, not
states. Parameter/result values are retained after completion; they are not
cleared on return. The next ordinary execution resets them before running again.

#### Retained-result mode: `>>`

A target without either `!` or `%` and one or more `>>` requests forms a
retained-result read. Writing `!` on this target is an error; do not silently reinterpret an
execution-marked call as a read.
This mode does not reset parameters, results, or body state and does not execute
the target body. It copies the requested stored results to the receivers using
their types' value/reference-copy rules.

```text
sensor
 measurement >> value
 error >> last_error

sensor
 >> value
 >> last_error
```

Rules:

- No parameter transmission through `<` is allowed in this mode.
- Every requested result must use `>>`; mixing `>` and `>>` is an error,
  regardless of their order. It is not necessary to request every declared result.
- Only declared results are readable, not parameters or private body variables.
- At least one `>>` is required. A target without `!` and without result
  requests is invalid; zero-argument execution must use `!`.
- `>>` is a single contiguous token. `> >` is not equivalent.
- Validate the target marker and complete argument list before resetting storage
  or beginning any execution. Reject `target!` combined with `>>`, and reject
  `<` or `>` under an unmarked read target, regardless of argument order.
- Omitting `!` means the target body is not executed. Result copies still write
  the caller's receiver variables; it is not a promise of universal immutability.
- This is a read of retained interface values, not a computed getter. References
  in results still identify shared objects; reading does not freeze those targets.

A standalone `>` remains early return, and `?` remains the condition marker.
Neither acquires an additional query meaning.

#### Access checks shared by invocation and reading

The runtime validates and acquires the selected instance for entry-start,
resumption, or retained-result reading. It rejects missing or busy instances and protects
acquired state from deletion or relocation. Hold access throughout the operation,
including all requested result copies, then release it on all supported exit
paths. A second instance of the same module is a separate target. No manual
`busy` result is required.

An attached `:` handles inability to start the call, including a missing target,
busy target, invalid indexed selection, or `%` resumption without a saved
position. It does not handle errors after the
body starts. Failed acquisition does not assign results. Unhandled refusal
reports its cause and active Ama execution trace, then terminates with failure.
Before entering `:` the runtime stores the common `AmaStatus` refusal in the
caller's service context. A normal library reader returns that saved value through
an ordinary declared result; no special binding syntax or magic variable is
introduced. Successful/reader calls do not clear the snapshot. A nested callee's
own refusal updates its context, not its caller's. The next refused operation
initiated by the same instance replaces its snapshot; a previously copied status
variable remains unchanged. New owned copies start with an empty/success refusal
context, not inherited service history. Static type errors remain compile errors.
There are no language-level exceptions.

Zero-argument calls are complete execution-mode statements such as `worker!`
or `target&!`. Retained-result reading omits `!` and requires indented `>>`
requests. Resumption uses `worker%` or `target&%`, with indexed forms such as
`items[position]%`. Resumption of an entire list is not defined.
Whole-list calls use `items[]!` or `targets&[]!`; element calls use
`items[position]!` or `targets&[position]!`. Their retained-read forms omit `!`,
for example `items[position]` followed by an indented `>>` request. Brackets alone
select storage; they do not execute its module. Whole-list calls use the fixed
positional contract below. Literal call arguments and hidden integer temporaries
are forbidden.

#### Whole-list operation contract

Whole-list `!` performs one of six library actions: Inspect, Append, Insert,
Remove, Clear, or Swap. These explanatory names are not reserved words. Action
and outcome are typed enumeration values obtained through shared constant reads,
not a new string argument convention.

Input positions are action (default Inspect), primary position (default zero),
typed source reference (default empty), and secondary position (default zero).
Outputs are current length, completion condition value, and outcome enumeration.
The first/second scalar result types provide the `[]`/`?` library protocols.
Empty positional `<` and `>` entries skip the corresponding positions normally.
Only the inputs relevant to the selected action are interpreted.

Append uses source; Insert uses primary position and source; Remove uses primary
position; Clear and Inspect need neither position nor source; Swap uses both
positions. Owning Append/Insert default-create an element for empty source or
independently clone a nonempty source without saved execution positions.
Reference-list Append/Insert store the handle, including empty, without cloning.
Inspect never changes structure; Swap moves identities only; Remove/Clear obey
ownership destruction and order preservation. Self-Swap and empty Clear succeed.

Invalid positions, unavailable owned-copy sources, and busy/acquired owned deletion
targets are ordinary operation refusals with false completion, a specific outcome,
and unchanged structural state/length. Clear checks all owned targets before
mutation. Reference-list removal never destroys or requires target availability.
Technical allocation/construction/copy/destruction errors follow the common fatal
status/cleanup rules; post-entry outcomes do not select call `:`.

Stored length is correct after list initialization and every completed/refused
structural operation; `>>` reads it without executing a computed getter. Whole-list
results describe the last whole-list operation. Indexed element calls/assignments
do not run list actions. Traversal uses ordinary loops and named current positions,
not new iterator syntax. The complete tables/examples are in the technical
specification; no whole-list resumption is introduced.

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
A standalone `>` completes the current call, leaving its result storage for
the caller and removing its saved execution position. Reaching the end of the
body also completes execution. Reject `^` outside a loop during semantic validation.

Truth conversion is supplied by a library contract mapped by the compiler;
there is no universal `size_t` truth rule in the target language. Reentry into a
busy instance is refused; a different available instance of the same module is
a separate target.
List traversal syntax is not inferred from these condition-controlled loops.

### 4.4 Suspension, resumption, and explicit continuation copying

```ebnf
<suspend(n)> ::= <indent(n)> "%" <line-end>
```

| Form | Operation |
|---|---|
| `task!` | Start at the body entry, preserving permanent instance data |
| `%` on its own line | Suspend the current invocation and return control |
| `task%` | Resume from the saved position |
| `%task` as a source | Independently copy data and the saved execution position |

Reference forms are `target&!`, `target&%`, and `%target&`. Symbols apply to
complete accesses, including indexed accesses. They do not require English
keywords, user-written label names, or new restrictions on Unicode identifiers.

#### Suspension and resumption

A standalone `%` is an executable statement in an Ama body. Once reached, it
records the position immediately after that `%` in the current instance,
then returns control to the current caller without relocating instance data. The caller can receive requested
results produced so far. The instance becomes yielded and available for later
resumption; it is not still busy waiting for that caller. Its completed return
connection is released.

```text
prepare!
%
process!
%
finish!
```

The first `task!` runs `prepare!` and yields. A subsequent `task%` runs `process!`
and yields again. Another `task%` runs `finish!` and completes. `task!` always
starts again with `prepare!`; starting is not resuming. Multiple suspension points
use compiler-generated positions, not user-selected names.

Saved execution position includes the control location inside branches and
loops needed to continue correctly; no hidden C local activation may be needed
across a suspension. `%` cannot appear inside a line literal as an instruction,
in a declaration section, or inside handwritten `@C` code. Native code must
return to Ama before Ama can suspend at one of its own statement boundaries.

A resume uses `>` for requested results, never `>>`, because it executes code.
It preserves permanent state and suspended interface values without an implicit
entry reset. Input bindings through `<` are forbidden on resumption. A resume
accepts no arguments or output requests through `>` only; validate the full
argument list before acquiring or modifying the instance. This is the agreed
language rule, not a temporary restriction. A resume with no saved position is an access refusal handled by the attached `:` alternative;
it must not silently start from the entry.

Suspension at `%` differs from waiting for a nested invocation. In the latter
case, the caller remains busy and retains its outstanding return connection.
Keeping its state in the heap does not make that unfinished invocation resumable
through `task%`. Busy access still follows the refusal/`:` rules above.

#### Explicit continuation copy

```text
continuation
.jobs.task

continuation
 %original

continuation!

continuation
 %original

continuation%
```

Both assignments explicitly clone the source's owned data. The first following
invocation uses `!` and therefore starts at the entry; the second uses `%` and
therefore resumes the copied position. Copying itself never executes or yields.
A copy can also initialize a new owned declaration:

```text
continuation
.jobs.task
 %original
```

Owned children and collections follow the same independent-clone and internal
reference-remapping rules as ordinary copying. `%` preserves the saved position
of every included instance, not only the root; a missing position stays missing.
A root with no position may own yielded children but cannot itself be resumed.
Ordinary copying instead clears positions throughout the copied tree.

Busy flags, pool-slot identity/generation, caller links, and return selectors are
never inherited. Destination replacement preserves its stable identity. Every
included source instance must be available; a running/waiting owned descendant
blocks copying even if the root is available. External referenced objects are
outside that boundary. Copying never duplicates an outstanding invocation chain.

A type without native `=` support cannot be copied as owned state. A supported
clone can still fail technically. Preparation failure releases new resources and
preserves the old destination; destruction errors during replacement commit follow
the terminal cleanup rules. Standalone copy errors use status/diagnostics, not a
new assignment form of `:`. `%` requires a distinct owned destination and rejects
ancestor/descendant overlap just as ordinary copying does.

## 5. Type and implementation boundaries

### Stackless execution requirement

Ama execution must not use the C function-call stack to invoke modules. Ordinary
Ama modules compile to labeled blocks inside one generated C execution function.
Invocation transfers control with `goto` to the module entry; completion transfers
control to the caller's continuation label. Do not replace this with C functions
for individual Ama modules or expand every ordinary module body at every call site.

When a module has several callers, store an explicit continuation selector in
control storage for the active instance and dispatch to the appropriate label,
for example through a standard C `switch` with direct `goto` branches. This is
compiler bookkeeping, not a language-level integer type. Persistent state and
parameter/result data occupy explicit storage; they do not form C stack frames.
Instance state stays in its heap slot and generated code accesses its fields
directly. There is no per-module working area or set of global variables mirroring
all instance fields. Each module has a dynamic pool of independently allocated
blocks; pool growth may relocate the block directory but must preserve live slot
addresses and logical identities. Ama references remain checked slot/generation
handles rather than raw C pointers. Acquisition protects against deletion and
conflicting access; stable addresses do not make stale handles valid.

Before a nested invocation, acquire the target, record caller identity and return
selector in the callee's service fields, mark the caller waiting/busy, and select
the callee. No complete instance payload is copied, moved, saved, or loaded for
this control transfer. Interface reset and input binding remain separate
operations. On completion or yield, select the caller, transfer requested results
from the callee's retained interface while keeping the callee acquired, release
the return connection, update availability, and jump to the continuation. Error
paths must also release acquisitions and maintain valid ownership/control state.
Typed access pointers must be reselected at control-transfer boundaries; no
unfinished C activation is needed to remember the caller.

The explicit return chain is allowed and does not use C stack activations.
Reentry into a busy instance is refused through `:`; a different available
instance of the same module may execute while the first remains waiting in its
own slot. An explicitly yielded instance instead has no outstanding invocation
and is available for `%` resumption. Ordinary value and `%` continuation copying
are separate operations, never implicit parts of invocation or return.

### Program startup

Initialize shared constants successfully before creating the root. Startup also
creates a pool for each participating executable module type, excluding
constants-only modules and shared `@=` storage, then creates one root instance of the selected executable entry module, initializes
its owned state and interface defaults, and starts it at entry. Pool capacity is
separate from live instance count; other types have no implicit/default objects.
Owned declarations, collections, and copy operations explicitly create other
instances. Initializing children does not execute their bodies. Failed root
initialization cleans up constructed resources without starting the root body.
The host receives one of three outcomes: completed, yielded, or failed. A root
`%` returns yielded with all continuation state in instance/runtime storage,
not a preserved C activation. The host may explicitly resume the root without
input bindings, inspect retained results, or destroy it through ownership cleanup.
Completed/failed roots cannot resume; failure is not a yield. The standard launcher
exits successfully on completion and unsuccessfully with diagnostics on failure
or root yield without an explicitly configured handler. There is no automatic
resume loop. Active-instance destruction/cancellation is forbidden; disposal
occurs at a host boundary or after the runtime has ended failed execution and
released its active links. The host entry rule does not permit path invocation
inside Ama source.

### Native C execution boundary

`@C` entry bodies must be inlined into generated execution. Plain `inline`,
`static inline`, and LTO opportunities do not by themselves guarantee that entry
inlining; the supported backend/toolchain must enforce it. Inside native C code,
ordinary C rules apply: helpers, recursion, lifecycle callbacks, allocation, I/O,
and external library calls may use the C stack and need not inline transitively.
Native code must still respect ownership, reference lifetime, and acquisition.
It must not dispatch Ama modules through ordinary C calls or bypass runtime
cleanup. Native calls finish before returning to Ama; `%` cannot capture an
unfinished C activation or yield through an active native call.

Stacklessness concerns Ama module control flow, not the entire executable's
hardware-stack usage. Native calls, compiler spills/helpers, and startup may use
the stack. Verify Ama label transfers and native-entry inlining separately from
permitted C calls inside native implementations. The compiler's own C code is
also outside this execution constraint.

### Errors and debugging

Expected application failures are ordinary declared results. Technical failures
use the common native status: `uint32_t code`, `const char *message`, and
`size_t message_length`. Zero code means success; an all-zero status is success
without text. Nonzero codes distinguish refusals from technical failures. A
reserved invalid-index code can select an indexed invocation's `:`; unrelated
technical conversion errors cannot. Optional message bytes are immutable UTF-8
with explicit length and program lifetime, without a required NUL. Null message
requires zero length. The runtime borrows this storage; diagnostics do not require
heap allocation. Classify by code, never message wording.

An unhandled technical failure reports a diagnostic and ends current Ama execution
with a failed host outcome. The standard launcher exits unsuccessfully; an embedding
host receives failed status, not a recoverable Ama exception. Prior field changes,
output, and external side effects are not rolled back. Preparation failures preserve
an old replacement destination; destruction failure after commit begins is terminal.
Cleanup continues after destroy errors, retains the primary cause, adds secondary
errors, and never blindly repeats destruction of a partly finalized resource.

Diagnostics must include the current operation and active caller chain using
source module names, files, and locations. The compiler maps operation and call-site
identifiers to source spans; the runtime records the current operation and caller
sites in service metadata. Emit or capture the trace before destroying caller
links. Allocation-failure diagnostics must not require allocating a heap trace
buffer. Pre-execution initialization failures identify their initialization
context rather than inventing a call chain.

A last-acquisition-refusal status belongs to each calling instance's service
context and is initially zero/success. Its library reader obtains a read-only
value snapshot from the immediate caller using generated runtime access, without
retaining metadata pointers or altering busy/return fields. The shared status
format does not make technical failures catchable by `:`; post-entry list refusals
still use ordinary result values. Fatal diagnostics use the actual active error,
not a previously handled refusal. A successfully read nonzero refusal code is
ordinary result data, not a technical failure of the reader. See ERROR-06 in the
technical specification.

The trace is the active Ama path, not an invocation history. After `%`, an old
caller link no longer exists; resumption uses its new chain. Native code returns
normally with a status before Ama error reporting. Its internal C stack is not
automatically part of an Ama trace; native debuggers and `#line` source mappings
provide complementary facilities. Undefined native behavior and hardware faults
are not guaranteed managed traces. No additional exception syntax is introduced.

### Type boundaries

Every declaration has a module-defined type. Integer representation, arithmetic,
range, and overflow belong to library types. Compiler mappings to index and
condition operations must validate signatures and check host-size conversions.
Type names such as `.std.integer` and `.std.text` in examples are illustrative,
not already implemented library paths.

Native modules use the same Ama interfaces and instance call syntax, selecting
`@C`. Companion `.c` supplies implementations, `.h`/generated headers supply
representations and checked declarations, and `.abi` binds fixed protocols.
Native parameters/results use their declared C representations or typed borrowed
views of bound interface storage, not unchecked casts based on names. A borrowed
view must not turn an owned-value parameter into an alias of the original source.

Persistent Ama ownership and references exist only in declared, compiler-visible
Ama fields. Opaque C payloads own native resources only: buffers, native scalars,
files, sockets, and similar resources. They must not hide Ama handles, instance
or field addresses, collection-buffer addresses, return links, or resume positions,
including identities encoded as integers/bytes. `void *` is not an escape hatch
for persistent Ama links. Native code may borrow generated views during an
operation, but may not retain them in payloads, globals, or external callbacks.
Persistent relationships must be stored in declared Ama reference fields through
the generated access contract. Native code cannot edit runtime control metadata.

The compiler/runtime clones the visible ownership/reference graph and performs
all internal-reference fixups. Native clone hooks copy opaque native resources
only; no hidden-reference tracing or native graph-fixup protocol is required.
See [VALUE_ABI.md](VALUE_ABI.md) and the technical specification for the
complete native and lifecycle contract.

Generated C identifiers use deterministic hashes of complete source identities,
with collision checks, rather than ASCII restrictions or slash substitution.
Emit correctly escaped `#line` directives containing actual source paths,
including Unicode names, for source diagnostics and debugging. These mechanisms
must preserve source identity; line mapping alone does not restore variable names.

## 6. Native protocol companion (`.abi`)

A native type has a companion at its physical source filename plus `.abi`. This file is a
UTF-8 binding table, not an Ama module. It uses these six fixed symbols:

| Symbol | Native operation | Required |
|---|---|---|
| `+` | Default construction | Yes |
| `-` | Destruction | Yes |
| `\|` | Construction from literal bytes | Only for literal support |
| `=` | Independent native-data cloning | Only for owned-copy support |
| `?` | Read-only current-state conversion to C `bool` | Only for condition support |
| `[]` | Read-only current-state conversion to C `size_t` | Only for index support |

One binding declares support and names its C function. There are no separate
support flags, inference by function name, or runtime registration. Missing optional
support is an error only when an operation requires it. Missing `+` or `-`, unknown
symbols, duplicate bindings, malformed records, or incompatible C signatures are
compile-time/build errors. A type may be constructible without being copyable,
condition-capable, index-capable, or literal-capable.

```ebnf
<abi-file>         ::= { <abi-record> | <abi-blank> | <abi-comment-line> }
<abi-symbol>       ::= "+" | "-" | "|" | "=" | "?" | "[]"
<abi-function>     ::= ? one C-function identifier accepted by the supported toolchain ?
<abi-record>       ::= <abi-symbol> <space> { <space> } <abi-function> <line-end>
<abi-blank>        ::= { <space> } <newline>
<abi-comment-line> ::= { <space> } <comment> <newline>
```

Records start at column one, in any order. Blank lines, comments introduced by
`#`, trailing spaces and trailing comments are allowed. No sections, nesting,
includes, macros, expressions, or executable bodies are allowed. LF and CRLF
are supported, final newline is optional, lone CR and tabs are invalid. Leading
spaces are allowed only on blank/comment-only lines. `[]` is one contiguous symbol.
The bindings do not add source-level Ama operators or English reserved words.

```text
+ integer_initialize
- integer_destroy
| integer_from_literal
= integer_clone
? integer_to_condition
[] integer_to_index
```

All hooks use the common status contract, including destruction. Native module
execution reports that status through generated service output, separate from
its declared Ama results. The C compiler verifies generated typed uses against
native declarations. `?` and `[]` inspect state without executing/resetting the
module or mutating state; `>>` remains a read of stored results, not a conversion.

The lifecycle selects one constructor over fresh native storage: default `+`,
literal `|`, or clone `=`. There is no implicit default-construction followed by
replacement. Owned Ama fields are constructed in declaration order before their
native owner's payload. Failed native constructors clean their own partial
resources; the runtime cleans already-live children in reverse order, without
calling `-` on a failed payload. Destruction calls native `-` once while its owned
fields still exist, then destroys those fields in reverse construction order.
Errors do not stop remaining cleanup. Native hooks never own or independently
destroy the generated Ama fields. `.abi` fixes the protocol and lifetime rules;
exact generated C aliases/prototypes are backend implementation artifacts.

## 7. Conformance

Parsing, binding, lowering, code generation, runtime services, libraries and tests
must implement this grammar consistently with the technical specification.
Unsupported implementation work must be tracked explicitly; it does not introduce
alternate syntax or semantics. Exact algorithms and their formal justification
must satisfy ARCH-05/ARCH-06 in the specification.
