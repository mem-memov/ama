/** @file
 * @brief Remove only known operation-owned scaffold entries after a failed preparation.
 * [PROJECT-02](../../../spec/implementation/project-tools.md#project-02)
 * requires reporting retained staging rather than recursive deletion of unknown files.
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "internal.h"
/** Missing owned entries are already clean; other failures retain recovery state. */
static bool remove_entry(int parent, const char *name, int flags) {
    return unlinkat(parent, name, flags) == 0 || errno == ENOENT;
}
/** Descriptor identity prevents deleting a different directory now at the name. */
static bool same(int parent, const char *name, int descriptor) {
    struct stat expected, actual;
    return descriptor >= 0 && fstat(descriptor, &expected) == 0 &&
        fstatat(parent, name, &actual, AT_SYMLINK_NOFOLLOW) == 0 &&
        S_ISDIR(actual.st_mode) && expected.st_dev == actual.st_dev && expected.st_ino == actual.st_ino;
}
/** Remove only planned entries through retained descriptors. Unknown entries
 * cause rmdir refusal and preserve the stage for caller-owned recovery. The
 * orchestrator retains the primary error independently of cleanup success. */
static bool remove_stage(ProjectInitState *state) {
    if (!project_init_permit(state, PROJECT_INIT_CLEANUP, state->stage_path)) { return false; }
    if (state->stage < 0) {
        /* No entry was created inside an unopened stage. rmdir refuses any
         * nonempty replacement, and never traverses a symlink. */
        return remove_entry(state->parent, state->stage_name, AT_REMOVEDIR);
    }
    bool complete = true;
    if (state->packages >= 0) {
        if (!remove_entry(state->packages, "README.md", 0)) { complete = false; }
        if (!same(state->stage, "packages", state->packages) ||
            !remove_entry(state->stage, "packages", AT_REMOVEDIR)) { complete = false; }
    } else if (!remove_entry(state->stage, "packages", AT_REMOVEDIR)) { complete = false; }
    if (!remove_entry(state->stage, "README.md", 0)) { complete = false; }
    if (!remove_entry(state->stage, "AGENTS.md", 0)) { complete = false; }
    if (!same(state->parent, state->stage_name, state->stage) ||
        !remove_entry(state->parent, state->stage_name, AT_REMOVEDIR)) { complete = false; }
    return complete;
}
void project_init_cleanup(ProjectInitState *state) {
    if (state->staged && !remove_stage(state)) {
        state->result.cleanup_failed = true;
        state->result.retained_stage = state->stage_path;
        state->stage_path = NULL;
    }
    if (state->packages >= 0) { (void)close(state->packages); }
    if (state->stage >= 0) { (void)close(state->stage); }
    if (state->parent >= 0) { (void)close(state->parent); }
    state->allocator.release(state->allocator.context, state->stage_path);
    state->allocator.release(state->allocator.context, state->path);
}
