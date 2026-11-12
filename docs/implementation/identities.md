# Full identities and generated names

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/source/identity/`, implementing NAME-01/02. The registry owns original
encodings, readable source names and generated C names. Declaration spans borrow
source-manager storage. No source spelling is changed to accommodate C.

Encoding version 1, in order:

1. Eight bytes: `41 4d 41 2d 49 44 00 01` (`AMA-ID`, zero, version).
2. One category byte: type=1, constant=2, state member=3, interface member=4,
   label=5, runtime metadata=6, native namespace=7.
3. Unsigned big-endian 64-bit module component count, then each component's
   unsigned big-endian 64-bit byte length followed by its original UTF-8 bytes.
4. Unsigned big-endian 64-bit member/discriminator count and the same encoding
   for each member/discriminator.

The module is a valid dotted Unicode path. Member/discriminator strings are
nonempty valid UTF-8, allowing later lowering to name internal positions explicitly
without pretending they are user declarations. Counts, lengths and additions are
checked against representable storage and SHA-256's byte-length domain. No native
size_t representation or host endianness enters the bytes.

Decoding this encoding uniquely reconstructs version, category and both sequences:
the next fixed-width length selects exactly its payload, so induction on counts
proves injectivity before hashing. Category and module/member boundaries cannot
collapse, unlike concatenating names or replacing punctuation with underscores.
Source display names use `::` between members; the full encoding, not this display
string, is the identity key.

Digest: SHA-256, 256 bits, following
[FIPS 180-4](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf), sections
5.1.1, 5.3.3 and 6.2. Padding appends the one bit, zeros and the big-endian bit
length; expansion/compression follow unsigned 32-bit modular arithmetic. The code
preserves the eight-word round state and folds complete blocks followed by one or
two padding blocks. This is not a cryptographic certification claim.

C names are `ama_` + category code (`t/c/s/i/l/r/n`) + `_` + 64 lowercase hex digits.
The verified GCC backend supports their full length. Names are deterministic across
physical roots, traversal order and host byte order. Runtime numeric type-ID
allocation and generated declarations remain later compiler tasks; this registry
does not truncate a digest into a handle or replace generation/type checks.

Hash injectivity is neither assumed nor needed. On an equal digest, insertion
compares the full encoding. Equal encodings return the existing record; unequal
encodings reject with IDENTITY_COLLISION and retain the previous source name/span.
Only a complete successful record is linked. Failure leaves the caller output and
existing record list unchanged. Private arena allocations from rejected/repeated
registration live until registry destruction. Registration is not a global cache.

For key size k and r records, encoding/hash cost O(k); registry lookup costs
O(r + compared key bytes). Storage is the total size of attempted encodings and
successful records. SHA processing uses constant auxiliary space. Future exact
indexes may improve registry lookup without assuming away collisions.

Evidence: `make -C tests/unit/source/identity` passed C11/pedantic/Werror. Tests
cover independent hashlib block/padding vectors, standard `abc`, a fixed Chinese
qualified-identity vector, categories, ambiguous concatenations, case and Unicode
normalization distinctions, duplicate registration, invalid paths and each
allocation failure. The internal insertion boundary is tested with deliberately
equal digests and distinct real encodings, proving the rejection branch executes
without searching for a SHA-256 collision. The public algorithm is fixed, with no
user-selectable hash or test-only collision bypass.

Native namespace identities have no member components. The registry rejects
member-bearing namespace keys, retains the category on successful records, and
provides the collision-checked prefix used by [native C naming](native-namespaces.md).
Existing categories retain their encoding and stable vectors.
