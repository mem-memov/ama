# Ama

Ama is a minimalist procedure-oriented language with unsigned integer values,
explicit parameter/result passing, and a small punctuation-based syntax.
The compiler is written in C11 and translates Ama modules into C. Compile the
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
The Ama compiler generates C only; it does not invoke GCC itself.
Use `make C_COMPILER=clang` to build the compiler with Clang.

## A complete program

Create a directory `hello` containing one file named `main`:

```text
@<
add
.sys.math.add
print
.sys.io.print

@Ama
add
 < 5
 < 7
 > result

print
 < result
 > err
```

The `@<` annotation starts the parameter/alias section, and `@Ama` starts
the implementation. This module has no result section. Build and execute it:

```sh
./ama hello main hello.c
gcc -O3 hello.c -o hello-program
./hello-program
# 12
```

Keep generated output in its own directory. The compiler scans non-hidden
files recursively, excluding `.c` and `.h` files; the remaining files are Ama
interfaces or implementations. Companion C sources are compiled separately.

## Modules and declarations

Module sections begin with visible annotations on their own lines:

| Annotation | Contents | Required |
|---|---|---|
| `@<` | Parameters and module aliases | No |
| `@>` | Results (aliases are also accepted) | No |
| `@Ama` | Ama implementation | One implementation selector |
| `@C` | Implementation in the companion `.c` file | One implementation selector |

The order is `@<`, `@>`, then either `@Ama` or `@C`. Each section may appear once.
An omitted declaration section is empty. An annotation starts at indentation
zero; trailing spaces and a `#` comment are allowed. Blank lines and comments
may appear before annotations and within any section. They no longer delimit
sections. The existing condition-binding rule still forbids a blank between
a condition and its `?` or `{`.

A deliberately empty Ama module consists of `@Ama` alone. A file without
`@Ama` or `@C` is a draft: its declarations are not parsed and no code or C stub
is generated for it. Unreferenced drafts may remain unfinished. Selecting a
draft as the entry module or referencing it through an alias is an error. Unknown annotations and
the former blank-line section format are rejected.

Declarations are name/value pairs, one item per line at indent zero.
Blank lines and comments may separate declarations or a name from its value. A decimal
integer gives a parameter or result its default value. A dotted module path
declares an alias; `.math.double` refers to file `math/double` relative to the
project root. Declare aliases in the parameter section. The current parser
also accepts aliases in the result section; they are not returned values.

For example, file `math/double`:

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

The entry module must exist and have no value parameters. Aliases do not count
as value parameters. An empty implementation still requires `@Ama`.

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

## Native C modules

A module can expose an Ama interface while implementing its behavior in C.
For example, `project/math/double`:

```text
@<
value
0
@>
result
0
@C
```

Its companion `project/math/double.c` must include the generated header:

```c
#include "ama_native_math_double.h"

void ama_native_math_double(size_t parameter_value, size_t* result_result) {
    *result_result = parameter_value * 2;
}
```

The generated signature returns `void`, takes every parameter by value, then
every result by pointer, in declaration order. Aliases are not C arguments.
Defaults and named/positional Ama calls work as before. Every result receives
a valid pointer even when the caller ignores it. `@C` permits only blank lines
and comments after the selector; the executable implementation belongs in `.c`.

To create missing implementations from their interfaces:

```sh
./ama --generate-c-stubs project generated
```

This creates missing companion `.c` files without overwriting existing sources
and regenerates owned headers under `generated/`. Generated stubs contain a
`#error` directive; implement them and remove the directive before compiling.
Drafts without an implementation selector are skipped.

Ordinary translation also writes native headers beside its output C file, but
requires companion sources to exist and never creates or modifies them:

```sh
mkdir -p generated
./ama project main generated/program.c
gcc -std=c11 -O3 -Igenerated generated/program.c project/math/double.c -o generated/program
./generated/program
```

List the selected native sources (and any C helpers they need) in the C compiler
command. Ama does not invoke the C compiler or automatically link C sources.
A native module may also be the entry module if it has no value parameters.

See [the native module contributor guide](CONTRIBUTORS.md#native-c-modules) for
naming, ownership, defaults, signature changes, and tests.

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
  codegen/           C output primitives, native signatures/wrappers, intrinsics
  project/           directory scan, module loading, compilation pipeline
```

Start with `project/compile.c`: `compile_project` loads declarations, validates
and orders dependencies, emits module bodies, and writes the program.
`project/module.c` handles the two passes over each module; `project/scan.c`
registers source paths. `parser/statement.c` classifies statements and dispatches
to the appropriate parser. `parser/call.c` tracks argument mode, result ordering,
and execution in a `ModuleCallState`. Code is emitted directly, without an AST.

State belongs to explicit objects: `SymbolTable`, `Graph`, `Lexer`, and
`Emitter`. The graph has no dependency on the symbol table; the project layer
translates aliases into graph edges. `StatementParser` receives only the components
it uses. Internal `Project` owns a compilation's tables, files, and output and
releases them on success or failure.

Components return `bool`, a nullable result, or an error token together with an
`Error` diagnostic. They neither print errors nor terminate the process.
`main.c` prints the final diagnostic and chooses the exit code. The public
`compile_project` function can be called repeatedly, including after a failed
compilation. Call `error_clear` before a new sequence of lower-level operations;
`compile_project` initializes its diagnostic itself. A failure can leave a
component partially updated; initialize it again before reusing it when needed.

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

Generated C uses separate `static size_t symbol_<symbol_id>` variables and `void` module
functions. IDs are private compiler names, with no numeric meaning in Ama.
The compiler buffers function bodies until all local variables have been
registered, then writes their declarations before the functions. Callees are
emitted before callers. Native optimization is performed by the C compiler.
Recursion remains forbidden, including cycles formed by unused aliases.

## Tests

The test tree uses C and recursive Makefiles, with two kinds of harness:

```text
tests/
  Makefile                 explicitly runs unit/, then integration/
  common.c                 assertions and small file helpers
  leaf.mk                  shared build/run/clean recipe; no shared main
  unit/
    Makefile
    common.c               error fixture
    lexer/
      Makefile
      common.c             source-file and lexer lifecycle
      tokens/
        Makefile
        main.c             it_* functions, run(), local main()
      sections/ ...
    symbols/ ...
    graph/ ...
    parser/ ...
    codegen/ ...
    project/ ...
  integration/
    Makefile
    common.c               compile/run/compare helpers
    process.c              POSIX fork/exec/wait, without a shell
    01-hello/
      Makefile
      main.c
      project/main         input modules, separate from test metadata
      expected.txt
```

Each leaf builds its own executable. Parent Makefiles explicitly invoke child
Makefiles in reading order. A leaf's `main.c` lists `it_*` functions before
`run`, calls them in order, and supplies its own `main`. Each test explicitly
calls `setup_test` and `destroy_test`. Parent `common.c` files define guarded,
shared test state and helpers. Production components are linked as ordinary C
sources through their public headers. Makefiles name the required sources.

Unit suites cover individual components and combinations: lexer/parser/emitter,
module calls, native signatures, and repeated whole-project compilation within
one process. Integration cases also compile native sources as separate C
translation units and exercise the stub-generation workflow. Tests
use standard `assert`; `-UNDEBUG` keeps assertions enabled even if build flags
contain `-DNDEBUG`. There is no automatic registration or generated runner.

Integration scenarios cover language behavior and the explicit section format. `project/`
contains only input modules. `expected.txt` compares stdout byte for byte,
including final newlines; `expected-error.txt` contains a diagnostic substring
and requires compiler exit status 1. Crashes and missing executables are failures.

```sh
make test                                    # all suites
make -C tests/unit                           # unit and component assembly suites
make -C tests/unit/lexer/tokens               # one leaf
make -C tests/integration/01-hello            # one integration scenario
make -C tests/unit/parser/calls run           # keep artifacts after success
make test-clean                              # remove test artifacts
```

Every leaf owns `generated/`. `all` builds, runs, then removes generated files
on success. On failure it stops and preserves the executable and diagnostics
for debugging. `run` preserves artifacts even on success; `clean` removes them.
Integration leaves ensure the compiler is built, so they can run independently.
Each parent stops at its first failing child; run another leaf directly to
investigate it independently.

## Current limitations

The prototype has fixed limits of 512 graph nodes (including intrinsics) and
8192 symbols. Literal parsing still uses `int`/`atoi`; values above `INT_MAX`
are not reliably supported even though computed values use `size_t`.
Duplicate named arguments are not rejected. A `!` outside a loop is rejected
by the downstream C compiler rather than by Ama. There is no string/file input,
interactive debugger, or recovery within a failed compilation. Generated C function names
replace `/` with `_`; collisions between active user modules are rejected.
Native module paths and parameter/result names currently require ASCII
identifiers; ordinary Ama identifiers still accept non-ASCII bytes.

## Contributing

See [CONTRIBUTORS.md](CONTRIBUTORS.md) for naming, formatting, component design,
resource ownership, and test conventions.
