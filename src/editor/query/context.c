/** @file
 * @brief Refuse singular typed editor observations when a source module has several concrete capture contexts.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) preserves every specialization identity.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) forbids first-context guesses.
 */
#include "context.h"

const SemanticType *editor_query_type(const EditorAnalysis *analysis, const char *module) {
    if (!analysis || !module) { return NULL; }
    const ProjectModule *source = project_find(&analysis->frontend.project, module);
    const SemanticType *found = NULL;
    for (const SemanticType *type = analysis->frontend.semantic.first; type; type = type->next) {
        if (type->module != source) { continue; }
        if (found) { return NULL; }
        found = type;
    }
    return found;
}
