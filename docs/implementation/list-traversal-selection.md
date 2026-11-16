# Live foreach selection and lexical bindings

T081 implements the selection/lifetime layer beneath foreach and its actual C
storage layout. The contracts are
[FLOW-05](../../spec/language/lists.md#flow-05),
[LIST-13](../../spec/language/lists.md#list-13) and
[RT-02](../../spec/implementation/runtime.md#rt-02).
The [complete execution adapter](list-traversal-execution.md) connects this layer
to [mutation correction](list-traversal-mutation.md), origin checks and
continuation copying.

## One cursor per source site, shared cells for reused bindings

Each lexical foreach site has its own generated `AmaProgramForeachCursor` in
the owning stable pool representation. A cursor retains the full owner handle,
declared field ordinal, collection incarnation, numerical boundary, and current
membership/target identities. It retains no array pointer, visit history or
acquisition token. Each distinct binding symbol has one
`AmaProgramForeachBinding`. Nested sites reusing the same resolved symbol share
that cell, while retaining independent cursors. Exact semantic pointer identity
determines reuse; spelling comparisons and runtime guesses play no role.

Immutable generated offsets locate the cursor, binding metadata and handle
cell. A new local uses the binding's local handle; a reused persistent reference
uses its original declared field. Size/alignment checks precede exposing these
temporary views. The generated layout compiles with actual C `sizeof` and stable
typed members. Control cells are outside declared fields and native payloads;
ordinary object copying therefore creates inactive cells rather than inheriting
another invocation's scan state. Continuation transfer is deliberately reserved
for T084.

The representation cost is O(1) per lexical site and binding, independent of the
list length. Emitting unique cells uses exact pairwise scans of the lexical
inventory, O(K²) at compile time and no runtime symbol registry.

## Short acquisitions and eligibility

Entering a nonempty parsed body validates the exact current-instance list under
the existing service protection rules and saves its incarnation with boundary
zero. It closes that service before returning. Entry alone does not activate a
local or change a reused variable. The empty-body path validates only the
collection once: it need not even resolve cursor/binding offsets.

Selection reacquires the list, checks the saved incarnation and `q <= n`, takes
the membership at `q` and increments the boundary before checking eligibility.
Empty/stale reference targets are skipped; each occurrence still consumes one
candidate. Other failures remain errors. Owning entries must denote actual live
objects. Live targets are eligible regardless of activity, so selection never
silently skips a busy target or acquires it on behalf of an eventual call.

For an eligible entry, selection stores the target handle, current membership
and origin identity. The first eligible installation activates that cursor's
binding role; subsequent iterations replace the value without adding roles.
Before returning to application code, the service is closed. No index/condition
native protocol, clone, constructor, allocation or target execution is required.
Each candidate costs O(1), apart from the current O(T) nominal registry lookup;
skipping multiple absent entries takes proportional candidate work, not a
snapshot allocation.

The source list's original membership order is never copied. Mutation-boundary
adjustments are connected by [T082](list-traversal-mutation.md);
[origin/resume checks](list-traversal-origin.md) and
[continuation integration](list-traversal-execution.md) complete the executable adapter.
Whole-list incarnation replacement already fails before installing a new value,
rather than restarting at another collection or retaining an obsolete pointer.

## Binding activation and closure

A local becomes available only after an eligible candidate; an empty or
absent-only scan leaves it inactive. Reused persistent references retain their
old handles when no candidate is eligible. A selected handle is ordinary
nonowning data, so no referenced target receives a destruction credit.

Each active installed role contributes one to the shared cell's role count.
Nested reuse replaces the shared handle/origin without saving or restoring an
outer value. Ending an inner role leaves the metadata installed while another
role remains. Ending the last role discards origin metadata and a local handle;
the separate persistent handle remains unchanged. Abandonment is idempotent for
an inactive cursor and requires no source-list reacquisition, including after
collection replacement. [Missing-membership access and resume order](list-traversal-origin.md) are
implemented by T083; [T084](list-traversal-execution.md) supplies controller
and continuation integration.

The key invariant is that the role count equals the number of installed active
cursors sharing that cell. Entry contributes zero, first selection adds one,
subsequent selections preserve it, and exit subtracts exactly once. No local
activation can occur merely because source syntax exists or a loop entered.

## Evidence

Seven runtime cases cover empty bodies and lists, empty/stale targets, preserved
reused variables, local unavailability before activation, duplicate live busy
targets and their normal invocation refusal, zero-allocation selection, nested
reuse without restoration, exact collection invalidation, selected actual owned
children, and invalid offset/alignment rejection before collection acquisition.
Every teardown checks that lexical roles and list grants are gone before real
pool/resource cleanup.

The source-to-plan-to-compiled-C layout probe emits three cursors and exactly two
binding cells for two nested sites sharing a local plus a site reusing a declared
reference. Existing semantic scope tests cover compatible reuse and rejection
of incompatible shadowing. Strict component/composition tests, runtime ASan/UBSan,
the full strict suite, per-file coverage, standalone headers and reproducible
Doxygen validate this selection boundary. [T084](list-traversal-execution.md)
adds complete foreach execution and mapped continuation acceptance.
