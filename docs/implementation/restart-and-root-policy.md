# Restart a module, choose root yield policy explicitly

T071 accepts restart/resume and root-host behavior under
[CONT-01](../../spec/language/continuations.md#cont-01),
[CONT-03](../../spec/language/continuations.md#cont-03),
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[RT-14](../../spec/implementation/runtime.md#rt-14),
[RT-15](../../spec/implementation/runtime.md#rt-15) and
[RT-16](../../spec/implementation/runtime.md#rt-16).
The existing interface preparation, generated context owner and standalone
launcher supply the implementation. Focused compiled cases distinguish their
effects without adding an implicit restart, scheduler or recovery path.

## Entry prepares an interface; resume continues it

One task has an owned parameter defaulting to seven and a persistent body probe.
It prints the parameter, yields, then prints it again and completes.

| Invocation | Printed value and persistent probe count |
|---|---|
| Entry with 10 | 10, first probe call; yield. |
| Resume | 10, second probe call; complete. |
| Entry with 20 | 20, third probe call; yield. |
| Entry using the default | 7, fourth probe call; yield. |
| Resume | 7, fifth probe call; complete. |

The fourth invocation starts the body again and discards the old position.
Preparing the default changes its interface parameter, while the owned body
probe retains its native state. Resumes preserve the installed parameter and
start after the saved yield. A subsequent handled resume refuses NoResume.
The combined value/count trace distinguishes all three domains: interface,
persistent body state and saved control position.

## Admission and accepted execution have different effects

The generated context layer checks admission before executing Ama. A request to
resume an unstarted host creates no root, context identity or body effect. A
request to resume a completed root also performs no body work and leaves the
healthy owner, root identity and status intact. A later explicit entry remains
valid and preserves its body state.

A compiled custom launcher separately asserts those nonmutating rejection
postconditions and the outcomes of accepted execution. Its first entry yields,
explicit resume completes, and a later entry yields again at the same root and
context identities. Disposal consumes the yielded context without resuming its
body. A later execution request rejects; repeated disposal performs no new work.

Another accepted entry fails in its native body. It reports the original failure,
returns a failed outcome and disposes the context. Later entry and resume requests
cannot restart that failed computation, and repeated disposal retains the primary
status without retrying its native activation.

These tests use internal generated host declarations and context postconditions.
They certify root execution policy; the checked foreign-component token/response
ABI remains the separate compiled-library acceptance boundary.

## Standard launch enters once

The normal standalone launcher has no configured root-yield handler. A compiled
root performs one native action, yields, and has a second action afterward. The
launcher reports `Unhandled root yield`, destroys the yielded owned state and
exits unsuccessfully. Only the first action appears. One destructor effect
observes its unchanged native counter and proves cleanup did not execute the
continuation or retry the destructor.

A custom host may instead process a yielded outcome and explicitly request
resume, as in the preceding case. The language's module yield always returns to
its immediate caller; the choice of how to handle a root outcome belongs to that
host. There is no hidden resume loop in the default executable.

## Evidence

`make -C tests/unit/codegen/restart` runs four compiled cases covering the
entry/default trace, healthy host lifecycle, terminal admitted failure and default
launcher rejection. Target sanitizer flags instrument generated runtime/native
code. The independent `runtime/host-lifecycle` suite checks metadata and terminal
cleanup boundaries; earlier generated-context tests cover independent owners,
same-host admission gates and initialization/cleanup failure provenance.

The compiled cases preserve the original native-body diagnostic and never resume
a failed execution. They add no public foreign ABI or claim of pending scheduler,
foreach or compiled-component export support.
