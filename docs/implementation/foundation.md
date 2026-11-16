# Source and test foundation

The retained implementation contains source, parser, semantic, planning,
ownership, native and generated-execution components. Their current requirement
coverage is recorded in the [conformance matrix](conformance/README.md). The
[T001 audit](audit/README.md) preserves the original implementation checkpoint;
its missing-rule findings are historical. The [component map](components.md)
routes current responsibilities and consumers. Historical passing fixtures alone
are not evidence of current grammar conformance.

## Driver and project boundaries

`src/main.c` supplies process streams to `driver_run`. Request recognition selects
exact `--help`, `--check`, `--status`, `--native` and `--emit` commands. Operand and
option validation follows recognition; prefixes and malformed help requests do
not select a command accidentally. These are ordinary compiler C functions,
outside generated Ama execution constraints.

Arguments and streams are borrowed. The options owner holds checked-size arrays
for one command and releases them on every exit. Check/native/emit use the same
frontend owner; status inspection deliberately does not load dependency closure.
Help and reports require successful writes and flushes. Unknown requests and
failed stages return failure without publishing valid-looking success.
The [driver contract](contracts/driver.md), [checking boundary](checking.md) and
[project emitter](project-emission.md) give the narrow effects and actual tests.

## Test and build interfaces

- `make` builds the current compiler; `make test` runs recursive C/assert leaves.
- Each parent lists `SUBDIRECTORIES` and includes `tests/parent.mk`. New subtrees
  must be registered; recursive Make schedules independent siblings.
- Each leaf defines absolute `ROOT` and includes `tests/leaf.mk`. `SOURCES` names
  production C paths relative to `src/`; `LOCAL_SOURCES` defaults to `main.c`.
  `TEST_DEFINES` supplies required fixture definitions.
- Local and production objects occupy separate `generated/` trees. Actual compiler
  dependency files track headers and included fixtures. `-UNDEBUG` keeps assertions.
- `build` preserves outputs; `run` builds and executes; `all` cleans only after
  success. Failed leaves retain their evidence. `make test-clean` removes outputs.
- `make clean` also removes compiler/build and generated documentation outputs.
  `make rebuild` runs cleanup before rebuilding.
- `make docs` uses Doxygen and the checked read-only mapping; `make docs-complete`
  fails until every registered documentation/test debt is closed.

Source discovery follows the finite ordinary directory tree and sorts all C
paths; it assumes no cyclic directory symlink graph. Command manifests compare
exact commands, isolate candidates by live Make process and propagate I/O errors.
The [incremental-build argument](incremental-build.md), [test harness](test-harness.md)
and [documentation checks](documentation/README.md) state their domains and
independent/composed evidence. The [contract registry](contracts/README.md) maps
retained component arguments without certifying the unfinished language.
