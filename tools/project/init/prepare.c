/** @file
 * @brief Write all fixed scaffold files in an exclusively created sibling directory.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * forbids partial destination publication; all writes precede one rename.
 */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
/** Exclusive names remain private; every collision advances a finite counter. */
static bool stage(ProjectInitState *state) {
    size_t length = strlen(state->parent_path);
    if (length > SIZE_MAX - sizeof(state->stage_name) - 2) {
        return project_init_fail(state, PROJECT_INIT_MEMORY, 0, "The staging path exceeds addressable storage");
    }
    state->stage_path = state->allocator.allocate(state->allocator.context, length + sizeof(state->stage_name) + 2);
    if (!state->stage_path) { return project_init_fail(state, PROJECT_INIT_MEMORY, 0, "Cannot retain the staging path"); }
    for (size_t sequence = 0;; sequence++) {
        int written = snprintf(state->stage_name, sizeof(state->stage_name), ".ama-init-%ld-%zu", (long)getpid(), sequence);
        if (written < 0 || (size_t)written >= sizeof(state->stage_name)) {
            return project_init_fail(state, PROJECT_INIT_IO, 0, "Cannot represent a private staging name");
        }
        (void)sprintf(state->stage_path, "%s/%s", state->parent_path, state->stage_name);
        if (!project_init_permit(state, PROJECT_INIT_STAGE, state->stage_path)) { return false; }
        if (mkdirat(state->parent, state->stage_name, 0700) == 0) { state->staged = true; break; }
        if (errno != EEXIST || sequence == SIZE_MAX) {
            return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot create private project staging");
        }
    }
    state->stage = openat(state->parent, state->stage_name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    return state->stage >= 0 || project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot retain private staging");
}
/** Retry interrupted writes, require every byte and check close before publication. */
static bool file(ProjectInitState *state, int directory, const char *name, const ProjectInitFile *input) {
    if (!project_init_permit(state, PROJECT_INIT_FILE, input->path)) { return false; }
    int descriptor = openat(directory, name, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (descriptor < 0) { return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot create a scaffold file"); }
    const char *next = input->text;
    size_t remaining = strlen(next);
    bool complete = true;
    while (remaining) {
        ssize_t written = write(descriptor, next, remaining);
        if (written < 0 && errno == EINTR) { continue; }
        if (written <= 0) { complete = project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot write a complete scaffold file"); break; }
        next += written; remaining -= (size_t)written;
    }
    if (close(descriptor) != 0) { complete = project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot close a prepared scaffold file"); }
    return complete;
}
bool project_init_prepare(ProjectInitState *state) {
    if (!stage(state) || !project_init_permit(state, PROJECT_INIT_DIRECTORY, "packages")) { return false; }
    if (mkdirat(state->stage, "packages", 0700) != 0) {
        return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot prepare the packages directory");
    }
    state->packages = openat(state->stage, "packages", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (state->packages < 0) { return project_init_fail(state, PROJECT_INIT_IO, errno, "Cannot retain the prepared packages directory"); }
    size_t count;
    const ProjectInitFile *inputs = project_init_content(&count);
    for (size_t index = 0; index < count; index++) {
        bool nested = !strncmp(inputs[index].path, "packages/", 9);
        if (!file(state, nested ? state->packages : state->stage,
            inputs[index].path + (nested ? 9 : 0), &inputs[index])) { return false; }
    }
    return true;
}
