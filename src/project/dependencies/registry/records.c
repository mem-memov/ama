/**
 * @file
 * @brief Register exact required modules and dependency sites without duplicate loading.
 *
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02)
 * Reuse existing loading/ready records by complete supported identity, never root search precedence.
 */
#include <string.h>
#include "project/dependencies/internal.h"

ProjectModule *project_lookup(Project *project, const char *path) {
    for (ProjectModule *module = project->first; module != NULL; module = module->next) {
        if (strcmp(module->source->logical_path, path) == 0) {
            return module;
        }
    }
    return NULL;
}

const ProjectModule *project_find(const Project *project, const char *logical_path) {
    if (!project->valid) {
        return NULL;
    }
    for (const ProjectModule *module = project->first; module != NULL; module = module->next) {
        if (strcmp(module->source->logical_path, logical_path) == 0) {
            return module;
        }
    }
    return NULL;
}

bool project_require(Project *project, ProjectModule *requester,
    const char *path, SourceSpan requirement, ProjectModule **output) {
    ProjectModule *existing = project_lookup(project, path);
    if (existing != NULL) {
        *output = existing;
        return true;
    }
    const ModuleSource *source = NULL;
    ModuleStatus status = module_catalog_find(&project->catalog, path, &source);
    if (status != MODULE_OK) {
        return project_report(project, PROJECT_MISSING, requester, requirement, path, NULL);
    }
    ProjectModule *module = source_arena_allocate(&project->arena, sizeof(*module));
    if (module == NULL) {
        return project_memory_failure(project);
    }
    *module = (ProjectModule){.source = source, .requested_by = requester,
        .requirement = requirement, .state = PROJECT_MODULE_LOADING};
    ast_module_init(&module->syntax, project->arena.allocator);
    if (project->last == NULL) {
        project->first = module;
    } else {
        project->last->next = module;
    }
    project->last = module;
    *output = module;
    return true;
}

bool project_add_dependency(Project *project, ProjectModule *module, SourceSpan path) {
    size_t length = path.end - path.start;
    char *logical = source_arena_allocate(&project->arena, length + 1);
    ProjectDependency *dependency = source_arena_allocate(&project->arena, sizeof(*dependency));
    if (logical == NULL || dependency == NULL) {
        return project_memory_failure(project);
    }
    memcpy(logical, source_bytes(path.source) + path.start, length);
    logical[length] = 0;
    *dependency = (ProjectDependency){.requirement = path, .logical_path = logical};
    if (!project_require(project, module, logical, path, &dependency->target)) {
        return false;
    }
    if (module->last == NULL) {
        module->first = dependency;
    } else {
        module->last->next = dependency;
    }
    module->last = dependency;
    return true;
}
