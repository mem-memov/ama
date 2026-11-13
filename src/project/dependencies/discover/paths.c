/**
 * @file
 * @brief Retain one dependency request per written module-path occurrence.
 * [DEP-02](../../../../spec/implementation/compiler.md#dep-02) shares loaded
 * modules without discarding the requesting spans of distinct syntactic edges.
 */
#include <stdint.h>
#include "internal.h"
bool project_collect_path(Project *project, DependencyPaths *paths, SourceSpan path, const char *implicit) {
    if (paths->count == SIZE_MAX) { return project_memory_failure(project); }
    DependencyPath *request = source_arena_allocate(&project->arena, sizeof(*request));
    if (request == NULL) { return project_memory_failure(project); }
    *request = (DependencyPath){.next = paths->first, .path = path, .implicit = implicit};
    paths->first = request;
    paths->count++;
    return true;
}
