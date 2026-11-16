# Typed swap and length execution

T080 completes the six list methods.
[Foreach execution](list-traversal-execution.md) composes them with fixed
lexical cursors and live-boundary correction. The contracts are [LIST-04](../../spec/language/lists.md#list-04),
[LIST-08](../../spec/language/lists.md#list-08),
[LIST-10](../../spec/language/lists.md#list-10) and
[PROTO-03](../../spec/implementation/native-interface.md#proto-03).

Assuming an owning `items[]` and a compatible `second` integer are declared:

```text
items[]
 swap !
  <
  < second
 length !
  > count
```

The omitted first position is constructed from `|0`. Swap changes the order of
the two occurrences; length then copies the current size into an ordinary
inferred integer. There is no retained method outcome or reactive size variable.

## Swap changes order, not identity

The shared input pipeline evaluates supplied sources once in written order and
constructs the two integer parameters in declaration order. Each uses its
selected literal or copy constructor, never a native default followed by an
overwrite. Their explicit index protocol conversions run before publication.
Reserved invalid-index and checked bounds failures carry refusal provenance;
other native failures, including Busy, remain technical.

Publication exchanges the two membership identifiers and updates their inverse
positions under the exact parent service grant. No target is acquired, copied,
executed or destroyed. Thus busy reference targets and noncopyable element types
need no special case. A valid self-swap preserves every observable value. Invalid
positions preserve the complete sequence and create no result. Temporary input
roots are finalized in reverse declaration order through the existing frame
cleanup, including after refusal.

The membership bijection is preserved algebraically: swapping the order map's
two images and the inverse map's corresponding positions composes each with the
same transposition. Their composition remains identity. Record generations,
target handles, collection incarnation and the ownership ledger do not change.
The structural edit is O(1), excluding parameter construction/conversion.

## Length selects one fresh constructor

Length has no input roots. Its emitted descriptor binds a `void *` payload
adapter to the exact standard integer's header-declared typed `from_size` helper.
Recognition uses the complete standard-root identity, not a basename or payload
layout. Ordinary interface signature checking still rejects a missing or
incompatible helper. No function-pointer cast or additional ABI-table slot is
introduced.

The method reads protected structural `size_t` metadata and creates one stable
integer root with an `AMA_CONSTRUCT_SIZE` recipe. The ordinary object constructor
builds its declared Ama fields before selecting the size constructor for fresh
native storage. It calls neither the default nor literal native constructor for
that result. There is no decimal formatting/parsing or assumed integer layout.
The bridge accepts the full range through `SIZE_MAX`; allocation/native failures
remain technical even if their numeric status equals a method refusal.

After successful construction, the frame owns the integer through its entire
selected output subtree. Ordinary receiver copies survive method cleanup.
Nonowning aliases to the temporary result become stale afterwards. The existing
frame ancestry handles both direct result execution and technical failure in
that execution before a pending sibling method. Each successful native credit
is finalized once; an unsuccessful bridge has no native destruction credit.

Reading length is O(1). Result construction, declared child initialization and
cleanup retain their ordinary object costs; registry lookup currently costs
O(T). Fixed lexical method frames use O(1) storage independent of list length.

## Evidence

Four runtime cases exercise an actually running reference target, preserved
membership identities and targets, self-swap, invalid first/second positions,
technical conversion Busy, one direct size construction, stale escaped result
handles, technical bridge failure and exact cleanup. The same selected
construction recipe round-trips zero, one, native signed representation
boundaries and `SIZE_MAX` through the declared index protocol without fabricating
an impossible list with `SIZE_MAX` entries.

Four compiled projects verify actual swapped order, default-zero input staging,
sequential methods, current versus copied length, empty length, selected integer
body execution, invalid positions and negative conversion, technical bridge
failure, and technical failure in the selected result body. The test-owned
standard integer makes each size construction observable. Component and actual
generated-program ASan/UBSan, the full strict suite, per-file inventory,
standalone headers and reproducible Doxygen validate this boundary.
