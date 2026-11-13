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
 */
typedef struct {
    AmaContextLease lease;
    AmaContextReference root;
    uint64_t selector;
    AmaStatus status;
} AmaContextExecution;

/** Host execution outcome with a context-checked root only for healthy completion/yield.
 * Failure carries the first technical status and an empty reference after resource cleanup.
 * This internal representation is not the public compiled-library transport ABI.
 */
typedef struct {
    AmaHostOutcome outcome;
    AmaContextReference root;
    AmaStatus status;
} AmaContextOutcome;

/** Admit a checked selected root, then prepare entry or consume its resume selector.
 * Nonzero return rejects admission without changing Ama state or this empty owner.
 * Zero return transfers one lease into execution even when preparation failed;
 * inspect execution.status and always finish the accepted operation.
 * invocation is compiler-resolved borrowed data, target must be NULL because root
 * is explicit; resume accepts no input bindings. Entry resets parameters/results
 * through ordinary declaration-order preparation, never persistent body state.
 */
AmaStatus ama_context_execution_begin(AmaContextRegistry *, uint64_t, AmaContextReference,
    const AmaProgramInvocation *, AmaContextExecution *);
/** Finish an admitted operation after all native borrows/world operations close.
 * Generated code has already returned/yielded to the host sentinel using ordinary
 * explicit return helpers. Valid healthy outcomes return Idle; inconsistent
 * metadata, malformed status or technical error fail and clean this context.
 * Report before cleanup, preserve the first error, clear resource descriptors,
 * then consume the lease. Argument/active-borrow rejection leaves this owner live
 * for its caller to correct; output changes only after accepted finalization.
 */
AmaStatus ama_context_execution_finish(AmaContextExecution *, AmaHostOutcome,
    AmaStatus, AmaContextOutcome *);
#endif
