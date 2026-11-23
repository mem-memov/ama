/** @file
 * @brief Assemble exact scalar policies and explicit raw scope rows without last-value fallbacks.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * forbids repeated execution modes and conflicting dependency policies;
 * deletion grants no replacement or dependency-copy permission.
 */
#include "internal.h"
#include <string.h>
/** Reject equal raw keys in one list and contradictory families across lists.
 * Physical aliases using different spellings remain common capture obligations. */
static ProjectSourceRequestStatus root(ProjectSourceRequestWork *work, const ProjectSourceOption *option, size_t argument) {
    ProjectSourceRequest *request = work->request;
    bool destination = option->kind == PROJECT_SOURCE_OPTION_DESTINATION_ROOT || option->kind == PROJECT_SOURCE_OPTION_DESTINATION_STANDARD;
    ModuleRootKind kind = option->kind == PROJECT_SOURCE_OPTION_STANDARD || option->kind == PROJECT_SOURCE_OPTION_DESTINATION_STANDARD
        ? MODULE_ROOT_STANDARD : MODULE_ROOT_PROJECT;
    ProjectSourceRootRequest *rows = destination ? request->destination_roots : request->roots;
    size_t count = destination ? request->destination_root_count : request->root_count;
    const char *path = request->arguments[argument + 1];
    for (size_t i = 0; i < count; i++) { if (!strcmp(rows[i].value.path, path)) { return PROJECT_SOURCE_REQUEST_ROOTS; } }
    const ProjectSourceRootRequest *other = destination ? request->roots : request->destination_roots;
    size_t other_count = destination ? request->root_count : request->destination_root_count;
    for (size_t i = 0; i < other_count; i++) {
        if (!strcmp(other[i].value.path, path) && other[i].value.kind != kind) { return PROJECT_SOURCE_REQUEST_ROOTS; }
    }
    rows[count] = (ProjectSourceRootRequest){.value = {.path = path, .kind = kind}, .argument = argument + 1};
    if (destination) { request->destination_root_count++; } else { request->root_count++; }
    return PROJECT_SOURCE_REQUEST_OK;
}
/** Scalar flags have bounded distinct bits, while root/exclusion rows consume
 * distinct argv tokens and therefore fit the reserved argument-count capacity. */
ProjectSourceRequestStatus project_source_request_option(ProjectSourceRequestWork *work,
    const ProjectSourceOption *option, size_t argument) {
    ProjectSourceRequest *request = work->request;
    if (option->kind <= PROJECT_SOURCE_OPTION_COMPILER) {
        unsigned bit = 1u << option->kind;
        if (work->seen & bit) { return PROJECT_SOURCE_REQUEST_REPEATED; }
        work->seen |= bit;
    }
    unsigned modes = (1u << PROJECT_SOURCE_OPTION_PREVIEW) | (1u << PROJECT_SOURCE_OPTION_APPLY);
    if ((work->seen & modes) == modes) { return PROJECT_SOURCE_REQUEST_MODE; }
    if ((work->seen & ((1u << PROJECT_SOURCE_OPTION_COPY) | (1u << PROJECT_SOURCE_OPTION_EXTEND)))
        == ((1u << PROJECT_SOURCE_OPTION_COPY) | (1u << PROJECT_SOURCE_OPTION_EXTEND))) { return PROJECT_SOURCE_REQUEST_POLICY; }
    if (request->kind == SOURCE_OPERATION_DELETE && (option->kind == PROJECT_SOURCE_OPTION_COPY
        || option->kind == PROJECT_SOURCE_OPTION_EXTEND || option->kind == PROJECT_SOURCE_OPTION_REPLACE
        || option->kind == PROJECT_SOURCE_OPTION_DESTINATION_ROOT || option->kind == PROJECT_SOURCE_OPTION_DESTINATION_STANDARD
        || option->kind == PROJECT_SOURCE_OPTION_DESTINATION_EXCLUDE)) { return PROJECT_SOURCE_REQUEST_SCOPE; }
    switch (option->kind) {
        case PROJECT_SOURCE_OPTION_PREVIEW: request->mode = PROJECT_SOURCE_PREVIEW; break;
        case PROJECT_SOURCE_OPTION_APPLY: request->mode = PROJECT_SOURCE_APPLY; break;
        case PROJECT_SOURCE_OPTION_COPY: request->dependencies = SOURCE_COPY_CLONE_DEPENDENCIES; break;
        case PROJECT_SOURCE_OPTION_EXTEND: request->dependencies = SOURCE_COPY_EXTEND_PACKAGE; break;
        case PROJECT_SOURCE_OPTION_REPLACE: request->replace = true; break;
        case PROJECT_SOURCE_OPTION_COMPILER: request->compiler = request->arguments[argument + 1]; break;
        case PROJECT_SOURCE_OPTION_ROOT: case PROJECT_SOURCE_OPTION_STANDARD:
        case PROJECT_SOURCE_OPTION_DESTINATION_ROOT: case PROJECT_SOURCE_OPTION_DESTINATION_STANDARD:
            return root(work, option, argument);
        case PROJECT_SOURCE_OPTION_EXCLUDE: case PROJECT_SOURCE_OPTION_DESTINATION_EXCLUDE:
            work->exclusions[work->exclusion_count++] = (ProjectSourceExclusion){
                request->arguments[argument + 1], request->arguments[argument + 2], argument + 2,
                option->kind == PROJECT_SOURCE_OPTION_DESTINATION_EXCLUDE};
            break;
    }
    return PROJECT_SOURCE_REQUEST_OK;
}
