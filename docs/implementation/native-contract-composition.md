# Native declarations, error contexts and the fixed size bridge

T055 composes current checked source identities with reusable native declarations
under [CABI-06](../../spec/implementation/native-interface.md#cabi-06),
[NAME-05](../../spec/implementation/native-interface.md#name-05),
[ERROR-04](../../spec/implementation/diagnostics.md#error-04),
[PROTO-03](../../spec/implementation/native-interface.md#proto-03) and
[TEST-07](../../spec/implementation/tools-and-conformance.md#test-07).
It extends [native scaffolding](native-scaffolding.md) and
[native namespace planning](native-namespaces.md). Publication remains a separate
owned-output transaction; an implementation request does not silently authorize
replacing handwritten companions.

## Plan complete provider identities before emitting either context

The scaffold registers every current checked type/member/native prefix through
the full root-aware identity registry. Only then does
[provider integration](../../src/native/scaffold/model/providers.c) collect native
module prefixes and run the existing exact full-key provider planner. Sorted
complete encodings determine nonzero component-local namespaces, independently of
module discovery, output directory or the artifact type-table order. Every native
model retains its own exact assignment; nonnative types receive none.

[Representation headers](../../src/native/scaffold/emit/types.c) and
[body contexts](../../src/native/scaffold/emit/context.c) emit the same planned
`AMA_NATIVE_ERROR` namespace alongside `AMA_NATIVE`. Its SDK implementation checks
the native status contract and evaluates each argument once. End contexts remove
both macros, as well as positional field/input/output aliases. Companions retain
their original C declarations; no C parser, helper-name inference or rewriting
attempts to repair authored code. The SDK library-status header is checked as a
required input before any model or artifact is emitted.

The provider table owns scratch and borrows registry names; teardown destroys it
before the identity registry. Any planning/emission allocation failure leaves one
safely destructible prefix and no ready/published scaffold. The fixed planner
rejects duplicate/malformed full keys and more than 65535 participating native
providers; it never recycles a namespace or falls back to a truncated hash.
For N native providers, inventory/storage is O(N), existing heap ordering is
O(N log N) full-key comparisons, and exact lookup adds its documented key-byte
cost. Complete source key lengths remain part of comparison cost.

## Bind only the exact standard integer contract

[Bridge recognition](../../src/native/scaffold/model/bridge.c) compares the entire
canonical checked native identity `..integer`. A project `.integer`, standard
`..nested.integer` or differently cased name gains no bridge contract from its
basename or payload representation. This is the standard module's fixed contract,
including when it is emitted directly rather than reached through a list method.

After including that module's authored representation header,
[bridge emission](../../src/native/scaffold/emit/bridge.c) defines a generated
function type returning `AmaStatus` with the actual State pointer and `size_t`.
A C11 `_Generic` static assertion checks the already declared qualified
`from_size`; a const typed function-pointer alias binds it for generated consumers.
No generated replacement prototype can hide a missing or wrong authored
header declaration. The signature type/alias uses the distinct generated type
namespace and reserves no additional authored local C name.

The bridge remains separate from the six ABI protocol slots. Reusing its fixed
name for a standard integer ABI slot is incompatible and diagnosed before
emission. A newly requested standard representation header receives its required
prototype; a generated body template receives an honest unwritten helper returning
native failure. Existing authored headers/bodies remain untouched without explicit
overwrite. The template does not pretend to construct a size value or add an
ABI record. Actual list-result invocation remains the later execution adapter's
responsibility.

Recognition is exact constant-length identity comparison, and bridge output adds
constant declarations per participating integer type. C checks actual type
compatibility and ordinary linking resolves its implementation. No native State
layout is guessed and no decimal-text conversion is substituted for the bridge.

## Executable evidence

Run `make -C tests/unit/native/scaffold/contracts`. Actual current-source modules
include root-distinct Unicode/multiword names and a legal cycle of reference
interfaces. Their native local State/reason/helper spellings are identical.
Complete-key provider order deliberately differs from artifact lexical order.
Each interface compiles as a first include, with temporary macro cleanup checked.
Reverse header inclusion and forward body inclusion produce matching distinct
header/body library codes; side-effect counters verify single argument evaluation.
Frozen identity vectors remain independent producer regressions, while this
composition obtains its names from loaded source and the public current producer.

The exact standard integer bridge executes zero and `SIZE_MAX` into separate
fresh State values through the generated typed alias, then destroys both.
Project/nested/case variants compile without that contract. Missing, wrong-return
and const-destination prototypes fail real C compilation; ABI-name reuse fails
model validation. Explicit creation emits a compilable fail-first prototype/body
pair while preserving existing bodies and six-slot ABI metadata. Unique output
owners keep independent authoring contexts separate rather than overwriting one
module's artifacts with another's.

Every provider/model/declaration allocation is failed independently, including
failure during provider planning and after a complete plan. A partial SDK missing
the newly required library-status header fails before model/artifact creation.
Existing declaration/ownership/publication/tooling suites continue to verify
standalone interfaces, compatible signatures, untouched inputs, reference cycles
and atomic output ownership. Full strict tests, sanitizer checks, per-file/header
coverage and reproducible Doxygen generation complete the evidence. The
[registry](documentation/ready.json) identifies every new/reused source and case.
