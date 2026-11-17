/**
 * @file
 * @brief Extract concrete source dependencies from complete structural type expressions.
 *
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) retains all
 * contributing source occurrences. [TPL-01](../../../../spec/language/templates.md#tpl-01)
 * makes placeholder names atomic scoped identifiers, never filesystem requests.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) keeps the source
 * definition registry separate from later virtual specialization identities.
 */
#include "internal.h"

/** Project-owned pending work borrows one immutable expression owner.
 * Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * requires complete discovery without a C recursion depth limit.
 */
typedef struct PendingExpression {
    struct PendingExpression *next;
    const AstTypeExpression *expression;
} PendingExpression;

/** Queue one actual expression edge; omitted positional inputs need no record.
 * Component contract: [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * keeps all partial work project-owned on allocation refusal.
 */
static bool push(Project *project, PendingExpression **pending, const AstTypeExpression *expression) {
    if (expression == NULL) { return true; }
    PendingExpression *frame = source_arena_allocate(&project->arena, sizeof(*frame));
    if (frame == NULL) { return project_memory_failure(project); }
    *frame = (PendingExpression){*pending, expression};
    *pending = frame;
    return true;
}

/** Visit each owning input edge, collecting only concrete module heads.
 * Component contract: [TPL-01](../../../../spec/language/templates.md#tpl-01)
 * excludes a placeholder's name but not concrete arguments written beneath it.
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) retains repeated
 * source requests; the existing stable sort restores physical admission order.
 * Output aliases are not expression owners. Failure retains all scratch in the
 * project arena and prevents publication of the containing closure.
 */
bool project_collect_expression(Project *project, DependencyPaths *paths, const AstTypeExpression *expression) {
    PendingExpression *pending = NULL;
    if (!push(project, &pending, expression)) { return false; }
    while (pending != NULL) {
        const AstTypeExpression *current = pending->expression;
        pending = pending->next;
        if (current->kind == AST_TYPE_MODULE_PATH
            && !project_collect_path(project, paths, current->head, NULL)) { return false; }
        for (const AstStaticBinding *binding = current->first; binding != NULL; binding = binding->next) {
            if (!push(project, &pending, binding->input)) { return false; }
        }
    }
    return true;
}
