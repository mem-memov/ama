/**
 * @file
 * @brief Declare private selection phases with explicit shared/instance domains.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires exact resolved access trees rather than spelling or status heuristics.
 */
#ifndef AMA_VALUE_SELECTION_INTERNAL_H
#define AMA_VALUE_SELECTION_INTERNAL_H
#include "runtime/program/program.h"

/** Select a valid immutable declaration/current constant field; output borrows
 * constant storage and has no mutable pool identity. Invalid field indices fail.
 */
AmaStatus ama_program_shared_base(AmaProgram *, const AmaProgramAccess *, const AmaConstantValue *, AmaProgramValue *);
/** Resolve one live/private instance field. Output locations borrow pool storage;
 * this selects reference storage without acquiring its referent.
 */
AmaStatus ama_program_instance_base(AmaProgram *, AmaHandle, const AmaProgramAccess *, bool, AmaProgramValue *);
/** Narrow a valid list view to its indexed scalar element without running code.
 * Bounds/type failure leaves caller-owned list storage unchanged.
 */
AmaStatus ama_program_index(AmaProgramValue *, size_t);
/** Evaluate a finite acyclic resolved index chain once per node. Publish output
 * only after success; optionally classify target bounds refusal explicitly.
 * Native decoder failures propagate without becoming acquisition refusal.
 */
AmaStatus ama_program_access(AmaProgram *, AmaHandle, const AmaConstantValue *,
    const AmaProgramAccess *, bool, AmaProgramValue *, bool *selection_refusal);
#endif
