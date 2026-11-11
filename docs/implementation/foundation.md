# Source and test foundation

The implementation has been reset to a small executable foundation. The old
production files, numeric library and test scenarios have been removed without
analysis. The [component map](components.md) identifies the responsibilities to
implement next; it is an inventory of the removed tree, not existing API guidance.
The technical specification is unchanged.

## Driver contract

`src/main.c` supplies process streams to `driver_run`. The driver delegates request
recognition to `driver/request/` and response writing to `driver/output/`. These
are ordinary compiler C functions, outside generated Ama stackless constraints.

The argument domain is a process-style vector: count at least one, with that many
valid nonnull NUL-terminated argument strings. The two stream parameters are valid C streams; inability to write to the selected
stream is an output failure. Arguments are borrowed and never modified; the driver neither
closes nor retains the supplied streams. No driver operation allocates or owns a
compiler object or introduces mutable global state.

Request recognition has exactly two outcomes. For count two and an exact `--help`
argument, it selects Help; every other vector selects Unavailable. These sets are
disjoint and cover the domain. The parser never infers commands from prefixes,
argument contents, filenames or declared names. Its time and auxiliary space are
constant: comparison against a fixed spelling examines at most that spelling's
length plus its terminator. The driver does not specify future command syntax.

Help writes implementation status to the output stream. Unavailable writes an
explicit diagnostic to the error stream and returns failure. Writing includes
flushing; a write or flush failure returns failure. The driver returns success
only for Help with successful output. Thus no compilation/check request can
report success before its implementation exists. Output work is bounded by the
fixed response length, apart from the host stream's blocking/error behavior.

Unit request tests cover exact recognition, prefixes, extra arguments and requests
for unavailable operations. Composite driver tests check real parser/output
handoffs, stream routing and write failure. Integration tests execute the actual
launcher through POSIX process APIs without a shell and verify exit status and
stream separation. These are foundation tests, not Ama syntax acceptance tests.

## Test/build interfaces

- `make` builds the foundation; `make test` runs the recursive unit/integration tree.
- Each parent lists `SUBDIRECTORIES` and includes `tests/parent.mk` at the appropriate
  relative path. Child invocations use recursive Make; parallel siblings remain
  independent. Every new test subtree must be explicitly connected to its parent.
- Each leaf defines absolute `ROOT` and includes `tests/leaf.mk`. Optional `SOURCES`
  lists production C paths relative to `src/`; `LOCAL_SOURCES` defaults to `main.c`.
  Optional `TEST_DEFINES` supplies leaf-specific preprocessor definitions.
- Local and production test objects occupy separate trees under `generated/`.
  Compiler-generated dependency files track actual headers and included fixtures.
- `build` preserves outputs for inspection; `run` builds and executes; `all` runs
  then cleans. Failed leaves retain artifacts. Tests compile with `-UNDEBUG`.
- Integration launcher tests build the foundation as an order-only prerequisite,
  so the leaf also runs independently without a prior root build.
- `make test-clean` removes test outputs; `make clean` also removes `build/` and
  `ama`. `make rebuild` performs cleanup before building, including parallel Make.

Source discovery recursively visits the ordinary finite directory tree under
`src/`, collects `.c` files and sorts the resulting set. Termination follows from
the tree's finiteness; induction on depth shows each source is included. Cyclic
symlink directory graphs are outside this build-input contract. Compiler dependency
files determine header consumers rather than an approximate wildcard depth.
Recursive discovery and incremental rebuilding now have separate executable
acceptance checks and a [contract](incremental-build.md).

## Remaining work

Build invalidation, no-change behavior and nested dependency checks are completed
in the incremental-build task. [Test-harness acceptance](test-harness.md) also verifies output preservation,
cleanup ordering and assertions under release flags. The remaining foundation task
is to establish the reusable component-contract and proof-review procedure.

## Verification of this task

The foundation passed `make test` and `make -j4 test`. After `make clean`, the
integration leaf rebuilt and tested the launcher independently. The full parallel
suite also passed with `-DNDEBUG -Werror`; test compilation explicitly restores
assertions with `-UNDEBUG`. The three leaves contain eight explicit test cases.
These checks do not establish compiler or runtime conformance beyond the driver.
