/** @file
 * @brief Check destination emptiness through no-follow descriptor-relative probes.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * forbids merging, overwriting or following filesystem aliases.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
bool project_init_preflight(ProjectInitState *state) {
    if (!project_init_permit(state, PROJECT_INIT_PREFLIGHT, state->name)) { return false; }
    struct stat info;
    if (fstatat(state->parent, state->name, &info, AT_SYMLINK_NOFOLLOW) != 0) {
        return errno == ENOENT || project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot inspect the destination");
    }
    if (!S_ISDIR(info.st_mode)) { return project_init_fail(state, PROJECT_INIT_CONFLICT, 0, "The destination is not an ordinary directory"); }
    int descriptor = openat(state->parent, state->name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (descriptor < 0) { return project_init_fail(state, PROJECT_INIT_CONFLICT, errno, "The destination changed during preflight"); }
    DIR *directory = fdopendir(descriptor);
    if (!directory) { int error = errno; (void)close(descriptor); return project_init_fail(state, PROJECT_INIT_IO, error, "Cannot enumerate the destination"); }
    bool empty = true;
    errno = 0;
    struct dirent *entry;
    while ((entry = readdir(directory))) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) { empty = false; break; }
    }
    int error = errno;
    int closed = closedir(directory);
    if (!empty) { return project_init_fail(state, PROJECT_INIT_CONFLICT, 0, "The destination is nonempty; no files were changed"); }
    if (error || closed) { return project_init_fail(state, PROJECT_INIT_IO, error ? error : errno, "Cannot finish checking the destination"); }
    return true;
}
