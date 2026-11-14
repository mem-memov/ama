# Resumable generated programs

T074 accepts the scalar continuation slice through real source resolution,
generated C compilation and execution. It extends the independent
[continuation leaf](../../tests/unit/codegen/continuations/main.c), rather than
adding another executor or interpreting generated source in the test harness.

The governing contracts are [CONT-01](../../spec/language/continuations.md#cont-01),
[CONT-04](../../spec/language/continuations.md#cont-04) and
[ERROR-02](../../spec/implementation/diagnostics.md#error-02). Each instance owns
its saved statement; an invocation supplies a fresh caller connection. Copying
the former never copies the latter.

## Same-type nesting and explicit host driving

The new acceptance fixture declares two `.node` instances. The first calls the
second through a reference. The second's empty next reference refuses locally;
its empty refusal handler continues to its first probe and yield. The first then
executes its own probe and yields. Distinct probe counters and refusal snapshots
distinguish the two same-type slots without depending on their addresses.

The root makes an ordinary copy and an explicit continuation copy of the first
node, then yields to the C host. The host checks Yielded, explicitly resumes once,
checks Completed and checks that root identity has remained stable. That resumed
root advances the continuation copy, the original and the second node separately.
The ordinary copy refuses resume with NoResume; its local handler runs once.

The exact trace is:

```text
1:1:7
1:1:0
2:2:0
2:2:0
2:2:7
99:1:10
```

The first two lines occur before the host resumes. Equal subsequent copy/source
counters demonstrate independent advancement from the same saved native state.
The second node retains its own refusal history; the continuation copy does not
inherit the source's service history. No implicit scheduling or root resume loop
participates.

## Trace after copying

A second project enters a task through `.original`, copies the yielded task with
prefix `%`, and resumes the copy through `.resumer`. The resumed task fails in
its native worker. The diagnostic must include the task's actual failing call
and `.resumer`'s resume site, and must exclude `.original` entirely. This checks
the active connection directly in a failure trace, beyond successful value
comparisons. The existing non-copy trace case remains independently exercised.

## Evidence and limits

`tests/unit/codegen/continuations/support/acceptance.c` contains the two named
cases; the leaf's `main.c` explicitly dispatches them. Its Makefile compiles and
runs the generated programs with sanitizers when sanitizer flags are supplied.
Both new cases cover the support file, dispatch and build entry in the per-file
coverage registry. Existing cases additionally cover restart, branches, loops,
retained outputs, explicit disposal and terminal failure.

```sh
make -C tests/unit/codegen/continuations
make docs-complete
make docs-reproducible
```

The test uses the internal generated host scaffold. It does not claim the checked
foreign C ABI, asynchronous scheduling, list cursor copies or cross-context
continuation transfer. Those remain separate roadmap contracts.
