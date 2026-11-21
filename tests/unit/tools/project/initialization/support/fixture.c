/** @file
 * @brief Control initialization boundaries and verify the exact scaffold tree.
 * @see [T233 contract](../../../../../../docs/implementation/project-tool.md)
 */
#ifndef AMA_PROJECT_INIT_TEST_FIXTURE
#define AMA_PROJECT_INIT_TEST_FIXTURE
#include "../../common.c"
/** Check exact file membership and every generated guidance byte. */
static void assert_scaffold(void) {
    assert(members(root) == 1 && members(destination) == 3);
    char path[256]; (void)snprintf(path, sizeof(path), "%s/packages", destination);
    assert(members(path) == 1);
    size_t count; const ProjectInitFile *files = project_init_content(&count);
    for (size_t index = 0; index < count; index++) {
        (void)snprintf(path, sizeof(path), "%s/%s", destination, files[index].path);
        assert_file(path, files[index].text);
    }
}
/** Cancellation observation has no timing dependency or production global state. */
typedef struct {
    size_t calls, refuse;
    bool cleanup_refusal, conflict, unexpected, file_conflict;
    char staging[256];
    int start, completed;
} Control;
/** Admit or cancel a deterministic boundary and coordinate controlled competing effects. */
static bool permit(void *context, ProjectInitOperation operation, const char *path) {
    (void)path;
    Control *control = context;
    control->calls++;
    if (operation == PROJECT_INIT_STAGE) {
        int length = snprintf(control->staging, sizeof(control->staging), "%s", path);
        assert(length > 0 && (size_t)length < sizeof(control->staging));
    }
    if (operation == PROJECT_INIT_FILE && (control->unexpected || control->file_conflict)) {
        char extra[512];
        (void)snprintf(extra, sizeof(extra), "%s/%s", control->staging, control->file_conflict ? path : "user data");
        write_file(extra, "keep");
        if (control->unexpected) { return false; }
    }
    if (operation == PROJECT_INIT_CLEANUP && control->cleanup_refusal) { return false; }
    if (operation == PROJECT_INIT_PUBLISH && control->conflict) {
        int made = mkdir(destination, 0700); assert(!made);
        char file[256]; (void)snprintf(file, sizeof(file), "%s/user", destination); write_file(file, "keep");
    }
    if (operation == PROJECT_INIT_PREFLIGHT && control->start >= 0) {
        ssize_t sent = write(control->start, "x", 1); assert(sent == 1);
        int closed = close(control->start); assert(!closed); control->start = -1;
        char result; ssize_t received = read(control->completed, &result, 1); assert(received == 1 && result == '1');
        closed = close(control->completed); assert(!closed); control->completed = -1;
    }
    return control->calls != control->refuse;
}
/** Run the real initializer with counted allocation and optional explicit control. */
static ProjectInitResult initialize(Control *control) {
    return project_init(destination, allocator(), control ? (ProjectInitControl){control, permit} : (ProjectInitControl){0});
}
#endif
