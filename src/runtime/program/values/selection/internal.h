/**
 * @file
 * @brief Declare private selection phases with explicit shared/instance domains.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires exact resolved access trees rather than spelling or status heuristics.
 * [CALL-09](../../../../../spec/language/calls-and-results.md#call-09) keeps
 * result selection and index operand lookup in their explicit distinct domains.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) Retained origin checks preserve explicit structural authority and separate refusal provenance from native errors.
 */
#ifndef AMA_VALUE_SELECTION_INTERNAL_H
#define AMA_VALUE_SELECTION_INTERNAL_H
#include "runtime/program/program.h"
struct AmaProgramListService;
/** Borrow payload with an exact structural origin grant, retaining separate
 * checked-origin refusal provenance before any native decoder can execute.
 */
AmaStatus ama_program_payload_authorized(AmaProgram *, const AmaProgramValue *,
    const struct AmaProgramListService *, const void **, bool *origin_refused);

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
/** Resolve under an optional exact service grant. Only an actual busy-list check
 * sets list_refusal; unrelated errors never inherit its numeric classification.
 */
AmaStatus ama_program_instance_authorized(AmaProgram *, AmaHandle, const AmaProgramAccess *,
    bool, const struct AmaProgramListService *, bool *list_refusal, AmaProgramValue *);
/** Evaluate structural operands with the exact parent grant; ordinary body calls
 * use the existing null-authority entry point. No token is stored on program.
 */
AmaStatus ama_program_access_authorized(AmaProgram *, AmaHandle caller, AmaHandle owner,
    const AmaConstantValue *, const AmaProgramAccess *, bool,
    const struct AmaProgramListService *, AmaProgramValue *, bool *);
/** Evaluate one method-direct source with an explicit structural grant. Literal
 * recipes remain a separate constructor path; ordinary evaluation passes null.
 */
AmaStatus ama_program_evaluate_authorized(AmaProgram *, AmaHandle, const AmaConstantValue *,
    const AmaProgramSource *, bool, const struct AmaProgramListService *, AmaProgramValue *);
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
