# Native ABI implementation contract

This fixes engineering representations for section 2 of [TODO.md](../../TODO.md),
under CABI-01 through CABI-05, PROTO-01 through PROTO-11, LIFE-01 through LIFE-05,
STACK-01 through STACK-04 and ERROR-03 through ERROR-06. It does not change
[the specification](../../TECHNICAL_SPECIFICATION.md) or claim frontend support.

## Supported backend toolchain

The verified baseline is GCC 15.2.0, target `x86_64-linux-gnu`, C11, LP64,
eight-bit bytes. `src/native/toolchain/gcc.h` rejects other compiler/version/target
combinations rather than claiming their native-entry enforcement is verified.
The compiler implementation itself remains ordinary C; this restriction applies
to the generated native backend and its conformance harness.

Required backend flags are `-std=c11 -pedantic-errors -Wall -Wextra -Werror
-Werror=attributes -Werror=inline -fno-lto`. Verify both `-O0` and `-O2`.
LTO is not needed for correctness. Native entries use `AMA_NATIVE_ENTRY`, defined
as `static inline` plus GCC `always_inline`; all entry uses must be direct and
entry addresses must not escape. GCC documents a required diagnostic for failed
direct-call inlining, and explicitly does not provide that guarantee for indirect
calls: [GCC 15.2 function attributes](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Common-Function-Attributes.html).
The separate inlining task supplies executable positive/negative verification.
This is not a claim that plain inline or optimizer heuristics suffice.

## Representation and headers

Use native C layout, not packed structs or a serialized cross-platform ABI.
Include `<stdint.h>` for status/generation identifiers, `<stddef.h>` for lengths
and alignment, `<stdbool.h>` for condition transport, and representation headers
explicitly required by participating modules. No `wchar_t` transport is used.

`AmaByteView` in `src/native/abi/bytes.h` is `{const unsigned char *data;
size_t length;}`. It borrows immutable UTF-8 bytes for the constructor operation;
NULL requires zero length. Embedded zeros are data; no terminator is required.
UTF-8 validation belongs to source handling, not guessing in native constructors.

Native payload storage must satisfy `_Alignof(T)` and `sizeof(T)`, including
extended alignment. Typed aggregate declarations preserve alignment. Dynamic
storage must use checked size/rounding arithmetic and a suitable aligned allocator;
zero bytes do not imply construction. Pool allocation comes later, not a per-object
allocation mandate. The layout test checks 64-byte aligned native storage.

## Exact lifecycle and conversion shapes

For each module payload type `T`, generated typed uses require these signatures:

```c
AmaStatus AMA_NATIVE(initialize)(T *destination);
AmaStatus AMA_NATIVE(destroy)(T *state);
AmaStatus AMA_NATIVE(from_literal)(T *destination, AmaByteView literal);
AmaStatus AMA_NATIVE(clone)(T *destination, const T *source);
AmaStatus AMA_NATIVE(to_condition)(const T *state, bool *result);
AmaStatus AMA_NATIVE(to_index)(const T *state, size_t *result);
```

Only `+`/`-` are mandatory. Constructors receive fresh aligned destination storage;
clone source is live and distinct. A constructor failure cleans its own partial
resources. Conversions do not modify state; consume their outputs only on success.
Destroy is attempted once on a live payload and never retried after failure.
Native hooks own native resources, not generated owned Ama fields.

These readable names illustrate signatures, not reserved library identifiers.
The `.abi` binding selects the actual helper; generated type/adapter identities
follow full qualified module hashes. The `.abi` parser preserves module-local
identifiers; the [namespace emitter](native-namespaces.md) qualifies typed uses.
The full project C generator remains later work. C harnesses check typed uses,
not inferred capabilities or arbitrary native correctness.

## Exact native entry shape

For generated module-specific input/output aggregate types `P` and `R`:

```c
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(
    T *state, const P *parameters, R *results,
    const AmaNativeServices *services, AmaStatus *status);
```

All pointers above are valid for the call and borrowed; entries return normally.
`state` points only to that instance's native payload. Inputs are already bound
into the callee's interface storage before entry. Each owned input has a
`const U *` typed view of that bound storage, never an alias of the caller's
original owned source. Each owned output has a `U *` view of initialized target
result storage. Native code updates live outputs using their resource contracts;
it does not construct over live resources or own the enclosing Ama field.

Reference inputs carry checked identities, including empty/stale values without
implicit dereferencing. An operation that requests payload access must resolve
and acquire the target through a generated module-typed view; handle copying
alone does not acquire a target. Reference outputs designate module-typed
handle storage through generated assignment helpers; a copied stale identity
does not revive its target. List views designate
typed collection access, not an opaque native buffer containing hidden handles.
Generated declarations provide those module-specific view types; no universal
`void *` or unchecked word casts replace them. Actual reference/list adapters
are implemented with the later pool/type pipeline, not asserted by the scalar
ABI harness. Native code may not forge identities, retain views, or change service
metadata. Every view obeys the runtime acquisition contract.

`AmaNativeServices` supplies a read-only value snapshot of the immediate caller's
refusal status; it has no dispatcher, instance pointers, return links or mutable
runtime metadata. `status` is a separate technical output initialized to success
before entry. The entry assigns a status on every return; application failures
remain ordinary fields of `R`. The later metadata task supplies concrete service
and runtime bookkeeping declarations.

## Scope, laws and evidence

Layout follows the C object/alignment rules under the pinned toolchain. A byte
view preserves its explicit length independent of contents; its construction is
constant work and borrows storage. Typed helper assignments/calls establish exact
signature compatibility; they do not prove handwritten C obeys lifetime rules.
Native violations remain unsupported native code under CABI-05.

Run `make -C tests/unit/native/abi/layout`. Independent headers compile as C11;
byte-length and aligned-storage tests check the selected representation. Subsequent
section-2 tasks extend this contract with status, real hooks, lifecycle and
control-flow evidence. Nothing here is a completed Ama backend claim.
