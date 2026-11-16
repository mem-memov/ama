# Membership-backed declared list values

T076 replaces the handle-vector foundation with the checked
[membership representation](list-membership.md). Generated fields still use the
internal `AmaHandleList` name, now an alias of the membership-backed collection.
The obsolete capacity implementation is removed. Constants retain their separate
immutable value storage; they do not acquire mutable membership identities.

The governing rules are [LIST-02](../../spec/language/lists.md#list-02),
[LIST-03](../../spec/language/lists.md#list-03),
[LIST-11](../../spec/language/lists.md#list-11),
[LIST-13](../../spec/language/lists.md#list-13) and
[COPY-05](../../spec/language/values-and-references.md#copy-05).

## Construction and ownership

`runtime/lists/storage.c` prepares a private shape at its final checked owner,
field selector and incarnation. Object construction and cloning fill stable
record targets individually; no algorithm assumes a contiguous handle array.
The lifecycle child ledger still tracks successfully constructed owned roots,
not empty metadata slots. Cleanup destroys exactly that prefix, then releases
order, directory and record blocks. Reference metadata disposal never destroys
referenced objects.

New independent owners start a new namespace. Replacement under a stable root
identity checks each old list field's next incarnation before destructive commit;
the private replacement is built at that final owner namespace. The clone
reservation interface now receives the final root identity before preparing any
field, so a spare bank cannot accidentally use a temporary root namespace.
Native payloads remain in their final stable addresses.

## Indexed selection and stores

Index conversion still runs once per resolved operand and publishes its numeric
result only on success. Bounds are checked before reading the order array. The
selected current occurrence is reconstructed and validated before exposing its
target cell. Empty/stale targets refuse at the ordinary acquisition boundary,
before argument defaults or bodies. An unrelated native conversion failure
remains technical, even if its numeric status resembles a refusal.

An indexed receiver records its full membership identity through source
evaluation. A reference store revalidates that identity, then uses atomic
membership replacement. An identical complete handle preserves the occurrence;
a different handle retires it and installs a fresh occurrence at the same
position. Stale and empty handles remain valid reference data. Owned-element
payload replacement preserves the element's stable identity and containing
membership. No target is silently selected by searching for an equal handle.

## Whole-list transfer

Shape and nominal type checks, exact self-copy detection and owned-tree preflight
precede staging. Ordinary whole-list self-copy performs no allocation, cloning or
incarnation change. An explicit continuation self-copy remains invalid.

A distinct transfer reserves the destination's next incarnation and stages an
independent private membership sequence. Each record is reserved before its owned
root is constructed. A failed root removes its uncredited empty record; previous
successful roots remain reachable for reverse cleanup. Every copied owned root
participates in one forest map, so references crossing element boundaries are
redirected only after all destination identities exist. Reference lists copy
complete target handles without acquiring or cloning those targets.

Only complete staging can destroy the old destination and publish the new list.
Failure before commit preserves all old memberships and resources. Destruction
failure after commit begins remains terminal, with primary-error-preserving
cleanup. Fresh copied membership identities are independent even when reference
targets are equal. Replacement invalidates every saved old incarnation rather
than repairing it against new contents.

The membership bijection follows the component proof; field construction installs
targets into distinct already-reserved records, and mapping changes only targets,
not order or identity. The ownership ledger and staged-prefix cleanup establish
one destroy credit per successful owned root. Publication performs no fallible
allocation or application callback after the first structural commit.

Metadata selection is O(1). Shape preparation and metadata copying are amortized
O(n); owned lifecycle and the existing exact forest-map cost are additional and
are not claimed to become constant-time. Whole-root replacement reserves the
field namespaces independently of copied source incarnations.

## Executable evidence

`tests/unit/runtime/list-transfer/support/memberships.c` adds actual whole-list
and containing-root replacements, fresh identity checks, self no-op and
incarnation exhaustion before native effects. The existing leaf also exercises
private forest fixups, staged-prefix cleanup and reference handles to busy,
stale and absent targets.

`tests/unit/runtime/value-selection/support/memberships.c` assigns references
through the actual indexed receiver/store boundary and checks same-handle identity
preservation, changed-handle retirement and stored stale/empty values. Existing
index cases check dedicated refusal conversion, unrelated failure and unchanged
outputs. Empty/stale index operands never invoke their decoder or overwrite
a prior selected output. Object clone, replacement, construction and preflight fixtures use
individual record cells rather than raw arrays.

`tests/unit/codegen/list-values/main.c` compiles real inferred owned/reference list
result receivers and retained reads. Mutating an owned copy leaves the producer
unchanged; duplicate references resume the same actual task. Further compiled
projects reject empty/stale targets locally and propagate technical decoder
failure without running a handler. Generated-program sanitizer flags instrument
the actual emitted executables.

The ready registry maps every maintained changed file to its leaf scenarios;
headers retain standalone compilation. The authored generated-layout probe reads
record targets through the same declared-list accessor. The full strict suite checks composition
with the compiler, generated C assembly and native views. Typed [append/insert execution](list-insertion.md) adds service/result-frame
integration; [deletion](list-deletion.md), [ordering](list-ordering.md) and
[foreach](list-traversal-execution.md) exercise the other composed consumers.
