/**
 * @file
 * @brief Separate checked host admission from one explicit context execution and its outcome.
 *
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14) distinguishes admission
 * rejection from accepted failure. [RT-15](../../../../spec/implementation/runtime.md#rt-15)
 * preserves suspended state across resume without retaining C activations.
 */
#ifndef AMA_CONTEXT_EXECUTION_H
#define AMA_CONTEXT_EXECUTION_H
#include "runtime/context/context.h"
#include "runtime/program/calls/call.h"

/** Linear internal execution owner. Initialize to zero, do not copy or independently
 * consume its lease. Generated code inspects that lease, dispatches labels directly
 * using control.current, and keeps program.current synchronized for diagnostics.
 * status is accepted preparation failure; skip body dispatch when it is nonzero.
 * selector is zero for entry or the consumed nonzero resume selector.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
typedef struct {
    AmaContextLease lease;
    AmaContextReference root;
    uint64_t selector;
    AmaStatus status;
} AmaContextExecution;

/** Host execution outcome with a context-checked root only for healthy completion/yield.
 * Failure carries the first technical status and an empty reference after resource cleanup.
 * This internal representation is not the public compiled-library transport ABI.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
typedef struct {
    AmaHostOutcome outcome;
    AmaContextReference root;
    AmaStatus status;
} AmaContextOutcome;

/** Admit first execution of an explicitly selected type in a context with no roots.
 * Construct its root once in final pool storage, including interface defaults;
 * do not prepare that fresh interface a second time. Admission rejection leaves
 * this empty owner and context unchanged. After accepted construction, including
 * failure, always finish the execution; a failed root has an empty local identity.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
AmaStatus ama_context_execution_start(AmaContextRegistry *, uint64_t, uint64_t,
    AmaContextExecution *);

/** Admit a checked selected root, then prepare entry or consume its resume selector.
 * Nonzero return rejects admission without changing Ama state or this empty owner.
 * Zero return transfers one lease into execution even when preparation failed;
 * inspect execution.status and always finish the accepted operation.
 * invocation is compiler-resolved borrowed data, target must be NULL because root
 * is explicit; resume accepts no input bindings. Entry resets parameters/results
 * through ordinary declaration-order preparation, never persistent body state.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
AmaStatus ama_context_execution_begin(AmaContextRegistry *, uint64_t, AmaContextReference,
    const AmaProgramInvocation *, AmaContextExecution *);
/** Begin using the execution owner's already-held lease. Rejection preserves that lease
 * and all Ama state; accepted preparation retains it through dispatch/finalization.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
AmaStatus ama_context_execution_begin_leased(AmaContextExecution *, AmaContextReference,
    const AmaProgramInvocation *);
/** Validate/finish execution while retaining its lease for result construction and host-map
 * cleanup. Success clears execution state but leaves exclusive context access live.
 * A failed outcome has already consumed native/Ama resources; diagnostic storage remains.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
AmaStatus ama_context_execution_finish_retained(AmaContextExecution *, AmaHostOutcome,
    AmaStatus, AmaContextOutcome *);
/** Finish an admitted operation after all native borrows/world operations close.
 * Generated code has already returned/yielded to the host sentinel using ordinary
 * explicit return helpers. Valid healthy outcomes return Idle; inconsistent
 * metadata, malformed status or technical error fail and clean this context.
 * Report before cleanup, preserve the first error, clear resource descriptors,
 * then consume the lease. Argument/active-borrow rejection leaves this owner live
 * for its caller to correct; output changes only after accepted finalization.
 * Component contract: [RT-14](../../../../spec/implementation/runtime.md#rt-14), [RT-15](../../../../spec/implementation/runtime.md#rt-15). */
AmaStatus ama_context_execution_finish(AmaContextExecution *, AmaHostOutcome,
    AmaStatus, AmaContextOutcome *);
#endif
