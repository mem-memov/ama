# Ama

Ama is a minimalist procedure-oriented language with unsigned integer values,
explicit parameter/result passing, and a small punctuation-based syntax.
The compiler is written in C11 and translates Ama modules into C. Compile the
result with a C compiler and the standard library archive to obtain a native executable.

The language has no classes, exceptions, garbage collector, or recursion.
Variables hold values; programs cannot obtain addresses or access memory by
numeric index. There is no shared memory array or language-level allocator.
The normative syntax is described in [EBNF.md](EBNF.md).

## Build and run

Requirements: a C11 compiler, GNU Make, an `ar` archiver, and a POSIX system
(Linux or macOS).
There are no external library dependencies.

```sh
make
./ama <project_root> <entry_module> <output.c>
gcc -O3 output.c build/stdlib/libama_std.a -o program
./program
```

The CLI also supports `--help` and `--version`.
The Ama compiler generates C only; it does not invoke GCC itself.
Use `make C_COMPILER=clang` to build with Clang. `make` also builds the ordinary
C standard library at `build/stdlib/libama_std.a`; `make standard-library`
rebuilds just that library and its compiler prerequisite.

## A complete program

Create a directory `hello` containing one file named `main`:

```text
@<
add
.sys.math.add
print
.sys.io.print

@Ama
add!
 < 5
 < 7
 > result

print!
 < result
 > err
```

The `@<` annotation starts the parameter/alias section, and `@Ama` starts
the implementation. This module has no result section. Build and execute it:

```sh
mkdir -p generated
./ama hello main generated/hello.c
gcc -O3 generated/hello.c build/stdlib/libama_std.a -o generated/hello-program
./generated/hello-program
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
add!
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
There are no pointers, address-taking, dereferencing, or memory access modules.

## Calls

Call a module using its alias immediately followed by `!`, then arguments
indented by one space. Whitespace before `!` and inline arguments are errors:

```text
double!
 < 21
 > answer
```

Named arguments and results are also supported:

```text
double!
 x < 21
 result > answer
```

A call uses either positional or named arguments throughout. Parameters must
precede results. A zero-argument call is `worker!` on its own line. Every call
requires the suffix, including native calls. Bare aliases are not calls.

Each ordinary execution call resets the callee's parameters and results to their
declared defaults,
copies arguments by value, executes the procedure, and
copies requested results back by value. Omitted parameters keep their declared
defaults; omitted results are ignored. Too many positional arguments/results
and unknown named parameters/results are errors.

Body variables belong to the module instance. They start at zero once and
retain their values between calls, including variables written only inside a
conditional. An assignment in the body executes whenever reached; assigning
zero explicitly resets that variable. Body assignment targets and call-result
receivers are collected before execution code is parsed, so a variable can be
read before its first textual write. A name that is never declared or written
remains an error. Parameter/result defaults are reinstated before each ordinary
execution; completed values remain stored afterward.

### Reading saved results

Use `>>` instead of `>` to read results without running the body or resetting any
state. Named and positional reads are supported, using one style throughout:

```text
double!
 result >> answer
```

Every result in this call must use `>>`; parameters `<` and execution results `>`
are forbidden, regardless of their order. You may request a subset of named
results. `>>` is one contiguous token; `> >` is invalid. A zero-argument `double!`
still executes normally.

Before the first execution, reads return declared result defaults. After an
execution, they return the saved results, including values left by an early
return. Parameters and private body variables cannot be read through `>>`.
This is retrieval of stored values, not a computed getter. Native modules follow
the same rule: retained reads never invoke their companion C function. The full
call is checked before code generation, so a late `>>` cannot trigger a partial
execution or reset.

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
 ^
}
```

Bodies use exactly one additional space of indentation. Tabs are forbidden.
Conditions and their `?` or `{` may have comments between them, but no blank
line. `^` exits the nearest loop, including from a nested condition. A loop exit
outside a loop in the same module is a semantic error; it cannot exit a caller's
loop. A standalone `!` is invalid. A bare `>` returns from the current procedure.
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
#include "ama_native_a892f1603fea71df.h"

void ama_native_a892f1603fea71df(size_t parameter_value, size_t* result_result) {
    *result_result = parameter_value * 2;
}
```

The generated signature uses identity-hashed parameter names and returns `void`, takes every parameter by value, then
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
gcc -std=c11 -O3 -Igenerated generated/program.c project/math/double.c build/stdlib/libama_std.a -o generated/program
./generated/program
```

List the selected native sources (and any C helpers they need) in the C compiler
command. Ama does not invoke the C compiler or automatically link C sources.
A native module may also be the entry module if it has no value parameters.

See [the native module contributor guide](CONTRIBUTORS.md#native-c-modules) for
naming, ownership, defaults, signature changes, and tests.

## Standard library

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

The implementations live under `stdlib/modules/`, with one Ama interface and
companion C source per module. They use the same `@C` mechanism as project
modules; there is no compiler-side table of operations or special emitter.
Paths such as `.sys.math.add` and their existing signatures remain unchanged.

The compiler loads these module files after discovering the project. Duplicate
module paths between the project and library are errors. Generated programs
link the separately built `build/stdlib/libama_std.a` archive. Standard library
sources are never changed by the project's stub generator.

The default module directory is configured by Make's
`STANDARD_LIBRARY_DIRECTORY` (normally the repository's absolute
`stdlib/modules` path). Override it at runtime with `AMA_STDLIB`:

```sh
AMA_STDLIB=/path/to/other/modules ./ama project main generated/program.c
AMA_STDLIB= ./ama project main generated/without-library.c
```

An empty value disables library loading. A missing library directory is an
error. Link the matching C implementations/archive when selecting another
library. C callers can use `compile_project_with_library` to pass an explicit
library directory, with `NULL` or `""` disabling loading without changing the
environment. See [stdlib/README.md](stdlib/README.md) for library development.

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
  ast/               owned syntax trees and source identities/locations
  parser/            syntax construction: declarations, values, calls, control flow
  semantic/          declaration binding, receiver collection, name/call validation
  symbols/           symbol types and table
  graph/             dependency graph and cycle detection
  codegen/           validated AST emission, C primitives, native signatures/wrappers
  project/           project/library discovery, module loading, compilation pipeline
```

Start with `project/compile.c`: `compile_project` loads interfaces and orders
dependencies, parses all Ama bodies into ASTs, validates them, and only then emits
C. `project/module.c` loads modules and owns their syntax lifetimes;
`project/scan.c` records source paths. Interface declarations are parsed into
nodes and bound before dependency construction and native header generation.

`parser/` does not emit C, resolve symbols, or consult the dependency graph.
It records source spelling, source locations, declarations, assignments, calls,
arguments, branches, loops, breaks, and returns. A mandatory trailing `!`
distinguishes calls from assignments without looking up aliases. Parsing an unknown call succeeds syntactically; semantic validation
then checks its target.

`semantic/` first collects body write targets, then resolves reads, call targets,
named/positional arguments, and result receivers. It annotates the same tree with
symbol IDs and normalized callee paths. `codegen/ast.c` consumes only validated
trees and preserves execution order and source mappings. No additional lowered
IR or optimization framework is introduced.

Each module owns a chain of stable, aligned allocations. AST names and literal
spellings are copied, so token buffers are released before semantic validation;
source paths remain borrowed from the project. Cleanup is iterative even when
parsing fails partway through a deeply nested body. Parsing is limited to 128
active block levels, including the module body, to bound recursive compiler passes.

State belongs to explicit objects: `AstModule`, `SymbolTable`, `Graph`, `Lexer`,
and `Emitter`. The project layer connects the stages and releases all compilation
resources on success or failure. Native signatures, current numeric semantics,
and current executable syntax remain unchanged by this architectural refactor.

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
The Makefiles preserve source subdirectories under `build/` and track header
dependencies automatically with `-MMD -MP`. The standard library builds its own
headers with `--generate-c-headers`, which never creates source files.

Generated C groups each module's numeric variables into a `ama_state_<hash>`
structure. Its function takes an explicit pointer to that state; accesses to
its own variables use that pointer. Ordinary Ama calls currently select a
`static` default instance of the callee, initialized once with declared interface
defaults and zero body variables. Ordinary calls reset only parameters and results;
retained-result calls copy stored outputs without resets or execution. Body
variables retain their values in that instance. Language-level object and
reference declarations remain planned, not implemented.

A separate frontend API parses target declarations (`item`, `item&`, `items[]`,
`items&[]`), explicit module types, and indented `|` line literals or named
initializers. Its initializer checker verifies module identities, earlier
sources, and ownership forms. Raw literal bytes have explicit lengths and
preserve spaces, comments, and tabs after `|`. This API is covered by unit tests;
the CLI still accepts the numeric language described above. Typed execution,
literal interpretation, and reference/list operations await the module-value ABI.

State fields use identity hashes; symbol IDs are private table indices, not
Ama values. Original names are retained in safe generated comments. The
compiler resolves all body variables before starting C emission, then writes
state definitions before the buffered functions. Empty modules receive a
placeholder field for ISO C compatibility. Generated native wrappers also take
a state pointer; handwritten native signatures remain unchanged. Callees are
emitted before callers. Native optimization is performed by the C compiler.
Recursion remains forbidden, including cycles formed by unused aliases.

Generated executable statements use `#line` directives naming the absolute,
resolved Ama source path and its physical line. Unicode filenames are preserved;
quotes, backslashes, and control bytes are escaped for C. Calls are attributed
to their call line even when emitted after argument setup; argument/result copies
use their own source lines. Synthetic function scaffolding uses the virtual
filename `<ama-generated>` rather than inheriting the last Ama statement.

For source-level debugging, compile the generated program with `-g -O0`. Keep
the source files at their recorded paths or configure the debugger's source-path
substitution after moving them. These mappings do not rename hashed C variables
or guarantee exact stepping under optimization. Token/declaration locations also
retain one-based UTF-8 byte columns for future diagnostics and program nodes;
standard `#line` does not map columns, so downstream C diagnostic columns refer
to generated C. Native companion sources retain their normal C locations.

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

The prototype has fixed limits of 512 graph nodes (including standard library modules) and
8192 symbols, and 128 active AST block levels (including the module body). Literal parsing still uses `int`/`atoi`; values above `INT_MAX`
are not reliably supported even though computed values use `size_t`.
Duplicate named arguments are not rejected. Loop exits `^` are checked by Ama
before C generation. There is no string/file input,
interactive debugger, or recovery within a failed compilation. Generated types, functions, fields, native arguments, and headers use
deterministic hash names. Unicode paths and interface names are supported.
Actual hash collisions are rejected rather than merging different entities;
`#line` source mappings preserve original file/line attribution.

## Contributing

See [CONTRIBUTORS.md](CONTRIBUTORS.md) for naming, formatting, component design,
resource ownership, and test conventions.
