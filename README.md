# Ama

Ama is a minimal, international programming language built around modules and
explicit state. A module defines behavior and a type; instances own state, and
references identify instances without owning them. Domain capabilities belong in
libraries rather than an expanding collection of language primitives.

[TECHNICAL_SPECIFICATION.md](TECHNICAL_SPECIFICATION.md) is the source of truth.
The implementation is being rebuilt from this specification. Currently `./ama`
provides `--help`; compilation and checking explicitly fail as unimplemented.
The design described here is its acceptance contract, not a claim of availability.
The implemented source layer now provides immutable text/spans, pinned Unicode
identifiers, exact module discovery, collision-checked internal names and
[structured diagnostics/source mappings](docs/implementation/diagnostic-rendering.md).
The [syntax frontend](docs/implementation/parser-errors.md) now parses sections,
declarations, calls/control flow and native binding tables into source-mapped
structures. These components have independent and composed tests; semantic
resolution and CLI compilation/checking are still upcoming.

## Reading the project

- [Grammar](EBNF.md): source layout, declarations, calls and control flow.
- [Technical specification](TECHNICAL_SPECIFICATION.md): terminology, architecture,
  runtime, native contracts, translation scheme and acceptance requirements.
- [Native value ABI](VALUE_ABI.md): values, capabilities, lifetimes and errors.
- [Contributor guide](CONTRIBUTORS.md): readable components, formatting and tests.
- [Standard library](stdlib/README.md): library boundaries and native companions.
- [Implementation backlog](TODO.md): remaining engineering and verification work.

## Language outline

Files are modules. Their qualified paths determine type identity. A module file
`shop/item` may have submodules in `shop/item+/`; the directory marker is omitted
from logical paths such as `.shop.item.validate`. Names support Unicode.

Visible annotations delimit sections: `@=` declares shared immutable constants,
`@<` parameters, `@>` results, and `@!` an Ama body. `@C` selects a native C body.
Constants-only modules are complete; unfinished drafts cannot be dependencies.

Declarations put the name on one line and its module type on the next. Optional
initializers have one additional space of indentation. A `|` introduces literal
UTF-8 data through the physical end of the line. Literal interpretation belongs
to the declared type. The following paths illustrate library types:

```text
@!
quantity
.std.integer
 |12
item
.shop.item
item_ref&
.shop.item
 &item
items[]
.shop.item
```

A suffix `&` marks reference storage; `[]` marks a list. Type lines contain only
module paths. Assignment puts a receiver above its indented source. Integer
literals and implicit numeric variables have no special position in the language.

Entry-call inputs may also use `message < |Hello` or positional `< |Hello`.
The declared parameter type interprets the UTF-8 payload; only single owned values
with literal capability accept this form. `#` and trailing spaces after `|` are
data. This specification amendment is the next frontend task in the backlog.

An instance call ends in `!`; inputs and outputs occupy separate indented lines
using `<` and `>`. An unmarked target followed by `>>` reads retained results
without executing or resetting the instance. Acquisition refusal may have a `:`
alternative. Calls execute explicit instances, not module paths.

Body variables persist across calls. Parameters and results reset before entry
execution and retain their values afterward. `%` suspends, a target suffix `%`
resumes, and a source prefix `%` explicitly copies continuation state. Ordinary
copying produces independent owned state and clears saved positions.

## Translation and runtime

The compiler resolves project and library dependencies into typed IR, plans
storage and lifetimes, and emits one project C assembly included by a launcher.
Ama dependencies are loaded as modules rather than separately linked C functions.
The assembly declares generated types and native bodies before executable code.

Ama bodies execute as labels and explicit transfers over stable heap instances.
Instance metadata holds return links, saved positions and acquisition state.
There is no C call stack for Ama module calls. Native entry bodies inline; native
implementation helpers may use ordinary C calls and the stack.

Native companions use the fixed `.abi` table to declare lifecycle and conversion
capabilities. Persistent Ama links remain visible in declared Ama fields; opaque
native payloads own native resources. Exact algorithms, explicit invariants and
mathematical/algebraic justification are implementation requirements.

## Building and testing

```sh
make
./ama --help
make test
make test-clean
```

Compiler sources are under `src/`; library interfaces and C sources will be
introduced under `stdlib/modules/` as their contracts are implemented. Tests form recursive Makefile trees under `tests/unit/` and
`tests/integration/`. Each leaf builds its own C test executable using `assert`.
Successful leaves clean their generated files; failures preserve debugging output.

Run an individual suite with, for example, `make -C tests/unit/driver/request`.
See the contributor guide for component and composition tests and the backlog for
implementation stages. Documentation-only changes require consistency checks;
implementation claims require the corresponding executable acceptance evidence.
