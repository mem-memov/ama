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
established C/POSIX names such as `FILE`, `EOF`, and `waitpid`. Ama intrinsic
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
to them changes the program. Deliberate CRLF fixtures retain CRLF; ordinary
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
