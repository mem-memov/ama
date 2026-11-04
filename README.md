# Ама (Ama)

A minimalist, strictly-typed, procedure-oriented language built on a single
flat memory array and explicit indirect addressing. The compiler is written in
C and emits C, which is then compiled with `gcc -O3 -march=native` into a
native binary.

Ама is designed to return simplicity to programming: no classes, no exceptions,
no garbage collector, no operator precedence, no keywords, no recursion. The
whole language core fits on a napkin, and the compiler pipeline is small enough
for one person to understand end-to-end.

---

## Table of contents

- [What the language looks like](#what-the-language-looks-like)
- [Memory model](#memory-model)
- [Module structure](#module-structure)
- [Calls: parameters and results](#calls-parameters-and-results)
- [Control flow](#control-flow)
- [Indirect addressing](#indirect-addressing)
- [Error handling](#error-handling)
- [Built-in modules (`sys/*`)](#built-in-modules-sys)
- [Compiler pipeline](#compiler-pipeline)
- [Building the compiler](#building-the-compiler)
- [Compiling a project](#compiling-a-project)
- [What the compiler generates](#what-the-compiler-generates)
- [Current limitations](#current-limitations)

---

## What the language looks like

A module `math/abs` that returns `|x|`:

```
x
0

result
0

result
x

lt
 < x
 < 0
 > cond

cond
?
  sub
   < 0
   < x
   > result
:
  result
  x
```

A `main` module that calls it:

```
abs
.math.abs
print
.sys.io.print

abs
 < 5
 result > result

print
 < result
 error > err
```

Compile:

```
$ ./ama ./proj main out.c
$ gcc -O3 -march=native out.c -o proj
$ ./proj
```

Output:

```
5
```

---

## Memory model

The entire program memory is a single `size_t` array:

```c
static size_t storage[STORAGE_SIZE];
```

Every variable of every module lives here. Each variable gets a unique **offset**
assigned at compile time. `storage[offset]` is the variable's value.

There is **no stack** (no call frames), **no heap** (`malloc` is never called),
and **no garbage collector**. The storage array is split into two regions:

| Region | Purpose | Size |
|---|---|---|
| Static | Parameters, returns, locals of all modules | Fixed at compile time |
| Heap | Reserved for `sys.mem.alloc` | Configurable |

Address `0` is reserved as `nil`.

---

## Module structure

Every module file has **exactly three sections**, separated by **empty lines**:

```
<params>       # name / value pairs, values by default
<empty line>
<returns>      # name / value pairs, values by default
<empty line>
<code>         # executable statements
```

Any section may be empty. The separating blank lines are mandatory.

Example — `math/add`:

```
left
0
right
0

sum
0

add
 < left
 < right
 sum > sum
```

(`add` here is an alias — see below.)

---

## Calls: parameters and results

A call has the callee name at indent 0, then parameters and results at
indent 1, with arrows:

```
fn
 <x            # positional parameter
 left < y      # named parameter (left = y)
 > r           # positional result
 sum > r       # named result (sum -> r)
```

**Rules:**

- Parameters appear **above** results.
- Parameters and results must be **all positional** or **all named** — no mixing.
- `&x` passes the address of `x` (for parameters only).
- Zero-argument calls are written as a single line: `fn`.

Aliases for external modules are declared in the `params` section, not in the
code:

```
add
.math.add
```

Then `add` can be called as `add ...`. Direct paths in code are forbidden.

---

## Control flow

**Branching** (`?` / `:`):

```
cond
?
  # true branch (indent +1)
:
  # false branch (indent +1)
```

The condition is a variable name. If its value is `0`, the false branch runs.
`:` is optional.

**Loop** (`{` / `}`):

```
cond
{
  # body (indent +1)
  # ... update cond ...
}
```

The loop runs while `cond` is non-zero. `{` checks on entry, `}` checks on
continuation (re-reads `cond`). No `continue` — use a nested `?` instead.

**Break** (`!`): exits the nearest enclosing `{`.

**Return** (bare `>`): early return from the current procedure.

**Greedy binding:** the condition and its `?`/`{` must be on adjacent lines
(no blank line between them). Comments in between are allowed.

---

## Indirect addressing

```
# Load from address in pointer p
value
@p

# Store to address in pointer p
@p
value

# Take the address of x (returns its offset as a number)
ptr
&x
```

Under the hood: `@p` becomes `storage[storage[p_offset]]`, `&x` becomes the
constant `x_offset`. `nil` (`0`) is not dereferenceable; dereferencing `nil`
is a runtime panic.

---

## Error handling

By convention, modules that can fail declare an extra return slot named `error`:

```
left
0
right
0

quot
0
error
0

# ... code ...

quot
add
 < left
 < right
 > quot
error
0
```

`error = 0` means success. Non-zero values are error codes. Checking is
**explicit** — the compiler does not enforce it. The programmer decides when
to test `error` and what to do.

Early return from a procedure uses a bare `>`:

```
is_zero
?
  error
  1
  >
:
  # continue
```

---

## Built-in modules (`sys/*`)

The compiler ships with intrinsic modules that cannot be written in Ама itself:

| Path | Signature | Description |
|---|---|---|
| `sys/math/add` | `(left, right) -> sum` | `sum = left + right` |
| `sys/math/sub` | `(left, right) -> diff` | `diff = left - right` |
| `sys/math/mul` | `(left, right) -> prod` | `prod = left * right` |
| `sys/math/div` | `(left, right) -> quot, error` | `quot = left / right` |
| `sys/math/mod` | `(left, right) -> rem, error` | `rem = left % right` |
| `sys/math/lt` | `(a, b) -> result` | `result = a < b` (0 or 1) |
| `sys/math/le` | `(a, b) -> result` | `result = a <= b` |
| `sys/math/gt` | `(a, b) -> result` | `result = a > b` |
| `sys/math/ge` | `(a, b) -> result` | `result = a >= b` |
| `sys/math/eq` | `(a, b) -> result` | `result = a == b` |
| `sys/math/ne` | `(a, b) -> result` | `result = a != b` |
| `sys/math/and` | `(a, b) -> result` | `result = a && b` |
| `sys/math/or` | `(a, b) -> result` | `result = a \|\| b` |
| `sys/math/not` | `(a) -> result` | `result = !a` |
| `sys/io/print` | `(value) -> error` | Print `value` to stdout |
| `sys/mem/alloc` | `(size) -> ptr, error` | Allocate `size` words from the heap |
| `sys/mem/free` | `(ptr) -> error` | Currently a no-op (bump allocator) |
| `sys/mem/load` | `(addr) -> value` | `value = storage[addr]` |
| `sys/mem/store` | `(addr, value) -> error` | `storage[addr] = value` |

Use these via aliases:

```
add
.sys.math.add
print
.sys.io.print
```

---

## Compiler pipeline

```
project tree
   │
   ├─[1] scan directory      → list of files
   ├─[2] pass 1: declarations → G_Table (global symbol table, offsets)
   ├─[3] topological sort     → build order (rejects recursion)
   ├─[4] pass 2: code bodies  → out.c
   └─[5] gcc -O3 out.c        → executable
```

### Step 1 — Scan

`scan_dir` recursively walks the project root, collecting all non-hidden files.
Each file is one module. The module path is the file path relative to the root:
`math/add`, `main`, etc.

### Step 2 — Declarations

For each module, parse the `params` and `returns` sections. Every name becomes
a symbol in the global table with a unique offset in `storage[]`. Aliases
(path values) become `SYM_ALIAS` entries pointing to a target module.

Intrinsics (`sys/*`) are registered first.

### Step 3 — Topological sort

Build a dependency graph from aliases, run DFS with white/gray/black coloring:

- A gray node encountered again = recursion → **error**.
- Post-order DFS gives build order: leaves first, root last.

This guarantees that in C, `mod_callee` is defined before `mod_caller`.

### Step 4 — Code bodies

For each module in topological order, lex the `code` section and parse
statements:

- `parse_assign` — receiver / source
- `parse_deref_assign` — `@x` / source
- `parse_branch` — `cond` / `?` / body / `:` / body
- `parse_loop` — `cond` / `{` / body / `}`
- `parse_call` — `fn` + indented parameters/results

Each `emit_*` writes C code into `G_Out`.

### Step 5 — `gcc`

The output is one `.c` file: prologue, intrinsics, module bodies in topological
order, and `main()`. `gcc -O3 -march=native` compiles it.

---

## Building the compiler

Requirements:

- `gcc` or `clang` (C11)
- Linux or macOS (POSIX `dirent.h`, `sys/stat.h`)

```bash
gcc -O2 -Wall -Wextra -o ama ama_compiler.c
```

Single-file build; no external dependencies.

---

## Compiling a project

```bash
./ama <project_root> <entry_module> <output.c>
```

Example:

```bash
./ama ./examples/hello main hello.c
gcc -O3 -march=native hello.c -o hello
./hello
```

Project layout:

```
examples/hello/
  main          # entry module
  math/
    add
    abs
```

Entry point:

- Must be a module path (e.g. `main`, `math/square`).
- Must not take parameters (zero `<` in `params`).
- Its function is called from `main()` in the generated `.c`.

---

## What the compiler generates

For the `hello` project (see above), the generated `out.c` looks roughly like:

```c
/* Generated by Ама compiler */
#include <stdint.h>
#include <stdio.h>

#define STORAGE_SIZE 1048588
static size_t storage[STORAGE_SIZE];
static size_t g_heap_top = 12;

void mod_sys_math_add(void) {
    storage[1] = storage[0] + storage[0];
}
/* ... other intrinsics ... */

void mod_main(void) {
    storage[0] = 0; storage[1] = 0;
    storage[0] = 5;
    storage[1] = 7;
    mod_sys_math_add();
    storage[2] = storage[0];
    mod_sys_io_print();
}

int main(void) {
    mod_main();
    return 0;
}
```

Every variable is `storage[offset]`. Every module is a `void` function. Every
call is a plain C call. `gcc -O3` inlines everything it can and produces
near-optimal assembly with no runtime overhead.

---

## Current limitations

The prototype as it exists today:

- **No arity check** on calls. Passing fewer parameters than declared is
  silently accepted; missing arguments default to `0`.
- **`sys.mem/free` is a no-op.** The allocator is a bump allocator; no memory
  is reclaimed.
- **No runtime checks.** `@nil` reads `storage[0]` instead of panicking.
  Bounds on `storage[]` are not checked.
- **No `--debug` mode.**
- **No stub generator.** Creating stub files for non-existent modules is
  currently a manual step.
- **No cross-module type checking.** There are no types to check (everything
  is `size_t`), but arity is not yet verified at call sites.
- **Multiple return values** work positionally; named results must match the
  declarations in the callee.
```