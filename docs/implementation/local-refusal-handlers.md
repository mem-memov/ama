# A refusal belongs to the link that could not begin

T067 accepts compiled handler behavior under
[CALL-04](../../spec/language/calls-and-results.md#call-04),
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[CALL-09](../../spec/language/calls-and-results.md#call-09) and
[ERROR-06](../../spec/implementation/diagnostics.md#error-06).
The implementation is the existing typed refusal graph plus T066's
[bounded caller frames](direct-result-execution.md). These cases add composed
acceptance, not another exception mechanism or execution path.

## Release first, then enter the local handler

Assume `object` returns `middle`, which returns `branch`, which returns an owned
`live` method and an empty reference `missing&`. Within the initiating caller:

```text
object
 middle
  branch
   live !
    < |10
    > result
   missing& !
   :
    probe !
     < result
   live !
    < |999
probe !
 < result
```

The first method and its output transfer finish normally. The empty selected
reference refuses before a child frame activates. Abandonment walks the failed
link's exact planned ancestors, releasing `branch`, `middle` and `object` before
the handler calls `probe`. A normal handler completion reaches the statement
after the entire outer `object` operation. The pending call with 999 never runs.
The completed method effect and transferred `result` remain committed.

The compiled test extends this example with another `object` tree inside the
handler. Its successful acquisition proves every ancestor was released first.
The old tree also has pending siblings at multiple levels; none execute after
the handler. Method counters prove earlier mutations remain in actual storage.

The branch deliberately declares its own private `result` and `probe` names.
They cannot capture handler operands: those already resolve to the initiating
caller's identities in semantic IR. The test observes the caller's transferred
value, not the branch's unrelated default. The initiating caller's last-refusal
snapshot persists through successful handler calls and later operations.

## Syntactic ancestors and dynamic callers are different

An attached handler belongs to one operation link. If a descendant refuses without
its own handler, alternatives attached to its ancestors do not run. An empty
attached handler does handle refusal and skips the remaining tree. Both behaviors
are tested as compiled programs, including an unhandled descendant with three
different ancestor handlers whose output markers must remain absent.

A call written inside a selected method's ordinary body starts another initiating
operation. A refusal handled there releases only that inner operation, preserving
the outer caller's held method, ancestor grants and pending outputs. The compiled
method starts its own deep result tree, handles its empty-reference refusal and
returns its ordinary result. The enclosing caller then transfers that result,
runs its next selected sibling and continues normally. Its own refusal snapshot stays unchanged.

## Suspension does not resurrect a discarded tree

A handler may yield after abandonment. The selected links' transient frames are
already inactive, while the handler's ordinary module saves its actual statement
position under [CONT-01](../../spec/language/continuations.md#cont-01).

One compiled controller yields between two handler statements. Its unrelated
dynamic caller continues, then resumes the controller. Execution continues with
the next handler statement, the statement after the outer tree, and a new tree.
Skipped selected siblings remain skipped. Native counters and refusal snapshots
identify which module is executing at each step and distinguish resumption from
restarting a handler or restoring an abandoned acquisition.

## Technical failure keeps its fatal route

An accepted method's native body failure is not acquisition refusal. A compiled
case attaches alternatives to the method and every ancestor, then makes its body
fail. Only the body-entry marker appears; no handler runs, and the runtime reports
the original failure. T066 separately checks technical selected-index failure.

`make -C tests/unit/codegen/handlers` runs all six cases; the parent exposes the
independent `handlers` target. Sanitizer flags instrument generated runtime/native
programs as well as their assertion runner. T068 separately accepts
[loop/header exits](condition-loop-execution.md); this chapter does not claim pending
foreach, intrinsic-list or compiled-component ABI conformance.
