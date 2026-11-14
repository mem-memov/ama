# Canonical integer library and explicit standard roots

T085 publishes the maintained `..integer` module and validates its actual
standard-root native interface. [LIB-01](../../spec/language/library.md#lib-01)
keeps mathematical data interpretation in a module; the compiler sees nominal
types and explicitly declared protocols. [PROTO-03](../../spec/implementation/native-interface.md#proto-03)
additionally requires a typed constructor for every host list length.

## Public value and supported range

[The module](../../stdlib/modules/integer) lives at `stdlib/modules/integer`.
Its shared `zero` and `one` constants have the same nominal integer type.
The native entry is empty and preserves the mathematical value. The six-record
[ABI table](../../stdlib/modules/integer.abi) declares default zero, destruction,
decimal literal construction, independent cloning, nonzero truth and index
conversion. `from_size` is header-declared and is not a seventh ABI record.

Integer literals have an optional single ASCII minus followed by one or more
ASCII decimal digits. Leading zeroes are allowed; negative zero becomes ordinary
zero. Whitespace, plus, embedded NUL and nondecimal bytes are invalid. Parsing
uses the supplied byte length rather than a C terminator or locale. Magnitudes
are arbitrary precision within available allocation/representable byte lengths;
they are not narrowed to a machine signed integer. Allocation/size failures are
technical constructor failures, distinct from malformed literal data.

Truth is mathematical nonzero, including negative values and magnitudes larger
than a host size. Checked indexing accepts exactly nonnegative integers no larger
than `SIZE_MAX`; other valid integers return `AMA_INVALID_INDEX` and leave the
output untouched. The complete list-length range is therefore 0..`SIZE_MAX`,
not merely the positive range of a signed machine type. Actual allocations may
fail earlier and follow the normal technical-failure contract.

An example with an already configured standard root:

```text
@!
items[]
..integer
 |7
items[]
 append !
 length !
  > count
count
?
 count !
```

`count` is inferred as `..integer`; the length result owns an independent value.
Calling the scalar does not change it. A project module called `.integer` has
its own type and does not gain either the fixed bridge or implicit compatibility.

## Bounded scalar types and their consumers

The existing scalar interfaces now also resolve through explicit standard roots:
`..std.index` is a nonnegative `size_t` value, and `..std.condition` stores zero
or one. Both declare literal/default/clone/destruction/truth/index protocols.
Index literals reject malformed and overflowing decimal values before publishing
state; condition additionally rejects values above one. Their shared constants
use their own full standard identities. Neither type implicitly converts to
`..integer` or to the other scalar.

The checked `..std.index.next`, `..std.index.add` and `..std.index.before`
consumers have matching requalified native contexts and declaration paths.
Their arithmetic/comparison observations, defaults, named input ordering and
bounded overflow checks run in the standard-root library composition suite.
[T087](library-compositions.md) aligns bounded expected failures and adds exact
canonical arithmetic without changing normal false predicates.

## Representation, ownership and exact algorithms

The [representation header](../../stdlib/modules/integer.h) exposes signed
little-endian base-256 magnitude limbs to typed native consumers. Live values
own at least one limb; a canonical zero has one zero limb and a nonnegative sign.
This is a library layout, never a compiler-side cast or universal numeric ABI.
Every successful constructor creates one allocation, clone owns independent
bytes and destroy consumes that allocation once. Failed constructors release
private prefixes and leave the payload non-live.

[Storage construction](../../stdlib/modules/integer+/native/storage.c) decomposes
`size_t` by unsigned quotient/remainder. The quotient strictly decreases until
zero, using at most `sizeof(size_t)` limbs on the checked eight-bit-byte ABI.
Reconstructing with the index protocol is its inverse. It invokes neither the
default nor literal constructor and needs no decimal buffer or signed narrowing.

[Literal decoding](../../stdlib/modules/integer+/native/literal.c) allocates at
most one limb per digit and folds decimal digits with base-256 multiply/add.
For N digits, `10^N < 256^N`, so N limbs suffice. Each local intermediate is at
most 2559, safely within the selected C unsigned range. Normalization removes
leading zero limbs and canonicalizes sign. Work is O(N²) in this simple complete
fold, with O(N) allocation; no optimized complexity is claimed.

[Index conversion](../../stdlib/modules/integer+/native/conversion.c) folds limbs
from most significant to least. Before `value * 256 + limb`, it checks
`value <= (SIZE_MAX - limb) / 256`; output is published only after every step
succeeds. Conversion costs O(L) for L limbs. Default/clone/destruction use only
native resources and never hide Ama handles or retain borrowed views.

## Explicit command-line authority

`--standard-root DIR` supplies the standard root family to check, status, native
and executable-emission requests. Ordinary source roots and `--root DIR` remain
project/import roots. Both kinds retain argument order and the request's explicit
exclusions; physical folder names never select a kind. Existing catalog collision
and dependency rules apply to the resulting exact root table.

For a project using this library:

```sh
./ama --check project .main --standard-root stdlib/modules
./ama --emit project .main --standard-root stdlib/modules --out output --sdk src --compile
```

No library directory is inferred from the working directory. The frontend API
uses the same explicit `MODULE_ROOT_STANDARD` record. Missing configuration fails
resolution rather than binding a similarly named project module.

## Evidence and file coverage

[The input inventory](documentation/ready.json) maps every maintained file to
named tests, including standalone representation-header compilation.

- `make -C tests/unit/native/abi/integer` compiles the actual library components
  under counted allocation. It validates all six signatures, exact standard
  namespace, zero/one/`SIZE_MAX`, every base-256 boundary and adjacent values,
  clone isolation, truth, negative/overflow index refusal, malformed counted
  literals and allocation failure in all constructor families.
- `make -C tests/unit/codegen/scalar-library` compiles actual standard companions
  through the complete frontend/planner/native toolchain. It checks list
  positions/length, shared constants, ordinary copies, unchanged scalar entry,
  nonzero truth, index refusals, malformed construction and nominal separation
  from a project `.integer` with no fixed bridge. Six directed standard-scalar
  pairs also reject implicit conversion even when both literals denote one.
- `make -C tests/unit/codegen/library` checks the bounded scalar constants,
  malformed/range refusal and ordered successor/addition/comparison consumers.
- `make -C tests/unit/driver/options` checks exact spelling, command applicability,
  missing values, explicit root kinds, exclusions and argument order.
- `make -C tests/integration/standard-integer` runs real CLI check/verified compile
  and a maintained native observer. Without the root it fails resolution; with
  the root it prints list lengths three and zero through the real integer protocol.

Native component and generated-program ASan/UBSan, the full strict suite,
coverage/header checks, local links and reproducible Doxygen complete acceptance.
Arithmetic and failure/resource compositions now have their own
[complete acceptance](library-compositions.md), without extra native protocol
slots or parser-level numeric assumptions.
