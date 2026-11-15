# Native submission and retained completion storage

This component implements the native resource boundary of
[ASYNC-05](../../spec/language/async-and-parallelism.md#async-05).
It supplies storage and submission ordering needed by
[ASYNC-06](../../spec/language/async-and-parallelism.md#async-06).
Publishing the corresponding typed Ama registration and importing a completion
through an ordinary generated access boundary are the next task, T093.

## Two state domains remain separate

The native [interface](../../src/native/events/event.h) contains native service
owners, immutable tokens, callback leases, immutable byte owners and native result
codes. It imports byte/status declarations and no Ama pool, instance, reference,
continuation or execution API. Its callbacks have no executor function to invoke.

A service is an independently owned C resource with positive fixed capacity.
Only its owner performs prepare/arm/discard/take/dispose operations. Separate C
workers may post, query native completion state, copy native bytes and release
independently retained callback rights concurrently.

The [typed enrollment graph](task-enrollment.md) continues to own task references,
run epochs, registration generations and ready/waiting relationships in Ama.
A native token never encodes any of those Ama identities. Native job records and
keepalive resources contain native-owned state only. Submit routines must copy or
retain their native inputs before keeping them; they never retain the address of
a borrowed submission inside an Ama native payload or another borrowed view.
Retained callbacks never execute Ama, including an otherwise idle foreign context.

## Prepare, publish, arm, post and consume

The C boundary follows this explicit sequence:

1. `ama_native_event_service_prepare` reserves a native cell and a final immutable
   token, returning an owning callback lease. It performs no foreign operation.
2. The executor publishes that token together with its typed task reference and
   epoch/generation in the Ama registration. T093 provides that bridge.
3. `ama_native_event_service_arm` authenticates the reservation and marks it Armed
   under the native metadata gate. It then invokes the borrowed C submission
   routine **outside** the gate.
4. The submission routine may complete synchronously or clone its borrowed
   `AmaNativeEventSubmission` into a separately owned native worker record.
   Completion posts its original token and optionally moves an owned byte buffer.
5. The executor rechecks native readiness, reconciles its ordinary phase and
   later consumes retained records with `ama_native_event_service_take`.

An immediate callback can lock the native queue because arm holds no gate while
calling foreign C. A callback only publishes native data; it does not resume a
task, update an Ama field or consume another event. Each completion remains queued
until the owner consumes it or explicitly closes the service.

The native submit callback returns `void`. Backend submission errors are posted
as operation-specific native completion data, including an empty payload. The
reserved cell makes that possible even when producing a byte payload fails to
allocate. Queue publication itself allocates nothing.

Preparing an unarmed reservation and abandoning it uses `service_discard`.
Discard accepts Prepared only. Per-operation detachment of already armed work,
cancellation, reactor blocking and final native acknowledgment protocols are
subsequent components; closing the entire native service is implemented here.

## Authenticate the token, not an address

A token contains four fields:

| Field | Purpose |
| --- | --- |
| `domain` | Unique unrecycled native receipt for this physical code image |
| `service` | Nonwrapping service serial inside that image |
| `generation` | Nonwrapping submission serial inside that service |
| `slot` | Fixed native-cell locator, authenticated by the other fields |

Zero service/submission serials are unissued. A callback carries the token captured
when preparing its submission. It never reads a current counter or reused descriptor
to construct a replacement token. Full service identity and generation must match
the live located cell before any completion is accepted.

The image receipt is one heap byte, plus allocator overhead, intentionally retained
for process-lifetime identity history. It contains no data or release-function
dependency on unloaded code. The winning receipt is never recycled; losing
concurrent initialization candidates are freed. Service serials are issued under
a separate finite metadata gate. A physical reload creates a fresh receipt, so a
reset service serial cannot revive tokens from the previous image. This follows
from the disjoint storage of simultaneously live allocations, rather than from
a guessed load address or probabilistic identifier.

Each service reserves submission generations with the exact precondition
`current < UINT64_MAX`, computing the successor before publishing it. Exhaustion
changes no cell, callback owner or counter. Reusing the exact native cell is safe:
its next submission has a different authenticated generation. No per-submission
generation history is retained after recycling; only the monotone service counter
and the current cell metadata are needed.

## Constant-time queue operations and explicit ownership

The service allocates its cell array once, before publication. A free-index stack
provides constant-time reservation. Completed cells form an intrusive FIFO with
indices and backward links. Its order is the order of successful publication under
the native gate, independently of reservation order or physical completion time.

Each cell is exactly one of Free, Prepared, Armed or Completed. The free stack
contains exactly Free cells; the FIFO contains exactly Completed cells. Prepare
moves a cell from the free stack to Prepared, arm changes it to Armed, and one
successful post moves it to Completed at the queue tail. Duplicate posts leave
it unchanged. Take removes the queue head, moves its result to the consumer and
recycles the cell. An old callback lease can still exist after that locator is
reused; its earlier generation is rejected.

The publication/ready/prepare/take metadata operations are constant time. Byte
copying is linear in byte count; creation and close are linear in configured
capacity. Native allocation and byte release, submit functions and resource-retire
hooks run outside the metadata gates. Scheduling progress still assumes the
platform schedules runnable native threads; no hard real-time guarantee is made.

`AmaNativeEventBuffer` is a move-only immutable native byte owner. The copy routine
finishes copying a counted source before publication, including embedded NUL or
non-UTF-8 bytes. Empty bytes need no allocation. The buffer retains its creator
service and corresponding native/code resource lease. A post accepts only valid
bytes from that service or the canonical empty owner; a borrowed byte facade is
rejected. The callback operation table and matching release pointer route external
C workers into the resource's creator image, including independently compiled
components using the same SDK header.

A queued post moves the byte owner and empties the producer's facade. Every
rejection preserves it so the producer can release it. Take transfers the same
owner to the consumer without copying or allocating. The consumer may read those
bytes after service closure and releases them with `buffer_drop` or
`completion_drop`.

## Rejections and resource limits

| Observation | Native effect |
| --- | --- |
| Fixed capacity is full | Prepare returns `AMA_BUSY`; no generation is consumed |
| Identity/refcount limit reached | `AMA_LIMIT_EXCEEDED` before publication; no wrap |
| Native allocation fails | `AMA_OUT_OF_MEMORY`; output owners stay empty and supplied keepalive stays owned by caller |
| Callback before arm | `AMA_EVENT_UNARMED`; supplied bytes remain with producer |
| Duplicate retained completion | `AMA_EVENT_DUPLICATE`; no second record |
| Recycled or foreign token | `AMA_EVENT_STALE`; no access to the new operation |
| Closed service | `AMA_EVENT_CLOSED`; no cell access |
| Borrowed/foreign/mismatched byte owner | `AMA_EVENT_INVALID_BUFFER`; ownership remains with producer |

These C statuses describe the native component. A future typed native wrapper
maps temporary capacity/backpressure into its declared operation contract;
returning a nonzero native body status remains the ordinary technical-error path.
Native completion `code` is separate operation-specific data and may be negative.
Native status message bytes are image-owned; consume them while retaining the
corresponding native code/resource dependency, or copy them when needed longer.

## Closure, acknowledgment and code lifetime

Disposal first closes the service under its gate. New callback operations cannot
resolve any cell after that point. The owner then releases queued byte owners and
the complete cell array outside the gate, before dropping its own reference.
Callbacks can remain alive with a small closed native service, independently of
the destroyed Ama context. A late result is rejected and its producer cleans up
its own bytes. No callback dereferences former Ama storage.

The native reference count is exactly the unique owner (including its close
operation), plus owning callback leases and owning byte allocations. Every retain
checks its finite bound before incrementing. Queue buffers can refer back to their
service; explicit close breaks that ownership cycle while the unique owner remains
retained. The last reference can therefore retire only after closure, outside
every gate, and invokes the optional keepalive release exactly once.

An `AmaNativeEventKeepalive` is an already owned native code/resource lease.
Successful service creation moves it; failure preserves it. Its infallible release
performs native retirement only. A code-dependent provider supplies such a lease;
a permanently resident executable may omit it. Callback and byte owners keep it
alive after the service's original context/owner is gone. The tests use both a
retirement observer and the actual independent `AmaLibraryCodePins` registry.

The embedding host still owns the loader barrier required by
[EMBED-06](../../spec/implementation/c-library.md#embed-06). That barrier excludes
new and in-flight calls, including callback/last-release returns, before unloading
an image. An ownership count cannot protect code forcibly unloaded during a C
call. T099 integrates the native provider and this lease with exported component
workflows; this task does not claim a completed asynchronous Ama host interface.

## Build and test the native boundary

Native providers can link the component's C units or include
[`native/events/assembly.h`](../../src/native/events/assembly.h) once in their
project assembly. The latter imports no Ama dispatcher. The callback/resource
interface is an in-process SDK contract built against this exact header, separate
from the checked exported Ama library wire ABI.

```sh
make -C tests/unit/async/events
```

The independently runnable leaves cover:

- **Identity:** all small bounded counters, exact `UINT64_MAX`, every initial
  factory allocation failure, service disposal, exact native-cell reuse while
  the old callback lease remains alive, and reference/identity exhaustion.
- **Storage:** synchronous completion inside submit, observed FIFO order,
  independent binary bytes, full capacity, duplicate/unarmed/foreign/bad-owner
  rejection, native allocation faults and an allocation-free error completion.
  Consumed bytes survive closure, and real code pins stay busy until the last
  callback/byte owner retires.
- **Callbacks:** concurrent first-domain initialization, real C worker threads,
  exactly one winner among duplicate callbacks, owner closure before a paused
  callback and completion from a worker before submit returns. Synchronization
  uses joins and native atomic barriers, with no sleep-based expected ordering.
  The supported Linux harness uses POSIX `pthreads`; the installed TSan runtime
  crashes even in a standalone minimal C11 `thrd_create` smoke test.
- **Assembly:** actual execution through the SDK assembly header, repeated guarded
  inclusion, native byte lifetime and release-mode core compilation while consumer
  assertions remain enabled.

The leaves link no Ama frontend or execution runtime. ASan/UBSan exercise both
separate-unit and assembled forms; ThreadSanitizer passes the complete native
component through the POSIX harness. For example:

```sh
make -C tests/unit/async/events \
    C_COMPILER_FLAGS='-O1 -g -Wall -Wextra -Werror -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -DNDEBUG -fsanitize=thread -fno-omit-frame-pointer'
```

LeakSanitizer is unavailable in the current
environment; exact native allocation counters require only the documented single
image receipt to remain. Per-file purposes, cases and standalone header commands
are registered in [the documentation inventory](documentation/ready.json).
