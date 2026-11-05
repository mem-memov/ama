# Notes for coding agents

## Analysis scope

- Do **not** analyze binary files and build artifacts:
  - `ama` (compiler binary)
  - `build/` (object files, `build/tests/` intermediates)
  - any `*.o` or generated `.c` files under `build/`
- Source of truth is the `.c`/`.h` files under `src/`, with `README.md` and `EBNF.md`
  as the normative language specification.
- Test sources live in `tests/NN-name/` (module files named `main`,
  expected output in `expected.txt`).

## Workflow

- Build: `make` (binary `./ama`)
- Test: `make test` (runs `./tests/run.sh`)
- Do not commit unless the user explicitly asks.
