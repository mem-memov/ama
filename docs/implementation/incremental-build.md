# Incremental build contract

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This implements the source discovery and dependency-tracking task in
[TODO.md](../../TODO.md). It does not change the technical specification or
introduce a compiler/runtime feature. The shared rules serve the project and
independently built C test leaves.

## Inputs and ownership

Inputs are a finite ordinary source directory tree, Make configuration, compiler
command/flags, source/header/fixture files, and an output directory owned by this
build. Source paths and configured build paths must be representable as GNU Make
filename lists; the handwritten compiler tree uses paths without whitespace.
This is a build-system constraint, not a restriction on Ama source identifiers.

GNU Make 4 or later and the existing POSIX toolchain environment are required.
The compiler must support dependency generation with `-MMD -MP`. System headers
and compiler binaries are part of the selected toolchain; changing their contents
in place requires a clean build. Non-system dependency files must be intact, and
input modification timestamps must reflect edits, as for ordinary Make builds.
Content edits deliberately preserving timestamps require a clean build too.

One Make invocation owns each output tree. Parallel targets within it are
supported; independently launched concurrent builds into the same output tree
are outside the contract. Tests create their own isolated output trees.
Recursive test leaves may revisit the unchanged compiler configuration after
their root build prerequisite has completed. These same-command checks must not
share temporary candidates. This does not authorize overlapping rebuilds with
different settings or concurrent object/link writers.

Generated dependency/configuration files live under `build/` for the project and
under each test leaf's `generated/`. Neither handwritten sources nor fixtures are
rewritten by these rules. Command manifests are implementation artifacts, not
public ABI or language syntax.

## Exact algorithms

### Source membership

`make/sources.mk` recursively collects all `.c` files under `src/` and sorts the
result. Object paths preserve each source's relative path. Every file is visited
in its containing directory; induction on directory depth proves completeness.
Sorting makes enumeration deterministic. Directory enumeration visits the finite
tree; sorting work is the sum of O(S_d log(S_d + 1)) over directories d, where S_d
is the number of sources in that directory's subtree. Recursion depth is the tree
height. No fixed depth, sampling or guessed membership is involved. Cyclic directory symlinks are
outside the ordinary-tree domain.

### Header and fixture dependencies

Each compiled translation unit emits its actual non-system dependency graph into
a `.d` file. Make includes these files on subsequent builds. Included parent
`common.c` fixtures are dependencies just like headers. Consequently a changed
file invalidates precisely the object targets that depend on it, in addition to
any explicit command/rule dependencies. Local test objects and production test
objects have distinct output prefixes, so equal basenames do not collide.

### Command and link manifests

Each output tree records two complete strings:

- Compile configuration: the exact compiler command prefix, including preprocessor,
  language, optimization, LTO and test-definition flags actually passed to it.
- Link configuration: the exact link command, output path and ordered object list.

At each build, GNU Make writes each candidate directly to a temporary file. This
avoids evaluating configuration text as an extra shell command while recording it.
The candidate name combines the target path and its live Make process ID. Make
reads that ID once from the directly invoked shell's `PPID`; shell wrappers must
`exec` the POSIX shell rather than insert another persistent parent process.
If that identity is unavailable, configuration processing fails explicitly.
The Make process remains alive until its recipes finish. Distinct live Make
processes have distinct PIDs, and distinct targets within one process have
distinct paths, so the candidate-name function is injective over overlapping
supported checks. An exited process may leave a failed-build candidate; PID
reuse truncates only that inactive candidate, not another live check's file.

Byte-for-byte comparison has three outcomes: unchanged content removes the
candidate; changed or absent content replaces the manifest; an I/O/comparison
error fails the build. Comparison errors are not treated as content differences.
Unchanged manifests retain their timestamps.

Objects depend on compile configuration; the binary depends on link configuration
and its current objects. Adding a source changes membership, compiles the new
object and relinks. Removing a source changes the manifest even when every remaining
object is unchanged, so the binary relinks without the removed object. Obsolete
objects may remain in the owned output directory but cannot enter the link unless
they are in its current explicit list. Cleanup removes them.

Let C be compile command text and L be the link command/object sequence. Equality
is exact byte equality, not a hash or a heuristic. Under Make's dependency/timestamp
contract, unchanged C, L and dependencies require zero compiler invocations.
Changed C invalidates all objects; changed L invalidates the binary. Linker-only
changes preserve objects. The same reasoning applies to both project and test
builds because they include the same configuration rules. Manifest processing is
linear in its text length and uses proportional temporary disk storage.

This does not promise minimal recompilation for semantically equivalent command
spellings; different command text is deliberately a different configuration.
Compilers, recipes and file operations must succeed to publish the corresponding
outputs. Failures propagate to Make; `.DELETE_ON_ERROR` removes failed targets.

## Executable evidence

Run `make -C tests/integration/rebuild` or its `tracking/` leaf. Each case copies
the real project/leaf rules into its own generated project, builds real C files,
and checks behavior. A small C wrapper logs each compile/link invocation and then
executes the selected real compiler. No shell test script, mock compiler result,
sleep interval or comparison of generated-object timestamps is used.

The harness expects `C_COMPILER` to name one executable, optionally by path. The
production rules still accept ordinary compiler command prefixes. Nested Make
calls receive explicit settings and their own parallel job pool. Wrapper trace
records use one checked append write per invocation; test failures retain logs.

Thirteen explicit `it_*` cases in `tracking/` cover:

- Nested source discovery, initial compilation and zero work on unchanged builds.
- Transitive-header edits, exact consumer counts and changed executable behavior.
- Adding deeply nested sources and removing sources without recompiling others.
- Changes to compiler command, compile flags, preprocessor flags and LTO settings.
- Linker-only changes with no object recompilation.
- Parent fixture and transitive-header edits in independent test leaves.
- Test definitions, test compile settings and production-source list changes.
- Unchanged test builds and successful execution after incremental changes.

The independent `concurrent/` leaf verifies candidate isolation. It copies the
actual shared configuration rules into a minimal project and first publishes
the current manifest. Two Make processes then write their same-command candidates.
A small C shell fixture reports readiness immediately before each comparison
recipe and blocks on a release pipe. The parent waits for both readiness markers,
releases/reaps the first check, then releases/reaps the second. Both must succeed.
The former shared `.tmp` implementation deterministically failed the second
comparison with status 2 after the first removed its candidate. Private candidates
preserve both checks without sleeps, assumed sibling ordering or random stress.
This fixture's three C inputs are covered by the leaf's real compilation/process
execution, including its included pipe controller and shell-forwarding helper.
Its second case replaces the old manifest location with a directory and requires
the real comparison error to propagate without replacing that directory. An I/O
error must not be mistaken for merely different command content.

The tests do not inspect pre-existing build artifacts or mutate committed inputs.
They validate this build contract, not Ama language acceptance or future runtime
semantics. Test output cleanup/failure preservation is covered separately by
[test-harness acceptance](test-harness.md).

## T003 revalidation

The controlled overlap failed against the original shared candidate and passed
after introducing live-process-private candidates. The real `rebuild/` parent
passed all 15 tracking/concurrency/error scenarios, and `make -j4 test` passed
all 58 registered leaves after the shared-rule fix. A final focused parent run
also passed after clarifying the test helper's small process/logging operations.
GNU Make 4.4.1 and GCC 15.2.0 were used. The new leaf additionally passed:

```sh
make -C tests/integration/rebuild/concurrent \
  C_COMPILER_FLAGS='-O2 -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG' \
  'TEST_DEFINES=-DTEST_ROOT=\"$(ROOT)\" -DNDEBUG'
```

Assertions remained enabled by the actual leaf flags. Doxygen 1.11.0 regenerated
the reference, and local links/anchors on all three added C file pages resolved.
Existing whole-project documentation debt remains assigned to T005/T134; this
does not claim all old comments or rendered specification links are complete.
