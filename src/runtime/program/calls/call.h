/**
 * @file
 * @brief Declare ordered invocation preparation and stackless return transitions.
 *
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * Invocation records borrow resolved accesses and sources. Result storage remains acquired until the caller explicitly releases it.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) retains
 * enclosing acquisitions in fixed caller-owned transient slots.
 * [FLOW-04](../../../../spec/language/control-flow.md#flow-04) converts a
 * terminal result while its header owner remains protected.
 */
#ifndef AMA_PROGRAM_CALL_H
#define AMA_PROGRAM_CALL_H
#include "runtime/program/program.h"

/** One supplied interface input in textual binding order, with resolved destination ordinal and exact installation operation. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
typedef struct {
    size_t field;
    uint64_t operation;
    AmaProgramSource source;
} AmaProgramInput;
/** Borrowed invocation plan: target, mode, ordered inputs and declared installation operations. Keep alive through acquisition/preparation/result handling. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
typedef struct {
    const AmaProgramAccess *target;
    AmaAccessMode mode;
    const AmaProgramInput *inputs;
    size_t input_count;
    const uint64_t *installation_operations;
} AmaProgramInvocation;
/** Caller-owned transient chain slot. A selected link parks its enclosing
 * acquisition here while its own acquisition occupies outgoing. Pool allocation
 * and replacement zero this service-only area; value/continuation clones never
 * copy it. The parent remains acquired throughout the selected subtree.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
typedef struct {
    bool active;
    AmaHandle parent;
    AmaInstanceActivity previous;
} AmaProgramFrame;
/** Borrow a generated, aligned frame within the caller's stable storage.
 * Bounds/alignment and caller identity are checked before publishing the pointer.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_frame(AmaProgram *, AmaHandle, size_t offset, AmaProgramFrame **);
/** Acquire a result of outgoing, with caller-local indices. On success park the
 * parent in a previously empty caller frame; refusal leaves the parent intact.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_acquire_result(AmaProgram *, AmaHandle, const AmaProgramInvocation *, size_t offset, bool *);
/** Acquire one already materialized intrinsic result with a fresh return context.
 * Park outgoing, which may be empty for a root list service, in the checked
 * caller frame. No synthetic method owner or result field is constructed.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_acquire_value(AmaProgram *, AmaHandle, const AmaProgramValue *,
    AmaAccessMode, size_t offset, bool *refused);
/** Release a selected target and restore its still-protected parent. Inactive
 * slots are accepted only during explicit chain abandonment.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_release_result(AmaProgram *, AmaHandle, size_t offset, bool abandoning);
/** Select the target once under the world guard, preserve explicit refusal provenance and acquire only a valid scalar available instance. Store outgoing identity and previous activity on the caller. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_acquire(AmaProgram *, AmaHandle, const AmaProgramInvocation *, bool *);
/** Prepare entry inputs/results in defined order under the existing target acquisition; read/resume acquisitions skip interface reinitialization. Caller releases acquisition even after failure. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_prepare(AmaProgram *, AmaHandle, const AmaProgramInvocation *);
/** After protected result processing, restore target availability saved by the caller and clear the outgoing handle. Require a valid outstanding acquisition. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_release(AmaProgram *, AmaHandle);
/** Require an acquired outgoing target, attach the immediate caller/return selector/site, mark caller Waiting and target Running, and change current only after checks. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_connect(AmaProgram *, AmaHandle *, uint64_t, uint64_t);
/** Require an explicit nonzero saved resume selector and return to the immediate caller or host while preserving it; no active caller chain is copied. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_yield(AmaProgram *, AmaHandle *, uint64_t *);
/** Return normally to the immediate caller or host and clear the target saved resume position; output continuation selects the caller label or zero for host. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_complete(AmaProgram *, AmaHandle *, uint64_t *);
/** Resolve a local handle through the exact world type and public pool lookup; borrowed cell output is valid only after success. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_cell(AmaProgram *, AmaHandle, AmaPoolCell **);
/** Select receiver first and source once, then install under one guard. Explicit owned continuation copy uses continuation replacement; release protections/scratch on every path. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_assign(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramSource *);
/** Own guard, arena and protections for selection plus store of an already selected source; release temporary authority on every return. Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_transfer(AmaProgram *, AmaHandle, const AmaProgramAccess *, const AmaProgramValue *);
/** Select a readable scalar and invoke its explicit condition protocol under a guard.
 * Publish output only after checked success; technical failure releases the guard
 * without altering caller output or refusal history. No universal truth conversion exists.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_condition(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool *);
/** Convert an owned result of the current protected target. The leading base
 * belongs to outgoing; index operands belong to caller. Do not acquire/execute
 * the result or release header grants; output changes only after checked success.
 * Component contract: [CALL-08](../../../../spec/language/calls-and-results.md#call-08), [CALL-09](../../../../spec/language/calls-and-results.md#call-09), [FLOW-04](../../../../spec/language/control-flow.md#flow-04). */
AmaStatus ama_program_condition_result(AmaProgram *, AmaHandle caller, const AmaProgramAccess *, bool *);
#endif
