# Native status contract

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

This implements ERROR-03 through ERROR-05 and PROTO-09; source boundaries are
`src/native/status/{status,codes,index,library}.h`. It is transport and classification,
not a runtime exception, source diagnostic subsystem or recovery dispatcher.

`AmaStatus` is the exact code/message/length struct required by the specification.
All-zero is success. Message views borrow immutable program-lifetime UTF-8 with
explicit length. NULL requires zero length. The constructor's caller guarantees
encoding and lifetime; the view validator checks only the NULL/length structural
invariant, not pointer validity or UTF-8. No operation allocates or frees storage.

Runtime code zero is success; codes 1 through 13 have the distinct names in
`codes.h`. Values 14 through 65535 are reserved for future runtime service reasons,
not available for libraries. Invalid-index conversion is exactly code 1; bounds
refusal uses its separate service code. The conversion classifier maps zero to
Converted, one to Refused, all other codes to Failed. Only indexed acquisition
uses that conversion refusal to select `:`; other operation contexts keep their
own refusal/technical distinction. Message text never controls classification.

Library codes have nonzero upper and lower 16-bit fields. Before emission, the
compiler must sort full error-provider identities and assign unique namespaces
1 through 65535; exceeding that limit is an explicit compilation error. Each
provider publishes unique nonzero reason IDs within its namespace. No hashes,
truncation or duplicate IDs may allocate namespaces. Provider registration is a
later dependency-registry task, not implemented by the encoding helper.

`ama_library_status_code` accepts 16-bit namespace/reason values and a valid output
pointer. Zero fields return InvalidState without changing output. Otherwise code
is `namespace * 65536 + reason`. Quotient and remainder uniquely recover both
fields, proving injectivity; upper nonzero bits prove disjointness from all runtime
codes. The maximum pair maps to UINT32_MAX without overflow. These statements
cover the complete declared domain, not only tested pairs.

Constructing a view and classifying/encoding status take constant time/space.
Inputs are borrowed; helpers retain nothing beyond explicit returned copies.
A copied status has independent scalar fields and shares only immutable message
storage. Conversion outputs must not be read after nonzero status. The classifier
itself cannot change outputs because it receives only a status value.

Run `make -C tests/unit/native/abi/status`. Tests cover zero initialization,
nonterminated messages, invalid NULL/length, text-independent classification,
zero-field refusal with unchanged output, maximum values and exhaustive inversion
on namespace/reason pairs 1 through 8. This bounded enumeration checks the code;
the quotient/remainder argument covers all 16-bit pairs.
