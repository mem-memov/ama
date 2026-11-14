/** @file
 * @brief Describe fixed lexical boundary slots independently of body dispatch.
 * [LIST-13](../../../../spec/language/lists.md#list-13) permits exact fixed scans
 * over compiler-enumerated sites; [FLOW-05](../../../../spec/language/lists.md#flow-05)
 * retains checked identities and a scalar boundary across body suspension.
 */
#ifndef AMA_LIST_CURSOR_MODEL_H
#define AMA_LIST_CURSOR_MODEL_H
#include "runtime/lists/membership/membership.h"
/** Stable compiler-visible state; active also covers suspended scans. A binding
 * role is independent of list authority and owns no target lifecycle credit.
 */
typedef struct {
    AmaHandle owner;
    size_t field;
    uint64_t incarnation;
    size_t boundary;
    AmaMembership current;
    AmaHandle target;
    bool active, role_active;
} AmaListCursor;
/** Immutable ascending offsetof values for precisely the sites of one field.
 * The containing table has one entry per declared field; empty entries cost no scan.
 */
typedef struct {
    const size_t *offsets;
    size_t count;
} AmaListCursorSet;
/** Validate bounded, aligned, strictly ascending cursor extents without an instance. */
AmaStatus ama_list_cursor_schema(size_t size, size_t fields, const AmaListCursorSet *);
#endif
