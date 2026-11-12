/**
 * @file
 * @brief Declare ordered invocation preparation and stackless return transitions.
 *
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * Invocation records borrow resolved accesses and sources. Result storage remains acquired until the caller explicitly releases it.
 */
#ifndef AMA_PROGRAM_CALL_H
#define AMA_PROGRAM_CALL_H
#include "runtime/program/program.h"

/** One supplied interface input in textual binding order, with resolved destination ordinal and exact installation operation. */
typedef struct {
    size_t field;
    uint64_t operation;
    AmaProgramSource source;
} AmaProgramInput;
/** Borrowed invocation plan: target, mode, ordered inputs and declared installation operations. Keep alive through acquisition/preparation/result handling. */
typedef struct {
    const AmaProgramAccess *target;
    AmaAccessMode mode;
    const AmaProgramInput *inputs;
    size_t input_count;
    const uint64_t *installation_operations;
} AmaProgramInvocation;
/** Select the target once under the world guard, preserve explicit refusal provenance and acquire only a valid scalar available instance. Store outgoing identity and previous activity on the caller. */
AmaStatus ama_program_acquire(AmaProgram *, AmaHandle, const AmaProgramInvocation *, bool *);
/** Prepare entry inputs/results in defined order under the existing target acquisition; read/resume acquisitions skip interface reinitialization. Caller releases acquisition even after failure. */
AmaStatus ama_program_prepare(AmaProgram *, AmaHandle, const AmaProgramInvocation *);
/** After protected result processing, restore target availability saved by the caller and clear the outgoing handle. Require a valid outstanding acquisition. */
AmaStatus ama_program_release(AmaProgram *, AmaHandle);
/** Require an acquired outgoing target, attach the immediate caller/return selector/site, mark caller Waiting and target Running, and change current only after checks. */
AmaStatus ama_program_connect(AmaProgram *, AmaHandle *, uint64_t, uint64_t);
/** Require an explicit nonzero saved resume selector and return to the immediate caller or host while preserving it; no active caller chain is copied. */
AmaStatus ama_program_yield(AmaProgram *, AmaHandle *, uint64_t *);
/** Return normally to the immediate caller or host and clear the target saved resume position; output continuation selects the caller label or zero for host. */
AmaStatus ama_program_complete(AmaProgram *, AmaHandle *, uint64_t *);
/** Resolve a local handle through the exact world type and public pool lookup; borrowed cell output is valid only after success. */
AmaStatus ama_program_cell(AmaProgram *, AmaHandle, AmaPoolCell **);
/** Select receiver first and source once, then install under one guard. Explicit owned continuation copy uses continuation replacement; release protections/scratch on every path. */
AmaStatus ama_program_assign(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramSource *);
/** Own guard, arena and protections for selection plus store of an already selected source; release temporary authority on every return. */
AmaStatus ama_program_transfer(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramValue *);
/** Select readable scalar payload and invoke its explicit condition protocol under a guard. Technical protocol errors propagate; there is no universal truth conversion. */
AmaStatus ama_program_condition(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool *);
#endif
