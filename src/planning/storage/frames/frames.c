/** @file
 * @brief Enumerate lexical operation/traversal frames without following runtime control cycles.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) retains caller
 * output protection separately from a completed callee return; [RT-09](../../../../spec/implementation/runtime.md#rt-09)
 * permits only traversal control, not outstanding returns, in continuation state.
 */
#include "internal.h"
StorageFrame *plan_storage_frame(const PlannedType *type, const SemanticStatement *statement) {
    if (statement == NULL) { return NULL; }
    for (StorageFrame *frame = type->frames; frame != NULL; frame = frame->next) {
        if (frame->statement == statement) { return frame; }
    }
    return NULL;
}
/** Each call site reserves the complete general transient frame, including intrinsic output storage where applicable. */
unsigned plan_storage_frame_contents(const SemanticStatement *statement) {
    if (statement->syntax->kind == AST_FOREACH) {
        return FRAME_CURSOR_ACTIVITY | FRAME_COLLECTION_IDENTITY | FRAME_BOUNDARY
            | FRAME_CURRENT_MEMBERSHIP | FRAME_BINDING_ORIGIN;
    }
    unsigned result = FRAME_OUTPUT_CONTINUATION;
    if (statement->operation_kind != SEMANTIC_CONSTANT_OPERATION) { result |= FRAME_TARGET_PROTECTION; }
    AstCallMode mode = statement->syntax->as.call.mode;
    if (mode == AST_ENTRY_CALL) { result |= FRAME_INPUT_PROTECTION; }
    if (statement->operation_kind == SEMANTIC_INSTANCE_OPERATION
        && (mode == AST_ENTRY_CALL || mode == AST_RESUME_CALL)) { result |= FRAME_BODY_RETURN; }
    if (statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) { result |= FRAME_INTRINSIC_RESULT; }
    return result;
}
/** Link a traversal's binding to its unique introduced cell or existing permanent field; never allocate a duplicate alias. */
static bool binding(ProgramPlan *plan, PlannedType *type, StorageFrame *frame) {
    const SemanticForeachBinding *role = frame->statement->traversal;
    if (role == NULL) { return plan_fail(plan, frame->statement->syntax->span, "Traversal storage requires a resolved binding"); }
    for (const ReferenceOriginStorage *origin = type->origins; origin != NULL; origin = origin->next) {
        if (origin->symbol == role->binding) { frame->origin = origin; break; }
    }
    if (frame->origin == NULL) { return plan_fail(plan, frame->statement->syntax->span, "Traversal binding has no origin record"); }
    if (!role->binding->local) { frame->persistent_binding = role->binding; return true; }
    for (const ScopedStorageCell *cell = type->cells; cell != NULL; cell = cell->next) {
        if (cell->symbol == role->binding) { frame->cell = cell; return true; }
    }
    return plan_fail(plan, frame->statement->syntax->span, "Traversal storage has no exact scoped cell");
}
bool plan_storage_frames(ProgramPlan *plan, PlannedType *type) {
    if (!plan_storage_cells(plan, type)) { return false; }
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        AstNodeKind kind = statement->syntax->kind;
        if (kind != AST_CALL && kind != AST_FOREACH) { continue; }
        StorageFrame *frame = plan_allocate(plan, sizeof(*frame));
        if (frame == NULL) { return false; }
        *frame = (StorageFrame){.statement = statement, .slot = type->frame_count++, .contents = plan_storage_frame_contents(statement),
            .kind = kind == AST_FOREACH ? STORAGE_TRAVERSAL_FRAME : STORAGE_OPERATION_FRAME};
        if (kind == AST_FOREACH && !binding(plan, type, frame)) { return false; }
        if (type->last_frame == NULL) { type->frames = frame; }
        else { type->last_frame->next = frame; }
        type->last_frame = frame;
    }
    for (StorageFrame *frame = type->frames; frame != NULL; frame = frame->next) {
        frame->operation_parent = plan_storage_frame(type, frame->statement->enclosing_operation);
        const SemanticStatement *loop = frame->statement->enclosing_loop;
        while (loop != NULL && loop->syntax->kind != AST_FOREACH) { loop = loop->enclosing_loop; }
        frame->traversal_parent = plan_storage_frame(type, loop);
    }
    return true;
}
