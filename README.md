# Ама (Ama)

Ама is a minimalist procedure-oriented language with unsigned integer values,
explicit parameter/result passing, and a small punctuation-based syntax.
The compiler is written in C11 and translates Ама modules into C. Compile the
result with a C compiler to obtain a native executable.

The language has no classes, exceptions, garbage collector, or recursion.
Variables hold values; programs cannot obtain addresses or access memory by
numeric index. There is no shared memory array or language-level allocator.
The normative syntax is described in [EBNF.md](EBNF.md).

## Build and run

Requirements: a C11 compiler, Make, and a POSIX system (Linux or macOS).
There are no external library dependencies.

```sh
make
./ama <project_root> <entry_module> <output.c>
gcc -O3 output.c -o program
./program
```

The CLI also supports `--help` and `--version`.
The Ама compiler generates C only; it does not invoke GCC itself.
Use `make C_COMPILER=clang` to build the compiler with Clang.

## A complete program

Create a directory `hello` containing one file named `main`:

```text
add
.sys.math.add
print
.sys.io.print


add
 < 5
 < 7
 > result

print
 < result
 > err
```

The two blank lines after the aliases separate the parameter section, empty
result section, and code section. Build and execute it:

```sh
./ama hello main hello.c
gcc -O3 hello.c -o hello-program
./hello-program
# 12
```

Keep generated files outside the source directory: the compiler scans every
non-hidden regular file recursively and treats each as a module.

## Modules and declarations

A file contains three sections separated by the first two blank lines:
parameters, results, and code. Sections may be empty. Further blank lines in
code are allowed. Comments start with `#` and do not act as section separators.

Declarations are name/value pairs, one item per line at indent zero. A decimal
integer gives a parameter or result its default value. A dotted module path
declares an alias; `.math.double` refers to file `math/double` relative to the
project root. Declare aliases in the parameter section. The current parser
also accepts aliases in the result section; they are not returned values.

For example, file `math/double`:

```text
x
0
add
.sys.math.add

result
0

add
 < x
 < x
 > result
```

The entry module must exist and have no value parameters. Aliases do not count
as value parameters. Empty modules are allowed.

## Values and assignment

All values are `size_t`: platform-sized unsigned integers. Zero is false;
nonzero is true. Arithmetic wraps on unsigned overflow. Subtraction does not
produce signed negative numbers. There are no strings or floating-point values.

Assignment has the receiver on one line and a literal or variable on the next,
at the same indentation:

```text
x
42
y
x
```

An unknown receiver becomes a local variable. A source must already be known.
Names are scoped to their module; aliases cannot be used as numeric values.
There are no pointers, address-taking, dereferencing, or memory intrinsics.

## Calls

Call a module using its alias, followed by arguments indented by one space:

```text
double
 < 21
 > answer
```

Named arguments and results are also supported:

```text
double
 x < 21
 result > answer
```

A call uses either positional or named arguments throughout. Parameters must
precede results. A zero-argument call is just the alias name on its own line.

Each call resets the callee's parameters and results to their declared defaults
and locals to zero, copies arguments by value, executes the procedure, and
copies requested results back by value. Omitted parameters keep their declared
defaults; omitted results are ignored. Too many positional arguments/results
and unknown named parameters/results are errors.

## Control flow

A condition name immediately followed by `?` selects a branch:

```text
condition
?
 x
 1
:
 x
 2
```

The false branch is optional. A condition followed by `{` starts a while loop:

```text
condition
{
 # body; update condition to stop the loop
 !
}
```

Bodies use exactly one additional space of indentation. Tabs are forbidden.
Conditions and their `?` or `{` may have comments between them, but no blank
line. `!` exits the nearest loop. A bare `>` returns from the current procedure.
The condition is re-read on each loop iteration.

## Built-in modules

| Module | Parameters | Results |
|---|---|---|
| `sys/math/add` | `left`, `right` | `sum` |
| `sys/math/sub` | `left`, `right` | `diff` |
| `sys/math/mul` | `left`, `right` | `prod` |
| `sys/math/div` | `left`, `right` | `quot`, `error` |
| `sys/math/mod` | `left`, `right` | `rem`, `error` |
| `sys/math/lt`, `le`, `gt`, `ge`, `eq`, `ne` | `a`, `b` | `result` |
| `sys/math/and`, `or` | `a`, `b` | `result` |
| `sys/math/not` | `a` | `result` |
| `sys/io/print` | `value` | `error` |

Comparisons and logical operations return 0 or 1. Division/modulo by zero sets
`error` to 1 and the numeric result to 0; otherwise `error` is 0. Print writes
an unsigned decimal value and newline; its current implementation always
returns `error = 0`. Error checking is explicit and optional.

## Compiler architecture

The implementation is organized by compilation stage:

```text
src/
  main.c             CLI
  common/            limits, paths, diagnostics
  lexer/             tokens, character classification, section splitting, lexer
  parser/            declarations, values, assignments, calls, control flow
  symbols/           symbol types and table
  graph/             dependency graph and cycle detection
  codegen/           C output primitives, intrinsics, program wrapper
  project/           directory scan, module loading, compilation pipeline
```

Start with `project/compile.c`: `compile_project` loads declarations, validates
and orders dependencies, emits module bodies, and writes the program.
`project/module.c` handles the two passes over each module; `project/scan.c`
registers source paths. `parser/statement.c` classifies statements and dispatches
to the appropriate parser. `parser/call.c` tracks argument mode, result ordering,
and execution in a `CallState`. Code is emitted directly, without an AST.

Headers expose each subsystem's own types and interface. `parser/internal.h`
and `project/internal.h` declare helpers shared only within their subsystem.
`common/limits.h` contains implementation limits; there is no umbrella header.

Small token-stream, character-classification, and C output helpers are
`static inline` in headers. The default build uses `-O2 -flto` during compilation
and linking, allowing the compiler to inline across `.c` files as well.
Inlining is an optimization decision; recursive functions and library calls
are not forced inline. This setting optimizes the compiler itself; optimization
of the generated program is controlled by the separate C compiler invocation.

To build without LTO, use `make -B LTO_FLAGS=`. For a debug build:

```sh
make -B LTO_FLAGS= C_COMPILER_FLAGS="-O0 -g3 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L"
```

Use `make -B` when switching flags or compilers to rebuild existing objects.
The Makefile preserves source subdirectories under `build/` and tracks header
dependencies automatically with `-MMD -MP`.

Generated C uses separate `static size_t v_<id>` variables and `void` module
functions. IDs are private compiler names, with no numeric meaning in Ама.
The compiler buffers function bodies until all local variables have been
registered, then writes their declarations before the functions. Callees are
emitted before callers. Native optimization is performed by the C compiler.
Recursion remains forbidden, including cycles formed by unused aliases.

## Tests and current limitations

```sh
make test
```

Each `tests/NN-name/` directory is a project. `expected.txt` checks stdout;
`expected-error.txt` checks rejection and a diagnostic substring. The runner
copies module files into a clean staging directory, generates C, compiles it,
and executes successful programs. Intermediate files go to `build/tests/`.

The prototype has fixed limits of 512 graph nodes (including intrinsics) and
8192 symbols. Literal parsing still uses `int`/`atoi`; values above `INT_MAX`
are not reliably supported even though computed values use `size_t`.
Duplicate named arguments are not rejected. A `!` outside a loop is rejected
by the downstream C compiler rather than by Ама. There is no string/file input,
interactive debugger, or compiler error recovery. Generated C function names
replace `/` with `_`, so some distinct module paths can collide.
