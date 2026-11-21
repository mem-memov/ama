/** @file
 * @brief Retain a no-symlink parent directory and its nonblocking exclusive lease.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * rejects unsupported aliases and concurrent publication before creating files.
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include "internal.h"
/** Walk each component through the previous retained directory descriptor. */
static bool open_parent(ProjectInitState *state) {
    const bool absolute = state->parent_path[0] == '/';
    int descriptor = open(absolute ? "/" : ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (descriptor < 0) { return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot open the starting directory"); }
    state->parent = descriptor;
    char *component = (char *)state->parent_path + (absolute ? 1 : 0);
    while (*component) {
        while (*component == '/') { component++; }
        if (!*component) { break; }
        char *end = component;
        while (*end && *end != '/') { end++; }
        char saved = *end; *end = 0;
        int child = openat(state->parent, component, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        *end = saved;
        if (child < 0) { return project_init_fail(state, PROJECT_INIT_IO, errno, "Destination parent components must be existing ordinary directories"); }
        (void)close(state->parent); state->parent = child;
        component = end;
    }
    return true;
}
bool project_init_parent(ProjectInitState *state, const char *destination) {
    if (!project_init_permit(state, PROJECT_INIT_PARENT, destination)) { return false; }
    size_t length = strlen(destination);
    state->path = state->allocator.allocate(state->allocator.context, length + 1);
    if (!state->path) { return project_init_fail(state, PROJECT_INIT_MEMORY, 0, "Cannot retain the destination path"); }
    memcpy(state->path, destination, length + 1);
    while (length && state->path[length - 1] == '/') { state->path[--length] = 0; }
    char *slash = strrchr(state->path, '/');
    state->name = slash ? slash + 1 : state->path;
    if (!*state->name) {
        return project_init_fail(state, PROJECT_INIT_INVALID, 0, "The destination must name a child directory");
    }
    if (!strcmp(state->name, ".") || !strcmp(state->name, "..")) {
        /* Verify every authored component before resolving the existing dot
         * endpoint. realpath alone would silently admit symbolic links. The
         * physical endpoint is then checked against the retained descriptor. */
        state->parent_path = state->path;
        if (!open_parent(state)) { return false; }
        char *physical = realpath(destination, NULL);
        if (!physical) {
            return project_init_fail(state, errno == ENOMEM ? PROJECT_INIT_MEMORY : PROJECT_INIT_IO,
                errno, "Cannot identify the existing destination endpoint");
        }
        struct stat retained, resolved;
        bool identical = fstat(state->parent, &retained) == 0 && lstat(physical, &resolved) == 0 &&
            retained.st_dev == resolved.st_dev && retained.st_ino == resolved.st_ino;
        if (!identical) { free(physical); return project_init_fail(state, PROJECT_INIT_CONFLICT, 0, "The destination endpoint changed during resolution"); }
        (void)close(state->parent); state->parent = -1;
        state->allocator.release(state->allocator.context, state->path); state->path = NULL;
        /* A physical absolute endpoint has no terminal dot component, so this
         * second pass is the last one; '/' refuses as a non-child destination. */
        bool ready = project_init_parent(state, physical);
        free(physical); return ready;
    }
    if (slash) { *slash = 0; state->parent_path = slash == state->path ? "/" : state->path; }
    else { state->parent_path = "."; }
    /* Literal '.' and '/' contain no mutable separators; traversal never writes
     * them: skip the sole '.' component and the empty absolute component. */
    if (!strcmp(state->parent_path, ".")) {
        state->parent = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (state->parent < 0) { return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot open the current directory"); }
    } else if (!open_parent(state)) { return false; }
    if (!project_init_permit(state, PROJECT_INIT_LEASE, destination)) { return false; }
    if (flock(state->parent, LOCK_EX | LOCK_NB) != 0) {
        int error = errno;
        return project_init_fail(state, error == EWOULDBLOCK || error == EAGAIN ? PROJECT_INIT_BUSY : PROJECT_INIT_IO,
            error, "Cannot acquire exclusive project initialization access");
    }
    return true;
}
