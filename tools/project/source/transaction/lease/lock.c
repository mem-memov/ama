/** @file
 * @brief Union strongest inode demands, order nonblocking locks and verify retained namespace links.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exclusive access across overlapping configured domains, without
 * serializing disjoint roots that share only ancestor directories.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/file.h>
#include "internal.h"
/** Actual directory inode keys identify the kernel flock object. */
static int order(const SourceLeaseDirectory *a, const SourceLeaseDirectory *b) {
    if (a->metadata.st_dev != b->metadata.st_dev) { return a->metadata.st_dev < b->metadata.st_dev ? -1 : 1; }
    return a->metadata.st_ino < b->metadata.st_ino ? -1 : a->metadata.st_ino != b->metadata.st_ino;
}
SourceLeaseStatus source_lease_lock(SourceTransactionLease *lease) {
    size_t count = 0;
    for (SourceLeaseDirectory *row = lease->directories; row; row = row->next) {
        if (count == SIZE_MAX) { return source_lease_fail(lease, SOURCE_LEASE_MEMORY, row->path, 0); } count++;
    }
    if (count > SIZE_MAX / sizeof(SourceLeaseDirectory *)) { return source_lease_fail(lease, SOURCE_LEASE_MEMORY, NULL, 0); }
    SourceLeaseDirectory **rows = source_arena_allocate(&lease->arena, count * sizeof(*rows));
    if (!rows) { return source_lease_fail(lease, SOURCE_LEASE_MEMORY, NULL, 0); }
    size_t used = 0;
    for (SourceLeaseDirectory *row = lease->directories; row; row = row->next) {
        size_t index = 0; while (index < used && order(rows[index], row) < 0) { index++; }
        if (index < used && !order(rows[index], row)) { rows[index]->exclusive |= row->exclusive; continue; }
        memmove(rows + index + 1, rows + index, (used - index) * sizeof(*rows)); rows[index] = row; used++;
    }
    for (size_t i = 0; i < used; i++) {
        SourceLeaseDirectory *row = rows[i];
        if (!source_lease_permit(lease, SOURCE_LEASE_LOCK, row->path)) { return lease->status; }
        if (flock(row->descriptor, (row->exclusive ? LOCK_EX : LOCK_SH) | LOCK_NB) != 0) {
            int error = errno; return source_lease_fail(lease, error == EWOULDBLOCK || error == EAGAIN ? SOURCE_LEASE_BUSY : SOURCE_LEASE_IO, row->path, error);
        }
    }
    for (SourceLeaseDirectory *row = lease->directories; row; row = row->next) {
        if (!source_lease_permit(lease, SOURCE_LEASE_VERIFY, row->path)) { return lease->status; }
        struct stat current;
        int result = row->parent ? fstatat(row->parent->descriptor, row->name, &current, AT_SYMLINK_NOFOLLOW) : lstat(row->path, &current);
        if (result != 0) { return source_lease_fail(lease, SOURCE_LEASE_IO, row->path, errno); }
        if (!S_ISDIR(current.st_mode) || current.st_dev != row->metadata.st_dev || current.st_ino != row->metadata.st_ino
            || current.st_mode != row->metadata.st_mode) { return source_lease_fail(lease, SOURCE_LEASE_CHANGED, row->path, 0); }
    }
    return SOURCE_LEASE_OK;
}
