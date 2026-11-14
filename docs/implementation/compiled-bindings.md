# Literal inputs and ordered result receivers

T064 accepts compiled binding behavior under
[CALL-07](../../spec/language/calls-and-results.md#call-07),
[CALL-08](../../spec/language/calls-and-results.md#call-08),
[CALL-06](../../spec/language/calls-and-results.md#call-06),
[TYPE-06](../../spec/language/declarations.md#type-06) and
[LIFE-04](../../spec/implementation/native-interface.md#life-04).
It builds on [entry preparation](entry-preparation.md),
[ordinary binding resolution](ordinary-bindings.md) and
[structural binding parsing](call-bindings.md). The existing emitter/runtime
already implement these paths; this checkpoint adds independent byte/value/effect
observations rather than another binding executor.

## Raw payload, one selected constructor

Literal emission writes one unsigned C byte initializer for each source payload
byte and retains its explicit byte length. The empty-literal C sentinel does not
increase that length. No C string decoding, trimming, name splitting, comment
recognition or native-constructor execution is used to guess the value.
The receiving declaration selects the native literal capability. Entry preparation
chooses that constructor once over fresh replacement storage; an omitted input
instead selects its declared default. Body execution waits for every interface
installation to succeed.

The compiled byte fixture has a declared 64-byte native representation. This is
its own test-module capacity, not a language limit. Its default/literal hooks emit
different markers. The expected sequence distinguishes first construction,
exactly-once supplied literal construction, ordinary source copying, a repeated
identical literal and an explicit empty literal. The inferred caller receiver's
default marker appears before the first body operation, as required for a
persistent TYPE-06 field.

The native body prints typed input/result byte counts and raw data. A subsequent
call uses the inferred receiver as an ordinary source, exposing the actual stored
transfer rather than only the callee's input. Payloads include two leading and
two trailing spaces, `#`, a further `|` and UTF-8 bytes. LF and CRLF source forms
produce identical effects, including a CRLF module with no final newline.
Native payload address checks and actual C layout keep the byte buffer separate
from runtime control metadata.

A 65-byte payload is valid source but exceeds this native type's capacity. Its
constructor fails before body entry/result publication; the refusal alternative
does not run. Previously completed construction effects remain visible. The
compiler does not execute a constructor or guess capacity from its C body.

## Sequential result actions

Each successful output resolves its receiver at that moment. After k completed
transfers, those receivers contain their committed values; the next receiver sees
that state, including a new index written by an earlier output. An empty output
consumes its interface position without creating/selecting a receiver. New owned
or reference fields retain the exact result form and module identity; a copied
reference identifies the same retained instance.

One compiled case writes an index, then transfers another result into the newly
selected list element. Another exposes an inferred reference through both the
reference and original name, distinguishing identity transfer from an implicit
clone. A placeholder skips the first port while the second supplies its own
inferred receiver.

The partial-progress case first commits value 23 over an earlier receiver, then
fails native index conversion for the next receiver. Native destructor effects
show the replaced old value 3333 before failure and the committed value 23 during
terminal cleanup. The second 23 belongs to the callee's retained result. No
rollback restores the old receiver, no `:` body runs, and the other live values
are destroyed in their specified reverse order. This observes the promise through
ordinary native effects without retaining a mutable diagnostic pointer past a
context lease.

## Evidence and boundaries

`make -C tests/unit/codegen/bindings` runs five explicit cases through the real
frontend, emitter, C compiler, entry verifier and generated runtime. Setting its
compiler flags to ASan/UBSan also instruments the generated programs. The
recursive parent entry is `make -C tests/unit/codegen bindings`.

The shared native fixture has two narrow optional emission callbacks: a typed
entry fragment and an ABI-hook body. They borrow the actual scaffold model only
during emission, select exact known fixture identities, return static hook text,
and are reset after each custom case. Other leaves keep their existing native
contracts. These are test construction hooks, not production language dispatch.

The per-file coverage inventory names each byte/receiver case for the included
support files, leaf/parent Makefiles and changed shared fixture. Existing runtime
preparation/replacement tests supply failure-prefix and authority proofs. Planned
list-service result receivers connect their concrete storage in T076; this
checkpoint does not claim that later service integration.
