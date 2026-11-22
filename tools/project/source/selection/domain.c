/** @file
 * @brief Reject unanalysed and ignored physical copies without guessing namespace ownership.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves the original root boundary; [PKG-02](../../../../spec/project/packages.md#pkg-02)
 * never supplies missing modules/configuration from an ignored version copy.
 */
#include <string.h>
#include "domain.h"
#include "internal.h"
#include "source/path/version.h"
/** Find an ordinary storage version boundary using the same lexical predicate as
 * package discovery. The first version component ends the package namespace;
 * similarly named module descendants are never interpreted as package versions.
 */
static const char *version_end(const char *relative, bool leaf_directory) {
    const char *cursor;
    if (!strncmp(relative, "packages/", 9)) { cursor = relative + 9; }
    else if (!strncmp(relative, "dependencies/", 13)) { cursor = relative + 13; }
    else { return NULL; }
    while (*cursor) {
        const char *slash = strchr(cursor, '/');
        size_t length = slash ? (size_t)(slash - cursor) : strlen(cursor);
        if ((slash || leaf_directory) && module_version_component((const unsigned char *)cursor, length)) { return cursor + length; }
        if (!slash) { return NULL; }
        cursor = slash + 1;
    }
    return NULL;
}
/** A matching complete selected physical version is authority; logical-name equality is insufficient. */
static bool selected_version(const SourceRootCapture *capture, const char *path, const char *end) {
    size_t length = (size_t)(end - path);
    for (const ModuleVersion *version = capture->catalog.versions; version; version = version->next) {
        if (strlen(version->physical_path) == length && !memcmp(version->physical_path, path, length)) { return true; }
    }
    return false;
}
/** Root scopes combine by union. A covering independently explicit root is not
 * silently replaced by the first matching physical-prefix descriptor. */
static SourceTreeDomainStatus member(const SourceRootCapture *capture, const AcquisitionUnitEntry *entry) {
    const char *path = entry->file.path;
    bool excluded = false, unselected = false;
    for (size_t index = 0; index < capture->root_count; index++) {
        const ModuleRoot *root = &capture->roots[index];
        if (!source_tree_inside(path, root->path)) { continue; }
        size_t length = strlen(root->path);
        const char *relative = path + length;
        if (*relative == '/') { relative++; }
        if (module_path_excluded(root, relative)) { excluded = true; continue; }
        const char *end = root->kind == MODULE_ROOT_PROJECT ? version_end(relative, entry->directory) : NULL;
        if (end && !selected_version(capture, path, end)) { unselected = true; continue; }
        return SOURCE_TREE_DOMAIN_OK;
    }
    return unselected ? SOURCE_TREE_DOMAIN_UNSELECTED_VERSION
        : excluded ? SOURCE_TREE_DOMAIN_EXCLUDED : SOURCE_TREE_DOMAIN_OUTSIDE;
}
SourceTreeDomainStatus source_tree_selection_domain(const SourceTreeSelection *selection, const char **failed_path) {
    if (!failed_path) { return SOURCE_TREE_DOMAIN_INVALID; }
    *failed_path = NULL;
    if (!selection || !selection->valid || !selection->capture || !selection->capture->valid) { return SOURCE_TREE_DOMAIN_INVALID; }
    for (const AcquisitionUnitEntry *entry = selection->members; entry; entry = entry->next) {
        SourceTreeDomainStatus status = member(selection->capture, entry);
        if (status != SOURCE_TREE_DOMAIN_OK) { *failed_path = entry->file.path; return status; }
    }
    return SOURCE_TREE_DOMAIN_OK;
}
