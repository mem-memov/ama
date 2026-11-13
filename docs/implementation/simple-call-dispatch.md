# Stable-instance call and return dispatch

T057 accepts startup and ordinary calls through the single assembly under
[STACK-01](../../spec/implementation/compiler.md#stack-01),
[CALL-01](../../spec/language/calls-and-results.md#call-01) and
[RT-02](../../spec/implementation/runtime.md#rt-02).
It builds on [T056](project-assembly.md) and the existing checked runtime call
services. Interface binding acceptance continues in T063–T064.

## Explicit return chain

The generated host initializes shared constants before constructing its explicit
root. Calls acquire the selected stable instance, prepare entry, attach a return
link containing caller identity and a selector, mark the caller Waiting and the
callee Running, then jump to the callee's entry label. Completion restores the
immediate caller identity and yields its selector to the execution function.
Results remain acquired until caller-side processing and release finish.

The runtime checks current/caller identities, activity and outgoing-link
agreement before changing the chain. The generated return switch additionally
checks selector membership and the caller's nominal type before jumping.
Selectors are allocated globally, but that alone does not authorize a selector
of one caller type in another caller. Unknown or foreign-type selectors terminate
with `AMA_INVALID_STATE`. No payload is saved into working globals or restored
on return. Ordinary early completion uses the same return operation.

## Argument and evidence

For every return-switch row, the validated IR supplies a unique resume selector
and its owning planned type. Dispatch accepts exactly the pair
`(owner nominal type, resume selector)` from that row. Nonmembership fails before
entering its label. Runtime caller identity/outgoing agreement supplies the
instance-level part of this check; no C activation represents an Ama caller.

`make -C tests/unit/codegen/execution` runs
`it_returns_through_nested_no_binding_calls_without_reinitializing_caller_state`:
two ordinary module types nest without explicit inputs or requested outputs,
two independent outer instances run, and one runs again. Native payload address
assertions and local counters distinguish preserved state from reconstruction.
Early completion skips subsequent native calls at both ordinary levels.

`it_rejects_unknown_and_foreign_type_return_selectors_before_dispatch` uses an
explicitly hostile test-only host helper to corrupt the current return link.
This is fault injection outside the native authoring contract, not a permitted
way for native code to access metadata. Both `UINT64_MAX` and a valid root-owned
selector substituted into the inner caller fail with `AMA_INVALID_STATE`, emit
only the first probe, and leave no occupied pools or active native borrow.

Existing cases in the same leaf also exercise distinct nested instances of the
same nominal type, refusal of a busy self instance, pool growth while a caller
waits, and active-chain error reporting. Assembly lifecycle tests independently
cover constant-before-root construction and reverse cleanup. Per-file coverage
names the new cases for the body emitter, test harness, fixture and call support.
