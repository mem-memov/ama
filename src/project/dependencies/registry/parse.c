/**
 * @file
 * @brief Validate one required module and discover its structural dependencies.
 *
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * Required drafts/invalid entries fail with requester context rather than being silently skipped.
 */
#include "project/dependencies/internal.h"

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
    } else if (project_discover(project, module)) {
        module->state = PROJECT_MODULE_READY;
    }
    return project->status == PROJECT_OK;
}
