# Writing code and tests for Ama

The project should read like a clear book: an operation, its inputs, state changes,
result and failure path should be understandable without decoding abbreviations
or reconstructing hidden dependencies.

[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md) is the source of truth.
[EBNF.md](EBNF.md) maintains its grammar; [VALUE_ABI.md](VALUE_ABI.md) explains
the native contract. [TODO.md](TODO.md) tracks implementation work. Change the
specification only when the user explicitly requests it.

## Names explain roles

Use English for compiler/native identifiers, comments, documentation, diagnostics
and build messages. Ama itself supports international identifiers and paths;
this repository's prose convention is not a restriction on Ama programs.
Deliberate Unicode fixtures must retain meaningful encoding data; use Unicode
escapes in C when that makes the bytes' purpose clearer.

Use `snake_case` for C functions/variables, `PascalCase` for types and
`UPPER_SNAKE_CASE` for constants/enum members. Prefer `parser`, `parameter_count`,
`source_symbol_id` and `destination_symbol_id` to unexplained initials or ambiguous
counts. Name pointers after objects and IDs after identities. Preserve established
C/POSIX names such as `FILE`, `EOF` and `waitpid`.

Library paths and exported interface names are public library contracts,
not language keywords. Review interface changes with their declarations, native
implementation, documentation and tests. Keep internal names descriptive.

## Formatting follows the file's syntax

C uses four-space indentation, spaces instead of tabs, opening braces on the
control/declaration line, and separate statements for separate operations. Split
long calls/declarations at meaningful boundaries. Makefiles use
`.RECIPEPREFIX := >`, with `>` followed by four spaces in recipes.

Ama nesting uses exactly one space. Target sections are optional `@=` shared
constants, optional `@<`/`@>` interfaces, then `@!` or `@C`. Constants-only files
are complete; files with none of `@=`, `@!`, `@C` are drafts. Blank lines never
delimit sections. A literal's payload after `|` is data and must not be trimmed
or reformatted. Preserve deliberate CRLF fixtures; ordinary source/docs use LF.

## Components form a tree of small operations

Follow ARCH-01 through ARCH-04 in the technical specification. Every major stage
must decompose into cohesive nested directories: project/dependency loading,
source/path/UTF-8 handling, lexical layout/literals, parsing, semantic checking,
lowering, storage planning, C emission and runtime support.

Parent-level orchestration should read as short named operations. Functions have
one responsibility at one level of abstraction and should normally fit one editor
screen. Decompose long multi-phase or deeply nested logic; justify exceptions in
review. Do not create an unrestricted utility directory or artificial public APIs
merely to split trivial private helpers into files. The build recursively discovers C sources under `src/`; compiler dependency
files track included headers and fixture files. Keep deeper source directories
and new independently runnable test leaves connected to these build rules.

Keep private helpers `static`. Public headers compile independently; internal
headers expose only sibling contracts. Pass narrow contexts explicitly. Compiler
objects own their allocations and dependencies; do not introduce mutable global
compiler state or pass the whole project where a symbol table/source view suffices.

Compiler C helpers may use ordinary functions and the stack. Small visible helpers
may be `static inline`; LTO is an optimization. Those choices are distinct from
the target generated native-entry inlining requirement.

## Resolve dependencies, then generate

Ama dependencies come from declared module types, constant namespaces/sources and
transitive references under configured roots. Resolve logical identity once in a
project registry, including physical `name+/` mapping. Detect duplicates, missing
or required-draft modules and report requesting locations. A parent directory
relationship does not import declarations, instantiate objects or imply ownership.

Preserve source spans through parsing, resolution, typed IR, storage planning and
C emission. Parsing must not guess syntax from alias lookup or emit C strings.
Resolve before emission and do not publish valid-looking output after failure.
A syntax tree owns its storage; borrowed source paths/views must not outlive their
owners. Bound recursion/size arithmetic and clean partial work explicitly.

Model loading, eager ownership layout, constant initialization and execution as
different dependency problems. A loading/module-type cycle is not automatically
an invalid program; reject eager ownership and initialization cycles, while busy
instance access is checked dynamically.

Emit one project C assembly with dependency-correct declarations/includes. Ama
source files are parsed into the common IR, not textually included by C. Emit a
shared Ama body/type/constant table once, while each owned declaration retains
its distinct state. Track contributing source/native/ABI inputs for invalidation.
Native companions are included where required; external C libraries may link
normally. Do not turn Ama module invocation into a C call or separate C module link.

## Generated execution must be stackless

Ordinary Ama modules become labels/direct jumps in one C execution function.
Instances execute in stable heap slots; do not copy caller state into working
globals or save/load payloads around calls. Caller identity and return selectors
form the allowed explicit return chain. Refuse the same busy instance; distinct
available instances of one type may execute with proper current-instance selection.

`!` starts at entry and resets interface defaults. `%` resumes with output-only
bindings; standalone `%` yields. `>>` reads stored results without execution;
module-path `>>` reads shared constants. Empty positional `<`/`>` lines consume
positions without supplying an input or receiving a result. Keep indentation's
argument/early-return distinction explicit. Literal call arguments and implicit
numeric variables remain forbidden.

`@C` entry bodies must inline under a verified toolchain. Plain `inline`/LTO is
not proof. Native helpers, recursion, lifecycle callbacks, allocation, I/O and
external C libraries may use the stack. Never dispatch Ama modules through a C
call stack or preserve an unfinished native activation across Ama yield.

The host receives completed/yielded/failed outcomes. The standard launcher treats
unhandled root yield as a diagnosed unsuccessful exit. There is no implicit
scheduler, automatic resume loop or callable default instance per module.

## Target native contracts and state ownership

A native module has its source interface and companion `.h`, `.c`, `.abi` files.
The `.abi` file is a fixed UTF-8 binding table: `+`/`-` are mandatory; `|`, `=`,
`?`, `[]` are optional literal/copy/condition/index capabilities. The compiler
reads metadata and emits typed uses; the C compiler checks handwritten signatures.
Do not discover capabilities by helper names or add new parser protocols.

All persistent Ama ownership/references belong in declared Ama fields. Opaque
native payloads own native resources only. Borrowed instance/field/list views
cannot escape into payloads, globals, retained callbacks or asynchronous work.
Native code cannot edit generation/busy/return metadata; library context readers
use generated read-only access.

Follow LIFE-01 through LIFE-05. Construct fields in order and select one fresh
payload constructor, not default-construct then overwrite resources. A failed
native constructor releases its partial resources; the runtime unwinds live
children. Destroy the native payload once before children in reverse order.
Native hooks must not independently own/destroy generated Ama fields.

Prepare a replacement before destructive commit. Preparation failure preserves
the old destination; destruction failure after commit is terminal without rollback.
Continue cleanup, retain the primary cause and report secondary errors without
blindly retrying destruction. Move helpers used internally are not permission
to relocate live instances on ordinary invocation or pool growth.

Native copying requires `=`. Derive composed clone capability from owned fields;
references to noncopyable targets remain copyable. Clone generated ownership graphs
with internal-reference remapping; native clone hooks handle their payload only.
Ordinary copying clears positions recursively, while `%` copying preserves them.
Reject busy source descendants and ancestor/descendant overlap before mutation;
ordinary self-assignment is a no-op and `%` self-copy is invalid.

Use target `AmaStatus` code/message/length, with immutable program-lifetime text.
Capture the active trace
before cleanup; no exceptions or universal rollback are promised. Acquisition-only
`:` uses a caller-owned last-refusal snapshot. An ordinary library reader returns
that status as data and must not treat its nonzero code as failure of the reader.
List operation refusals remain ordinary results, distinct from technical errors.

Shared `@=` constants initialize once before the root, outside executable pools.
They are immutable, copyable values without mutable-object references/open external
resource identities. Qualified `.module.member` sources and path-plus-`>>` reads
resolve statically. Preserve initialization ordering and reject cycles; do not
run arbitrary native constructors at compiler time.

## Generated names and source mappings

Hash complete logical module/member/category identities with collision checks;
do not substitute slashes with underscores or use basename-only names. `name+/`
markers affect physical source paths, not logical identities. Unicode/XID tables
are pinned, identity is case-sensitive and bytewise, and filesystem aliases must
not merge distinct names.

Preserve physical file/line spans, original Unicode names and correctly escaped
`#line` directives. Compiler source columns are UTF-8 byte columns where recorded;
`#line` itself does not restore source columns or rename hashed C variables.
Maintain operation/call-site maps for active runtime traces as well as C debug
mappings. Keep source ownership valid until all dependent nodes/maps are released.

## Two test harnesses, at both component scales

Use C/assert and recursive Makefiles. `tests/unit/` covers meaningful leaf
components and assembled parents with the same harness; `tests/integration/`
compiles/runs projects and checks observable output/diagnostics/statuses.
Mirror the responsibility tree so a leaf, subtree or whole suite runs independently.
No shared monolithic executable, auto-registration, shell test framework or hidden
runner is required.

Each executable leaf has fixture includes, descriptive `it_*` functions, explicit
`void run(void)` listing them in order, and its own `main`. Guarded parent
`common.c` files provide shared setup/destroy support. Tests explicitly initialize
and clean their fixture; meaningful scenario relationships are visible in the test.
Use `assert` with `-UNDEBUG`. Perform side effects before assertions so release
flags do not remove the operation being tested. Link production units through
headers; fixture `.c` inclusion must not bypass production component boundaries.

Test real composed components together, including source span/type/storage
handoffs, initialization/error cleanup, lowered control flow and generated C.
Mocks cannot replace those handoff checks, and end-to-end tests cannot replace
independently runnable component checks. Test behavior rather than extracted
function-call counts. Cover boundary/negative/resource-failure paths and regressions.

Parents use `$(MAKE) -C ...`; dependencies enforce build/run/cleanup ordering even
under parallel Make. Keep handwritten inputs under each integration leaf's
`project/`, expected outputs beside it, and generated files under `generated/`.
Successful leaves clean; failed leaves preserve artifacts. Never overwrite native
source/header inputs or mutate committed fixtures in stub tests.

## Check a change at the right level

```sh
make
make -C tests/unit/driver/request
make -C tests/unit/driver/run
make test
make test-clean
```

Run focused suites for affected behavior and required whole-project checks for
semantic/backend changes. Run sanitizer/failure-injection and toolchain inlining
checks when relevant; document what was actually verified. Generated C behavior
must be tested by compiling/running it where appropriate, not only by comparing
an emitted substring. Documentation-only changes need link/structure/consistency
checks rather than claiming unrelated compiler tests ran.

## Exact solutions and evidence

Follow ARCH-05/ARCH-06 and TEST-08. Do not use heuristics, guessed interpretations,
or example-specific exceptions. Define component domains, invariants, failure
conditions and complexity, and give a general mathematical/algebraic argument for
the algorithm. Derive tests from those laws and exercise real component handoffs.
Tests supplement the argument; passing examples alone do not establish correctness.

## Native C modules

Keep the Ama interface and companion `.h`, `.c`, `.abi` together. Follow the
[native value ABI](VALUE_ABI.md), including common status, lifetime and cleanup
rules. Generated declarations determine C signatures. Native entry bodies inline
into the project assembly; internal C helpers may use ordinary calls and the stack.
Do not retain borrowed Ama views or dispatch Ama modules from C.

## Standard library contributions

Follow the [library guide](stdlib/README.md). Define capabilities through typed
module interfaces and the fixed ABI; publish paths and shared constants together
with their tests. Extend domain functionality through libraries while preserving
the language's minimal core. Do not add parser exceptions for individual libraries.
