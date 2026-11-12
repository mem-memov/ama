# Entry preparation and typed transfers

Implementation evidence for the subset recorded in the [T001 audit](audit/README.md).
The [specification](../../spec/README.md) defines current behavior; this
component description does not certify the open work in [TODO](../../TODO.md).

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

## Preparation phase boundaries

T143 separates the entry driver from sibling phases under
`src/runtime/program/calls/preparation/`. The outer driver owns its world guard,
scratch array and source protections. `sources.c` receives the caller and resolved
binding sequence; it evaluates nonliteral sources exactly once, writes diagnostic
locations in text order and acquires nothing. `protection.c` receives only the
selected array and a runtime protection operation. It acquires owned trees after
selection is complete, reusing exact source authority for repeated values. Literal,
shared and reference values do not cause referent acquisition in this phase.

`fields.c` checks every non-state interface field before installation. Temporary
check protections are restored even if a later field conflicts; selected source
protections remain with the outer owner. List conflict checks compare storage
identity even for empty lists, and reject an owning reset whose subtree contains
a selected list's container. An empty list is still a location in an object that
could otherwise be invalidated by reset.

`installation.c` receives the target's type, resolved invocation, selected inputs,
borrowed source authority and the exact diagnostic-operation location to update.
It walks parameter declarations and then result declarations, never binding-text
order. The per-field operation selects literal, supplied value or default and
owns its own scratch/cleanup. It borrows source authority without restoring the
outer owner's acquisitions. The driver releases those acquisitions once all
installation or failure cleanup is complete. Read/resume modes return before
these entry phases. This is the retained implementation boundary of
[CALL-08](../../spec/language/calls-and-results.md#call-08), not certification of
still-planned result trees, self references or intrinsic list services.

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
and leaves terminal state for final disposal. Typed list methods and their
result-frame lifetimes remain T075–T084; these data transfers are not executable
list services.

## Argument for correctness

During preparation, source identities remain valid and their owned data cannot
be destructively changed: every prospective reset is disjoint from the protected
boundaries, and native hooks cannot reenter the active lifecycle operation or
retain Ama views. Before installing field k, exactly the earlier interface fields
have their new contents. One constructor adds one destructor obligation, and the
existing initialized-prefix ledger accounts for all children. Induction over the
parameter sequence and then the result sequence proves CALL-07/CALL-08 ordering.

Extracting source selection and protection into separate operations preserves the
former two-loop order: no source can be reacquired while another binding still
needs to evaluate its index. Splitting affected-field checks and installation
preserves the all-checks-before-first-reset boundary. A successful installation
advances exactly one declaration; failure leaves the preceding commits intact.
Each extracted phase keeps the same branch conditions, statuses and effect order.

Every iteration advances a finite binding/field/element cursor. Ownership walks
are bounded by the exact occupied-slot population. The orchestration walks B bindings and F fields with O(B*F) binding matching
and explicit field-by-source conflict checks; tree traversal, registry/map lookup,
allocation and hook costs add separately. In particular, the current exact linear
identity-map searches do not justify a blanket linear-time claim. No timeout, spelling convention or
heuristic chooses ownership, construction or refusal semantics.

[Call tests](../../tests/unit/runtime/calls/main.c) check repeated literal
construction, result defaults, protected inputs, busy inputs, preservation after
constructor failure and every preparation allocation failure. [Forest tests](../../tests/unit/runtime/transfers/main.c)
check cross-element reference remapping, allocation-free self-assignment and every
list preparation allocation failure. [Generated execution](../../tests/unit/codegen/execution/main.c)
uses the real frontend, emitter, compiler and native entry verifier to test default
and skipped inputs, literal failures outside `:`, retained reads, ordered indexed
output receivers, branches, loops and nested calls.

The independent [preparation phase cases](../../tests/unit/runtime/call-preparation/main.c)
check once-only indexed source order, repeated owned sources held across two
independent clones, identical empty-list and nested-container reset conflicts,
and literal/default installation with a failure after the first commit. They
inspect diagnostic operation IDs and verify the target's persistent native
resource stays unchanged. Existing call cases enumerate each measured allocation
failure and generated execution exercises real nested calls. Doxygen, rather
than this contract, produces file/function reference documentation.
