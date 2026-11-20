# Pausing, resuming, and copying a computation

[Specification index](../README.md)

A continuation in Ama is an ordinary instance with a saved place at which to
continue. It has the same owned state, interface, and identity as before it
yielded. The three uses of `%` answer different questions:

| Form | Effect |
|---|---|
| Standalone `%` in a body | Save the next position and return results to the immediate caller |
| `task %` | Resume that same instance at its saved position |
| `% task` as a value source | Make an independent owned copy that also preserves saved positions |

**Example.** The following complete module body, stored as `/example/demo/v1.0.0/src/two steps`, yields
one value and then completes with another. It assumes the standard `//integer`
literal and copy contracts:

```text
@>
value
//integer
@!
first
//integer
 |1
second
//integer
 |2

value
 first
%
value
 second
```

A caller can consume those two stages:

```text
@!
sequence
/example/demo/v1.0.0/src/two steps

sequence !
 > first value
sequence %
 > second value
```

`first value` receives `1`; `second value` receives `2`. The second call finishes
the module body and clears its saved position. A third `sequence %` would refuse
to begin because there is no continuation to resume. Calling `sequence !` starts
the same instance at entry again.

**Why.** An explicit saved position makes suspension independent of the C stack
and makes restart distinct from resume. It also permits independently owned
copies of suspended computations when all included resources can be cloned.
This mechanism supports the cooperative task protocol in
[Asynchrony and parallelism](async-and-parallelism.md). A yield by itself neither
registers an event nor schedules the instance for later execution.

## Yielding and resuming the same instance

### Return to the immediate caller

<a id="cont-01"></a>

**CONT-01.** `%` alone in an Ama body records the position immediately after
itself in the instance, transfers requested results, and returns to the caller
without relocating its data. The instance becomes available in the yielded state, and its old caller
connection is released. Only this invocation yields: its immediate caller
continues after the call. Yield never implicitly suspends the whole chain of
callers or turns a nested call into an asynchronous wait.

**Example.** Assume `prepare`, `process`, and `finish` are declared child instances:

```text
prepare !
%
process !
%
finish !
```

The first entry-start call reaches the first yield; successive resume calls
continue with `process !` and then `finish !`. A fresh `!` always starts at entry,
preserves permanent data, reinitializes the interface under STATE-02/CALL-07, and discards the old position.

### Store everything needed to resume explicitly

<a id="cont-02"></a>

**CONT-02.** A saved position includes all control information required to resume
inside branches and loops. It cannot depend on a live C activation, automatic
local address, or return address on the C stack. Completion clears the position.
Resumption without a position must not silently execute from entry.

#### What a suspended loop keeps

For foreach, this includes each active cursor and active local reference cell
from FLOW-05. Store them explicitly in generated instance/control storage, not
in a C local, native opaque payload, array-buffer address, or retained lock.
Yield preserves these cells; completion, early return, entry restart, and scope
exit discard the appropriate cells/cursors without destroying their targets.

### Resume preserves the interface and accepts no new inputs

<a id="cont-03"></a>

**CONT-03.** Resumption accepts no input bindings: `<` is forbidden under `%`.
It may request ordinary results through `>` or use CALL-09 selected-result actions,
or have no arguments. It preserves the suspended interface and permanent state
without an entry reset. Reject all input bindings before acquiring or modifying
the target. This is the language contract,
not a temporary limitation awaiting an expanded resume syntax. `>>` remains
exclusive to nonexecuting retained-result reads.

## Making an independent continuation

**Explanation.** Receiving a result or assigning an ordinary owned value copies its data and clears saved positions. Prefix `%` is an explicit request to keep those positions in the new owned value as well.

### Copy data, owned descendants, and saved control state

<a id="cont-04"></a>

**CONT-04.** Prefix `%` on a copy source explicitly clones data and its saved
position into a distinct owned instance of the same type:

**Example.** Assume `original` is a `/example/demo/v1.0.0/src/jobs/task` instance that has already
yielded when the assignment below executes. Its declared result `output` is
compatible with `destination`:

```text
continuation
/example/demo/v1.0.0/src/jobs/task

continuation
 % original

continuation %
 output > destination
```

The declaration creates default storage. The executable assignment then clones
the yielded `original`, and the call resumes the independent copy. Resuming it
does not advance `original` or give either instance the other's caller link.

The copy does not execute. It clones owned descendants and collections according
to lifecycle contracts, redirects internal references under COPY-04 through
COPY-05, and preserves external reference targets. It does not
inherit slot identity, generation, busy flags, outstanding caller links, or
return selectors. Replacing an existing destination preserves that destination's
identity. Saved execution positions are copied for the complete ownership tree:
the root and each owned descendant retain their respective saved positions,
including descendants in owned lists. An included source with no saved position
produces a corresponding destination with no saved position. A root without a
position may therefore own resumable children, but cannot itself be resumed.

#### Copying a live foreach boundary and its binding

Before preparing a continuation copy, validate every active foreach cursor's
source collection identity/incarnation. Legitimate membership changes do not
invalidate copying. Build an exact map from each live source membership to its
new destination membership while cloning list contents; translate each cursor's
current-membership identity and every origin association through that map, keyed
by the full source owner, field, collection incarnation, and membership identity.

#### Removed memberships remain removed

Origins retained from an already-exited inner loop are included even when no
active cursor names that collection. An origin from an obsolete incarnation or
removed membership stays a permanently dead association; never rebind it to the
current field contents or a reused record slot. Preserve each cursor's numeric
boundary `q`, already corrected by all committed source mutations, its active
state, and the saved body position. Preserve the current list order and retarget
cursor collection identities to the copied fields' final incarnations, with
`0 <= q <= copied length`. Construct copied lists before activating their copied
cursors; constructing them must not apply insertion corrections to a cursor
whose final boundary has already been copied. No historical membership array or
visit history exists to clone. Copy active local handles with COPY-04's reference
mapping; membership state and these cells are non-owning control state, not extra
objects in the copy boundary. A vanished current membership remains vanished, so
resumption skips that body's remainder and selects from the preserved boundary
under FLOW-05.

#### Source and copy evolve independently

Future mutations of source and copy adjust only their respective
cursors. Do not clone busy flags, acquired tokens, or pending return links.
Ordinary copying discards this state under COPY-03; escaped ordinary handles in
persistent fields still copy normally.

Continuation copying stays within one execution context. It is not a message
format, cross-context transfer, or migration of a suspended native operation.
Parallel contexts exchange explicit serializable data under
[Asynchrony and parallelism](async-and-parallelism.md).


### Availability and resource-clone requirements

<a id="cont-05"></a>

**CONT-05.** A continuation-copy source and all its owned descendants must be
consistently available under COPY-07. Any running or waiting instance in that
tree causes refusal, even if the root itself is available. External references
do not extend this boundary. Active invocations and their return links cannot
be silently duplicated.
Unsupported resource cloning must fail without partially replacing the destination.
Failures of standalone copy/declaration operations use the lifecycle error
protocol, not an invented assignment form of `:`.

## Native boundaries, failure, and cancellation

### Yield only at an Ama statement boundary

<a id="cont-06"></a>

**CONT-06.** Yield is permitted only at Ama statement boundaries. Native C code
cannot yield from inside its function body. A waiting caller remains busy;
a yielded instance has no outstanding invocation and is available. These are
different runtime states.

Application code may use owned lists or references to build explicit queues,
work stacks, or collections of continuations. The runtime must not silently
create a separate heap activation for every call to make busy reentry succeed.

A failed execution is not a saved continuation and cannot be resumed. Cooperative
cancellation follows the explicit task/resource protocol in
[Asynchrony and parallelism](async-and-parallelism.md); it does not inject an
exception into a suspended body or execute that body's remaining statements.
