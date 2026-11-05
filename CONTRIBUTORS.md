# Writing code and tests for Ama

The project should read like a clear book. A reader should be able to understand
an operation, its inputs, its result, and its failure path without reconstructing
hidden state or decoding abbreviations.

## Names explain roles

Use English for identifiers, comments, diagnostics, documentation, and build
messages. Non-English characters belong only in deliberate encoding test data;
write those characters as Unicode escapes in C so their purpose is explicit.

Use `snake_case` for functions and variables, `PascalCase` for types, and
`UPPER_SNAKE_CASE` for constants and enum members. Prefer complete domain words:

- `parser`, `lexer`, and `stream`, rather than `P`, `L`, or `ts`.
- `source_symbol_id` and `destination_symbol_id`, rather than `src` and `dst`.
- `condition_token` and `condition_symbol`, rather than `cond` and `cs`.
- `parameter_count`, `symbol_count`, and `node_count`, rather than an ambiguous
  shared `count`.
- `GraphTraversal` and `NodeVisitState`, rather than a generic `Sort` structure
  or unexplained numeric traversal states.

Name pointers after the object they reference and integer IDs after what they
identify. A `destination_symbol` is a symbol pointer; a `destination_symbol_id`
is an integer. Use `caller_index` and `callee_index` when their roles differ.
A local `index` is sufficient when one nearby collection makes its meaning clear.
Use named enum values when a number represents a state.

Names should explain a role without repeating the whole algorithm. Preserve
established C/POSIX names such as `FILE`, `EOF`, and `waitpid`. Ama standard library
paths and their named parameters are language interfaces: changing names such
as `sys/math/lt` or its parameters requires a separate language change, with
matching updates to the grammar, examples, and tests.

## Formatting follows the file's syntax

C sources and headers use four spaces per indentation level, including
continuations. Use spaces, never tabs. Keep opening braces on the declaration
or control-flow line. Expand functions that perform several actions onto
separate lines. Keep statements in execution order and avoid packing unrelated
actions onto one line. Split long declarations and argument lists at meaningful
boundaries; continuation lines add one four-space level.

Makefiles use GNU Make's `.RECIPEPREFIX := >`. Each recipe line starts with
`>` followed by four spaces. The prefix is syntax, not indentation. Continuation
lines in Make variable lists also use four spaces.

Ama fixtures and Ama examples retain the language's semantic indentation:
**one space per nesting level**, as specified in `EBNF.md`. Applying C formatting
to them changes the program. Module files use `@<`, `@>`, and `@Ama`
for section boundaries; empty lines are formatting, not delimiters. `@C`
selects a companion C implementation. Files without either implementation
selector are drafts; an intentional empty Ama implementation still uses `@Ama`. Deliberate CRLF fixtures retain CRLF; ordinary
source and documentation files use LF. `.editorconfig` records these exceptions;
`.gitattributes` preserves the deliberate CRLF fixture and makes Git whitespace
checks recognize its line endings.

## Small operations have explicit dependencies

Organize production code by responsibility under `src/`. A file should tell one
coherent part of the story. Extract a function when it represents a meaningful
operation with understandable inputs and outputs. A short helper does not need
its own file or public interface merely because it is short.

Keep implementation helpers `static`. Public headers should describe the
component's own types and operations and compile independently. Subsystem
`internal.h` files expose only helpers shared within that subsystem.

Pass mutable state explicitly. `SymbolTable`, `Graph`, `Lexer`, and `Emitter`
are independent objects. The project layer connects them and owns compilation
resources. Do not introduce mutable global production state or pass an entire
project object to an operation that only needs one component.

Use `static inline` for small header helpers whose implementations need to be
visible at call sites. Ordinary helpers remain ordinary functions; the default
LTO build can inline across source files. Inlining is the optimizer's decision.

## Failures and resource ownership are visible

Components return a status, nullable result, or error token and populate an
`Error` diagnostic. They do not print diagnostics or terminate the process.
The CLI prints the final error and chooses the exit code.

Initialize an error before a sequence of operations. Check every fallible
operation before using its result. `compile_project` clears its own diagnostic
and releases compilation resources on both success and failure, so another
compilation can run in the same process.

Make ownership visible: the code that acquires a file or allocation must either
release it on every path or explicitly transfer ownership. Preserve the useful
failure diagnostic while cleaning up. Prefer straightforward cleanup over
hidden control transfers or generic lifecycle machinery.

## Two test harnesses are enough

`tests/unit/` tests C components and combinations of components. The same
harness covers a single symbol-table operation and the lexer/parser/emitter
working together. `tests/integration/` runs the compiler and generated programs
as separate processes and compares their observable behavior.

Each parent Makefile explicitly invokes child Makefiles in reading order.
Every leaf builds and runs its own executable. There is no shared executable
entry point, automatic test registration, or generated test runner. Shared
build rules live in `tests/leaf.mk`; a leaf names its production sources and
fixture dependencies explicitly.

Each leaf's `main.c` contains:

1. Its fixture includes and any small, clearly named test helpers.
2. Test functions named `it_<behavior>`, each describing one observable rule.
3. `void run(void)`, listing those tests in a deliberate reading order.
4. Its own `main`, calling `run` and reporting `test_count`.

Guarded parent `common.c` files define shared test variables and helpers. Exactly
one fixture implementation supplies `setup_test` and `destroy_test` for a leaf.
Each test calls both explicitly. Setup provides technical initialization;
objects and relationships important to the scenario are constructed in the test.
Tests must not depend on state left by a preceding test.

Use standard `assert`. Perform mutations and resource operations before the
assertion, then assert their results. Assertions must never be required for the
operation itself to happen. Test builds use `-UNDEBUG` so assertions remain
active even when inherited build flags contain `-DNDEBUG`.

Link production sources normally through their headers. Test files may include
parent fixture `.c` files, but should not include production `.c` files to bypass
component boundaries. Test meaningful operations through their interfaces;
keep private mechanical helpers private.

## Inputs, expectations, and generated files stay separate

An integration leaf stores input modules under `project/`. Keep expected files
beside the test, outside that source tree. A leaf may contain additional fixture
subdirectories when a scenario needs them; their names should explain their role.

`expected.txt` compares stdout byte for byte, including final newlines.
`expected-error.txt` names a diagnostic substring and requires compiler exit
status 1. A crash, missing executable, or failed C compilation must fail the test.
Processes are launched with argument arrays through POSIX process functions,
without constructing shell commands.

Every leaf owns its `generated/` directory. `all` builds, runs, and cleans after
success. Failure preserves the executable and diagnostic files for debugging.
`run` preserves output even after success; `clean` removes only generated files.
Normal test execution never rewrites expected files.

## Check a change at the right level

Use the smallest relevant leaf during development, then run the complete tree
for changes affecting shared interfaces, language behavior, or test infrastructure:

```sh
make
make -C tests/unit/codegen/emitter
make -C tests/integration/01-hello
make test
git diff --check
```

Add tests for behavior, error handling, and component interaction when those
change. Avoid tests that merely duplicate the implementation. Keep `README.md`
and `EBNF.md` consistent with public behavior. Build artifacts and generated
files are outputs, never the source of truth. Commit only when explicitly asked.

## Generated module state

Generated module functions receive an explicit pointer to their module's state
structure. Own-variable reads and writes must use this pointer, not a global
current-instance variable. Ordinary Ama calls still initialize and execute a
callee's default instance. Preserve current reset behavior during this structural
transition; the persistent-body-variable design is tracked separately in
`TODO.md`.

Keep native wrappers on the same state-access path. Their internal context
parameter does not change the public handwritten `ama_native_*` signature.
State structures and generated function names are internal output details, not
a stable C embedding API. The independent-state integration test uses a C driver
to exercise these internals before the language exposes object declarations.

## Native C modules

Keep the interface and implementation together. A module at `math/double` has
an Ama interface file named `double` and a handwritten source named `double.c`
in the same directory. The interface contains `@<` and `@>` declarations as
needed and ends with `@C`. Only comments and blank lines may follow `@C`.

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

The interface is the source of truth for types, ordering, and defaults. Do not
repeat or invent an independent signature. Create the initial source safely:

```sh
make
./ama --generate-c-stubs project generated
```

The command creates missing companion `.c` files and generated headers. Existing
C sources are opened with exclusive-create semantics and never overwritten.
Header directories are created as needed. A declaration-only draft, even one
with an incomplete parameter pair, does not receive a C stub. To request a stub,
choose `@C` explicitly. Choosing `@Ama` requests an Ama implementation instead.
Structural errors in annotations still require correction.

Edit `project/math/double.c`, remove the generated `#error`, and implement the
function using its generated header:

```c
#include "ama_native_math_double.h"

void ama_native_math_double(size_t parameter_value, size_t* result_result) {
    *result_result = parameter_value * 2;
}
```

The template's `#error` prevents an untouched stub from silently behaving like
a completed implementation. Native parameter and result names are prefixed to
avoid collisions with C keywords. Aliases never become arguments. A function
without parameters or results has a `(void)` signature; a native entry named `main` implements
`ama_native_main`, not the C program's `main`.

A complete caller in `project/main` is:

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

### Build and signature checks

Translate normally after implementing the companion source:

```sh
mkdir -p generated
./ama project main generated/program.c
gcc -std=c11 -O3 -Igenerated generated/program.c project/math/double.c build/stdlib/libama_std.a -o generated/program
./generated/program
```

Ama generates `generated/ama_native_math_double.h` from the current interface
on every successful translation. Include that header from the implementation
and place `generated/` on the C compiler's include path. Compile each native
source as an ordinary, separate C translation unit. This preserves normal C
file boundaries, including independent `static` helpers. List additional helper
sources and libraries explicitly when linking. Ama does not invoke GCC for you.

After editing an interface, regenerate before rebuilding C. A stale handwritten
function definition conflicts with the generated declaration and is rejected
by the C compiler when it includes the header. Existing `.c` sources are never
rewritten to follow a changed interface. Native function names preserve case;
header guards preserve that distinction too. Active user modules whose paths
collapse to the same generated function name are rejected.

Generated headers belong to the output directory and can be replaced. Handwritten
C files belong to the source tree. Do not edit generated headers or commit them
as authoritative interfaces. Keep output from different projects in separate
directories. The scanner ignores `.c` and `.h` files; an unselected companion
file does not turn a draft or an Ama module into a native module.

### Values, defaults, and ownership

Every parameter is a `size_t` value. All result arguments are `size_t*`, supplied
after the parameters in declaration order, and the function returns `void`.
Assigning a parameter changes the C copy, never the Ama caller's variable.

The Ama caller initializes all parameter and result defaults before execution.
Every result pointer is valid and non-null, including ignored results. Write
through those pointers to return values. Leaving a result untouched preserves
its declared default on that call. An `error` result is an ordinary optional
result following the same explicit error-checking convention as Ama modules.

Do not retain result pointers after the call or use them to access unrelated
state. C local variables, allocations, and static variables obey C ownership
and lifetime rules; they do not acquire Ama's local-reset semantics. The C
implementation is responsible for its own resource cleanup and correctness.
Recursion through Ama module dependencies remains forbidden.

Native paths and parameter/result names currently use ASCII letters, digits,
and underscores, with slashes separating module path components. This provides
portable C identifiers. Ordinary Ama modules retain their existing Unicode
identifier support.

### Test a native contribution

Use `tests/unit/codegen/native` for signature and wrapper rules. An integration
leaf should contain the interface and companion under `project/`, then call
`expect_native_program_output` with an explicit list of native source paths.
Those sources are compiled separately with `-Igenerated`; no shell command is
constructed. Keep fixture C indentation at four spaces, while Ama fixtures use
one-space semantic indentation.

Cover defaults, omitted results, named and positional calls, and errors relevant
to the module. Changes to native infrastructure should also verify empty
signatures, native entry modules, conflicting C definitions, missing companions,
name collisions, and safe repeated stub generation. Stub-generation tests must
create editable projects under their leaf's `generated/` directory, never alter
checked-in fixtures, and verify that handwritten edits survive regeneration.

```sh
make -C tests/unit/codegen/native
make -C tests/integration/26-native-calls
make -C tests/integration/38-native-stubs
make test
```

## Standard library contributions

The standard library is a separate source tree under `stdlib/modules/`, not a
set of compiler intrinsics. Every exported operation has a normal Ama interface
and implementation. For example, `stdlib/modules/sys/math/add` selects `@C`,
and `add.c` implements `ama_native_sys_math_add` using the generated prototype.
Adding an operation does not require a compiler registration table or emitter.

Preserve existing module paths, argument names, result names, defaults, and
unsigned arithmetic behavior unless making an explicit language/library change.
The short published names such as `sys/math/lt` and parameters `a`/`b` remain
compatibility interfaces; new internal C identifiers should still explain roles.

Build and exercise a library change with:

```sh
make
make -C tests/integration/14-standard-library
make test
```

`stdlib/Makefile` compiles the C sources into `build/stdlib/libama_std.a` using
ordinary C objects. Its explicit `MODULE_DIRECTORIES` list defines the library's
build categories; add a directory there when introducing another category.
Generated headers live in `build/stdlib/include`. The header-generation command
is `--generate-c-headers`; unlike stub generation it cannot create or alter
companion C sources, and reports missing implementations.

The library build sets `AMA_STDLIB=` while exporting its own interfaces, so the
library is scanned once as the input project rather than imported a second time.
Extensionless Ama interfaces are input data; Make explicitly disables implicit
C-executable recipes for those paths. Never build an executable over an interface
merely because a sibling `.c` has the same basename.

The compiler's configured default path can be overridden with `AMA_STDLIB`.
Use that to test a replacement library, and an empty value to test without it.
Do not silently shadow a standard library module from a user project. If testing
that API needs a different implementation, select a separate library root.
The explicit C API `compile_project_with_library` is preferable to environment
mutation in in-process unit tests.

Project stub generation may emit library headers into the requested output
folder, but never creates files inside the imported library tree. To create a
new library stub intentionally, select the library tree as the project and turn
off implicit library loading:

```sh
AMA_STDLIB= ./ama --generate-c-stubs stdlib/modules build/stdlib/include
```

Integration test compilation links the default archive after project-native
sources. A replacement-library test supplies the replacement C implementations
explicitly. Keep library build outputs under `build/`; they are generated
artifacts, not source or authoritative interface definitions.
