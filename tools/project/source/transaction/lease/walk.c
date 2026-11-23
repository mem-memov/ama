/** @file
 * @brief Retain exact ancestor descriptors without following symbolic components.
 * [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 * rejects filesystem aliases; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires unchanged original root identities before application.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** Copy a bounded component/prefix into the lease arena, retaining exact bytes. */
static char *copy(SourceTransactionLease *lease, const char *bytes, size_t length) {
    if (length == SIZE_MAX) { return NULL; }
    char *result = source_arena_allocate(&lease->arena, length + 1);
    if (result) { memcpy(result, bytes, length); result[length] = 0; } return result;
}
/** Register ownership before opening so every host/fstat failure is destructible. */
static SourceLeaseDirectory *directory(SourceTransactionLease *lease, SourceLeaseDirectory *parent,
    const char *path, size_t length, const char *name, size_t width) {
    for (SourceLeaseDirectory *row = lease->directories; row; row = row->next) {
        if (strlen(row->path) == length && !memcmp(row->path, path, length)) { return row; }
    }
    SourceLeaseDirectory *row = source_arena_allocate(&lease->arena, sizeof(*row));
    if (!row) { source_lease_fail(lease, SOURCE_LEASE_MEMORY, path, 0); return NULL; }
    *row = (SourceLeaseDirectory){.next = lease->directories, .parent = parent, .descriptor = -1};
    lease->directories = row;
    row->path = copy(lease, path, length); row->name = name ? copy(lease, name, width) : NULL;
    if (!row->path || (name && !row->name)) { source_lease_fail(lease, SOURCE_LEASE_MEMORY, path, 0); return NULL; }
    if (!source_lease_permit(lease, SOURCE_LEASE_OPEN, row->path)) { return NULL; }
    int flags = O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC;
    row->descriptor = parent ? openat(parent->descriptor, row->name, flags) : open(row->path, flags);
    if (row->descriptor < 0) { source_lease_fail(lease, SOURCE_LEASE_IO, row->path, errno); return NULL; }
    if (!source_lease_permit(lease, SOURCE_LEASE_STAT, row->path)) { return NULL; }
    if (fstat(row->descriptor, &row->metadata) != 0) { source_lease_fail(lease, SOURCE_LEASE_IO, row->path, errno); return NULL; }
    return row;
}
SourceLeaseStatus source_lease_walk(SourceTransactionLease *lease) {
    for (size_t i = 0; i < lease->original->root_count; i++) {
        const char *path = lease->original->roots[i].path;
        if (!path || path[0] != '/') { return source_lease_fail(lease, SOURCE_LEASE_INVALID, path, 0); }
        size_t prefix = 0; while (path[prefix] == '/') { prefix++; }
        SourceLeaseDirectory *parent = directory(lease, NULL, path, prefix, NULL, 0);
        if (!parent) { return lease->status; }
        const char *cursor = path + prefix;
        while (*cursor) {
            const char *slash = strchr(cursor, '/'); size_t width = slash ? (size_t)(slash - cursor) : strlen(cursor);
            if (!width || (width == 1 && cursor[0] == '.') || (width == 2 && !memcmp(cursor, "..", 2)) || (slash && !slash[1])) {
                return source_lease_fail(lease, SOURCE_LEASE_INVALID, path, 0);
            }
            parent = directory(lease, parent, path, (size_t)(cursor - path) + width, cursor, width);
            if (!parent) { return lease->status; }
            if (!slash) { break; } cursor = slash + 1;
        }
        const SourceCatalogImageEntry *original = NULL;
        for (size_t j = 0; j < lease->original->count; j++) {
            if (!strcmp(lease->original->entries[j].path, path)) { original = &lease->original->entries[j]; break; }
        }
        if (!original || !original->metadata_captured || !S_ISDIR(original->metadata.st_mode)
            || original->metadata.st_dev != parent->metadata.st_dev || original->metadata.st_ino != parent->metadata.st_ino
            || original->metadata.st_mode != parent->metadata.st_mode) { return source_lease_fail(lease, SOURCE_LEASE_CHANGED, path, 0); }
        parent->exclusive = true;
    }
    return SOURCE_LEASE_OK;
}
