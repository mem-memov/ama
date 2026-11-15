/** @file
 * @brief Inspect retained foreach boundaries and currently installed lexical roles without advancing execution.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) keeps inactive locals absent;
 * [LIST-13](../../../../spec/language/lists.md#list-13) stores exact numerical boundaries and occurrence identities.
 */
#include "../internal.h"
/** Compiler-validated scope offsets refer only to runtime control cells and declared references, not native State. */
AmaStatus ama_debug_cursors(AmaDebugSession *session, AmaContextReference reference, AmaDebugCursorVisitor visit, void *context) {
    if (visit == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaProgramType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_debug_resolve(session,reference,&type,&cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_debug_visit_begin(session);
    if (status.code != AMA_OK) { return status; }
    const unsigned char *storage = ama_pool_payload(&type->object->pool,cell);
    const AmaDebugType *names = ama_debug_type(session,reference.local.type_id);
    for (size_t index = 0; index < type->foreach_count; index++) {
        const AmaProgramForeachScope *scope = &type->foreach_scopes[index];
        const AmaProgramForeachCursor *cursor = (const AmaProgramForeachCursor *)(storage+scope->plan.cursor_offset);
        const AmaProgramForeachBinding *binding = (const AmaProgramForeachBinding *)(storage+scope->plan.binding_offset);
        AmaDebugCursor value = {.scope=index,.binding_name=names == NULL ? NULL : names->bindings[index],.local=scope->plan.local,
            .active=cursor->active,.role_active=cursor->role_active,.header=scope->header,.repeat=scope->repeat};
        if (cursor->active) {
            value.boundary = cursor->boundary;
            value.collection_owner = ama_debug_reference(session,cursor->owner);
            value.collection_field = cursor->field;
            value.incarnation = cursor->incarnation;
            value.current = cursor->current;
            value.target = ama_debug_reference(session,cursor->target);
        }
        if (binding->roles != 0) {
            value.binding_roles = binding->roles;
            value.associated = binding->associated;
            value.value = ama_debug_reference(session,*(const AmaHandle *)(storage+scope->plan.value_offset));
        }
        if (!visit(context,&value)) { break; }
    }
    ama_debug_visit_end(session);
    return ama_status_success();
}
