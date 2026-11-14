/**
 * @file
 * @brief Declare private selection phases with explicit shared/instance domains.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires exact resolved access trees rather than spelling or status heuristics.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) keeps
 * result selection and index operand lookup in their explicit distinct domains.
 */
#ifndef AMA_VALUE_SELECTION_INTERNAL_H
#define AMA_VALUE_SELECTION_INTERNAL_H
#include "runtime/program/program.h"

/** Form a reference to exact current storage without reading a field or acquiring
 * the target. Private construction authority permits Constructing/Live identities;
 * public evaluation requires publication. Output changes only on success.
 */
AmaStatus ama_program_self(AmaProgram *, AmaHandle, const AmaProgramSource *, bool, AmaProgramValue *);
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
/** Use a protected result owner for the outer base only; nested index operands
 * retain the caller domain. Other effects/output/refusal contracts match access.
 */
AmaStatus ama_program_access_from(AmaProgram *, AmaHandle caller, AmaHandle owner,
    const AmaConstantValue *, const AmaProgramAccess *, bool, AmaProgramValue *, bool *);
#endif
