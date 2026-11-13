/**
 * @file
 * @brief Publish nonfatal parsed body warnings before source owners expire.
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) requires warnings
 * independently of reachability. Only complete parsed modules publish warning lists.
 */
#include "commands.h"
#include "diagnostics/render/render.h"
bool driver_warnings(const Project *project, FILE *output) {
    for (const ProjectModule *module = project->first; module != NULL; module = module->next) {
        if (!module->syntax.valid) { continue; }
        for (const AstWarning *warning = module->syntax.warnings; warning != NULL; warning = warning->next) {
            if (diagnostic_render(output, &warning->diagnostic) != DIAGNOSTIC_RENDER_OK) { return false; }
        }
    }
    return true;
}
