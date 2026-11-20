# Ama

Ama is a minimal, international programming language built around modules and
explicit state. A module defines behavior and a type; instances own state, and
references identify instances without owning them. Domain capabilities belong in
libraries rather than an expanding collection of language primitives.

[The product specification](spec/README.md) is the source of truth.
The last accepted implementation baseline maps **214 requirements** and
**150 acceptance rows** in the
[reviewed conformance evidence](docs/implementation/conformance/README.md).
The specification additionally defines
[static type markers](spec/language/static-results.md#tpl-14) and
[declaration type expressions](spec/language/templates.md#tpl-15). Implementation and
renewed conformance acceptance are pending [T221](TODO.md#t221) and
[T226](TODO.md#t226).
The newly specified [versioned project model](spec/project/packages.md),
[slash/relative paths](spec/language/source-and-names.md#src-06), single-dot
static names, scoped `.ini` aliases and
[independent project commands](spec/implementation/project-tools.md) await the
separate T231–T237/T224 alignment plan. The current executable workflows below
belong to the recorded implementation edition; the new command names and
spellings are target contracts, not claimed current support.
The later [CMake build contract](spec/implementation/application-builds.md),
[named application inputs](spec/implementation/launch-and-embedding.md) and
[no-main source inclusion](spec/implementation/c-library.md#embed-09) are also
specification targets pending the T239–T242 implementation tasks. The command
examples below do not claim those new workflows are already implemented.
[Ordinary templates](spec/language/templates.md) and separate
[static modules](spec/language/static-results.md) provide compile-time inference,
partial composition and concrete specializations. Complete type constructors
remain valid in ordinary templates. Static modules remove repetitive concrete
module assembly; behavioral properties belong to module contracts and tests
under [CORE-06](spec/introduction.md#core-06). Native definitions remain
concrete; C applications use checked concrete facades.
The [release guide](docs/implementation/conformance/release.md) gives reproducible
commands, supported configurations and the limits of that evidence.
[Executable examples](docs/implementation/examples.md) and
[generic list algorithms](examples/template-lists/README.md) show supported workflows;
[TODO.md](TODO.md) lists selected work; [BACKLOG.md](BACKLOG.md) holds future
proposals; [DONE.md](DONE.md) preserves completed tasks and their acceptance
evidence. The [audit](docs/implementation/audit/README.md) describes the original implementation
before its replacement and is historical evidence.
`./ama --check ROOT .module` validates dependencies, syntax, types and typed plans without emitting or
compiling C. `./ama --status ROOT .module` reports source activation without
validating a project. `./ama --emit ROOT .module --out DIR --sdk /path/to/ama/src --compile`
produces a verified stackless executable at `DIR/program`. Entry calls, retained
reads, assignments, branches, loops, suspension and continuation copying execute;
typed list services and live traversal execute with checked origins and copied
continuation boundaries ([implementation](docs/implementation/list-traversal-execution.md)).
Custom C hosts can explicitly resume or dispose of yielded roots; the standard launcher rejects
unhandled root yield. Use `--standard-root stdlib/modules` for
[the canonical integer library](docs/implementation/scalar-library.md). `./ama --help` describes commands. [Checking](docs/implementation/checking.md),
[semantic analysis](docs/implementation/semantic-analysis.md) and
[typed plans](docs/implementation/typed-plans.md) have independent and composed
C/assert tests. Check-only success does not validate handwritten C bodies.

## Reading the project

- [Grammar](spec/grammar.md): source layout, declarations, calls and control flow.
- [Product specification](spec/README.md): terminology, architecture,
  runtime, native contracts, translation scheme and acceptance requirements.
- [Native value ABI](VALUE_ABI.md): values, capabilities, lifetimes and errors.
- [C-library export checking](docs/implementation/c-library-exports.md): explicit
  exports, transport profiles and resolved ABI metadata.
- [Public C header](docs/implementation/c-library-header.md): independent consumer
  types, ownership operations and checked target/version/layout boundaries.
- [C-library lifecycle](docs/implementation/c-library-lifecycle.md): explicit contexts,
  exported roots, checked disposal and independent immutable diagnostics.
- [C-library transport](docs/implementation/c-library-transport.md): typed inputs,
  atomic retained reads and independent scalar/list snapshots.
- [C-library execution](docs/implementation/c-library-execution.md): explicit
  entry/resume outcomes, stackless dispatch and native callback exclusion.
- [C-library artifacts](docs/implementation/c-library-packaging.md): static/shared
  components, exact symbol isolation, reload identities and code lifetime pins.
- [Native parent and Ama child](docs/implementation/c-library-nested.md): exclusive
  foreign context ownership, typed value transfer and partial-construction cleanup.
- [Nested suspension and supervision](docs/implementation/c-library-supervision.md):
  independent continuations, copied failure reports and explicit child reset.
- [C host examples](docs/implementation/c-library-examples.md): reproducible multi-file
  static/shared consumers and native-owned compiled children through the CLI.
- [Ordinary task phases](docs/implementation/task-phases.md): typed Ready/Awaiting/Completed
  values, explicit nested waiting and terminal application outcomes.
- [Visible task enrollment](docs/implementation/task-enrollment.md): declared owners,
  checked task references and nonwrapping epoch/wait generations.
- [Native completion storage](docs/implementation/native-events.md): immutable native
  tokens, retained results and independently acknowledged callback resources.
- [Contributor guide](CONTRIBUTORS.md): readable components, formatting and tests.
- [Development standard](docs/development/README.md): dependency planning,
  correctness arguments, step records, verification and task acceptance.
- [Standard library](stdlib/README.md): library boundaries and native companions.
- [Executable examples](docs/implementation/examples.md): stateful methods, self-reference,
  live list mutation, independent continuations and owned file-operation failures.
- [Work queue](TODO.md): selected engineering and verification tasks.
- [Product backlog](BACKLOG.md): future changes and open design questions.
- [Completed work](DONE.md): task history, decisions and acceptance evidence.

## Language outline

This outline describes the language; use the executable examples and task evidence
for the currently verified implementation boundary. Files are modules. Their qualified
paths determine type identity. A module file
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
..integer
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
data. The frontend preserves these bytes and source spans; parameter type and
capability checks are performed by the semantic stage.

An instance call places one separating space before `!`; inputs and outputs occupy
separate indented lines using `<` and `>`. An unmarked target followed by `>>` reads retained results
without executing or resetting the instance. Acquisition refusal may have a `:`
alternative. Calls execute explicit instances, not module paths.

Body variables persist across calls. Parameters and results reset before entry
execution and retain their values afterward. `%` suspends, `target %`
resumes, and the source prefix `% source` explicitly copies continuation state.
Ordinary copying produces independent owned state and clears saved positions.

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
capabilities. Module-owned native C identifiers use `AMA_NATIVE(name)` within
their generated module context; [namespace probes](docs/implementation/native-namespaces.md)
verify distinct qualified names in one translation unit. Persistent Ama links remain visible in declared Ama fields; opaque
native payloads own native resources. Exact algorithms, explicit invariants and
mathematical/algebraic justification are implementation requirements.

## Building and testing

Generate the C file and interface reference with `make docs` (requires Doxygen and the pinned Python Markdown parser),
then open `docs/technical/index.html`. See the
[documentation build guide](docs/documentation.md) for inputs and cleanup.

```sh
make
./ama --help
./ama --check path/to/project .main
./ama --emit path/to/project .main --out generated --sdk "$PWD/src" --compile
./generated/program
make test
make test-clean
```

Compiler sources are under `src/`; library interfaces and C sources live under
`stdlib/modules/`. The [library guide](stdlib/README.md) describes integers, text, failures, tasks,
messages and channels, including their ownership and native contracts. Tests form recursive Makefile trees under `tests/unit/` and
`tests/integration/`. Each leaf builds its own C test executable using `assert`.
Successful leaves clean their generated files; failures preserve debugging output.

Run an individual suite with, for example, `make -C tests/unit/driver/request`.
See the contributor guide for component and composition tests and the backlog for
implementation stages. Documentation-only changes require consistency checks;
implementation claims require the corresponding executable acceptance evidence.

Native interface authoring is available through `--native ROOT .module --out DIR
--sdk /path/to/ama/src`; add `--create` to propose missing companions. See the
[native scaffolding guide](docs/implementation/native-scaffolding.md).
