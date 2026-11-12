/**
 * @file
 * @brief Own requester-chain diagnostics and preserve original dependency failure context.
 *
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * First-discovery parent links form a finite forest; errors copy borrowed entry spellings into project ownership.
 */
#include <stdint.h>
#include "project/dependencies/internal.h"

bool project_memory_failure(Project *project) {
    project->status = PROJECT_OUT_OF_MEMORY;
    project->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Dependency loading could not allocate storage"};
    return false;
}

/** Construct a borrowed missing/draft/entry diagnostic with its exact request site and original path; no allocation or guessed repair.
 */
static Diagnostic dependency_error(ProjectStatus status, SourceSpan requirement, const char *path) {
    const char *message = "Required module source was not found";
    DiagnosticCode code = DIAGNOSTIC_MISSING_MODULE;
    if (status == PROJECT_DRAFT) {
        message = "Required module is an inactive draft";
        code = DIAGNOSTIC_SYNTAX;
    } else if (status == PROJECT_INVALID_ENTRY) {
        message = "The project entry must be an executable module";
        code = DIAGNOSTIC_TYPE_MISMATCH;
    }
    return (Diagnostic){.code = code, .message = message, .subject = path,
        .primary = {.span = requirement}};
}

bool project_report(Project *project, ProjectStatus status, ProjectModule *requester,
    SourceSpan requirement, const char *path, const Diagnostic *cause) {
    project->status = status;
    project->error = cause == NULL ? dependency_error(status, requirement, path) : *cause;
    /* Entry names are borrowed from callers, unlike catalog names and AST spans. */
    const char *owned_path = source_arena_copy(&project->arena, path);
    if (owned_path == NULL) {
        return project_memory_failure(project);
    }
    project->error.subject = owned_path;
    size_t count = cause == NULL ? 0 : cause->related_count;
    for (ProjectModule *module = requester; module != NULL; module = module->requested_by) {
        if (count >= SIZE_MAX / sizeof(DiagnosticRelated)) {
            return project_memory_failure(project);
        }
        count++;
    }
    if (count == 0) {
        return false;
    }
    DiagnosticRelated *notes = source_arena_allocate(&project->arena, count * sizeof(*notes));
    if (notes == NULL) {
        return project_memory_failure(project);
    }
    size_t index = 0;
    for (; index < project->error.related_count; index++) {
        notes[index] = project->error.related[index];
    }
    for (ProjectModule *module = requester; module != NULL; module = module->requested_by) {
        notes[index++] = (DiagnosticRelated){{requirement, NULL},
            "required by module", module->source->logical_path};
        requirement = module->requirement;
    }
    project->error.related = notes;
    project->error.related_count = count;
    return false;
}
