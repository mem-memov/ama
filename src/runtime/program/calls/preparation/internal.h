/**
 * @file
 * @brief Declare entry-preparation phase views and lifetime obligations.
 *
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08)
 * requires selecting all sources before acquiring or resetting any interface field.
 */
#ifndef AMA_CALL_PREPARATION_INTERNAL_H
#define AMA_CALL_PREPARATION_INTERNAL_H
#include "runtime/program/transfers/internal.h"

/** One borrowed binding and its selected value. Zero initialization represents
 * literal/no selected value; pointers and access tables outlive the outer call.
 */
typedef struct {
    const AmaProgramInput *input;
    AmaProgramValue value;
} AmaPreparedInput;

/** Evaluate once in binding-text order; on failure only the evaluated prefix is
 * initialized. No source protection is acquired and no target reset occurs.
 */
AmaStatus ama_call_select_sources(AmaProgram *, AmaHandle caller,
    const AmaProgramInvocation *, AmaPreparedInput *);
/** Acquire owned source trees only after selection succeeds. Partial protections
 * belong to operation and must be restored even if this phase fails.
 */
AmaStatus ama_call_protect_sources(AmaObjectOperation *, const AmaPreparedInput *, size_t);
/** Check all affected fields with temporary leases; release them on every path.
 * Reject identical list storage and owning ancestors of selected list containers.
 */
AmaStatus ama_call_check_fields(AmaProgram *, AmaHandle target, const AmaProgramType *,
    const AmaPreparedInput *, size_t);
/** Install in declaration order under borrowed source authority; field commits
 * are individual, not rolled back as a group. Update current_operation before
 * each installation only when the resolved invocation supplies those locations.
 */
AmaStatus ama_call_install_fields(AmaProgram *, AmaHandle target, const AmaProgramType *,
    const AmaProgramInvocation *, const AmaPreparedInput *, const AmaObjectProtection *, uint64_t *current_operation);
#endif
