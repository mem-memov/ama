# Resume the next statement, return to the current caller

T070 accepts statement-boundary suspension under
[CONT-01](../../spec/language/continuations.md#cont-01),
[CONT-02](../../spec/language/continuations.md#cont-02),
[CONT-03](../../spec/language/continuations.md#cont-03),
[RT-09](../../spec/implementation/runtime.md#rt-09) and
[STACK-01](../../spec/implementation/compiler.md#stack-01).
The existing saved-position, return-link and generated-label machinery supplies
the implementation. This checkpoint composes those boundaries in focused
compiled programs rather than adding another coroutine executor.

## Two records with different lifetimes

Before `%`, generated control stores the next resolved statement selector in the
current instance's saved-position record. Yield then uses the current return
connection to reach the immediate caller. The connection ends with that return;
the saved position remains available for a future invocation.

The target stays acquired while that caller processes its outputs. Releasing the
call restores Yielded availability. A later `%` acquires that available instance,
creates a new caller connection, consumes its saved selector and dispatches to
the exact label. It does not reset parameters or infer an entry from body fields.
Normal completion clears the saved position. An additional resume then refuses
NoResume instead of replaying the last segment.

The runtime call component independently checks Running/Waiting states, matching
caller/target identities, detached old connections, retained saved selectors and
the availability restored after caller-side output processing. Generated resume
dispatch separately validates selector membership and the current nominal type.

## A waiting caller is still busy

One compiled task receives an explicit reference to its outer caller. In each of
three segments it tries to invoke that caller. The caller is Waiting while the
task executes, so acquisition refuses Busy before resetting or executing it.
The task handles that refusal locally. Its first two segments yield; the final
segment completes normally.

The observable sequence is:

| Executing module | Effect |
|---|---|
| Task, first segment | Handle Busy, print 1, yield. |
| Outer caller | Print 10, resume the same task. |
| Task, second segment | Handle Busy, print 2, yield. |
| Outer caller | Print 20, resume the same task. |
| Task, final segment | Handle Busy, print 3, complete. |
| Outer caller | Print 30, complete. |
| Root caller | Print 40. |

Native counters distinguish the independent task/outer/root instances. Refusal
snapshots also identify their owners: only the task reports Busy. Successful
resumes prove that its previous yielded activation released correctly; outer
effects before root continuation prove yield returned to the immediate caller.

## The branch was already chosen

Another task enters a positive branch, prints 1 and yields before its next body
statement. It explicitly returns a reference to the native module implementing
its condition. While the task is Yielded, its caller uses that declared reference
to set the condition false through an ordinary typed call.

Resumption prints 2 from the already chosen branch, then 3 after the branch.
It neither rechecks the now-false predicate nor enters its alternative. The saved
statement label already encodes the needed structured position; no C automatic
branch value must survive suspension. The native setter retains no borrowed Ama
pointer and performs no nested Ama execution.

The [condition-loop acceptance](condition-loop-execution.md) separately shows
body resumption before the next header test. The
[constructor project](synchronous-slice.md) supplies retained methods that capture
owned parameters once and perform repeated actions through explicit resumes.

## Evidence and limits

`make -C tests/unit/codegen/positions` runs the waiting-caller, completed-position
and externally changed branch cases through the actual C compiler and runtime.
`make -C tests/unit/runtime/calls` independently checks return/control metadata;
the existing `codegen/continuations` suite checks repeated yields and loop-body
positions. Sanitizer flags instrument generated programs as well as their runner.

No active caller connection, acquisition grant or C activation is saved as a
resume position. [Foreach restoration/copying](list-traversal-execution.md)
and [cross-context identity checks](execution-contexts.md) have separate
acceptance boundaries and do not enlarge the saved transient state.
