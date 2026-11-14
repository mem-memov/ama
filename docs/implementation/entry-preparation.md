# Entry preparation and typed transfers

The [specification](../../spec/README.md) defines current behavior. This component
implements entry preparation and typed transfers; T063 connects the compiled
caller/default-owner self source to the T049–T051 final-identity foundation.
Other execution slices remain tracked in [TODO](../../TODO.md).

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
still-planned result trees or intrinsic list services.

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
and leaves terminal state for final disposal. Typed list methods use separate
[service/result frames](list-service-frames.md);
[live traversal](list-traversal-execution.md) adds checked cursor control.

## One self descriptor, explicit runtime owner

The [source emitter](../../src/codegen/emit/source.c) maps a typed standalone
`&` to `AMA_SOURCE_SELF` plus its exact nominal type ID. It emits no local access
chain, hidden variable, source snapshot or instance. The descriptor is immutable
and contains no concrete context/slot identity.

Evaluation already receives an explicit instance owner. A supplied input uses
the initiating caller; a declared default uses the callee/field owner. The
existing getter validates that owner and forms its checked reference without
acquiring it. Fresh construction uses its reserved final identity; reconstruction
uses the protected destination's final identity rather than temporary storage.
Stored references in an ordinary clone are remapped by the existing complete-key
clone map instead of rerunning default initializers.

The former C-generation self guard is removed because construction, replacement,
clone identity and the descriptor adapter are now connected. Unsupported selected
result trees, condition-command loops and foreach retain their own explicit
execution guards. No grammar or language requirement changes.

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

## Compiled preparation and self evidence

`make -C tests/unit/codegen/preparation` checks named bindings in reverse text
order, omitted/default parameters, a positional placeholder, and result defaults.
A later default observes a newly supplied earlier parameter, and the result
observes the completed interface.

A reference-only relay accepts the caller's `&`, tries to call that waiting
caller and handles its Busy refusal. Its default `&` returns the relay itself;
calling the returned reference increments the same retained probe again. This
checks both caller/callee environments and reference formation without acquiring
a busy referent.

Another case clones the relay after execution and reads its stored self result
without executing/resetting the copy. Calling that reference and the original
separately yields independent retained counters. This exposes clone remapping
before a new entry can conceal an error by recomputing the self default.

A later invalid literal constructor prevents the native body and the `:` handler
and reports the binding/type context. The same leaf instruments actual generated
programs under ASan/UBSan when its compiler flags request those sanitizers.

`make -C tests/unit/runtime/call-preparation` independently checks indexed source
order before source protection, shared authority for repeated copy sources,
preflight rejection of same/overlapping containers, and
`it_installs_one_constructor_per_field_and_retains_the_successful_prefix`.
The latter counts exactly one constructor per field, observes earlier commits,
and keeps the later old field intact when preparation fails. Existing T049–T051
construction/clone/replacement tests supply the final-identity foundation.

Per-file coverage maps source emission, capability validation, the adjusted
assembly guard case and the new independently runnable leaf to explicit cases.
The complete strict suite supplies surrounding frontend/control/runtime and CLI
composition; target sanitizer checks supplement the lifecycle proof.
