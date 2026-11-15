/** @file
 * @brief Enumerate borrowed compiler errors and parser warnings without rendering or inventing corrections.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) keeps structured locations;
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) preserves empty-body warnings.
 */
#include "../analysis/analysis.h"
/** A caller can stop synchronously; every diagnostic owner remains with its original AST/frontend. */
bool editor_diagnostics(const EditorAnalysis *analysis, EditorDiagnosticVisitor visit, void *context) {
    if (!analysis->attempted || visit == NULL) { return false; }
    if (analysis->diagnostic.message != NULL && !visit(context,&analysis->diagnostic,false)) { return true; }
    bool selected_seen = false;
    for (const ProjectModule *module = analysis->frontend.project.first; module != NULL; module = module->next) {
        if (!module->syntax.valid) { continue; }
        if (module->source == analysis->selected) { selected_seen = true; }
        for (const AstWarning *warning = module->syntax.warnings; warning != NULL; warning = warning->next) {
            if (!visit(context,&warning->diagnostic,true)) { return true; }
        }
    }
    if (!selected_seen && analysis->inspection.valid) {
        for (const AstWarning *warning = analysis->inspection.warnings; warning != NULL; warning = warning->next) {
            if (!visit(context,&warning->diagnostic,true)) { return true; }
        }
    }
    return true;
}
