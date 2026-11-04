# Ама — Grammar

This document specifies the syntax of the Ама language. It is the normative
reference for the parser in `src/parser.c` and the lexer in `src/lexer.c`.

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

Only positive decimal integers are supported. No sign, no hex, no float, no
string literals. A negative value is written as `0 - x` via a module call.

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
but **do not** count as blank lines for the `is_after_blank` flag (see
§3.3). A line consisting only of `#...` is not a blank line.

### 2.7 Punctuation

```ebnf
<arrow-less>    ::= "<"
<arrow-greater> ::= ">"
<at>            ::= "@"
<amp>           ::= "&"
<question>      ::= "?"
<colon>         ::= ":"
<lbrace>        ::= "{"
<rbrace>        ::= "}"
<bang>          ::= "!"
```

---

## 3. Physical structure of a module file

A module is a file with **exactly three sections**, separated by **exactly
two blank lines** (lines whose content is only spaces).

### 3.1 File structure

```ebnf
<module-file>   ::= <params-section>
                    <blank-line>
                    <returns-section>
                    <blank-line>
                    <code-section>

<params-section>  ::= { <declaration> }
<returns-section> ::= { <declaration> }
<code-section>    ::= { <code-line> }
```

A section may be empty. The two separating blank lines are mandatory even
when a section is empty. A file with zero or more than two blank separators
is rejected.

A blank line is a line whose only characters before `\n` are spaces (and,
for CRLF files, `\r`).

### 3.2 Declarations (params and returns)

```ebnf
<declaration>   ::= <identifier-line> <value-line>

<identifier-line> ::= <identifier> <newline>
<value-line>      ::= <value> <newline>

<value>         ::= <literal>
                  | <identifier>
                  | <path>
```

A declaration is a pair: an identifier on one line at indent 0, and a value
on the next line at indent 0.

Semantics:

- The identifier becomes a symbol in the current module.
- If the value is a `<path>`, the symbol is an **alias**: it refers to the
  module at that path. Aliases do not consume a storage slot.
- If the value is a `<literal>`, the symbol's default is that integer.
- If the value is an `<identifier>` (a variable reference), the compiler
  currently rejects it: "defaults that reference other variables are not
  supported yet".

In the params section, non-alias declarations become `SYM_PARAM`. In the
returns section, non-alias declarations become `SYM_RETURN`.

### 3.3 Code section

```ebnf
<code-line>     ::= <statement> | <comment-line> | <blank-line>

<comment-line>  ::= <indent> <comment> <newline>
<blank-line>    ::= <indent> <newline>
```

Comments and blank lines in the code section are ignored by the parser.
They do not produce tokens.

---

## 4. Statements

```ebnf
<statement>     ::= <assignment>
                  | <deref-assignment>
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
                  | <at> <identifier>
                  | <amp> <identifier>
```

The receiver (first line) and the source (second line) must be at the **same
indent** `n`.

Forms:

| Source form    | Meaning                                    |
|----------------|--------------------------------------------|
| `42`           | literal constant                           |
| `x`            | copy the value of `x`                      |
| `@x`           | read from address stored in `x`            |
| `&x`           | address (storage offset) of `x`            |

The receiver is created as a local variable on first use, unless it was
declared in params/returns. If the receiver name is already an alias, it is
an error ("alias cannot be used as a receiver").

The source must be a known non-alias name (or a literal or a leading
`@`/`&` on a known non-alias name). Using an alias as a value is an error.

### 4.2 Indirect assignment

```ebnf
<deref-assignment> ::= <indent-n> <at> <identifier> <newline>
                       <indent-n> <source-value> <newline>
```

Writes to the address stored in the identifier's slot. Example:

```
@ptr
42
```

generates `storage[storage[ptr_off]] = 42;`.

### 4.3 Branch

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

### 4.4 Loop

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

### 4.5 Break

```ebnf
<break>         ::= <indent-n> <bang> <newline>
```

Exits the innermost enclosing loop. Outside a loop it is a runtime-level
`break` in C, which does not compile — this is currently **not** checked
statically (see "Known limitations").

### 4.6 Return

```ebnf
<return>        ::= <indent-n> <arrow-greater> <newline>
```

Early return from the current procedure. In C, this becomes `return;` (all
module functions are `void`). Return values are **not** returned in the C
sense: they are left in the module's return slots, which the caller reads
after the call.

### 4.7 Call

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
                  | <amp> <identifier>
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
    1. `emit_call_init(callee)` — reset all callee slots to defaults;
    2. one `emit_call_param_*` per parameter, in source order;
    3. `emit_call_exec(callee)` — the actual C call;
    4. one `emit_call_result` per result, in source order.
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
storage[add_left]  = storage[x_off];
storage[add_right] = 7;
mod_add();
storage[result_off] = storage[add_sum];
```

---

## 5. Whole-module grammar

Putting it together:

```ebnf
<module>        ::= <declarations-params>
                    <blank>
                    <declarations-returns>
                    <blank>
                    <code>

<declarations-params>  ::= { <declaration> }
<declarations-returns> ::= { <declaration> }

<code>          ::= { <statement-0> | <comment> | <blank> }
```

`<statement-0>` is a statement at indent 0.

---

## 6. Lexical rules not expressible in EBNF

The following rules are enforced by the lexer/parser but are not captured
by the grammar above.

1. **Tabs are forbidden anywhere.** The lexer rejects `\t` with an error
   on the containing line.

2. **Lone `\r`** (not part of `\r\n`) is rejected.

3. **Indent step is exactly one space.** Indent increases by exactly 1 per
   nested body. Jumps of two or more are errors.

4. **Exactly two blank separators per file.** The structure splitter
   requires exactly two blank lines; a third or a missing one is an error.

5. **No blank line between a condition and `?` or `{`.** The `is_after_blank`
   flag tracks this. Comments are fine; blank lines are not.

6. **Same indent for assignment receiver and source.** Both must be at
   indent `n`.

7. **Aliases cannot be values, receivers, or conditions.** Only the
   params section may declare an alias (by a `<path>` value); elsewhere,
   the name must resolve to a non-alias symbol.

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

1. **Arity.** Missing parameters default to 0 (the callee's declared
   default for that slot). Extra parameters are caught only by the "too
   many positional parameters" check; missing ones are silent.

2. **Duplicate named parameters.** `left < 5` and `left < 7` in the same
   call: the second write wins; no error is reported.

3. **`break` outside a loop.** Compiles to `break;` in C; gcc rejects it
   with its own error.

4. **Address bounds.** `@x` reads `storage[storage[x_off]]` without
   checking that the value is within `[0, STORAGE_SIZE)`. Out-of-bounds
   reads are undefined behavior at the C level.

5. **String literals.** There is no string type. Only positive integers.

6. **Negative literals.** `-5` is not a token. Use a module call.

---

## 8. End-to-end example

`math/abs` (returns the absolute value of its argument):

```
x
0

result
0

lt
 < x
 < 0
 > is_negative

is_negative
?
  sub
   < 0
   < x
   > result
:
  x
  result
```

`main` (calls `math/abs` and prints the result):

```
abs
.math.abs
print
.sys.io.print

abs
 < 5
 result > r

print
 < r
 error > err
```

Compile:

```bash
./ama ./examples/math main out.c
gcc -O3 -march=native out.c -o prog
./prog
# prints: 5
```
