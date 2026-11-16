/** @file
 * @brief Compare observable input metadata explicitly instead of comparing unspecified struct padding.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses changed file identities before later replacement.
 */
#include "../internal.h"
/** Reads may update access time; identity, ownership, type, bytes' length and modification/change times must remain stable. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_staging_metadata(const struct stat *left, const struct stat *right) {
    return left->st_dev == right->st_dev && left->st_ino == right->st_ino
        && left->st_mode == right->st_mode && left->st_uid == right->st_uid
        && left->st_gid == right->st_gid && left->st_nlink == right->st_nlink
        && left->st_size == right->st_size
        && left->st_mtim.tv_sec == right->st_mtim.tv_sec && left->st_mtim.tv_nsec == right->st_mtim.tv_nsec
        && left->st_ctim.tv_sec == right->st_ctim.tv_sec && left->st_ctim.tv_nsec == right->st_ctim.tv_nsec;
}
