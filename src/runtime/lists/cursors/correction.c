/** @file
 * @brief Scan exact lexical slots and atomically update every matching boundary.
 * [LIST-13](../../../../spec/language/lists.md#list-13) fixes lexical order and
 * O(K) work without an active registry; [FLOW-05](../../../../spec/language/lists.md#flow-05)
 * keeps insertion at the boundary in the future and subtracts only removed past entries.
 */
#include "correction.h"

/** Select one previously bounded actual C cursor slot, never an entry array. */
static AmaListCursor *ama_list_cursor_at(const AmaListCursorView *view, size_t index) {
    return (void *)(view->payload + view->set->offsets[index]);
}
/** Exact full collection identity distinguishes active/suspended scans from old
 * incarnations and unrelated fields; inactive slots require no registration.
 */
static bool ama_list_cursor_matches(const AmaListCursor *cursor, const AmaListCursorView *view) {
    return cursor->active && ama_handle_equal(cursor->owner, view->owner)
        && cursor->field == view->field && cursor->incarnation == view->incarnation;
}

AmaStatus ama_list_cursors_prepare(AmaObjectType *type, AmaPoolCell *cell, AmaHandle owner,
    const AmaHandleList *list, AmaListCursorView *view) {
    *view = (AmaListCursorView){0};
    if (type->list_cursors == NULL) { return ama_status_success(); }
    if (list->field >= type->field_count || !ama_handle_equal(list->owner, owner) || list->incarnation == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    const AmaObjectField *field = &type->fields[list->field];
    unsigned char *payload = ama_pool_payload(&type->pool, cell);
    if ((field->kind != AMA_FIELD_OWNING_LIST && field->kind != AMA_FIELD_REFERENCE_LIST)
        || (const void *)(payload + field->offset) != list) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    const AmaListCursorSet *set = &type->list_cursors[list->field];
    /* Validate just this field's catalog: mutation work is O(K), not O(all fields). */
    AmaStatus status = ama_list_cursor_schema(type->pool.payload_size, 1, set);
    if (status.code != AMA_OK) { return status; }
    *view = (AmaListCursorView){.set = set, .payload = payload,
        .owner = owner, .field = list->field, .incarnation = list->incarnation};
    for (size_t index = 0; index < set->count; index++) {
        const AmaListCursor *cursor = ama_list_cursor_at(view, index);
        if (ama_list_cursor_matches(cursor, view) && cursor->boundary > list->count) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
    }
    return ama_status_success();
}

void ama_list_cursors_insert(const AmaListCursorView *view, size_t position) {
    if (view->set == NULL) { return; }
    for (size_t index = 0; index < view->set->count; index++) {
        AmaListCursor *cursor = ama_list_cursor_at(view, index);
        if (ama_list_cursor_matches(cursor, view) && position < cursor->boundary) { cursor->boundary++; }
    }
}

void ama_list_cursors_remove(const AmaListCursorView *view, size_t position, size_t count) {
    if (view->set == NULL) { return; }
    for (size_t index = 0; index < view->set->count; index++) {
        AmaListCursor *cursor = ama_list_cursor_at(view, index);
        if (ama_list_cursor_matches(cursor, view) && position < cursor->boundary) {
            size_t past = cursor->boundary - position;
            cursor->boundary -= past < count ? past : count;
        }
    }
}
