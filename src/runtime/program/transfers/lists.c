/**
 * @file
 * @brief Orchestrate checked list staging, forest fixups, commit and failure cleanup.
 *
 * [COPY-02](../../../../spec/language/values-and-references.md#copy-02)
 * requires complete preparation before replacing destination storage.
 */
#include "list/internal.h"

/** Store a shape-compatible source/default list under caller-held operation
 * ownership. Prepared handles remain private until commit; any failure discards
 * the completed staging prefix while preserving the primary status.
 */
AmaStatus ama_program_store_list(AmaProgram *program, AmaProgramValue *destination, const AmaProgramValue *source,
    const AmaFieldRecipe *initializers, AmaObjectOperation *operation) {
    bool owning = destination->form == AMA_FIELD_OWNING_LIST;
    bool no_op;
    AmaStatus status = ama_transfer_list_validate(destination, source, operation, &no_op);
    if (status.code != AMA_OK || no_op) { return status; }
    AmaHandleList prepared = {0};
    status = ama_transfer_list_prepare(program, destination, source, initializers, operation, &prepared);
    if (status.code == AMA_OK && owning) { status = ama_transfer_list_fixup(operation, source != NULL); }
    if (status.code == AMA_OK) { status = ama_transfer_list_commit(program, destination, operation, &prepared); }
    return status.code == AMA_OK ? status : ama_transfer_list_discard(program, &prepared, owning, status);
}
