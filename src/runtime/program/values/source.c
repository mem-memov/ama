/**
 * @file
 * @brief Expose local/shared selection and source-mode evaluation entry points.
 *
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * preserves caller-side selection order and explicit acquisition/continuation flags.
 */
#include "selection/internal.h"

AmaStatus ama_program_select(AmaProgram *program, AmaHandle current, const AmaProgramAccess *access,
    bool private_access, AmaProgramValue *output) {
    return ama_program_access(program, current, NULL, access, private_access, output, NULL);
}

AmaStatus ama_program_select_shared(AmaProgram *program, const AmaConstantValue *current,
    const AmaProgramAccess *access, AmaProgramValue *output) {
    return ama_program_access(program, (AmaHandle){0}, current, access, false, output, NULL);
}

/** Evaluate one resolved local/shared source into a borrowed value view. Literal
 * construction is a separate path. Explicit acquisition marks reference form,
 * rejects immutable scalars and never itself acquires the referent. Saved-position
 * copying is carried as an explicit flag for the later transfer operation.
 */
AmaStatus ama_program_evaluate(AmaProgram *program, AmaHandle current, const AmaConstantValue *shared,
    const AmaProgramSource *source, bool private_access, AmaProgramValue *output) {
    if (source->kind == AMA_SOURCE_SELF) {
        return shared == NULL ? ama_program_self(program, current, source, private_access, output)
            : ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (source->kind == AMA_SOURCE_LITERAL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramAccess global = {.field = source->shared, .shared = true};
    const AmaProgramAccess *access = source->kind == AMA_SOURCE_SHARED ? &global : source->access;
    AmaStatus status = ama_program_access(program, current, shared, access, private_access, output, NULL);
    if (status.code == AMA_OK && source->acquire && output->constant != NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (status.code == AMA_OK && source->acquire) { output->form = AMA_FIELD_REFERENCE; }
    if (status.code == AMA_OK) { output->continuation = source->continuation; }
    return status;
}

AmaStatus ama_program_select_target(AmaProgram *program, AmaHandle current, const AmaProgramAccess *access,
    AmaProgramValue *output, bool *selection_refusal) {
    return ama_program_access(program, current, NULL, access, false, output, selection_refusal);
}
