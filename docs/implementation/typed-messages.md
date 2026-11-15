# Explicit typed message encoding

T103 implements a native data codec under
[PAR-03](../../spec/language/async-and-parallelism.md#par-03).
A sender copies application data into an independently owned envelope. A receiver
validates the complete envelope before allocating decoded storage, then its typed
provider constructs fresh Ama values. No pool, checked identity, reference cell,
saved selector, continuation or native resource pointer belongs in message data.
Casting one to an integer does not authorize its transmission. The codec does not
try to recognize pointer-looking integer patterns: the explicit application schema
and its typed binding determine which mathematical data is being sent.

## Supported native profile

The public [message interface](../../src/native/messages/message.h) declares an
ordered record of scalar fields or bounded lists of scalars. Its primitive types
are conditions, arbitrary-precision signed integers, UTF-8 text and binary bytes.
Providers bind that data schema explicitly to their application modules. This is
not reflection over arbitrary native payloads and does not serialize a C struct's
padding, addresses or memory layout. The primitive/list record is the supplied
profile; custom application codecs must obey the same canonical PAR-03 boundary.
The existing synchronous C-library transports remain a separate EMBED-04 API.

`AmaNativeMessageSchema` supplies an exact nonempty byte name, a positive application
version, the ordered field kinds/shapes, positive list count limits, per-value
byte/magnitude limits and a positive complete-envelope byte limit. A scalar has
exactly one item; an empty list is valid. Zero text/bytes limits admit empty values
only. An integer needs at least one permitted magnitude byte. Conditions carry no
byte payload. Incompatible application layouts need a new declared name/version;
a provider cannot silently redefine one protocol identity.

The record can contain no fields. Scalars have canonical representations:

- A condition is flag zero or one and an empty byte slice.
- An integer is sign zero/one and a nonempty big-endian magnitude. Zero has exactly
  one zero byte and a nonnegative sign; leading zero bytes and negative zero fail.
- Text has flag zero and exactly scalar UTF-8 bytes. Embedded NUL is data; overlong
  encodings, surrogates, truncation and values above U+10FFFF fail.
- Binary bytes have flag zero and arbitrary counted bytes, including NUL and 255.

The schema describes data, not an action selector for an Ama list or module.

## Exact wire layout

All integers below are eight-byte unsigned network-order values, independent of
native pointer size, endianness or C padding. Values above the receiving platform's
representable size are rejected before arithmetic or allocation.

| Sequence | Encoding |
| --- | --- |
| Format | Four bytes: `41 6d 4d 01` (`AmM` followed by format byte one) |
| Application version | One eight-byte value; it must equal the declared version |
| Schema identity | Eight-byte name length followed by exactly those name bytes |
| Record shape | Eight-byte field count, equal to the declared count |
| Each field | One kind byte (condition 1, integer 2, text 3, binary 4), one shape byte (scalar 0/list 1), then eight-byte item count |
| Each item | One flag byte, eight-byte payload length, then exactly those payload bytes |

Kinds/shapes must match the schema; counts and lengths must satisfy all declared
bounds. Trailing bytes are invalid. This profile has one representation for each
valid record value; empty borrowed pointers and nonnull empty slices both denote
the same empty data, not different message values. No terminator or implicit
alignment is present.

## Ownership and failure provenance

`ama_native_message_encode` checks schema/owner configuration, field shape and
whole-message size before scanning scalar payloads. One counting emitter and one
writing emitter use the identical traversal. Only a completely validated record
obtains a code credit and a single envelope allocation. Failure preserves the
initialized-empty destination. Source fields/schema are synchronous immutable
borrows; the encoded owner retains neither.

`ama_native_message_validate` performs full allocation-free admission. A channel
can reject unsupported format, schema/version mismatch, malformed data or limits
before accepting bytes. This does not consume bytes, construct Ama, or grant any
borrowed storage a longer lifetime.

`ama_native_message_decode` first validates the entire borrowed envelope and counts
scalar slots without allocating. It checks array-size multiplication, obtains its
own native creator-code credit, allocates field/value arrays and a wire copy, then
fills private views into that copy. No decoded view aliases the sender envelope or
schema storage. Every allocation failure cleans its initialized prefix. An empty
record/list avoids corresponding zero-length array allocations. Drop consumes the
owner before freeing its arrays/wire and releasing its final code credit; repeated
empty drop is harmless. The host loader barrier still covers release return.

`AmaNativeMessageResult.status` carries technical configuration/allocation/code
admission failure. For an OK status, `reason` is an ordinary accepted/unsupported/
schema/malformed/limit result. Applications can expose rejection as one ordinary
`..failure` module with its owned description. A valid message that cannot be
constructed because allocation or native lifecycle fails is a technical receiver
failure, not malformed data or a successful partial result.

After native decoding, a typed Ama provider prepares its owned result fields while
they remain unavailable during the call. It publishes only on normal completion.
If construction fails after an earlier child became live, native temporary owners
retire locally and runtime cleanup consumes the generated ownership graph. No
partially initialized graph escapes through an output binding.

## Argument, bounds and acceptance

Every wire read checks remaining length before addition/dereference. Each field
consumes a fixed header and each item at least nine bytes. Thus no forged count can
force more item iterations than the remaining bounded input admits. The scalar
count sum and allocation multiplications are checked. UTF-8 consumes one complete
scalar at each iteration. Record/list processing is iterative and uses no C
recursion. Encoding and decoding take linear time in the actual record/envelope
and use linear native storage. Decoding metadata can exceed the wire byte count,
but it is bounded by field headers and minimum item widths, with checked arithmetic
and ordinary technical OOM handling. There is no unbounded implicit queue here.

Fixed lengths, exact schema ordering and canonical scalar grammars determine one
unique parse. The sizing/writing traversal has the same sequence of width and slice
contributions, so the allocated size equals the emitted size. Complete validation
precedes allocation; private candidate ownership precedes publication. These
invariants explain rejection and cleanup generally, rather than assuming a passing
round trip proves format correctness.

`make -C tests/unit/async/messages` exercises real codec components: independent
owned values/code credits, all truncated prefixes and trailing data, wrong schema
and limits, canonical integer/UTF-8 boundaries, empty records/lists, malformed
configuration and nonempty outputs, and every actual allocation prefix. Independent
fixed wire vectors check encoding. A one-condition grammar has exactly two valid
strings; all 12,288 single-byte substitutions of its complete envelope are checked
against that finite oracle without allocation.

`make -C tests/unit/codegen/message-codecs` compiles actual native bindings and an
Ama receiver record with owned condition/text/integer fields. A Unicode value and
an integer beyond 64 bits survive release of native decoding storage and replacement
of the Ama envelope. A malformed envelope becomes an ordinary failure description.
An injected technical error after constructing the owned text child prevents
publication and cleans the complete runtime/native prefix. The generated `NDEBUG`
consumer wraps direct project `malloc/free` and requires zero live allocations
following both completed and failed context disposal. ASan/UBSan supplements these
exact ownership counters; it does not replace them.

Per-file named cases, standalone headers and commands are registered in
[ready.json](documentation/ready.json). Message-count/byte-capacity ownership,
FIFO and sender/receiver closure remain T104–T105.
