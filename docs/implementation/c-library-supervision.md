# Independent suspension and child failure supervision

This example implements
[EMBED-08](../../spec/implementation/c-library.md#embed-08)'s exact chain:
an outer Ama module calls its owned native adapter, which calls a separately
compiled child component. The child contains real Ama code and native helpers.
The parent and child retain independent execution contexts and saved positions.

Run the composition with:

```sh
make -C tests/unit/library/supervision
```

## Yield ends a call; it does not retain the C stack

The [child body](../../tests/unit/library/supervision/child/project/main) performs
three explicit worker steps separated by two `%` statements. Its native worker
owns an address-sensitive resource and a counter, so recreation, shallow
copying or relocation would change observable behavior or fail its address checks.

The [outer body](../../tests/unit/library/supervision/project/main) makes its first
adapter call, examines the declared `waiting` condition, and explicitly yields
inside a loop while another child step is needed. A resumed outer body calls
the adapter again. The adapter separately selects child resume from its stored
child outcome and supplies no new inputs on that path.

The order is deliberate:

1. Child Ama reaches `%` and returns through its C host API.
2. The adapter's ordinary C helper returns.
3. The native entry finishes copying/releasing results and returns.
4. The outer Ama code decides whether to execute its own `%`.

There is no transitive await, implicit scheduler enrollment, spin-resume loop
or stored C activation. The outer loop passes a false `reset` on later steps,
so a retained true reset input cannot recreate the child on every outer resume.
Ordinary entry/reset semantics remain separate from continuation resume.

The test uses explicitly uninlined parent and child C helpers. A versioned,
scalar-only [C observer service](../../tests/unit/library/supervision/probe.h)
records their enter/leave intervals and native resource ownership. At every
returned parent call both depths are zero. The child callback also tries reads,
resumes and disposal in both active contexts; all are rejected before execution,
including access through an idle sibling instance. The observer stores no Ama
reference or borrowed value, and no callback successfully executes Ama.
It is an explicit foreign C link dependency for instrumentation, not mutable
state shared between the Ama programs.

Retained parent reads and stepping another parent instance do not resume the
first child's computation. Parent disposal abandons any remaining saved positions
without executing another worker step. Child snapshots are copied/released before
the parent returns; retained parent snapshots survive both contexts' destruction.

## Two explicit failure policies

The adapter's declared `propagate` condition is an interface parameter, not
language exception syntax. Its default is true. It selects behavior only for
an accepted child call whose host outcome is FAILED:

- **True:** return a new parent technical status with immutable parent-owned
  message bytes. The parent fails under its ordinary runtime rules and disposes
  every owned adapter/child, including an unexecuted sibling resource.
- **False:** copy the child's diagnostic bytes into the parent's declared
  `..failure` result, release the child diagnostic owner, and dispose the failed
  child context. The parent remains healthy and returns an ordinary coherent
  failure value with its condition and description together.

Child API admission rejection is a boundary-contract error and remains a parent
technical failure. An allocation/encoding failure while constructing the report
is likewise a new parent technical failure; it cannot manufacture application
success or be caught as a child failure. Every measured parent allocation point
in input preparation, report copying and output snapshot creation is exercised.

The [worker fixture](../../tests/unit/library/supervision/child/project/worker.c)
deliberately fails for the requests `fault` and `fault-cleanup`; the latter also
reports a secondary destructor failure after releasing its resource. The parent
copies the primary execution report before child cleanup. It never selects a
policy from translated diagnostic text and never forwards a child-owned pointer
as its own program-lifetime `AmaStatus` message.

A failed context's admitted disposal can return its saved primary failure again.
The consumed context control proves retirement; the numeric nonzero primary
status alone does not imply an unreleased context. All returned diagnostic owners
are released even on this path. Tests require zero live child resources and code
dependencies after retirement, and preserve the primary description despite
the deliberately reported secondary cleanup failure.

## Terminal state and explicit reset

After supervised failure the payload retains only an empty foreign holder with
a terminal outcome. Its child context/instance controls are empty. A later step
without reset performs no child call and reports that explicit reset is required.
Changing the policy to propagation can deliberately fail the parent; it still
cannot restart the child implicitly.

A true `reset` closes any available old child and calls the same checked fresh
construction operation used by the native initializer. It creates a new context
and roots rather than trying to continue a failed one. Reset/construction errors
remain parent technical failures. The shared resource lifecycle implementation
is reused by the simpler [native-parent example](c-library-nested.md); neither
adapter declares native `=` or hides parent references in opaque storage.

Successful steps clear both the declared failure bit and its description. Tests
follow a failure with explicit reset and several normal yields/completion,
checking that no stale description survives. A retained failure snapshot remains
readable after child disposal, reset, successful execution and parent disposal.

## Acceptance boundaries

The C consumer is compiled separately with only generated public headers and
the scalar observer header; no compiler SDK include path is supplied. Child
code is a verified isolated archive/shared component. Parent Ama/native code,
child archive and the explicit observer object are linked as separate inputs.

The suite checks independent parent/child continuation progress, repeated fresh
entry, retained state and values, no helper activation after yield, rejected
active-ancestor/current-context reentry, child abandonment, both supervision
policies, terminal/reset behavior, collateral cleanup of another owned child,
diagnostic ownership, secondary failure handling and all measured parent report
allocation failures. ASan/UBSan cover the generated child, parent and host.

These nested fixtures exercise a serialized native observer contract; they do not
claim concurrent use of those observer globals. [Parallel acceptance](parallel-supervision.md)
records the readiness/executor checks, the recursive child-resource concurrency
and affinity audit, and the separately synchronized two-actor example.
Reproducible user-facing build recipes and the complete EMBED rule matrix are
collected by T131.
