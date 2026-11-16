# Full identities and generated names

Boundary: `src/source/identity/`, implementing NAME-01/02. The registry owns original
encodings, readable source names and generated C names. Declaration spans borrow
source-manager storage. No source spelling is changed to accommodate C.

Encoding version 2, in order:

1. Eight bytes: `41 4d 41 2d 49 44 00 02` (`AMA-ID`, zero, version).
2. One category byte: type=1, constant=2, state member=3, interface member=4,
   label=5, runtime metadata=6, native namespace=7.
3. One root-kind byte: project/import=1, standard=2. It is separate from all components.
4. Unsigned big-endian 64-bit module component count, then each component's
   unsigned big-endian 64-bit byte length followed by its original UTF-8 bytes.
5. Unsigned big-endian 64-bit member/discriminator count and the same encoding
   for each member/discriminator.

The module is a valid absolute single/double-dot path with complete Unicode
multiword components. Strip exactly its root marker before counting components;
Standard never inserts an empty component. The fixed fields consume 26 bytes,
and each module/member component adds eight length bytes and its original data. Member/discriminator strings are
nonempty valid UTF-8, allowing later lowering to name internal positions explicitly
without pretending they are user declarations. Counts, lengths and additions are
checked against representable storage and SHA-256's byte-length domain. No native
size_t representation or host endianness enters the bytes.

Decoding this encoding uniquely reconstructs version, category, root kind and both sequences:
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
allocation and [generated declarations](project-emission.md) are separate
consumers; this registry does not truncate a digest into a handle or replace
generation/type checks. [Context admission](execution-contexts.md) checks the
origin of those runtime identities before local storage is accessed.

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
Every category shares the same complete root-aware format and full-key checks.


T145 documents each encoding/hash/registry operation and its borrowed/owned
lifetimes next to the code. T017 connects the complete root-kind/multiword model
to version-two measured encoding. SHA-256 is only a candidate index; forced
equal-digest/unequal-key tests remain the exact collision authority.

## T017 measurement, display ownership and fixed vectors

Encoding is split into numeric measurement, component serialization, display copy
and publication. `identity_encoding_part_size` proves each eight-byte prefix plus
payload fits size_t and SHA-256's maximum byte count floor(UINT64_MAX/8) before
committing total. Counts are bounded by their measured prefix bytes. Display
length and its convenience terminator are checked separately. Invalid metadata
or limit failure changes no measured output and acquires no storage.

The encoder allocates one binary key and one exact display string. The display
copies the original path and each `::`/member part once; it does not allocate and
retain every growing concatenation prefix. Key/hash/display work and private
storage are O(k) for the complete input key. Repeated registration may still
retain its temporary key/display scratch until registry destruction, as stated
above; this is one compiler owner's transaction arena, not a process-wide cache.
Only a newly published identity adds its persistent Name record.

Frozen independent vectors now pass through public `identity_register`, then
compare every encoded byte and the entire C prefix. They include project and
standard roots for the same multiword Cyrillic components; their binary keys
differ only at the root byte, while their full SHA prefixes remain distinct.
Both discovery orders reuse identical registry records. Other cases separate all
seven categories, spaces from dot-separated components, case and NFC/NFD bytes.
Different member sequences can have identical `::` display text but must retain
different binary keys and generated names. No display string controls equality.

Every measured allocation failure of a root-aware multi-member registration
preserves earlier records and caller outputs; borrowed input mutation after
success cannot change the copied key/display. Size-endpoint cases prove numeric
bounds without allocating enormous arrays, and a 0/1/32-member case bounds
encoding at two private allocations rather than one per display prefix. Raw
insertion also rejects invalid category or missing key/display/digest/output
shape before category indexing or Name publication. Existing forced collision
cases continue comparing complete unequal keys instead of assuming SHA injectivity.

Namespace/provider composition uses these same current vectors. The maintained
integer ABI harness now checks the actual `..integer` namespace; the generated
fixed bridge is still T055. Maintained C companion references were updated through
an exact mapping from each validated scaffold key to its current version-two
name. No names were inferred from a basename, shortened digest or approximate C
parser. Real generated compilation validates header/body/type agreement afterward.
Public compiled-component transport and external symbol qualification remain
the export roadmap; these are compiler-internal component-local identity names.
