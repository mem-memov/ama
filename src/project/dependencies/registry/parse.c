/**
 * @file
 * @brief Validate one required module and discover its structural dependencies.
 *
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) prepares defining scopes
 * before any dependency operand is resolved.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) captures local configuration
 * only when its definition becomes a required active module.
 * Required drafts/invalid entries fail with requester context rather than being silently skipped.
 */
#include <assert.h>
#include "project/dependencies/internal.h"

/** Capture local configuration, then native companions for parsed native activation.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) retains each required
 * definition's own alias file; [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * preserves original failure sites and the requesting chain without partial readiness.
 */
static bool required_companions(Project *project, ProjectModule *module) {
    ModuleStatus status = module_catalog_load_configuration(&project->catalog, module->source);
    if (status == MODULE_OK && module->syntax.sections.kind == MODULE_NATIVE) {
        status = module_catalog_load_native(&project->catalog, module->source);
    }
    if (status == MODULE_OK) { return true; }
    if (status == MODULE_OUT_OF_MEMORY) { return project_memory_failure(project); }
    Diagnostic cause;
    DiagnosticRelated related;
    bool adapted = diagnostic_from_module(&project->catalog.issue, &related, &cause);
    assert(adapted);
    (void)adapted;
    return project_report(project, PROJECT_CATALOG_ERROR, module->requested_by,
        module->requirement, module->source->logical_path, &cause);
}

bool project_parse_required(Project *project, ProjectModule *module) {
    ParseStatus parsed = parser_module_parse(&module->syntax, module->source->text);
    if (parsed == PARSE_OUT_OF_MEMORY) {
        project_memory_failure(project);
    } else if (parsed != PARSE_OK) {
        project_report(project, PROJECT_SYNTAX_ERROR, module->requested_by,
            module->requirement, module->source->logical_path, &module->syntax.error.diagnostic);
    } else if (module->syntax.sections.kind == MODULE_DRAFT) {
        project_report(project, PROJECT_DRAFT, module->requested_by,
            module->requested_by == NULL ? (SourceSpan){module->source->text, 0, 0}
                : module->requirement, module->source->logical_path, NULL);
    } else if (module == project->entry && module->syntax.sections.kind == MODULE_CONSTANTS) {
        project_report(project, PROJECT_INVALID_ENTRY, NULL,
            (SourceSpan){module->source->text, 0, 0}, module->source->logical_path, NULL);
    } else if (required_companions(project, module) && project_prepare_aliases(project, module)
        && project_discover(project, module)) {
        module->state = PROJECT_MODULE_READY;
    }
    return project->status == PROJECT_OK;
}
