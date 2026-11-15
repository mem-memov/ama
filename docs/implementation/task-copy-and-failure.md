# Task copies, external operations and failed contexts

[ASYNC-10](../../spec/language/async-and-parallelism.md#async-10) separates copying
a value from scheduling it. A task snapshot can contain saved positions, ordinary
identifiers and references to external objects. None of those fields creates a
new native operation or inserts the copy into a queue.

This component also implements the context-wide boundary in
[ASYNC-11](../../spec/language/async-and-parallelism.md#async-11): a technical error
stops the context, cleans its owned waits, and reaches the host as Failed. It is
not a cancellation result that permits the remaining tasks to run.

## Preserve explicit snapshots through parameter installation

A regression exposed by fresh enrollment was in the common scalar store: the
caller selected `% source` correctly, but parameter replacement constructed an
ordinary clone and cleared its saved position. The corrected path keeps the
explicit mode through all four stages:

1. Source selection records the continuation-copy flag. For `% reference&`, the
   selected value is an owned snapshot source of the referenced target, rather
   than a request to copy a reference handle. Ordinary reference sources retain
   their ordinary reference form. Explicit reference acquisition remains a separate
   mode with a reference-shaped result; snapshot normalization does not override it.
2. Source protection validates the complete owned snapshot and its continuation
   controls before any parameter replacement. Repeated ordinary/snapshot uses of
   one source still validate the stronger snapshot mode under the same temporary
   source authority.
3. The scalar clone recipe carries the selected flag into staged replacement.
   Ordinary defaults and ordinary result transfers continue to clear positions.
4. Existing clone mapping preserves permitted positions and remaps internal
   references, without copying an active return connection or refusal history.

The flag is explicit input data, never inferred from a source's current activity.
For a valid source snapshot, each explicit snapshot copy preserves its saved
selector; an ordinary copy clears it. Repeating explicit copies preserves this
property through a parameter and then into the scheduler-owned task. Availability,
copy capability and continuation validation still precede construction effects.

The implementation is in `src/runtime/program/values/source.c`,
`calls/preparation/protection.c` and `transfers/value.c` under the same program
component. The independently runnable
`make -C tests/unit/runtime/call-preparation` covers ordinary versus explicit
parameter modes, reference targets, unchanged source positions, ordinary default
clearing and invalid controls before the first destination field changes.

## A copyable task and a noncopyable enrollment owner

`..task.enrollment` is an ownership-only native marker. Its declared Ama field
identity belongs to one record; its tiny resource-free payload contains no task
reference, callback pointer, hidden registry or epoch. It deliberately has no `=`
capability. Calling its entry neither schedules a task nor advances an epoch.
All actual scheduling relationships remain ordinary declared Ama fields.

The maintained [record](../../tests/unit/codegen/task-boundaries/project/kind/record)
owns this marker and a task, while the task itself remains copyable. Consequently:

- A task can be snapshotted and can retain an explicitly external cancellation or
  operation reference under COPY-04.
- A record cannot be copied, nor can a scheduler owning its record list, even
  when the list is empty. A fresh default record and an explicit constructor call
  are needed to adopt a copied task and publish a new epoch/membership.
- Queue entries and other data remain ordinary values. Copying raw queue data is
  not an additional documented dispatch port or a new enrollment. The scheduler
  owns its queue and only its protocol may dispatch tasks under ASYNC-04.

This is a conservative library boundary implemented through existing copy
capabilities, not a hidden runtime hook on every `!`, `%` or clone. For the owned
capability graph, a type is copyable exactly when it cannot reach a native type
without `=` through owned edges. Reference edges do not propagate the restriction;
empty owning-list declarations still do. The same fixed-point check rejects
copying `..task.operation`, which owns an actual in-flight registration and does
not offer a fictitious endpoint clone.

The sample adoption constructor accepts a yielded continuation through an explicit
`% source` input, copies it into its independently owned task, reserves a fresh
run epoch and publishes ordinary readiness. Enqueue is still explicit. Adoption
is for valid saved positions; the resume step treats an absent position as a
technical protocol error instead of silently restarting at entry. A caller may
start an ordinary unenrolled value with `!` before taking a resumable snapshot.

The task worker demonstrates independent native counter state and stable addresses
in the copied ownership tree. Actual callback-owning operations remain noncopyable;
this component advertises no independent asynchronous-operation clone. The general
resourceful clone tests (`make -C tests/unit/runtime/objects/clone`) separately
exercise fresh native allocations for types that implement cloning. A type owning
callbacks must satisfy that stronger independent-resource contract rather than
copy endpoint bits.

## Shared external references do not create subscribers

The copyable task has ordinary reference fields for an external cancellation
port and an external `..task.operation`. Both remain external after copying.
Changing the referenced cancellation port therefore affects both independent
continuations; it does not place either continuation in a queue.

The native event service is **single-consumer**: taking a completion removes it
from its retained FIFO. Copying an issued token or sharing an operation reference
does not reserve another operation, duplicate that packet or create a live wait.
A second consumer sees an empty queue. A new wait needs its own registration,
including a new full native token; the old token cannot complete its reused cell.
The wait validator and membership guards from the preceding components remain
required for scheduler delivery. This component does not add broadcast semantics
or allow two copied references to claim independent callback rights.

## Failure cleans before the host observes Failed

The technical-failure composition starts two tasks that yield Awaiting with
independently owned native operations. A test native provider captures separate
callback endpoints and counted binary bytes. Those host-owned native records
contain no Ama view, reference or payload address.

A different native entry then fails technically. Before `ama_project_execute`
returns Failed, context cleanup destroys both suspended ownership trees and
closes their source. The host attempts another resume, which must fail without
entering Ama again. Delayed native publication then returns Closed for both
captured endpoints and preserves their byte ownership. The host acknowledges all
bytes and endpoints afterward. No task code after `%` and no subsequent root
statement executes during that cleanup.

The host probe runs before explicit host disposal, so it cannot accidentally
mistake disposal cleanup for the required failure-boundary cleanup. A capture
counter also distinguishes a rejected resume from an accidental second execution
that happens to fail again. This standalone fixture keeps its C image loaded;
loadable-provider code pinning is covered by the subsequent asynchronous host
acceptance task.

## Executable evidence

The per-file map is in [ready.json](documentation/ready.json).
`make -C tests/unit/codegen/task-boundaries` compiles real native interfaces and
executes inert copies followed by explicit adoption, independent owned worker
state, shared cancellation ports, shared operation references, one-consumer token
semantics and context-wide failure cleanup. Its static cases reject both ordinary
and continuation copies of enrollment markers, records, empty owning scheduler
lists and native operations with the precise missing-clone cause.

The shared execution harness also reports captured diagnostics for unexpected
ordinary exit statuses, rather than hiding them behind a status assertion. Its
normal expected-failure behavior remains unchanged. Component and generated-code
sanitizer runs cover the actual source/installation/cleanup handoffs.
