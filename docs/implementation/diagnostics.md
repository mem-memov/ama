# Structured compiler diagnostics

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

Boundary: `src/diagnostics/record/diagnostic.h`, `record/` and `adapters/`.
Implements COMP-04 source/type context and preserves NAME-03 original names.
It does not introduce runtime exceptions or a language parser.

A diagnostic is a borrowed, allocation-free record: stable code, required message,
optional original subject, primary location, cause, expected construct, precise
help and zero or more related locations with roles/names. A location holds a
half-open source byte span or a physical path when no source could be loaded.
An entirely absent location is allowed for failures before input acquisition.
A source span and an explicit path must agree. Empty/EOF spans are valid.

Text, source-manager records and caller-owned related arrays must outlive all
consumers. There is no mutable global last-error state and no heap allocation to
report out-of-memory. The future compiler's diagnostic collection may own storage;
this view representation does not silently assume that ownership.

Validation checks the finite code domain, required text, all span bounds/path
consistency and related-array consistency. Induction over related entries proves
all exposed locations satisfy the same location invariant. It does not interpret
or guess a correction from source spelling. Syntax/type codes are consumed by later stages; these record tests alone do not
certify their parsing/type-checking semantics.

Module adaptation maps each explicit ModuleStatus to its own message/cause;
ambiguous/aliased sources retain both physical paths. A malformed UTF-8 issue
selects the lead byte of the failing sequence in the original retained source.
Identity-collision adaptation retains both qualified names and declaration spans,
never only a digest. Adapters validate a local complete record before publishing
it and borrow at most one caller-provided related slot. Invalid/no-error inputs
publish nothing. Structural data is independent of any terminal output format.

Validation costs O(related count + compared path bytes). Adaptation is constant
work apart from validation, with constant auxiliary storage. Allocation failures
already recorded by a source component require no additional allocation to adapt.

Evidence: `make -C tests/unit/diagnostics/record` passed C11/pedantic/Werror.
Tests exercise multiple declaration/type notes, original international names,
span/path failures, every module failure category and collision context. Rendering
and composed filesystem-to-message tests are documented in [rendering](diagnostic-rendering.md).


T145 adds Doxygen contracts for every record/adapter/rendering boundary and links
the exact source-coordinate/collision/diagnostic rules in source comments. Records
and renderers remain separate owners/views; declarations explain which storage
must outlive consumers and when output may be partial. Header compilation and
actual source/identity-to-diagnostic compositions check those handoffs; file and
interface reference pages come from Doxygen, not this narrative.
