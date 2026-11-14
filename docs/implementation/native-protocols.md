# Checked native protocol completion and the integer size bridge

This is T009 implementation evidence. The canonical contracts are
[PROTO-01–PROTO-11](../../spec/implementation/native-interface.md#proto-01),
[LIFE-01–LIFE-05](../../spec/implementation/native-interface.md#complete-lifecycle-and-cleanup-protocol)
and [ERROR-03–ERROR-06](../../spec/implementation/diagnostics.md#error-03).
The [roadmap](../../TODO.md) distinguishes these foundations from current-language
frontend, intrinsic list and public embedding consumers.

## Check transport without losing native ownership

A native constructor returns its code/message/length record after acquiring its
resource. The raw code tells the caller whether that acquisition succeeded.
Checking the message fields is a separate operation. For example, code zero with
a null message and length one is malformed transport, but the constructor already
claimed to have completed its native payload. Losing that cleanup obligation
would leak the resource when reporting the malformed status.

`runtime/lifecycle/native.{h,c}` implements `ama_lifetime_finish_native` over a
valid Constructing ledger whose owned-child prefix is complete:

| Raw native completion | Resource obligation | Returned result |
|---|---|---|
| Nonzero code | No native destructor credit; the failed hook cleans its own partial acquisition | Checked failure status |
| Zero code, coherent transport | Finish the lifecycle, recording one native credit when the type has native state | Normal lifecycle completion |
| Zero code, malformed transport | Record the same completed native credit before private unwind | Static InvalidState failure |

The helper allocates nothing and publishes no instance identity. A live private
payload and a publicly accessible instance are distinct states. Construction and
clone preparation publish only after their complete tree succeeds, so the last
row remains private and is finalized exactly once.

When a child fails status validation after raw success, its parent has not yet
credited that child. Unwind first finalizes the current child, then each parent's
previously credited prefix. The child is therefore neither skipped nor destroyed
again through the parent. Runtime tests cover both default and clone paths at
this boundary, as well as root completion and immutable shared-to-owned copying.

Shared constants have their own lifecycle representation. Their `native_live`
bit is set from raw zero completion before checking transport. Failure of a
later constant cleans its own completed payload, all earlier constants and all
factory storage before returning an unpublished context result.

## Destruction and conversions

Object and constant destruction consume the native credit before invoking the
hook. Its returned transport is checked before reporting or accumulating status.
A malformed success is a technical failure; the consumed credit stays consumed.
All remaining owned children and constants are still released, and later cleanup
errors cannot replace the primary failure. Automatic cleanup never retries a
partly finalized native value.

Condition conversion uses a local Boolean and publishes it only after checked
success. A failure releases the world operation guard and leaves the caller's
output and last-refusal record unchanged. InvalidIndex is technical in a condition
operation; only the indexed-selection boundary gives that code refusal meaning.
The [status contract](native-status.md) describes that explicit provenance.

The generated native-entry call checks its separate technical status after C
returns and the native operation guard is released. Failed completion goes to
terminal reporting before result actions or later Ama calls. External effects
already performed by the body are retained, as required by ERROR-02. No exception,
message-text classification or additional `:` handler is introduced.

## Select the fixed integer size constructor once

The lifecycle constructor domain is Default, Literal, Clone and Size. Size is an
internal construction recipe for the exact standard-library integer bridge in
[PROTO-03](../../spec/implementation/native-interface.md#proto-03). It adds no Ama
source form, native `.abi` record or general numeric cast.

A trusted resolved consumer supplies the typed bridge adapter and structural host
metadata through `AmaRecipe.from_size` and `size_value`. The ordinary construction
engine reserves the final stable identity, constructs declared owned fields,
then invokes precisely that callback on fresh aligned native payload storage.
It does not first invoke `+` or the literal parser. Normal success publishes the
completed instance; native failure unwinds its owned-field prefix; malformed
zero completion also finalizes the completed native payload.

The recipe borrows executable adapter code for the operation; it stores no Ama
reference or persistent callback in opaque native state. The returned actual
integer instance belongs to its caller until explicit disposal. List service
frames will own/protect such results under LIST-04 in their later roadmap tasks.

The runtime consumes compiler-validated nominal descriptors. It does not discover
the standard integer by payload layout or helper basename. [T055](native-contract-composition.md) binds the exact
standard-root identity to its declared typed `from_size` symbol and rejects a
missing/incompatible declaration. Typed list lowering supplies the recipe;
[the canonical scalar library](scalar-library.md) now validates its real native
implementation alongside independent runtime constructor tests.

## Maintained integer and native protocol tests

[The maintained integer](../../stdlib/modules/integer) supplies all six explicitly
bound protocols and the typed `from_size(State *, size_t)` helper. Real AST/ABI
readers check its declaration and exactly six records. Counted native tests
compile the actual library components; the full-root identity registry checks
their exact `..integer` namespace. [T055 bridge binding](native-contract-composition.md)
checks header signatures, and [T085](scalar-library.md) compiles and executes
actual standard-root/list-method consumers through the frontend and CLI.

The native value owns a signed arbitrary-precision magnitude in little-endian
base-256 limbs. Literal parsing accepts optional minus and decimal digits;
negative zero is canonicalized. Cloning allocates an independent magnitude, and
read-only truth/index protocols inspect that current value without entering its
empty native body. Negative values and magnitudes above SIZE_MAX return the
dedicated InvalidIndex code without changing the output.

Direct size construction counts its base-256 digits, allocates once and repeatedly
stores `value % 256`, replacing `value` by `value / 256`. The nonzero quotient
strictly decreases, and at most sizeof(size_t) limbs suffice with eight-bit bytes.
Index conversion folds high-to-low only while
`prefix <= (SIZE_MAX - next_limb) / 256`. Each prefix of a constructed size is at
most the original input, so this guard cannot reject any input in 0…SIZE_MAX and
the fold reconstructs that input exactly. No signed narrowing, out-of-range shift,
decimal formatter or literal parser is involved.

For N decimal input digits, N base-256 limbs suffice because 10^N < 256^N. The
maintained parser costs O(N squared), with O(N) allocated native bytes; each
multiply/add is at most 2559 and fits unsigned arithmetic. The production library
uses this explicit complete fold; no faster parsing complexity is claimed.
Direct size construction/conversion take O(sizeof(size_t)) work and storage.
Allocation or literal failure releases every private limb allocation and leaves
no live destination credit. Observer macros independently show that size
construction does not invoke the default or literal callbacks.

## Typed rejection and executable evidence

- `make -C tests/unit/runtime/protocols`: ten real-resource cases cover malformed
  default/literal/native-source/clone completion, uncredited children, failed shared
  factories, failed constructor self-cleanup, object/constant destruction,
  condition-output preservation and single size construction after owned fields.
- `make -C tests/unit/native/abi/integer`: six cases check the real module/table,
  all exact signatures, zero/one/SIZE_MAX and every representable limb boundary
  with neighbors, independent cloning, negative/overflow refusal and allocation
  failure with no live partial payload.
- `make -C tests/unit/native/abi/signatures`: a positive C candidate must compile;
  eight incompatible candidates must fail with the exact type-mismatch assertion,
  covering all six protocols, the void entry boundary and full-width size bridge.
- `make -C tests/unit/codegen/execution`: real generated programs reject malformed
  native-entry/condition completion before continuing their Ama control flow.
- Existing semantic native/type cases reject missing mandatory companions and
  uses of unbound optional capabilities before execution. A C function's spelling
  does not supply a missing symbolic binding.

The [toolchain contract](native-toolchain.md) requires inlining only at the native
entry boundary. Its paired setjmp probes accept a retained ordinary inline-hinted
helper that returns normally and reject an uninlinable native entry. Both O0 and
O2 inspect the actual object; optional helper inlining is not a transitive rule.

T009 acceptance passes the strict full suite with `-DNDEBUG -Werror`, all O0/O2
toolchain probes, and ASan/UBSan runs of runtime protocols, integer ABI, signature
probes and generated execution (leak detection disabled). Complete documentation
coverage checks 724 maintained inputs with zero debt. Two warning-free Doxygen
generations produce byte-identical output with 555 maintained file pages,
2292 documented interfaces and all 286 canonical/page destinations. These counts
record T009 completion rather than a fixed target for later tasks.
