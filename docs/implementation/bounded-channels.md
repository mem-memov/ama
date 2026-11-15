# Bounded native channels between isolated contexts

T104 implements [PAR-04](../../spec/language/async-and-parallelism.md#par-04)
with explicit native ownership under
[PAR-05](../../spec/language/async-and-parallelism.md#par-05).
The [channel SDK](../../src/native/channels/channel.h) connects one logical sender
and one logical receiver. Its queue stores complete independently owned
[typed envelopes](typed-messages.md), not Ama objects or shared backing from an
Ama text/bytes value. Each context owns its own native endpoint resource.

## Creation and the native boundary

`ama_native_channel_create` takes a declared message schema, positive message-count
and encoded-byte capacities, an explicit creator-code provider and two distinct
initialized-empty endpoint owners. It validates configuration with
`ama_native_message_schema_check`, checks metadata-size multiplication, acquires a
code credit, copies the complete name/field schema and allocates a fixed ring.
It publishes both ports only after complete construction. Failed creation cleans
its initialized prefix and preserves both destinations.

Each port is a move-only native right with a creator callback table. Calls return
to the creator implementation rather than interpreting private service layout in
another component. The service retains creator code until both ports and any wait handles retire.
Message owners separately retain their own creator-code credits, so a received
envelope can outlive the whole channel service. The loader must still exclude new
calls and wait for active creator-code calls to return before unload.

These are native services, not checked Ama references. A native module can own one
port in its payload and must declare no clone capability. Its generated Ama fields
remain the only place for persistent Ama ownership/references. A provider must
move distinct ports into different contexts through its declared native resource
protocol; bit copying an endpoint does not create another owner.

Each endpoint owner serializes its own operations. The independent sender and
receiver may run concurrently. Multiple logical producers require separate
channels or explicit application arbitration. The internal queue gate is not an
implicit multiple-producer policy or an Ama scheduler.

## Whole-message send and receipt

`ama_native_channel_send` borrows the sender and a native envelope owner. It checks
native owner shape and individual channel size, validates the entire declared
protocol outside the gate, then rechecks closure and count/byte availability under
the gate. Validation rejection reports the exact ordinary message reason.
Acceptance moves one complete owner into the tail and clears the caller's owner.
Every rejected send preserves all caller ownership. The queue operation allocates
nothing and never calls a release function under the gate.

An application adapter must copy borrowed Ama bytes into independent native
storage before enqueueing; retaining the borrowed view or passing an Ama backing
pointer as an owned allocation is invalid. The maintained typed fixture shows this
copy and releases its prepared candidate on every unaccepted send. Full, closed,
aborted, oversize and schema rejection are ordinary application results, distinct
from technical allocation or native configuration failure and from busy Ama
module refusal.

`ama_native_channel_take` requires an entirely empty output. Under the gate it
moves the head owner to the caller, clears the cell, advances the ring and refunds
exactly that message's encoded length and one count credit. Decoding and
application processing occur after those capacities are available again. This is
receipt of native bytes, not acknowledgment of application processing.

For an open empty channel, take reports Empty. After normal sender close, it
continues to return accepted messages and then reports Closed. Aborted delivery
is a separate result. No operation waits for capacity or data; callers receive
ordinary results and can run other ready work. Task readiness/waits and
cancellation are explicit [channel wait registrations](channel-waits.md). Native test-driver retry loops are not
an implicit busy-waiting Ama executor.

## Shutdown and independent lifetime

Closing the sender is idempotent and preserves the queue. Dropping its port
performs that close and releases only its endpoint credit. The receiver keeps the
queue/service alive, including after sender-context failure or disposal.

Aborting the receiver marks delivery aborted and detaches the whole queued range
under the gate. It then releases all detached message owners outside the gate.
No sender can modify those cells after abort. Repeated abort is harmless; a new
send reports Aborted without consuming its input. Dropping the receiver performs
abort and releases its endpoint credit. The last service owner frees metadata and finally
its creator-code credit. Already taken owners remain independent.

The fixed ring metadata costs linear space in the configured count limit. Queued
encoded data never exceeds the byte limit; both bounds apply simultaneously.
Caller-owned rejected candidates, received data and decoded receiver graphs are
outside queue capacity and remain the corresponding owner's responsibility. No
process-wide memory bound is implied by a channel's byte limit.

## Invariants and exact concurrency argument

Let `Q` be the ordered live ring range, `N` its count, and `B` the sum of its owned
envelope lengths. Initially `Q` is empty and `N=B=0`. The invariant is:

- each live cell owns exactly one complete envelope; every other cell is empty;
- `N=|Q|` and `B` equals the exact length sum;
- `N` and `B` stay within their independent positive capacities;
- a message has exactly one owner: caller, queue or receiver;
- only producer close preserves queued delivery; receiver abort explicitly ends it.

Acceptance checks `length <= maximum_bytes - B` before addition, moves one owner
and appends its exact length. Take removes only the head and refunds that exact
length before unlocking. Ring advancement avoids integer wrap. Thus FIFO and
capacity hold under every interleaving of the two endpoint owners. The gate's
release/acquire ordering publishes byte contents as well as owner metadata.

Abort atomically prevents enqueue and detaches the range before any release.
If send linearizes first, abort consumes that accepted owner; if abort linearizes
first, send preserves caller ownership. Detached cells cannot be reused afterward.
The receiver's port keeps service memory alive through outside-gate retirement.
Native endpoint drop and final metadata release follow the same exact ownership
counts; no allocator, callback or Ama code runs in a gate critical section.

Send costs linear validation in the bounded envelope plus constant ring work.
Take and close use constant metadata work. Abort performs constant detachment then
linear work in actual queued owners. Construction copies explicit schema/ring
metadata once. There is no per-send queue-node allocation, ownership scan or
heuristic retry limit.

## Executable evidence

`make -C tests/unit/async/channels` checks actual SDK components:

- independent count/byte bounds, exact variable-length refunds and oversize sends;
- whole-owner preservation on full/schema/native-shape rejection, empty outputs,
  repeated ring reuse, and independent channels without global delivery order;
- copied schema independence, every creation allocation failure, native code
  admission failure, sender drain, receiver abort and taken data after service death;
- 4096 ordered messages over actual POSIX producer/consumer threads with ring
  capacities one and seven;
- 128 coordinated abort/send races with both permitted ownership linearizations.

Counters are atomic during concurrency. Tests wrap both `malloc` and `calloc`
because an optimized compiler may fold allocation plus zero initialization into
a zeroing allocation; `free` consumes the same counted ownership. Native optimized
release, ASan/UBSan and TSan checks use the real queue/endpoint code.

`make -C tests/unit/codegen/channels` compiles actual isolated Ama contexts.
Distinct native sender/receiver modules are noncopyable; the queue never shares
Ama storage. A full send becomes an ordinary `..failure`. Receiving frees capacity
before decoding; the sender resumes and queues another message. Disposing the
sender context closes its port, and the receiver later drains that retained value,
constructing its own condition/text/arbitrary-precision integer fields. The
release-generated consumer checks exact cleanup of all direct runtime/native
allocations after both contexts disappear. [ready.json](documentation/ready.json)
registers per-file named evidence and standalone headers.
