# Entry preparation and typed transfers

`runtime/program/calls/` connects the resolved invocation tables to stable pool
instances. Target selection occurs once. Bounds failure, empty/stale identities,
busy state and a missing resume position are acquisition refusals. A native
index-conversion error is a technical error even if its numeric code resembles
an acquisition refusal. The selector returns this distinction explicitly; callers
never infer it from an error-code range. Only acquisition refusal selects `:`.

Entry preparation selects all supplied sources in binding text order, before any
interface reset. Literal bytes need no source evaluation. It then protects entire
owned source boundaries. Reference transfers copy identity without acquiring the
referent. A list source additionally leases its storage location: reset checks
reject the same buffer and an owning ancestor of its container, including empty
lists. Every interface subtree is checked before the first replacement.

Parameters install in declaration order, followed by results. Each field selects
one path: supplied literal, supplied value, or the declared/default initializer.
A literal invokes `|` once, directly over fresh replacement storage. It does not
invoke `+` first. Later defaults read already installed earlier parameters.
Persistent body fields and the target's native payload remain intact. Initial
root construction is separate and does not repeat entry preparation.

## Source authority and lifetime

Preparation owns one transient protection context. Individual field replacements
borrow read authority from its exact identity records. Such a borrowed record
uses the original available service state for cloning and never releases its
parent context's protection. It authorizes copying only; destructive preflight
still refuses a protected source. All records are released before entering an Ama
body, so no automatic C preparation frame survives an Ama transfer.

Each replacement constructs in the destination slot's inactive payload bank.
Preparation failure preserves the old value. Successful preparation begins the
nonrollback destruction/commit phase. Native resource pointers never move by
byte-copy; only the declared native clone constructor can reproduce native data.
Failure after earlier parameter/result commits does not roll those commits back.
No body begins before all interface construction succeeds.

Whole owning-list transfers use one transient forest mapping across all elements.
References between elements follow their corresponding new identities. Empty
handles never participate in that map. Reference lists copy handles only. Parent
lifetime counts change with the committed list shape; failed staging cleans its
completed prefix, while failed destruction cleans both old and prepared values
and leaves terminal state for final disposal. List action dispatch and retained
list-operation results belong to section 10, not these value transfers.

## Argument for correctness

During preparation, source identities remain valid and their owned data cannot
be destructively changed: every prospective reset is disjoint from the protected
boundaries, and native hooks cannot reenter the active lifecycle operation or
retain Ama views. Before installing field k, exactly the earlier interface fields
have their new contents. One constructor adds one destructor obligation, and the
existing initialized-prefix ledger accounts for all children. Induction over the
parameter sequence and then the result sequence proves CALL-07/CALL-08 ordering.

Every iteration advances a finite binding/field/element cursor. Ownership walks
are bounded by the exact occupied-slot population. Work is linear in visited
value trees excluding hook costs, with linear registry/map searches and the
explicit field-by-source conflict checks. No timeout, spelling convention or
heuristic chooses ownership, construction or refusal semantics.

[Call tests](../../tests/unit/runtime/calls/main.c) check repeated literal
construction, result defaults, protected inputs, busy inputs, preservation after
constructor failure and every preparation allocation failure. [Forest tests](../../tests/unit/runtime/transfers/main.c)
check cross-element reference remapping, allocation-free self-assignment and every
list preparation allocation failure. [Generated execution](../../tests/unit/codegen/execution/main.c)
uses the real frontend, emitter, compiler and native entry verifier to test default
and skipped inputs, literal failures outside `:`, retained reads, ordered indexed
output receivers, branches, loops and nested calls.
