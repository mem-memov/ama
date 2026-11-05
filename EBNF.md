# Ama — Grammar

This document specifies the syntax of the Ama language. It is the normative
reference for the parser in `src/parser/` and the lexer in `src/lexer/`.

The notation is EBNF:

| Symbol        | Meaning                              |
|---------------|--------------------------------------|
| `::=`         | definition                           |
| `"x"`         | literal terminal                     |
| `{ x }`       | zero or more repetitions of `x`      |
| `[ x ]`       | optional (zero or one)               |
| `x \| y`      | alternative                          |
| `? ... ?`     | informal description                 |
| `( x )`       | grouping                             |

---

## 1. Encoding and line endings

Source files are UTF-8, byte-oriented.

| Aspect          | Rule                                                     |
|-----------------|----------------------------------------------------------|
| Encoding        | UTF-8. Non-ASCII bytes (≥ 0x80) are allowed in identifiers. |
| Line ending     | `\n` is canonical. `\r\n` is normalized to `\n`.          |
| Lone `\r`       | Rejected: "stray `\r` (not followed by `\n`)".           |
| Tab (`\t`)      | Rejected anywhere in the file: "tab character not allowed". |
| BOM             | Not specially handled. A leading BOM becomes part of the first identifier (usually invalid). |

---

## 2. Lexical grammar

### 2.1 Characters

```ebnf
<letter>        ::= "a".."z" | "A".."Z" | ? any byte >= 0x80 ?
<digit>         ::= "0".."9"
<underscore>    ::= "_"
```

The "byte >= 0x80" rule is a pragmatic approximation: any UTF-8 lead or
continuation byte counts as an identifier character. This covers all
non-ASCII Unicode letters but does not validate the code point.

### 2.2 Identifiers

```ebnf
<identifier>    ::= <ident-start> { <ident-char> }
<ident-start>   ::= <letter> | <underscore>
<ident-char>    ::= <letter> | <digit> | <underscore>
```

Identifiers are case-sensitive and compared byte-for-byte.

### 2.3 Literals

```ebnf
<literal>       ::= <digit> { <digit> }
```

Only nonnegative decimal integers are supported. No sign, no hex, no float, no
string literals. Subtraction is unsigned arithmetic; it does not produce signed negative values.

### 2.4 Paths

```ebnf
<path>          ::= "." <identifier> { "." <identifier> }
```

A path always starts with `.` and represents a reference to a module or its
member. Used only in the params section as a value for an alias.

The path `.math.square` is normalized at compile time to `math/square`
(dots become slashes, leading dot dropped). The path `.sys.math.add` becomes
`sys/math/add`.

### 2.5 Whitespace and indentation

```ebnf
<space>         ::= " "
<indent>        ::= <space> { <space> }
```

Indentation is **semantic**: it defines block structure. Only spaces are
allowed; tabs are rejected. The indent of a line is the number of leading
spaces before the first non-space character.

A line's "logical indent" is one more than its parent's. Jumps of two or
more levels are a syntax error ("indent cannot jump by more than one level").

### 2.6 Comments

```ebnf
<comment>       ::= "#" { ? any character except "\n" ? }
```

A comment runs from `#` to end of line. Comments are ignored by the parser
but **do not** count as blank lines for the `has_preceding_blank_line` flag (see
§3.3). A line consisting only of `#...` is not a blank line.

### 2.7 Punctuation

```ebnf
<arrow-less>    ::= "<"
<arrow-greater> ::= ">"
<question>      ::= "?"
<colon>         ::= ":"
<lbrace>        ::= "{"
<rbrace>        ::= "}"
<bang>          ::= "!"
```

---

## 3. Physical structure of a module file

A module has optional parameter and result sections and at most one
implementation selector, `@Ama` or `@C`. Every present section starts with an explicit annotation. Blank lines
are whitespace, never section delimiters.

### 3.1 File structure

```ebnf
<module-file>       ::= <section-layout>
                        [ <parameters-section> ]
                        [ <results-section> ]
                        [ <implementation-section> ]

<parameters-section> ::= "@<" <annotation-tail> <declaration-body>
<results-section>    ::= "@>" <annotation-tail> <declaration-body>
<implementation-section>
                    ::= "@Ama" <annotation-tail> <code-section>
                      | "@C" <annotation-tail> <section-layout>

<annotation-tail>   ::= { <space> } [ <comment> ] <newline>
<declaration-body>  ::= <section-layout>
                        { <declaration> <section-layout> }
<section-layout>    ::= { <blank-line> | <comment-line> }
<code-section>      ::= { <code-line> }
```

Rules:

- An annotation starts at indentation zero and occupies its own line. Only
  spaces and an optional `#` comment may follow it.
- The order is `@<`, `@>`, then `@Ama` or `@C`. Repeated or out-of-order
  annotations, including two implementation selectors, are errors.
- Either declaration section may be absent; an absent section is empty.
- Every present section may also be empty. A minimal empty module is `@Ama`.
- A file without an implementation selector is a draft. Its declarations
  are not parsed, so incomplete declaration pairs are allowed in drafts.
  Drafts are not emitted and do not receive native headers or stubs.
- A draft cannot be the entry module or an alias dependency. Other unreferenced
  drafts do not prevent compilation. Structural validation still applies:
  unknown/repeated annotations, forbidden tabs, and stray CR are errors.
- Content before the first annotation is an error, except blank lines and
  comments. The old implicit blank-line format is not accepted.
- `@C` selects the companion source at the Ama file path plus `.c` and
  allows only blank lines or comments afterward. C code is never embedded
  in an Ama source file.
- Unknown annotations and general-purpose annotation arguments are errors.
- An indented line beginning with `@` is also rejected, including inside a body.
- A final newline is optional, including after the final `@Ama` annotation.

Blank lines and comments are permitted throughout each section. A blank line
contains only spaces before its newline. CRLF line endings are accepted and
the physical line numbers are preserved for diagnostics. Annotation lines are
excluded from the token ranges passed to the declaration and code parsers.

### 3.2 Declarations (params and returns)

```ebnf
<declaration>   ::= <identifier-line> <section-layout> <value-line>

<identifier-line> ::= <identifier> <newline>
<value-line>      ::= <value> <newline>

<value>         ::= <literal>
                  | <identifier>
                  | <path>
```

A declaration is a pair: an identifier on one line at indent 0, and a value
on the next nonblank, noncomment line at indent 0. An annotation ends the
section, so an incomplete declaration cannot consume a value from another section.

Semantics:

- The identifier becomes a symbol in the current module.
- If the value is a `<path>`, the symbol is an **alias**: it refers to the
  module at that path. Aliases do not hold numeric values. The parser currently accepts them in
  either declaration section; use the params section by convention.
- If the value is a `<literal>`, the symbol's default is that integer.
- If the value is an `<identifier>` (a variable reference), the compiler
  currently rejects it: "defaults that reference other variables are not
  supported yet".

In the params section, non-alias declarations become `SYMBOL_PARAMETER`. In the
returns section, non-alias declarations become `SYMBOL_RESULT`.

### 3.3 Code section

```ebnf
<code-line>     ::= <statement> | <comment-line> | <blank-line>

<comment-line>  ::= { <space> } <comment> <newline>
<blank-line>    ::= { <space> } <newline>
```

Comments and blank lines in the code section are ignored by the parser.
They do not produce tokens.

---

## 4. Statements

```ebnf
<statement>     ::= <assignment>
                  | <call>
                  | <branch>
                  | <loop>
                  | <break>
                  | <return>
```

Every statement begins at the current indent level `n`.

### 4.1 Assignment

```ebnf
<assignment>    ::= <indent-n> <identifier> <newline>
                    <indent-n> <source-value> <newline>

<source-value>  ::= <literal>
                  | <identifier>
```

The receiver (first line) and the source (second line) must be at the **same
indent** `n`.

Forms:

| Source form    | Meaning                                    |
|----------------|--------------------------------------------|
| `42`           | literal constant                           |
| `x`            | copy the value of `x`                      |

The receiver is created as a local variable on first use, unless it was
declared in params/returns. If the receiver name is already an alias, it is
an error ("alias cannot be used as a receiver").

The source must be a literal or a known non-alias name. Using an alias as a
value is an error. Assignment copies a value; variables have no observable
addresses.

### 4.2 Branch

```ebnf
<branch>        ::= <indent-n> <identifier> <newline>
                    <indent-n> <question> <newline>
                    <body-n+1>
                    [ <indent-n> <colon> <newline>
                      <body-n+1> ]

<body-n+1>      ::= { <statement-n+1> }
```

Rules:

- The condition identifier and the `?` must be on adjacent lines at the
  same indent `n`.
- **No blank line** may appear between the condition and `?`. Comments are
  allowed. If a blank line is present, the parser reports:
  "blank line between a condition and '?' is not allowed; remove the blank
  line".
- The true-body statements are at indent `n+1`.
- An optional `:` at indent `n` (on its own line) begins the false body.
- The body of each branch (including the false branch, if present) is one
  or more statements at indent `n+1`.
- The whole construct ends when the next token has indent < `n+1` (that is,
  back at `n` or shallower).

The condition's value is read once at the `if`. Modifying the condition
variable inside either body does not affect the branch decision (there is
no re-check).

### 4.3 Loop

```ebnf
<loop>          ::= <indent-n> <identifier> <newline>
                    <indent-n> <lbrace> <newline>
                    <body-n+1>
                    <indent-n> <rbrace> <newline>
```

Rules:

- The condition identifier and the `{` must be on adjacent lines at the
  same indent `n`.
- **No blank line** between them.
- The body is one or more statements at indent `n+1`.
- The closing `}` is at indent `n`.
- The condition is **re-read on each iteration** (the loop is `while (cond)`).
- The body is responsible for updating the condition variable. If it does
  not, the loop is infinite.
- `break` (`!`) exits the innermost loop.

### 4.4 Break

```ebnf
<break>         ::= <indent-n> <bang> <newline>
```

Exits the innermost enclosing loop. Outside a loop it is a runtime-level
`break` in C, which does not compile — this is currently **not** checked
statically (see "Known limitations").

### 4.5 Return

```ebnf
<return>        ::= <indent-n> <arrow-greater> <newline>
```

Early return from the current procedure. In C, this becomes `return;` (all
module functions are `void`). Return values are **not** returned in the C
sense: they are left in the module's result variables, which the caller reads
after the call.

### 4.6 Call

```ebnf
<call>          ::= <indent-n> <identifier> <newline>
                    { <param-or-result-n+1> }

<param-or-result-n+1>
                ::= <indent-n+1> <param> <newline>
                  | <indent-n+1> <result> <newline>

<param>         ::= <arrow-less> <param-value>
                  | <identifier> <arrow-less> <param-value>

<result>        ::= <arrow-greater> <identifier>
                  | <identifier> <arrow-greater> <identifier>

<param-value>   ::= <literal>
                  | <identifier>
```

Rules:

- The alias identifier and its arguments must be at adjacent indent
  levels: alias at `n`, every parameter or result at `n+1`.
- The alias name must be known and must resolve to an existing module in
  the project.
- Parameters must precede results. A parameter after a result is an error.
- A call uses either all **positional** arguments or all **named**
  arguments. Mixing is an error.
- Positional parameters are matched in declaration order of the callee's
  params. Positional results are matched in declaration order of the
  callee's returns.
- Named parameters and results use the declared name, e.g. `left < x` or
  `sum > r`.
- The call is emitted as:
    1. `emit_call_init(callee)` — reset callee parameters/results to declared defaults and locals to zero;
    2. `emit_constant_assignment` or `emit_copy` per parameter, in source order;
    3. `emit_call_exec(callee)` — the actual C call;
    4. one `emit_copy` per result, in source order.
- Step 3 is emitted lazily: on the first result token, or at the end of
  the argument list if there are no results.

Example:

```
add
 left < x
 right < 7
 sum > result
```

generates roughly:

```c
/* IDs here are illustrative internal symbol names. */
symbol_1 = symbol_4;
symbol_2 = 7;
module_add();
symbol_5 = symbol_3;
```

---

## 5. Whole-module grammar

The section-level grammar is defined in §3.1. Code inside `@Ama` consists of
statements at indentation zero, with optional comments and blank lines:

```ebnf
<code-section> ::= { <statement-0> | <comment-line> | <blank-line> }
```

`<statement-0>` is a statement at indentation zero. A section annotation is
handled by the module splitter, not as an executable statement or a value.

---

## 6. Lexical rules not expressible in EBNF

The following rules are enforced by the lexer/parser but are not captured
by the grammar above.

1. **Tabs are forbidden anywhere.** The lexer rejects `\t` with an error
   on the containing line.

2. **Lone `\r`** (not part of `\r\n`) is rejected.

3. **Indent step is exactly one space.** Indent increases by exactly 1 per
   nested body. Jumps of two or more are errors.

4. **Explicit section annotations.** Optional `@<` and `@>` precede the
   implementation selector (`@Ama` or `@C`), at indentation zero, without
   repetition. A missing selector denotes a draft. Blank lines never delimit sections.

5. **No blank line between a condition and `?` or `{`.** The `has_preceding_blank_line`
   flag tracks this. Comments are fine; blank lines are not.

6. **Same indent for assignment receiver and source.** Both must be at
   indent `n`.

7. **Aliases cannot be values, receivers, or conditions.** Aliases are declared by a `<path>` value in a declaration section;
   they cannot be used as numeric values in code.

8. **Unknown names in the source position are errors.** Unlike the
   receiver (which auto-creates a local), the source must already exist.

9. **Module existence is checked via the dependency graph, not the symbol
   table.** This is an implementation detail that allows calling modules
   with no params, no returns, and no locals.

10. **Recursion is forbidden.** The dependency graph must be acyclic. A
    cycle at any depth aborts the build with a full path of the cycle.

---

## 7. Known limitations (not enforced)

The current compiler does **not** enforce:

1. **Arity.** Missing parameters keep the callee's declared
   default value. Extra parameters are caught only by the "too
   many positional parameters" check; missing ones are silent.

2. **Duplicate named parameters.** `left < 5` and `left < 7` in the same
   call: the second write wins; no error is reported.

3. **`break` outside a loop.** Compiles to `break;` in C; gcc rejects it
   with its own error.

4. **String literals.** There is no string type. Only nonnegative integers.

5. **Negative literals.** `-5` is not a token; subtraction wraps as unsigned
   arithmetic.

---

## 8. End-to-end example

File `math/double`:

```text
@<
x
0
add
.sys.math.add

@>
result
0

@Ama
add
 < x
 < x
 > result
```

File `main`:

```text
@<
double
.math.double
print
.sys.io.print

@Ama
double
 < 21
 > answer
print
 < answer
```

```sh
./ama ./examples/double main out.c
gcc -O3 out.c build/stdlib/libama_std.a -o prog
./prog
# prints: 42
```

## 9. Value semantics

Every numeric variable holds a platform-sized unsigned integer (`size_t`).
Zero is false; any nonzero value is true. Arithmetic and comparisons are
unsigned. A call copies values: defaults, arguments, execution, then results.
All Ama callee locals are reset to zero at every call, including locals introduced
inside a conditional body. The entry module receives the same initialization.
There are no addresses, pointers, dereferencing operators, or built-in memory
allocation/access modules. The compiler emits separate C variables; their
internal symbol IDs are not values accessible to a program.

The standard library exports `sys/math/{add,sub,mul,div,mod,lt,le,gt,ge,eq,ne,
and,or,not}` and `sys/io/print`; see README.md for signatures. These are ordinary
modules under `stdlib/modules/`, implemented through the native C contract
below rather than compiler-specific operations.

## 10. Native C implementation contract

An active module ending in `@C` has the same Ama call syntax and interface as
an Ama module. Its signature is generated from the declaration sections:

- The external function is `ama_native_<module-path>`, with `/` replaced by `_`.
- It returns `void`. All `SYMBOL_PARAMETER` entries become `size_t` arguments
  in declaration order, followed by all `SYMBOL_RESULT` entries as `size_t*`.
- Parameter names receive the prefix `parameter_`; result names receive
  `result_`. This also permits Ama names that are C keywords.
- Aliases are compile-time dependencies and do not appear in the C signature.
- An empty signature is `(void)`. The generated header includes `<stddef.h>`.
- Native paths and numeric interface names require ASCII letters, digits, and
  underscores; paths additionally permit `/`. Function-name collisions caused
  by replacing `/` with `_` are errors among active user modules.
- Before every call, Ama initializes parameter and result defaults. Missing
  parameters retain those defaults. Every result is passed by a non-null pointer,
  even when the Ama caller omits it. Unwritten results retain their defaults.
- C parameters are copies. Result pointers must not be retained for later use.
  C locals and static state follow C rules, not Ama's local-reset behavior.
- The native source must include its generated header. A conflicting definition
  is then rejected by the C compiler; Ama itself does not parse C code.
- The native entry module has no value parameters, just like an Ama entry.

The companion C file must exist for normal translation. `.c` and `.h` files
are excluded from Ama module discovery. The compiler emits a private Ama-call
wrapper and a generated header next to its output C file. Native sources are
compiled and linked separately, with that output directory on the include path.

`ama --generate-c-stubs <root> <header-directory>` generates headers and creates
missing companion `.c` templates for `@C` modules. It never overwrites an existing
C implementation. Templates deliberately contain `#error` until implemented.
This command ignores drafts and does not require an entry module. See
`CONTRIBUTORS.md` for the complete contributor workflow.

## 11. Standard library discovery and linkage

The compiler imports modules from a standard library directory in addition to
the project directory. Both trees use the same file format, declaration parser,
dependency graph, and native C wrappers. A module path must be unique across the
trees; a project cannot shadow an imported module. C and header files remain
excluded from Ama discovery.

`AMA_STDLIB` selects the imported directory. When absent, the build-configured
default is used; an empty value disables import. The standard library is not
part of the grammar and does not reserve a compiler-side list of operations.
Its interfaces under `stdlib/modules/sys/` define the published `sys/*` APIs.

The default C implementations are built separately as `build/stdlib/libama_std.a`
and linked after generated program and project-native sources. Selecting another
library requires linking its matching native implementations. Stub generation
never creates sources inside an imported library. `--generate-c-headers` exports
headers without creating any companion sources and is used for library builds.
