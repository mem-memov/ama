# Native status contract

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
refusal uses its separate service code. After checking transport shape, the conversion classifier maps zero to
Converted, one to Refused, all other codes to Failed. Only indexed acquisition
uses that conversion refusal to select `:`; other operation contexts keep their
own refusal/technical distinction. Message text never controls classification.

Library codes have nonzero upper and lower 16-bit fields. Before emission, the
compiler must sort full error-provider identities and assign unique namespaces
1 through 65535; exceeding that limit is an explicit compilation error. Each
provider publishes unique nonzero reason IDs within its namespace. No hashes,
truncation or duplicate IDs may allocate namespaces. T010 implements the isolated [complete-key provider planner](native-namespaces.md#deterministic-native-error-providers);
T017 supplies current full-root identities; [T055](native-contract-composition.md) connects dependency emission. The encoding
helper itself performs no registration.

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

## Checked status and caller-refusal boundaries

T007 adds `ama_status_has_valid_code` and `ama_status_checked`. The allocated
core domain is exactly 0 through 13; a library encoding requires both nonzero
16-bit fields. Reserved core values and zero provider/reason fields are rejected.
This encoding check does not establish provider membership: compiler dependency
planning must still assign collision-free provider namespaces before emission.
The [native namespace roadmap](../../TODO.md) retains that integration work.

A checked status preserves every valid field, including nonterminated immutable
message bytes. An inconsistent NULL/length pair or unallocated code becomes
InvalidState with a static immutable fallback. No allocator or message read is
needed, and an invalid zero-code status cannot masquerade as successful conversion.
Pointer extent, UTF-8 and program lifetime remain explicit native C preconditions;
a representation check cannot prove that borrowed C memory outlives execution.

The index classifier checks transport before deciding whether a conversion
succeeded, explicitly refused or failed technically. The actual selection and
acquisition path now marks precisely a valid InvalidIndex result as a refusal.
Other decoder errors, even IndexBounds, remain technical at this boundary; bounds
produced by selecting from the target list are a separate refusal origin. A failed
conversion's output is never used, even if the decoder wrote SIZE_MAX before
returning failure. No target is acquired or executed on these paths.

Recording an acquisition refusal checks its message shape before replacing the
initiating caller's snapshot. Invalid records, success and technical codes leave
all saved fields unchanged. Nested callees own independent snapshots; native readers
copy code/length and borrow only immutable bytes. Ordinary successful access does
not clear the snapshot. Host structural outcomes reject malformed status encodings.
Context reporters normalize status before publishing terminal diagnostics, and the
streaming program reporter normalizes before reading optional message bytes.

This foundation supplies checked transport and the refusal/index/report consumers.
[T009](native-protocols.md) composes all six native protocol boundaries, their
resource-credit rules and the fixed integer size bridge; T013 composes the host
lifecycle, and [T061](generated-contexts.md) wires isolated generated contexts.
The status helper alone is not evidence that every native hook already uses it.

The algorithms use fixed field checks and disjoint numeric domains: allocated
core codes have zero upper halfword, while every library code has nonzero upper
halfword. Normalization selects either the unchanged input or one immutable static
failure record, proving both allocation independence and absence of borrowed
malformed-message access. Refusal recording changes exactly one caller record only
after those checks; all rejection paths return before its assignment.

Evidence: T007 introduced six ABI status cases; T010 adds checked provider-error
construction with exact borrowed message preservation. native metadata adds malformed-refusal
and nested-copy preservation, runtime selection exercises dedicated refusal versus
technical/malformed decoder outcomes with deliberately unusable outputs, and the
context leaf verifies allocation-free normalization with fallback bytes still valid
after disposal. The generated execution leaf's existing
`it_reports_failure_with_no_available_runtime_heap_or_root_frame` checks reporting
with the real heap unavailable, rather than merely inspecting the helper's code.

T007 acceptance passes the strict full suite with `-DNDEBUG -Werror` and focused
ASan/UBSan runs of ABI status, native metadata, runtime selection and contexts
(leak detection disabled). The documentation gate covers 695 maintained inputs
with zero debt. Two warning-free Doxygen generations produce identical bytes,
with 531 maintained file pages, 2216 documented interfaces and all 285 required
canonical/page destinations. These are T007 completion counts, not fixed targets
for later tasks.
