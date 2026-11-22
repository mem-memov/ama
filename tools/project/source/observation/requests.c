/** @file
 * @brief Observe only explicitly demanded original opaque paths and exact ancestor membership.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires original observations before projection; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps original roots/selection authoritative despite additional material reads.
 */
#include "internal.h"
#include <string.h>
/** Keep immutable owned prefixes so every retained callback witness survives. */
static const char *prefix(SourceObservedImage *observed, const char *path, size_t length) {
    char *copy = source_arena_allocate(&observed->arena, length + 1);
    if (copy) { memcpy(copy, path, length); copy[length] = 0; }
    return copy;
}
/** Select exact longest root; a request is structural, never relative traversal. */
static const char *root_for(SourceObservedImage *observed, const char *path) {
    const char *root = NULL;
    if (!path) { return NULL; }
    for (size_t i = 0; i < observed->original->root_count; i++) {
        const char *candidate = observed->original->roots[i].path;
        if (source_image_below(path, candidate) && (!root || strlen(candidate) > strlen(root))) { root = candidate; }
    }
    if (!root) { return NULL; }
    size_t boundary = strlen(root);
    if (!path[boundary]) { return root; }
    if (path[boundary] != '/') { return NULL; }
    const char *part = path + boundary + 1;
    while (*part) {
        const char *slash = strchr(part, '/'); size_t length = slash ? (size_t)(slash - part) : strlen(part);
        if (!length || (length == 1 && part[0] == '.') || (length == 2 && part[0] == '.' && part[1] == '.')) { return NULL; }
        if (!slash) { return root; }
        part = slash + 1;
    }
    return NULL;
}
/** Walk finitely many exact components; optional absence stops at its proof. */
static ModuleStatus request_path(SourceObservedImage *observed, const SourceObservationRequest *request,
    const char *root, struct stat *metadata) {
    ModuleStatus status = source_observation_root(observed, &observed->catalog, root, metadata);
    const char *parent = root, *cursor = request->path + strlen(root);
    while (status == MODULE_OK && *cursor) {
        const char *name = cursor + 1, *slash = strchr(name, '/');
        size_t length = slash ? (size_t)(slash - request->path) : strlen(request->path);
        const char *full = prefix(observed, request->path, length);
        const char *child = full ? full + (name - request->path) : NULL;
        if (!full) { return source_observation_fail(observed, &observed->catalog,
            SOURCE_OBSERVATION_MEMORY, MODULE_OUT_OF_MEMORY, request->path); }
        status = source_observation_child(observed, &observed->catalog, parent, child, full, metadata);
        parent = full; cursor = request->path + length;
    }
    return status;
}
SourceObservationStatus source_observation_requests(SourceObservedImage *observed,
    const SourceObservationRequest *requests, size_t count) {
    for (size_t i = 0; i < count; i++) {
        const SourceObservationRequest *request = &requests[i]; const char *root = root_for(observed, request->path);
        if (!root || request->kind < SOURCE_OBSERVE_METADATA || request->kind > SOURCE_OBSERVE_CHILDREN) {
            source_observation_fail(observed, &observed->catalog, SOURCE_OBSERVATION_INVALID, MODULE_INVALID_PATH, request->path);
            return observed->status;
        }
        struct stat metadata;
        ModuleStatus status = request_path(observed, request, root, &metadata);
        if (status == MODULE_MISSING && request->optional) { observed->catalog.issue = (ModuleIssue){0}; continue; }
        if (status != MODULE_OK) {
            source_observation_fail(observed, &observed->catalog, SOURCE_OBSERVATION_PHYSICAL, status, request->path);
            return observed->status;
        }
        if (request->kind == SOURCE_OBSERVE_CONTENT) {
            if (!S_ISREG(metadata.st_mode)) {
                source_observation_fail(observed, &observed->catalog, SOURCE_OBSERVATION_PHYSICAL, MODULE_UNSUPPORTED_PATH, request->path);
                return observed->status;
            }
            const SourceText *text = NULL;
            if (source_observation_read(observed, request->path, &text) != SOURCE_OK) { return observed->status; }
        } else if (request->kind == SOURCE_OBSERVE_CHILDREN) {
            const char **names = NULL; size_t children = 0;
            if (source_observation_entries(observed, &observed->catalog, request->path, &names, &children) != MODULE_OK) { return observed->status; }
        }
    }
    return SOURCE_OBSERVATION_OK;
}
