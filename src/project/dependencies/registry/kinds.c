/** @file
 * @brief Check source occurrence roles after the entire loading queue has parsed.
 *
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02) permits loading
 * cycles: a target's kind is authoritative only after parsing its source.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) makes graph
 * producers distinct from ordinary Types and requires explicit export selection.
 */
#include <assert.h>
#include "project/dependencies/internal.h"

/** Describe one incompatible use without allocating or inspecting type equations.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) reserves producer
 * use and export selectors for static modules; ordinary paths cannot denote graphs.
 * A null result means this source-kind relation is satisfied, not that later
 * specialization, export lookup or layout checking has succeeded.
 */
static const char *mismatch(const ProjectDependency *edge) {
    bool graph = edge->target->syntax.sections.kind == MODULE_STATIC;
    switch (edge->role) {
        case PROJECT_ORDINARY_PATH:
            return graph ? "A static module is not an ordinary type or constant namespace" : NULL;
        case PROJECT_STATIC_PRODUCER:
            return graph ? NULL : "A static producer use requires a static module";
        case PROJECT_STATIC_SELECTOR:
            return graph ? NULL : "An exported type selector requires a static module";
    }
    assert(false);
    return NULL;
}

bool project_validate_kinds(Project *project) {
    for (ProjectModule *module = project->first; module != NULL; module = module->next) {
        for (const ProjectDependency *edge = module->first; edge != NULL; edge = edge->next) {
            assert(edge->target->state == PROJECT_MODULE_READY);
            const char *message = mismatch(edge);
            if (message == NULL) { continue; }
            Diagnostic cause = {.code = DIAGNOSTIC_TYPE_MISMATCH, .message = message,
                .primary = {.span = edge->requirement}};
            return project_report(project, PROJECT_INVALID_KIND, module,
                edge->requirement, edge->logical_path, &cause);
        }
    }
    return true;
}
