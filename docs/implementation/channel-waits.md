# Cooperative channel readiness and shutdown

T105 composes [bounded channels](bounded-channels.md) with
[native events](native-events.md) and explicit task suspension under
[PAR-04](../../spec/language/async-and-parallelism.md#par-04).
A full send or empty receive remains an ordinary result. A caller can register
readiness, recheck its event, mark its task awaiting and yield to its immediate
caller. Other ready work can run while the registration owns native resources.
Readiness permits retry; it neither transfers a message nor reserves capacity.

## Registration and the lost-wakeup argument

`ama_native_channel_wait_send` takes the complete encoded length of the pending
message. Its predicate is count capacity AND sufficient byte capacity, or a
terminal endpoint transition. Oversize requests are immediately ready so the
caller can observe their ordinary rejection. Zero requested length is invalid.
Receive readiness means queued data, sender closure or receiver abort.

The caller prepares an event and registers inside its synchronous arm callback.
The source must already be armed. The event acceptance callback distinguishes
armed, prepared, completed, retired and closed sources using the full native key.
Prepared or malformed sources are technical errors; completed, retired and closed
sources need no additional registration. The source probe never executes Ama.

Registration clones an event endpoint before taking the channel gate. Under that
gate it either observes readiness and selects a notification, or installs one
pending source for that direction. Queue and shutdown operations select pending
notifications under the same gate. Therefore either the state change precedes
registration and is observed by its predicate, or registration precedes the state
change and that operation selects its notification. There is no unchecked interval
between inspection and installation. A second pending registration returns Busy
without replacing the first. Sender and receiver registrations are independent.

Notifications are posted and endpoint rights released after unlocking. No
allocator, release callback, event callback or Ama code executes under the channel
gate. A selected source may concurrently complete, retire or close: posting then
returns queued, duplicate, stale or closed. None grants permission to retain a
borrowed Ama view or resume a continuation from a native callback.

## Exact cancellation and lifetime

A move-only `AmaNativeChannelWait` owns a channel-service credit and a complete
native event key. Cancellation removes only the matching pending key in its
specified direction. It is idempotent and cannot cancel a later registration
reusing the event slot with a different generation. Dropping the handle cancels
its pending registration and releases that service credit. The owner is cleared
before invoking creator callbacks.

Cancellation alone does not withdraw a notification already selected for posting.
A task first clears visible waiting state and detaches its operation token, then
cancels the channel handle. A concurrently selected notification is subsequently
rejected as stale; accepted channel data remains queued. Task cancellation does
not silently discard a message or imply successful delivery.

Sender close preserves accepted messages and wakes an empty receiver to observe
closure. Receiver abort releases queued owners and wakes a blocked sender to
observe aborted delivery. Metadata and creator-code ownership survive both port
drops while a wait handle remains. Closing an event source retires publication
rights without retaining the source's Ama context. The final native owner releases
service storage and its code credit. The loader must still exclude new calls and
wait for executing creator calls before unloading code.

## Executable evidence

`make -C tests/unit/async/channel-waits` composes the real channel and event SDKs:

- registration before and after readiness; exact variable-byte capacity predicates;
- prepared-source rejection, duplicate completion, stale and closed sources;
- conflicting registrations, exact cancellation and event-slot generation reuse;
- closure and abort wakeups, both ports disappearing before the last wait handle;
- reference-credit exhaustion and unchanged failed outputs without allocation;
- 128 publication/registration races and 128 cancellation/publication races;
- a native host blocked on the actual event wait, woken by endpoint closure.

Atomic counters check allocation and creator-code ownership. One documented
process-lifetime event-image identity receipt is excluded; no live service or code
credit is excluded. Optimized tests count both malloc and compiler-folded calloc.
Native ASan/UBSan and TSan exercise these same implementations.

`make -C tests/unit/codegen/channel-waits` compiles actual Ama tasks and two isolated
contexts. Five scenarios cover normal backpressure, sender failure, receiver
failure, cancellation and a receiver that starts before its sender. A deliberately
premature resume rechecks readiness and yields without consuming a message. The
root performs unrelated ready work before waiting; explicit task phase transitions
and operation detachment control resumption. Endpoint modules own native rights;
their borrowed operation and port references remain declared Ama parameters.
Failure becomes an ordinary result in the surviving peer, and accepted data drains
after sender teardown. The release-generated C consumer checks native source
closure and exact cleanup after both contexts disappear. Per-file evidence and
standalone header checks are recorded in [ready.json](documentation/ready.json).
