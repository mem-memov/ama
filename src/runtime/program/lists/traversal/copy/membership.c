/** @file
 * @brief Map live full membership identities and preserve permanently dead origins.
 * [CONT-04](../../../../../../spec/language/continuations.md#cont-04) includes
 * origins from exited inner scopes. Position is used only after exact source
 * namespace/record/generation validation; removed or obsolete identities map to
 * reserved generation zero rather than a reused source/destination record.
 */
#include "internal.h"

AmaStatus ama_foreach_copy_membership(const AmaObjectControlCopy *copy, const AmaProgramType *type,
    AmaMembership source, AmaMembership *output) {
    if (!ama_handle_equal(source.owner, copy->source)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaHandleList *original = NULL, *destination = NULL;
    AmaStatus status = ama_foreach_copy_list(type, copy->source_storage, source.field, &original);
    if (status.code == AMA_OK) { status = ama_foreach_copy_list(type, copy->destination_storage, source.field, &destination); }
    if (status.code != AMA_OK) { return status; }
    if (!ama_handle_equal(destination->owner, copy->destination) || destination->field != source.field || destination->incarnation == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    const AmaMembershipRecord *record = NULL;
    status = ama_membership_lookup(original, source, &record);
    if (status.code == AMA_STALE_REFERENCE) {
        *output = (AmaMembership){.owner = copy->destination, .field = source.field,
            .incarnation = destination->incarnation, .record = SIZE_MAX, .generation = 0};
        return ama_status_success();
    }
    if (status.code != AMA_OK) { return status; }
    AmaHandle target;
    return ama_membership_at(destination, record->position, output, &target);
}
