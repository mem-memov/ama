# Executable Ama examples

These are maintained source projects, compiled by the real CLI and executed by
independent C/assert integration leaves. They are informative examples, not an
alternative language specification. Start with the source, predict the effect,
then compare the complete trace in the test.

Build the compiler with `make`. For the examples below, run from the repository
root (the file example intentionally uses paths relative to its project leaf):

```sh
make -C tests/integration/examples
make -C tests/integration/objects
```

To compile and run one example manually:

```sh
cd tests/integration/examples
../../../ama --emit project /test/fixture/v1.0.0/src/simulations --standard-root ../../../stdlib/modules --out generated/manual --sdk ../../../src --compile
generated/manual/program
```

Replace `/test/fixture/v1.0.0/src/simulations` with `/test/fixture/v1.0.0/src/traversal`, `'/test/fixture/v1.0.0/src/self example'`, `/test/fixture/v1.0.0/src/files`, `/test/fixture/v1.0.0/src/empty`,
`'/test/fixture/v1.0.0/src/empty path'` or `'/test/fixture/v1.0.0/src/ordinary copy'`. Quote logical names containing spaces.
The standard root is explicit: `//text` and `//failure` refer to standard-library
identities, while `/test/fixture/v1.0.0/src/simulation` belongs to this example project. Outputs go under
`generated/`; the successful automated test cleans them. Handwritten native
companions stay under `project/` and are never overwritten.

## Persistent state and constructor-owned methods

Read the constructor example guide in `tests/integration/objects/README.md`,
then its [main module](../../tests/integration/objects/project/packages/test/fixture/v1.0.0/src/main),
[constructor](../../tests/integration/objects/project/packages/test/fixture/v1.0.0/src/factory) and
[method](../../tests/integration/objects/project/packages/test/fixture/v1.0.0/src/method). A method captures its
own seed copy and yields before each update. Its permanent state survives resume;
the two methods and two constructed objects evolve independently. The exact
[trace](../../tests/integration/objects/expected.txt) includes resource destruction,
so it checks lifetime as well as numeric results. This demonstrates
[CONT-03](../../spec/language/continuations.md#cont-03) and
[COPY-03](../../spec/language/values-and-references.md#copy-03): resuming and copying
have different, explicit effects on control state.

## Returning the original instance

The [self module](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/self) initializes its
reference result with standalone `&`. Its [caller](<../../tests/integration/examples/project/self example>)
executes the module, receives that reference, and executes through it. The trace
contains `description=self entry` twice. There is one owned instance; transferring
the reference creates no second one. [REF-01](../../spec/language/values-and-references.md#ref-01)
defines which instance `&` denotes. Calling the returned reference happens after
the first acquisition ends, so the example does not attempt busy reentry.

## A live, changing list

[Traversal](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/traversal) starts with
`first, second`. On the first iteration it appends `appended`, then removes the
current first element. Each iteration copies the selected text through `% item&`
into an owned value for the output module. It prints `first`, `second`, `appended`,
then ends with an empty list. A second traversal prints nothing.

[FLOW-05](../../spec/language/lists.md#flow-05) uses a boundary in the current
order. Appending beyond it creates another candidate; removing a past/current
entry corrects the boundary so its successor is not skipped. Unconditionally
appending on every iteration could prevent termination. The explicit `once`
condition is the algorithm's finite-growth argument, not a runtime traversal cap.
The selected reference must not be used after deleting its occurrence.

## Independent simulations from one saved position

[Simulation](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/simulation) yields before
each selected list element is reported and removed from its owned list. Its [driver](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/simulations)
starts one simulation, copies it with `% simulation`, then resumes both twice.
The exact output is `first, first, second, second`, each prefixed `description=`.
The copy owns its own list and saved traversal binding; it does not alias the
original mutable list. No scheduler is implicit: the driver names every resume.
[CONT-04](../../spec/language/continuations.md#cont-04) preserves saved positions
only for the explicit continuation copy. The
[ordinary-copy counterexample](<../../tests/integration/examples/project/ordinary copy>)
therefore fails on resume with runtime status 10. The
[empty simulation](<../../tests/integration/examples/project/empty simulation>)
completes at entry without yielding or executing the loop body.

## A file operation with an owned failure result

[Files](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/files) calls the synchronous
[open probe](../../tests/integration/examples/project/packages/test/fixture/v1.0.0/src/open). The example's native
implementation attempts `fopen` and closes a successful stream before returning;
it does not read the file or retain a descriptor. This is an example module,
not a claim that a full standard file library has been implemented.

The missing path returns a true `//failure` with `Cannot open file`. A positive
`?` handles it, and two `>>` reads return the same description without executing
or clearing the failure. A later successful call on an existing maintained file
returns false/empty, so the negative branch prints `opened`. There is no separate
success flag. The [empty-path case](<../../tests/integration/examples/project/empty path>)
returns `Invalid file path`. Embedded NUL is also rejected by the native path
contract rather than being silently truncated to a different C filename.

[ERROR-01](../../spec/implementation/diagnostics.md#error-01) distinguishes these
ordinary results from technical allocation failure and refusal before execution.
The native probe returns allocation errors through `AmaStatus`; its borrowed text
and temporary C string never escape the activation, and every successful open
has exactly one close attempt. Error descriptions are fixed, locale-independent
text so the demonstration's expected output is reproducible.

## Coverage and limits

`tests/integration/examples/main.c` explicitly registers normal and target
ASan/UBSan execution, retained failure reads, success after failure, returned
self-reference, live mutation, empty traversal and invalid resume after ordinary
copy. Its guarded `common.c` separates compiler diagnostics from program streams
and checks exit status and complete output. The Makefile exercises the same real
CLI, verified native toolchain and source project as the manual commands.

Every file under `tests/integration/examples/project/` belongs to the scenario
named by its entry; `simulation` and `report` are shared by its drivers. Native
headers additionally compile standalone through the per-file documentation gate.
The constructor leaf independently covers persistent native resources, captured
state, literal/body errors, complete cleanup and target instrumentation. These
examples complement the focused library/runtime tests; they do not claim exhaustive
file-system fault injection, parallel execution or asynchronous I/O support.
