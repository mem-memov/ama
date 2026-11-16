/** @file
 * @brief Validate resolved scope preorder and typed binding locations before use.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) uses lexical scope,
 * exact nominal types and outermost-first validation, never runtime name guessing.
 */
#include "traversal.h"

/** Validate an extent without overflowing and before forming a typed pointer. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05). */
static bool ama_foreach_schema_extent(size_t size, size_t offset, size_t width, size_t alignment) {
    return offset % alignment == 0 && offset <= size && width <= size - offset;
}

AmaStatus ama_program_foreach_schema(size_t size, const AmaObjectField *fields, size_t field_count,
    const AmaProgramForeachScope *scopes, size_t count) {
    if ((count != 0 && scopes == NULL) || (field_count != 0 && fields == NULL)
        || count > size / sizeof(AmaProgramForeachCursor)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    for (size_t index = 0; index < count; index++) {
        const AmaProgramForeachScope *scope = &scopes[index];
        const AmaProgramForeachPlan *plan = &scope->plan;
        if ((scope->parent != SIZE_MAX && scope->parent >= index) || scope->header == 0 || scope->repeat == 0
            || plan->field >= field_count || fields[plan->field].type_id != plan->element_type
            || (fields[plan->field].kind != AMA_FIELD_OWNING_LIST && fields[plan->field].kind != AMA_FIELD_REFERENCE_LIST)
            || !ama_foreach_schema_extent(size, plan->cursor_offset, sizeof(AmaProgramForeachCursor), _Alignof(AmaProgramForeachCursor))
            || !ama_foreach_schema_extent(size, plan->binding_offset, sizeof(AmaProgramForeachBinding), _Alignof(AmaProgramForeachBinding))
            || !ama_foreach_schema_extent(size, plan->value_offset, sizeof(AmaHandle), _Alignof(AmaHandle))) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        if (index != 0 && scopes[index - 1].plan.cursor_offset + sizeof(AmaProgramForeachCursor) > plan->cursor_offset) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        if (scope->parent != SIZE_MAX) {
            size_t ancestor = index - 1;
            while (ancestor != SIZE_MAX && ancestor != scope->parent) { ancestor = scopes[ancestor].parent; }
            if (ancestor == SIZE_MAX) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        }
        if (plan->local) {
            if (plan->value_offset != plan->binding_offset + offsetof(AmaProgramForeachBinding, local_value)) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
        } else {
            bool found = false;
            for (size_t field = 0; field < field_count; field++) {
                if (fields[field].offset == plan->value_offset && fields[field].type_id == plan->element_type
                    && fields[field].kind == AMA_FIELD_REFERENCE) { found = true; break; }
            }
            if (!found) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        }
    }
    return ama_status_success();
}
