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

ProjectStatus project_load(Project *project, const ModuleRoot *roots,
    size_t root_count, const char *entry) {
    assert(!project->attempted && module_path_valid(entry));
    project->attempted = true;
    ModuleStatus status = module_catalog_build(&project->catalog, roots, root_count);
    if (status != MODULE_OK) {
        project->status = status == MODULE_OUT_OF_MEMORY
            ? PROJECT_OUT_OF_MEMORY : PROJECT_CATALOG_ERROR;
        bool adapted = diagnostic_from_module(&project->catalog.issue,
            &project->catalog_related, &project->error);
        assert(adapted);
        (void)adapted;
        return project->status;
    }
    if (!project_require(project, NULL, entry, (SourceSpan){0}, &project->entry)) {
        return project->status;
    }
    for (ProjectModule *module = project->first; module != NULL; module = module->next) {
        if (!project_parse_required(project, module)) {
            return project->status;
        }
    }
    project->valid = true;
    return PROJECT_OK;
}
