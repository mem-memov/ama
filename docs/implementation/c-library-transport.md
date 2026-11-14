# Typed C inputs and independent result snapshots

The checked C boundary can now prepare every declared input profile and read
retained results into independently owned public snapshots. Context and instance
[factories](c-library-lifecycle.md) still construct declared state without entering
a body. [Public entry/resume execution](c-library-execution.md) consumes these
prepared recipes and snapshots under the same executor lease. Static/shared
artifact packaging follows in T128. These internal generators do not introduce a
library-build CLI command or claim that a header alone is a compiled component.

This implements the transport part of
[EMBED-03](../../spec/implementation/c-library.md#embed-03) and
[EMBED-04](../../spec/implementation/c-library.md#embed-04). The
[export configuration](c-library-exports.md) selects profiles explicitly. Nominal
C types, the complete ABI bootstrap and bounded input shapes establish admission;
typed contents are decoded only after admission.

## From a C borrow to fresh Ama state

`library_codecs_write` borrows the checked export model and the matching project
scaffold. It compares the frontend owner exactly and resolves each nominal schema
type in that scaffold's descriptor inventory before emitting anything. Schema
order need not match dependency order: scalar/list prototypes precede definitions.
An unrecognized type or another build's scaffold produces no partial definitions.

For each export, the generator creates an input-shape predicate and a typed
argument builder. A NULL argument selects the declared default. A supplied
argument becomes an arena-owned `AmaFieldRecipe`, carried by the private
`AMA_SOURCE_PREPARED` entry-input descriptor. The builder and all recipes borrow
C input buffers only through synchronous entry preparation. No recipe pointer or
C borrow is stored in a native payload or continuation. After successful
construction, the caller can change or free its original input buffers.

`AMA_CONSTRUCT_TRANSPORT` selects a compiler-owned typed construction callback
instead of a default payload constructor followed by replacement. It is an
internal runtime recipe, not Ama syntax or a seventh `.abi` capability. The usual
lifecycle ledger still constructs owned fields first, records a native destroy
credit only when appropriate, and cleans a failed prefix. A malformed native
zero-code status receives its cleanup credit before transport validation reports
failure. Earlier field commits follow normal entry order and are not rolled back
as a group; an admitted technical failure terminates the context.

| Profile | Construction and independent snapshot representation |
|---|---|
| `integer` | `..integer.from_magnitude` validates canonical big-endian magnitude and reverses it into fresh arbitrary-precision limbs. Encoding reverses live limbs directly into owned snapshot bytes. No host-integer narrowing or decimal formatting/parsing occurs. |
| `text` | The declared typed text literal constructor validates and copies counted UTF-8. Encoding reads the documented text view and copies bytes into a separate snapshot allocation, including embedded NUL. |
| `condition` | The selected native literal protocol receives exactly one byte, `0` or `1`. Encoding uses the declared condition protocol and publishes a Boolean transport. |
| `failure` | A child text recipe constructs the owned description before the native discriminator. Encoding checks the condition and reads that exact declared text field. False requires an empty description; true can carry an empty or nonempty description. These remain one result. |
| `list` | The builder creates an ordered constructor array of the declared scalar schema. The encoder builds an independent ordered array of scalar owners, including the nested bytes of failure elements. |

Integers reject empty magnitudes, leading zero octets and negative zero rather
than guessing an alternative interpretation. The one-octet positive zero is
canonical. Lists support all four scalar element profiles; list-of-list and
reference transports remain build-time errors under the selected edition.

## Admission and semantic errors

Shape predicates inspect only the declared C representation: nonnull/count rules,
Boolean discriminants, representable array sizes and recursively valid element
shapes. They do not parse UTF-8 or interpret numeric bytes. Invalid shape,
nonempty output slots and aliased owner slots are rejected before accessing
mutable Ama state. The public C declarations reject incompatible schema pointers
at C compilation; forced incompatible casts and invalid C pointer lifetimes are
outside their contract.

Malformed UTF-8, noncanonical integer contents and false/nonempty failure records
reach the selected typed decoder after admission and therefore cause technical
context failure. Required internal recipe/snapshot allocation failures do the
same. They do not become recoverable busy refusals or successful application
failure values. Status codes determine control flow; diagnostic wording does not.

## Snapshot ownership

`library_snapshots_write` defines the public opaque `Schema_Value` owners and
implements view, explicit clone and consuming release. Scalar byte buffers belong
to the snapshot, not to an Ama pool, native text backing or caller input. Lists
own a contiguous borrowed-view array plus a private array of scalar owners. A
completed element is credited only after its encoder/copy succeeds, so cleanup
walks exactly the completed prefix in reverse.

Each successful clone owns separate storage. A view borrows immutable snapshot
data only until its own owner is released; it remains usable after the original
Ama instance/context and other independent snapshots are disposed. Releasing an
initialized NULL pointer is a no-op; releasing a live owner leaves it NULL.
A C shallow pointer/struct assignment does not create another releasing owner.
As with diagnostics, component code must remain loaded until all owners are
released, under [EMBED-06](../../spec/implementation/c-library.md#embed-06).

The manifest's schema encoder/decoder identifiers name generated **private typed
codec declarations**, whose signatures include runtime views and recipe owners.
They are not raw C consumer entry points or promises of dynamic-symbol lookup.
Public callers use typed export operations and checked snapshot ownership
functions. No private runtime header or native State layout enters the public
consumer header.

## Retained reads and atomic publication

A selected public retained read checks initialized-empty, distinct output slots
and the exact component/context/export-bound instance. One executor lease covers
availability checks, snapshot construction and publication. It refuses a busy
root or requested owned result before encoding. NULL destinations skip results,
so an unrelated unavailable result need not be read.

All requested owned trees are preflighted with temporary protections. Those
protections are restored before typed encoding; the held context lease prevents
intervening Ama execution. Each snapshot remains unpublished until every selected
encoder succeeds. Success publishes all requested owners together. Failure frees
the completed snapshot prefix and leaves every output empty before terminal
context cleanup. A refusal preserves retained values and saved positions.

A retained read never enters a body, resets an interface, or consumes a saved
position. Diagnostic reporting attaches an admitted failure to the selected Ama
target/declaration before cleanup. Fresh-root factory failures clear stale caller
trace state and retain the current construction site, rather than reporting an
unrelated previously constructed constant or executed root.

## Bounds, invariants and executable evidence

For N transported scalar/list elements and B owned bytes, recipe storage and
independent snapshot storage take O(N + B) space. Integer reversal and byte copy
take O(B); list traversal visits each element in declaration order. Snapshot list
release visits exactly the completed prefix. Public buffer/array arithmetic is
checked before multiplication; internal recipe allocation checks its own size
product. Ownership never depends on unbounded retry, guessed capacity or parsing
a diagnostic string. Native text can share its immutable backing inside Ama;
wire snapshots deliberately own separate bytes for independent host lifetimes.

The following suites cover different boundaries:

- `make -C tests/unit/library/snapshots` compiles independent public consumers for
  every wire profile, verifies explicit clones after releasing their sources and
  fails every measured list-clone allocation while checking exact live ownership.
- `make -C tests/unit/library/transport` generates real standard-native descriptors,
  codecs and retained-read implementations. Its separate public-only C consumer
  verifies input-buffer independence, all scalar/list profiles, repeated reads,
  failure discriminator/description transitions, skipped/busy results, output alias rejection,
  continuation preservation and snapshots after context disposal. It fails every
  measured real read allocation and requires all unpublished slots and live
  allocation counts to be empty after cleanup. Test-only probes exercise the
  generated input builder through ordinary entry preparation and copy the list
  through the ordinary runtime transfer; they do not replace a public executor.
- `make -C tests/unit/runtime/call-preparation` verifies selected transport
  constructors, ordered defaults, list inputs, old-field preservation and cleanup
  credits for malformed native outcomes. The lifecycle reference model includes
  the transport constructor in its exhaustive finite transition/failure cases.
- `make -C tests/unit/native/abi/integer` checks both signs and every magnitude
  length through 257 octets, canonical zero, invalid encodings and allocation
  failure without default/literal conversion or retained input borrows.

Sanitizer builds of the two public transport suites also compile their generated
C consumers/components with ASan/UBSan. Counted native/runtime/wire allocation
checks remain active even where LeakSanitizer is unavailable.
