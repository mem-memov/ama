# Notes for coding agents

## Analysis scope

- Do **not** analyze binary files and build artifacts:
  - `ama` (compiler binary)
  - `build/` (object files, `build/tests/` intermediates)
  - any `*.o` or generated `.c` files under `build/`
- Source of truth is the `.c`/`.h` files under `src/`, with `README.md` and `EBNF.md`
  as the normative language specification.
- Tests use recursive Makefiles under `tests/unit/` and `tests/integration/`.
  Each leaf has its own `main.c` with `it_*` functions and an explicit `run()`;
  guarded parent `common.c` files hold test fixtures. Assertions use `assert`.
- Integration module files live under each leaf's `project/`; expectations are
  `expected.txt` or `expected-error.txt` beside it.
- Do not analyze generated artifacts in any test's `generated/` directory.

## Style

- Follow `CONTRIBUTORS.md` for naming, formatting, component boundaries, and tests.
- Use English text and descriptive identifiers. C indentation is four spaces;
  Ama fixtures retain semantic one-space nesting. Make recipes use `>` followed
  by four spaces.

## Workflow

- Build: `make` (binary `./ama`)
- Test: `make test`; individual suites: `make -C tests/unit/lexer/tokens`.
- Test cleanup: `make test-clean`. Successful leaves clean automatically; failures
  preserve generated files for debugging.
- Do not commit unless the user explicitly asks.
