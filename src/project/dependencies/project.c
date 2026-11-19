/**
 * @file
 * @brief Compose catalog construction and iterative required-module loading.
 *
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03)
 * Only a complete valid closure is consumable; partial AST/catalog ownership survives failure until destroy.
 */
#include <assert.h>
#include "internal.h"

void project_init(Project *project, SourceAllocator allocator) {
    *project = (Project){0};
    module_catalog_init(&project->catalog, allocator);
    source_arena_init(&project->arena, allocator);
}

void project_destroy(Project *project) {
    for (ProjectModule *module = project->first; module != NULL; module = module->next) {
        ast_module_destroy(&module->syntax);
    }
    source_arena_destroy(&project->arena);
    module_catalog_destroy(&project->catalog);
    *project = (Project){0};
}

ProjectStatus project_load_required(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry, const char *const *required, size_t required_count) {
    assert(!project->attempted && module_path_valid(entry));
    ModuleStatus status = module_catalog_build(&project->catalog, roots, root_count);
    if (status != MODULE_OK) {
        project->attempted = true;
        project->status = status == MODULE_OUT_OF_MEMORY
            ? PROJECT_OUT_OF_MEMORY : PROJECT_CATALOG_ERROR;
        bool adapted = diagnostic_from_module(&project->catalog.issue,
            &project->catalog_related, &project->error);
        assert(adapted);
        (void)adapted;
        return project->status;
    }
    return project_load_catalog(project, entry, required, required_count);
}

/** Require the complete finite seed set before parsing so loading cycles share already registered modules. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
static ProjectStatus load_catalog_modules(Project *project, const char *const *required, size_t required_count) {
    for (size_t index = 0; index < required_count; index++) {
        ProjectModule *module = NULL;
        if (!project_require(project, NULL, required[index], (SourceSpan){0}, &module)) {
            return project->status;
        }
    }
    for (ProjectModule *module = project->first; module != NULL; module = module->next) {
        if (!project_parse_required(project, module)) { return project->status; }
    }
    if (!project_validate_kinds(project)) { return project->status; }
    project->valid = true;
    return PROJECT_OK;
}

/** Reuse one already owned immutable catalog for every required source, including explicit editor candidates. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load_catalog(Project *project, const char *entry,
    const char *const *required, size_t required_count) {
    assert(!project->attempted && project->catalog.built && module_path_valid(entry));
    project->attempted = true;
    if (!project_require(project, NULL, entry, (SourceSpan){0}, &project->entry)) { return project->status; }
    return load_catalog_modules(project,required,required_count);
}

/** Analysis domains reuse the same unique registry/parse queue without inventing a runnable entry. Component contract: [DEP-03](../../../spec/implementation/compiler.md#dep-03). */
ProjectStatus project_load_catalog_set(Project *project, const char *const *required, size_t required_count) {
    assert(!project->attempted && project->catalog.built && (required_count == 0 || required != NULL));
    project->attempted = true;
    return load_catalog_modules(project,required,required_count);
}

ProjectStatus project_load(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry) {
    return project_load_required(project, roots, root_count, entry, NULL, 0);
}
