/**
 * @file
 * @brief Verify exact directory entries before inspecting their filesystem metadata.
 *
 * [SRC-02](../../../spec/language/source-and-names.md#src-02)
 * Reject spelling aliases and unsupported symlinks rather than guessing a match.
 */
#include <stdbool.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "exact.h"

PathProbe path_exact_child(const char *parent, const char *name,
    const char *full_path, struct stat *metadata) {
    DIR *directory = opendir(parent);
    if (directory == NULL) {
        return PATH_IO_ERROR;
    }
    bool found = false;
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, name) == 0) {
            found = true;
            break;
        }
    }
    int read_error = errno;
    int closed = closedir(directory);
    if (read_error != 0 || closed != 0) {
        return PATH_IO_ERROR;
    }
    if (!found) {
        if (lstat(full_path, metadata) == 0) {
            return PATH_UNSUPPORTED;
        }
        return errno == ENOENT ? PATH_MISSING : PATH_IO_ERROR;
    }
    if (lstat(full_path, metadata) != 0) {
        return PATH_IO_ERROR;
    }
    return S_ISLNK(metadata->st_mode) ? PATH_UNSUPPORTED : PATH_FOUND;
}
