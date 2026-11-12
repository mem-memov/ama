# Notes for coding agents

## Analysis scope

- Do **not** analyze binary files and build artifacts:
  - `ama` (compiler binary)
  - `build/` (object files, `build/tests/` intermediates)
  - any `*.o` or generated `.c` files under `build/`
- Compiler source lives under `src/`; standard library interfaces and C sources
  live under `stdlib/modules/`.
- Tests use recursive Makefiles under `tests/unit/` and `tests/integration/`.
  Each leaf has its own `main.c` with `it_*` functions and an explicit `run()`;
  guarded parent `common.c` files hold test fixtures. Assertions use `assert`.
- Integration module files live under each leaf's `project/`; expectations are
  `expected.txt` or `expected-error.txt` beside it.
- Native test interfaces and handwritten companion `.c` files live together
  under `project/`. Generated native headers belong under `generated/`.
- Do not analyze generated artifacts in any test's `generated/` directory.

## Specification authority

- The entire `spec/` directory is the source of truth for language semantics,
  product behavior, and implementation requirements. Start at `spec/README.md`;
  use `spec/requirements.md` to locate a stable requirement ID. The canonical
  grammar is `spec/grammar.md` and follows the language chapters. Root-level
  specification/EBNF files are navigation links, not parallel contracts.
- Do not modify files in `spec/` unless the user explicitly requests
  that change. An implementation or refactoring request does not authorize changing
  requirements to fit the code. Report contradictions and ask for a decision.
- No backward-compatibility documentation is required. Remove obsolete proposals
  and API descriptions instead of preserving alternative contracts.
- Follow ARCH-05/ARCH-06: use exact algorithms, forbid heuristics, and provide
  general mathematical/algebraic justification with explicit invariants and tests.
- Keep specification chapters readable: explain the model and motivation, show
  Ama examples with assumptions and expected effects, then state complete rules.
  Label informative explanation/examples; define each requirement once and link
  related chapters rather than duplicating semantics in the grammar.

## Style

- Follow `CONTRIBUTORS.md` for naming, formatting, component boundaries, and tests.
- Use English text and descriptive identifiers. C indentation is four spaces;
  Ama fixtures retain semantic one-space nesting. Make recipes use `>` followed
  by four spaces.

## Target implementation boundaries

- Follow the compiler decomposition and component/composite testing requirements
  in `spec/implementation/compiler.md` and
  `spec/implementation/tools-and-conformance.md` (`ARCH-01` through `ARCH-06`,
  `TEST-06` through `TEST-08`).
- Ama modules translate through resolved IR into labels in one project C assembly;
  native entry bodies inline, while calls inside native C may use the stack.
- Instances execute directly in stable pool slots. Do not introduce working-state
  save/restore or implicit default instances. Persistent Ama references belong to
  declared Ama fields, never opaque native payloads.
- Asynchrony is explicit and cooperative. `%` returns to the immediate caller;
  native callbacks do not retain borrowed Ama state or execute Ama. Parallelism
  uses isolated execution contexts with one executor each and explicit messages,
  under `spec/language/async-and-parallelism.md`; do not share mutable instances
  or checked Ama references between contexts.
- `name+/` is physical submodule storage, not part of the logical identity.
  Document implementation support only when backed by acceptance evidence.

## Workflow

- Build: `make` (compiler `./ama`); use `--emit ... --compile` for a verified project executable.
- Test: `make test`; individual suites: `make -C tests/unit/driver/request`.
- Test cleanup: `make test-clean`. Successful leaves clean automatically; failures
  preserve generated files for debugging.
- Do not commit unless the user explicitly asks.
