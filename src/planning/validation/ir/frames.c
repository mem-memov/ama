/** @file
 * @brief Validate fixed lexical frame/cell identities independently of runtime loop contents.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) bounds slots by lexical
 * sites; [NAME-06](../../../../spec/language/declarations.md#name-06) forbids
 * duplicate scoped cells for nested reuse. Logical reservations do not activate cells.
 */
#include "internal.h"
#include "planning/storage/frames/internal.h"
/** Lookup by exact symbol identity without interpreting a name or dereferencing a foreign frame operand. */
static const ScopedStorageCell *cell_for(const PlannedType *type, const SemanticSymbol *symbol) {
    for (const ScopedStorageCell *cell = type->cells; cell != NULL; cell = cell->next) {
        if (cell->symbol == symbol) { return cell; }
    }
    return NULL;
}
/** Origins are shared by all lexical roles for one reference cell; permanent handles remain in their declared fields. */
static const ReferenceOriginStorage *origin_for(const PlannedType *type, const SemanticSymbol *symbol) {
    for (const ReferenceOriginStorage *origin = type->origins; origin != NULL; origin = origin->next) {
        if (origin->symbol == symbol) { return origin; }
    }
    return NULL;
}
/** Validate one conditional-cell introduction and each unique reference-origin reservation in first-role order. */
static bool cells(const PlannedType *type) {
    const ScopedStorageCell *cell = type->cells;
    const ScopedStorageCell *last = NULL;
    const ReferenceOriginStorage *origin = type->origins;
    const ReferenceOriginStorage *last_origin = NULL;
    size_t count = 0;
    size_t origins = 0;
    for (const SemanticForeachBinding *role = type->type->foreach_bindings; role != NULL; role = role->next) {
        bool first = true;
        for (const SemanticForeachBinding *earlier = type->type->foreach_bindings; earlier != role; earlier = earlier->next) {
            if (earlier->binding == role->binding) { first = false; break; }
        }
        if (first) {
            if (origin == NULL || origin->symbol != role->binding || origin->slot != origins++) { return false; }
            last_origin = origin;
            origin = origin->next;
        }
        if (!role->introduced) { continue; }
        if (cell == NULL || cell->symbol != role->binding || cell->introduction != role || cell->slot != count++
            || !cell->symbol->local || !cell->symbol->reference || cell->symbol->list) { return false; }
        last = cell;
        cell = cell->next;
    }
    return cell == NULL && last == type->last_cell && count == type->cell_count
        && origin == NULL && last_origin == type->last_origin && origins == type->origin_count;
}
bool plan_ir_frames(ProgramPlan *plan, const PlannedType *type) {
    if (!cells(type)) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0}, "Scoped storage cell/origin inventory differs from lexical binding identities");
    }
    const StorageFrame *frame = type->frames;
    const StorageFrame *last = NULL;
    size_t count = 0;
    for (const SemanticStatement *statement = type->type->statements; statement != NULL; statement = statement->next) {
        AstNodeKind kind = statement->syntax->kind;
        if (kind != AST_CALL && kind != AST_FOREACH) { continue; }
        const SemanticStatement *loop = statement->enclosing_loop;
        while (loop != NULL && loop->syntax->kind != AST_FOREACH) { loop = loop->enclosing_loop; }
        if (frame == NULL || frame->statement != statement || frame->slot != count++
            || frame->kind != (kind == AST_CALL ? STORAGE_OPERATION_FRAME : STORAGE_TRAVERSAL_FRAME)
            || frame->contents != plan_storage_frame_contents(statement)
            || frame->operation_parent != plan_storage_frame(type, statement->enclosing_operation)
            || frame->traversal_parent != plan_storage_frame(type, loop)) {
            return plan_fail(plan, statement->syntax->span, "Fixed lexical frame inventory or parent identities are incomplete");
        }
        const SemanticSymbol *symbol = kind == AST_FOREACH ? statement->traversal->binding : NULL;
        if (frame->cell != (symbol != NULL && symbol->local ? cell_for(type, symbol) : NULL)
            || frame->persistent_binding != (symbol != NULL && !symbol->local ? symbol : NULL)
            || frame->origin != (symbol != NULL ? origin_for(type, symbol) : NULL)) {
            return plan_fail(plan, statement->syntax->span, "Traversal frame does not use the exact shared reference cell and origin");
        }
        last = frame;
        frame = frame->next;
    }
    if (frame != NULL || last != type->last_frame || count != type->frame_count) {
        return plan_fail(plan, (SourceSpan){type->type->module->source->text, 0, 0}, "Fixed lexical frame count differs from resolved source sites");
    }
    return true;
}
