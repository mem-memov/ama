# Owned failure values, immutable text and refusal snapshots

T086 implements [ERROR-01](../../spec/implementation/diagnostics.md#error-01)'s
ordinary condition-capable failure result. It remains separate from fatal native
status and [ERROR-06](../../spec/implementation/diagnostics.md#error-06)'s last
acquisition-refusal snapshot. The compiler recognizes none of these result names
as language markers; ordinary type/protocol/interface rules compose them.

## One complete application outcome

[`..failure`](../../stdlib/modules/failure) owns one public result `description`
of type `..text`. Its native payload contains only a Boolean discriminator.
Default construction gives false/empty. `?` reads that discriminator without
executing the module or resetting its result. Retained result reading copies the
owned description normally; it does not consume or clear a failure.

[`..failure.make`](../../stdlib/modules/failure+/make) has one owned `message`
input and one owned `failure` result. It constructs a present failure with that
message; even an empty message is valid text and does not determine presence.
Calling an existing failure value with `!` starts a new invocation: result reset
clears the description, and its native body clears the discriminator. There is
no second success flag, sibling reason result, hidden exception or implicit
propagation.

For example, the maintained CLI test declares a text reporter and executes:

```text
builder
..failure.make
builder !
 < |expected application failure
 > outcome
outcome
?
 outcome
  description >> message
 report !
  < message
```

`outcome` and `message` are inferred from their declared result types. They own
independent Ama instances. Copying `outcome` copies both its discriminator and
description. Reusing the producer, clearing the original, or destroying either
copy cannot change the other's value.

The native representation's `=` hook copies only its discriminator. Generated
graph cloning copies the declared text child; generated cleanup finalizes that
child. No Ama reference or description owner is hidden in opaque native payloads.
The failure type declares `+`, `-`, `=`, `?`, but no literal/index capability.
`failure.make` is an ordinary typed constructor module, not compiler syntax.

## Native producer contract

The [failure header](../../stdlib/modules/failure.h) declares checked native
helpers using its exact generated full write-view type. A producer passes its
borrowed owned failure output to the qualified `set` helper with counted UTF-8
bytes. The helper prepares/replaces the owned text first, then sets presence.
There is no user-code interleaving between those publications. Preparation
failure preserves the complete old value and returns its technical status.
The `clear` helper establishes false/empty together; empty replacement allocates
nothing.

The constructor and compiled test producers propagate helper status unchanged.
An expected failure returns technical success with a true failure value. A
failed description allocation returns technical OOM and skips output transfer;
its `:` does not catch the error. A new successful invocation cannot retain an
old failure description because normal result reset and clear both establish
the no-failure state.

## Text is an independent value with shared immutable bytes

[`..text`](../../stdlib/modules/text) declares default empty, counted UTF-8
literal construction, clone, destruction and nonempty truth. It has no numeric
index protocol. Text entry preserves its value. Embedded NUL is valid Unicode
text and retains its explicit byte length; no operation treats bytes as an
implicitly terminated C string. There is no normalization or locale conversion.

The [public native header](../../stdlib/modules/text.h) exposes a value payload
containing an opaque backing pointer and live-state marker. A nonempty value
owns one share of a backing with immutable length/bytes and a mutable native
share count. Empty values have no backing allocation. Different Ama values and
pool slots remain distinct even when their native byte storage is shared.

The [UTF-8 validator](../../stdlib/modules/text+/native/unicode.c) recognizes
Unicode scalar encodings through exact lead/first-continuation ranges. It rejects
overlong encodings, surrogates, values above U+10FFFF and truncated/invalid
continuations before allocation. Every iteration consumes one scalar; bounded
remaining-length checks precede every byte read. Validation and construction
are O(N) in bytes; storage is O(N). A borrowed view owns no storage and cannot
escape its enclosing Ama native activation.

[Clone/finalization](../../stdlib/modules/text+/native/storage.c) use the
[PROTO-06](../../spec/implementation/native-interface.md#proto-06) share contract.
Clone checks representability, acquires one share, then publishes the destination.
It costs O(1) and allocates no bytes. Finalization drops exactly one share and
frees backing on the last drop. Production counts have `size_t` range; a reduced
constant limit in the component test exercises exhaustion without wrapping.
Backings stay within one serially executed context; they are not cross-context
message storage or a reason to share Ama views between executors.

[Replacement](../../stdlib/modules/text+/native/values.c) validates and constructs
new backing before releasing the old live value. This permits source bytes to
alias the old backing. No published bytes are ever edited. Failure preserves
pointer, bytes and shares; success changes only that value's backing. Allocation
and release overrides must be supplied as a pair, permitting counted native
failure tests without changing production ownership semantics.

## Refusal data stays ordinary and read-only

The retained reader/status providers now use actual standard-root identities:
`..std.refusal` returns one `status` of type `..std.status`. Its C representation
holds the common `AmaStatus` code, length and immutable program-lifetime message
view. It is not an owned application description or a condition-capable failure.
Native readers can inspect the declared status representation without gaining
metadata pointers or mutation rights.

The reader copies its immediate caller's snapshot and reports technical success,
even when that snapshot's code is nonzero. Successful calls, expected application
failures and reads do not clear the snapshot. Only another acquisition/preflight
refusal initiated by that instance replaces it. Native status message lifetime
remains [ERROR-04](../../spec/implementation/diagnostics.md#error-04)'s immutable
program lifetime; ordinary value copying does not retain a borrowed service view.

## Ownership argument and executable evidence

For each nonempty backing, its count equals outstanding native ownership credits.
Construction contributes one; successful clone adds one before publication;
destruction removes one. A failed increment adds none, so it cannot require a
compensating decrement. Counts are checked before incrementing. Generated
initialized-prefix cleanup consumes earlier successful shares after a later
child clone fails. Therefore destruction order does not affect surviving values
and failed preparation restores the original counts.

Text replacement first owns an independent prepared credit. After successful
preparation, it consumes the old credit and transfers the prepared value into
the existing native payload. With one context executor, no Ama body can observe
intermediate bytes or a discriminator paired with a different description.
Declared Ama graph cloning preserves complete module identities and lifetime;
sharing only immutable native storage does not merge that graph.

[ready.json](documentation/ready.json) maps every maintained source/header,
interface, ABI record, fixture and Makefile to named executable evidence.

- `make -C tests/unit/runtime/text-values` checks both destruction orders,
  independent replacement, aliased input bytes, allocation/UTF-8 rollback,
  reduced-limit exhaustion, Unicode scalar boundaries, embedded NUL, empty
  no-allocation values and actual owned-tree clone-prefix cleanup. Its two
  source children retain two shares after first-clone success/second-clone
  exhaustion rolls back.
- `make -C tests/unit/codegen/failure-values` compiles actual standard providers
  and typed test producers. It checks false/true/false transitions, retained
  descriptions, owned copies, same-name results, UTF-8 text, explicit clearing,
  empty descriptions with explicit presence, the real constructor, unchanged
  last-refusal code and description-allocation failure remaining technical.
- `make -C tests/unit/codegen/execution` verifies the existing read-only snapshot
  consumer under its newly explicit standard-root configuration.
- `make -C tests/integration/failure-values` runs verified CLI compilation and
  the maintained constructor/positive-check/retained-description/clear project.

Native/generated ASan/UBSan, whole-project strict tests, standalone headers,
coverage, local links and reproducible Doxygen complete acceptance. Arithmetic
expected-failure result alignment remains T087; file/I/O examples remain later
roadmap work.
