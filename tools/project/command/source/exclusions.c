/** @file
 * @brief Attach exclusions to an exact raw root key independently of option order.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * records explicit root/exclusion settings; a shorter prefix or another root's
 * same basename never supplies authority for the requested scope.
 */
#include "internal.h"
#include <stdint.h>
#include <string.h>
/** Exact relative physical keys contain nonempty ordinary components; no slash
 * folding, parent navigation or logical-module-name validation is performed. */
static bool relative(const char *path) {
    if (!path || !*path || *path == '/') { return false; }
    while (*path) {
        const char *end = strchr(path, '/'); size_t length = end ? (size_t)(end - path) : strlen(path);
        if (!length || (length == 1 && path[0] == '.') || (length == 2 && path[0] == '.' && path[1] == '.')) { return false; }
        if (!end) { return true; }
        path = end + 1; if (!*path) { return false; }
    }
    return false;
}
/** Require exactly an existing explicit list member, never a namespace guess. */
static ProjectSourceRootRequest *scope(ProjectSourceRequest *request, const ProjectSourceExclusion *exclusion) {
    ProjectSourceRootRequest *rows = exclusion->destination ? request->destination_roots : request->roots;
    size_t count = exclusion->destination ? request->destination_root_count : request->root_count;
    for (size_t i = 0; i < count; i++) { if (!strcmp(rows[i].value.path, exclusion->root)) { return &rows[i]; } }
    return NULL;
}
/** Count before allocation and fill in original option order. Strings remain
 * borrowed; failed partial arrays stay in the request arena until destruction. */
static ProjectSourceRequestStatus attach(ProjectSourceRequestWork *work, ProjectSourceRootRequest *row, bool destination) {
    size_t count = 0;
    for (size_t i = 0; i < work->exclusion_count; i++) {
        const ProjectSourceExclusion *item = &work->exclusions[i];
        count += item->destination == destination && !strcmp(item->root, row->value.path);
    }
    if (count > SIZE_MAX / sizeof(const char *)) { return PROJECT_SOURCE_REQUEST_LIMIT; }
    const char **paths = count ? source_arena_allocate(&work->request->arena, count * sizeof(*paths)) : NULL;
    if (count && !paths) { return PROJECT_SOURCE_REQUEST_MEMORY; }
    row->value.excluded = paths; row->value.excluded_count = count;
    size_t used = 0;
    for (size_t i = 0; i < work->exclusion_count; i++) {
        const ProjectSourceExclusion *item = &work->exclusions[i];
        if (item->destination == destination && !strcmp(item->root, row->value.path)) { paths[used++] = item->path; }
    }
    return PROJECT_SOURCE_REQUEST_OK;
}
ProjectSourceRequestStatus project_source_request_exclusions(ProjectSourceRequestWork *work) {
    ProjectSourceRequest *request = work->request;
    for (size_t i = 0; i < work->exclusion_count; i++) {
        const ProjectSourceExclusion *item = &work->exclusions[i]; request->failed_argument = item->argument;
        if (!relative(item->path)) { return PROJECT_SOURCE_REQUEST_EXCLUSION; }
        if (!scope(request, item)) { request->failed_argument = item->argument - 1; return PROJECT_SOURCE_REQUEST_EXCLUSION; }
        for (size_t j = 0; j < i; j++) {
            const ProjectSourceExclusion *old = &work->exclusions[j];
            if (old->destination == item->destination && !strcmp(old->root, item->root) && !strcmp(old->path, item->path)) {
                return PROJECT_SOURCE_REQUEST_REPEATED;
            }
        }
    }
    for (size_t i = 0; i < request->root_count; i++) {
        ProjectSourceRequestStatus status = attach(work, &request->roots[i], false);
        if (status != PROJECT_SOURCE_REQUEST_OK) { return status; }
    }
    for (size_t i = 0; i < request->destination_root_count; i++) {
        ProjectSourceRequestStatus status = attach(work, &request->destination_roots[i], true);
        if (status != PROJECT_SOURCE_REQUEST_OK) { return status; }
    }
    return PROJECT_SOURCE_REQUEST_OK;
}
