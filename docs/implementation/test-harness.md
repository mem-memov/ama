# Test harness contract and acceptance

This closes the unit/integration harness task in [TODO.md](../../TODO.md).
The harness remains recursive Makefiles and independent C/assert executables.
There is no test registry, common executable main, shell test script, or new
language syntax. See the [contributor guide](../../CONTRIBUTORS.md).

## Tree and responsibilities

Parents list immediate `SUBDIRECTORIES` and include `tests/parent.mk`. A leaf
includes `tests/leaf.mk`, defines its source dependencies and implements `it_*`
functions followed by explicit `run()` and `main`. Guarded parent `common.c`
files own fixtures and setup/destruction functions. Production units are compiled
through headers rather than textually included as fixture code.

Unit leaves cover individual components and their real assembled parents.
Integration leaves launch real processes and inspect status/output. Fixtures are
handwritten under `project/`; runtime copies, objects, logs and binaries are owned
under `generated/`. Tests never edit committed inputs. Shared file-operation
fixtures live under `tests/integration/support/`; scenario-specific setup remains
in the immediate parent's `common.c`.

Assertions remain enabled: `-UNDEBUG` follows both general flags and
`TEST_DEFINES`. Side effects occur before assertions, never solely inside them.
The deliberate failure fixtures verify this with `-DNDEBUG` in both locations.
Test source code must not redefine `NDEBUG` after the build flags take effect.

## Preconditions and effects

The input is a finite, explicitly connected test directory tree. Each invocation
owns its leaf output directory; separate Make invocations must not concurrently
mutate that same directory. A parent can run independent children concurrently.
The toolchain/build assumptions are those in
[incremental-build.md](incremental-build.md). The integration acceptance fixtures
use POSIX processes, filesystem operations and resource limits.

`build` produces the test executable without running it. `run` builds and runs,
retaining outputs even after success for debugging. `all` runs and cleans only
on success. `clean` explicitly removes owned generated output and does not remove
handwritten inputs. A failed test may leave its own fixture resources unfinished;
its process termination releases process resources, and the retained files are
available for diagnosis. The harness does not disguise failure as successful
fixture teardown.

Make returns failure for a failed compile, link, test execution, or cleanup command.
A failed build or execution never authorizes automatic success cleanup. An already
running sibling may finish and clean its own successful outputs; no requirement
is made about whether a sibling has been scheduled when another fails.
Explicit parent cleanup traverses children; a cleanup error propagates instead of
being reported as successful cleanup.

## Ordering argument

For each leaf, dependencies establish the strict order

```text
build -> execute -> automatic cleanup
```

`run` depends on `build`, and the `all` recipe is reachable only after `run`
succeeds. GNU Make therefore cannot start the automatic cleanup recipe before
execution finishes, including under `-j4`. On compile/link/execute failure the
failed prerequisite blocks that recipe, preserving artifacts. `run` and `build`
have no automatic cleanup recipe. Explicit `clean` is a separate operation.

For a parent, `all` depends on its child invocations. Induction on the finite tree
shows that success means each child succeeded; a child failure makes its parent
fail and propagates through every ancestor. Children own disjoint output trees,
so parallel sibling execution cannot clean another child's artifacts. Parent
cleanup lists only child cleanup commands; leaf cleanup removes only `generated/`.

Termination of traversal follows from the finite directory tree, conditional on
termination of compiler processes and user test functions. The harness cannot
prove termination of an arbitrary test. If there are D directories and L leaves,
parent dispatch work is O(D + L); leaf work is its dependency build, test execution
and cleanup. Cleanup cost is proportional to the generated tree being removed;
parallel scheduling does not change the dependency order within each leaf.

## Executable evidence

Run `make -C tests/integration/harness` or its `lifecycle/` leaf. Nine explicit
`it_*` scenarios use copies of the actual parent/leaf rules and real C compilers.
They verify:

- Successful `all` records setup, execution and teardown before output removal.
- `build` does not execute; successful `run` keeps its executable.
- Assertion failure retains the executable and test-created artifacts until clean.
- Assertions remain active even when test definitions also supply `-DNDEBUG`.
- Compilation and linking failures preserve relevant outputs and do not run tests.
- Parallel parents run and clean successful children and propagate failed children.
- Explicit recursive cleanup preserves handwritten main and fixture files.

The C harness uses `fork`/`exec` and checks normal Make exit status. Expected
assertion failures disable core dumps in the isolated child process tree, retain
logs under generated output and do not pollute the repository with core files.
Observation files also belong to the isolated generated project, outside the
individual leaf's cleanup directory, allowing ordering to be checked after cleanup.
No fixed temporary path, sleep duration or assumed sibling schedule is required.

The complete suite passed with parallel Make and `-DNDEBUG -Werror`. These are
harness acceptance tests; they are not evidence of unimplemented Ama semantics.
