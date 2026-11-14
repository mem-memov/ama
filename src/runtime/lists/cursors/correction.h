/** @file
 * @brief Preflight and correct all exact collection boundaries during publication.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) supplies the numerical
 * insert/remove equations; [LIST-13](../../../../spec/language/lists.md#list-13)
 * requires the complete correction before application code observes the mutation.
 */
#ifndef AMA_LIST_CURSOR_CORRECTION_H
#define AMA_LIST_CURSOR_CORRECTION_H
#include "runtime/objects/objects.h"
/** Ephemeral prepared scan view; no acquisition or raw pointer is saved in a
 * cursor. prepare validates all matching boundaries before destructive work.
 */
typedef struct {
    const AmaListCursorSet *set;
    unsigned char *payload;
    AmaHandle owner;
    size_t field;
    uint64_t incarnation;
} AmaListCursorView;
/** Validate this field's exact catalog/identity/boundaries without allocating or
 * modifying any cursor. Empty catalogs need no instance traversal.
 */
AmaStatus ama_list_cursors_prepare(AmaObjectType *, AmaPoolCell *, AmaHandle,
    const AmaHandleList *, AmaListCursorView *);
/** Apply insertion after prepared membership publication; arithmetic is bounded
 * by the successful insertion's checked new length. No callback can intervene.
 */
void ama_list_cursors_insert(const AmaListCursorView *, size_t position);
/** Apply one range removal, including clear and irreversible terminal cleanup. */
void ama_list_cursors_remove(const AmaListCursorView *, size_t position, size_t count);
#endif
