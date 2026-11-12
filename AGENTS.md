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
  grammar is `spec/grammar.md` and follows the language chapters. Do not recreate
  root-level specification or EBNF files as alternative copies or navigation stubs.
- Do not create, edit, move, or delete any file in `spec/` without prior explicit
  user authorization for that specification change. A TODO task, implementation
  request, refactoring request, or discovered contradiction does not provide this
  authorization. Never change requirements to fit the code. Existing authorization
  for a specific specification change is sufficient; do not ask again for it.
- No backward-compatibility documentation is required. Remove obsolete proposals
  and API descriptions instead of preserving alternative contracts.
- Follow ARCH-05/ARCH-06: use exact algorithms, forbid heuristics, and provide
  general mathematical/algebraic justification with explicit invariants and tests.
- Keep specification chapters readable: explain the model and motivation, show
  Ama examples with assumptions and expected effects, then state complete rules.
  Label informative explanation/examples; define each requirement once and link
  related chapters rather than duplicating semantics in the grammar.

## Resolve ambiguity before implementation

- If a requirement is ambiguous or contradictory, or required observable behavior
  is unspecified, stop the affected task before choosing an interpretation,
  heuristic, fallback, or workaround. Do not switch to another TODO task to avoid
  resolving the blocker. Read-only investigation needed to explain it may continue.
- Record the issue in the root [AMBIGUITIES.md](AMBIGUITIES.md) registry with a
  stable ID, `Open` status, current TODO task, exact specification links, a minimal
  example, the precise question, and the consequences of the possible answers.
  Ask the user for clarification and wait for an explicit answer. Silence and
  elapsed time are not decisions; do not mark or commit the task as completed.
- Record the user's answer and its source before resuming. An answer authorizes
  the stated behavior; it authorizes editing `spec/` only when it explicitly
  requests that specification change. If the answer requires a specification
  change without authorizing it, request that authorization before proceeding.
- Routine engineering choices with unambiguous specified behavior remain the
  agent's responsibility. Decide them using exact algorithms, explicit contracts,
  and evidence; do not turn ordinary design work into repeated permission requests.

## Style

- Follow `CONTRIBUTORS.md` for naming, formatting, component boundaries, and tests.
- Use English text and descriptive identifiers. C indentation is four spaces;
  Ama fixtures retain semantic one-space nesting. Make recipes use `>` followed
  by four spaces.
- Decompose implementation as far as cohesive responsibilities allow, using nested
  directories, small single-operation functions, and narrow explicit interfaces.
  Keep orchestration separate from parsing, checks, state changes, and cleanup.
  Do not satisfy decomposition with empty folder levels, one-line public wrappers,
  unrestricted utility modules, or arbitrary file/function line-count targets.
- Every maintained code file needs a purpose comment and explicit test coverage.
  Follow the Doxygen and per-file coverage rules in `CONTRIBUTORS.md`, including
  headers, native companions, test support, scripts, and build infrastructure.
  New C source/header files and test C files use Doxygen `@file` documentation;
  document each nontrivial function/type/interface's purpose, contracts, ownership,
  side effects, and failure behavior. Explain why non-obvious code is needed.

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

- When asked to implement or continue implementation, work from [TODO.md](TODO.md),
  respecting prerequisites and the stated acceptance criteria. Choose a ready task
  and finish its implementation, documentation, component/composition tests,
  and required checks before marking it complete.
  Do not replace the roadmap with unrelated improvements or claim unsupported
  implementation coverage. Keep task status and evidence accurate.
- Commit after every completed task. This instruction is standing user
  authorization for those commits; no separate request is needed. Include the
  task's code, tests, documentation, and TODO status in the same commit. Use
  explicit staging paths, inspect the staged diff, and preserve unrelated user
  changes. Never mark an incomplete or blocked task complete merely to commit it.
  Do not combine several independently completed TODO tasks into a later batch.
- Build: `make` (compiler `./ama`); use `--emit ... --compile` for a verified project executable.
- Test: `make test`; individual suites: `make -C tests/unit/driver/request`.
- Test cleanup: `make test-clean`. Successful leaves clean automatically; failures
  preserve generated files for debugging.
