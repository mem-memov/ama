/** @file
 * @brief Exercise real empty-project transactions, exact failures and concurrent command isolation.
 * @see [T233 contract](../../../../../docs/implementation/project-tool.md)
 */
#define _XOPEN_SOURCE 700
#include "support/fixture.c"
/** Require the complete fixed scaffold for both admitted destination states. */
static void it_publishes_exact_scaffold_into_new_and_empty_destinations(void) {
    for (int existing = 0; existing < 2; existing++) {
        setup();
        if (existing) { int made = mkdir(destination, 0700); assert(!made); }
        ProjectInitResult result = initialize(NULL);
        assert(result.status == PROJECT_INIT_OK && !result.retained_stage);
        assert_scaffold(); destroy();
    }
}
/** Preserve conflicting files and reject final or intermediate symlink aliases. */
static void it_refuses_files_nonempty_directories_and_symlink_components(void) {
    for (int kind = 0; kind < 4; kind++) {
        setup();
        char nested[256];
        if (kind == 0) { write_file(destination, "keep"); }
        if (kind == 1) {
            int made = mkdir(destination, 0700); assert(!made);
            (void)snprintf(nested, sizeof(nested), "%s/user", destination); write_file(nested, "keep");
        }
        if (kind == 2) { int linked = symlink("missing", destination); assert(!linked); }
        if (kind == 3) {
            char link[256]; (void)snprintf(link, sizeof(link), "%s/alias", root);
            int linked = symlink(".", link); assert(!linked);
            (void)snprintf(destination, sizeof(destination), "%s/alias/project", root);
        }
        ProjectInitResult result = initialize(NULL);
        assert(result.status != PROJECT_INIT_OK && !result.cleanup_failed && !result.retained_stage);
        assert(members(root) == 1);
        if (kind == 0) { assert_file(destination, "keep"); }
        if (kind == 1) { assert_file(nested, "keep"); }
        destroy();
    }
}
/** Cancel every observed boundary and require unchanged destination membership. */
static void it_preserves_destinations_at_every_prepublication_cancellation(void) {
    setup(); Control probe = {.start = -1, .completed = -1};
    ProjectInitResult result = initialize(&probe); assert(result.status == PROJECT_INIT_OK);
    size_t boundaries = probe.calls; destroy();
    for (size_t stop = 1; stop <= boundaries; stop++) {
        for (int existing = 0; existing < 2; existing++) {
            setup();
            if (existing) { int made = mkdir(destination, 0700); assert(!made); }
            Control control = {.refuse = stop, .start = -1, .completed = -1};
            result = initialize(&control);
            assert(result.status == PROJECT_INIT_CANCELLED && !result.cleanup_failed && !result.retained_stage);
            assert(members(root) == (size_t)existing);
            if (existing) { assert(members(destination) == 0); }
            destroy();
        }
    }
}
/** Sweep every measured allocation refusal and require no retained ownership. */
static void it_releases_every_failed_allocation_without_publishing(void) {
    setup(); ProjectInitResult result = initialize(NULL); assert(result.status == PROJECT_INIT_OK);
    size_t count = allocations; destroy();
    for (size_t failure = 1; failure <= count; failure++) {
        setup(); fail_allocation = failure;
        result = initialize(NULL);
        assert(result.status == PROJECT_INIT_MEMORY && !result.retained_stage && !result.cleanup_failed);
        assert(members(root) == 0); destroy();
    }
}
/** Retain the recovery path while preserving the original cancellation cause. */
static void it_reports_retained_staging_without_replacing_the_primary_error(void) {
    setup(); Control control = {.refuse = 7, .cleanup_refusal = true, .start = -1, .completed = -1};
    ProjectInitResult result = initialize(&control);
    assert(result.status == PROJECT_INIT_CANCELLED && result.cleanup_failed && result.retained_stage);
    struct stat metadata; int read = lstat(result.retained_stage, &metadata); assert(!read && S_ISDIR(metadata.st_mode));
    assert(members(root) == 1);
    erase(result.retained_stage); release(NULL, result.retained_stage);
    destroy();
}
/** Refuse publication after a user introduces a destination file. */
static void it_rechecks_user_files_created_before_publication(void) {
    setup(); Control control = {.conflict = true, .start = -1, .completed = -1};
    ProjectInitResult result = initialize(&control);
    assert(result.status == PROJECT_INIT_CONFLICT && !result.cleanup_failed);
    char path[256]; (void)snprintf(path, sizeof(path), "%s/user", destination); assert_file(path, "keep");
    assert(members(root) == 1 && members(destination) == 1); destroy();
}
/** Use pipe ordering to prove exclusive publication and execute the real tool. */
static void it_runs_the_separate_command_and_serializes_concurrent_initializers(void) {
    setup();
    int start[2], done[2]; int opened = pipe(start); assert(!opened); opened = pipe(done); assert(!opened);
    pid_t child = fork(); assert(child >= 0);
    if (!child) {
        (void)close(start[1]); (void)close(done[0]);
        char signal; ssize_t got = read(start[0], &signal, 1); if (got != 1) { _exit(4); }
        FILE *output = tmpfile(), *errors = tmpfile(); if (!output || !errors) { _exit(5); }
        const char *arguments[] = {"ama-project", "init", destination};
        int status = project_command_run(3, arguments, output, errors);
        char reply = status == 1 ? '1' : '0'; ssize_t sent = write(done[1], &reply, 1);
        (void)fclose(output); (void)fclose(errors); _exit(sent == 1 && status == 1 ? 0 : 6);
    }
    (void)close(start[0]); (void)close(done[1]);
    Control control = {.start = start[1], .completed = done[0]};
    ProjectInitResult result = initialize(&control);
    assert(result.status == PROJECT_INIT_OK); assert_scaffold();
    int status; pid_t waited = waitpid(child, &status, 0); assert(waited == child && WIFEXITED(status) && !WEXITSTATUS(status));
    erase(destination);
    child = fork(); assert(child >= 0);
    if (!child) {
        int quiet = open("/dev/null", O_WRONLY); if (quiet < 0) { _exit(7); }
        if (dup2(quiet, STDOUT_FILENO) < 0) { _exit(8); }
        execl(PROJECT_EXECUTABLE, PROJECT_EXECUTABLE, "init", destination, (char *)NULL); _exit(127);
    }
    waited = waitpid(child, &status, 0); assert(waited == child && WIFEXITED(status) && !WEXITSTATUS(status));
    assert_scaffold(); destroy();
}
/** Real open refusal and nonempty private staging must preserve foreign bytes. */
static void it_reports_os_file_conflicts_and_never_removes_unplanned_staging_entries(void) {
    for (int unexpected = 0; unexpected < 2; unexpected++) {
        setup();
        Control control = {.unexpected = unexpected, .file_conflict = !unexpected, .start = -1, .completed = -1};
        ProjectInitResult result = initialize(&control);
        assert(result.status == (unexpected ? PROJECT_INIT_CANCELLED : PROJECT_INIT_IO));
        if (unexpected) {
            assert(result.cleanup_failed && result.retained_stage);
            char path[512]; (void)snprintf(path, sizeof(path), "%s/user data", result.retained_stage);
            assert_file(path, "keep"); erase(result.retained_stage); release(NULL, result.retained_stage);
        } else { assert(!result.cleanup_failed && members(root) == 0); }
        destroy();
    }
}
/** Treat metacharacters as filesystem bytes and refuse invalid endpoints. */
static void it_checks_absolute_paths_literal_metacharacters_and_invalid_endpoints(void) {
    setup();
    char *absolute = realpath(root, NULL); assert(absolute);
    int length = snprintf(destination, sizeof(destination), "%s/literal $ name; text/", absolute);
    assert(length > 0 && (size_t)length < sizeof(destination)); free(absolute);
    ProjectInitResult result = initialize(NULL); assert(result.status == PROJECT_INIT_OK);
    /* A terminal slash remains the same directory; it cannot permit a merge. */
    result = initialize(NULL); assert(result.status == PROJECT_INIT_CONFLICT);
    const char *invalid[] = {"", "/", ".", "..", "./", "missing-parent/new"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        result = project_init(invalid[index], allocator(), (ProjectInitControl){0});
        assert(result.status != PROJECT_INIT_OK && !result.cleanup_failed);
    }
    destroy();
}
/** Resolve an existing dot endpoint after verifying its physical traversal. */
static void it_accepts_existing_empty_dot_endpoints_without_following_symlinks(void) {
    setup();
    int made = mkdir(destination, 0700); assert(!made);
    size_t length = strlen(destination); memcpy(destination + length, "/.", 3);
    ProjectInitResult result = initialize(NULL); assert(result.status == PROJECT_INIT_OK);
    destination[length] = 0; assert_scaffold();
    destroy();
}
/** Require useful usage and destination diagnostics without source mutation. */
static void it_reports_usage_and_conflicts_through_the_command_adapter(void) {
    setup();
    FILE *output = tmpfile(), *errors = tmpfile(); assert(output && errors);
    const char *arguments[] = {"ama-project", "unknown", destination};
    int status = project_command_run(3, arguments, output, errors); assert(status == 2 && members(root) == 0);
    arguments[1] = "init"; write_file(destination, "keep");
    status = project_command_run(3, arguments, output, errors); assert(status == 1); assert_file(destination, "keep");
    int rewound = fseek(errors, 0, SEEK_SET); assert(!rewound);
    char text[1024]; size_t read = fread(text, 1, sizeof(text) - 1, errors); text[read] = 0;
    assert(strstr(text, "Usage: ama-project init DESTINATION") && strstr(text, destination) && strstr(text, "not an ordinary directory"));
    int closed = fclose(output); assert(!closed); closed = fclose(errors); assert(!closed); destroy();
}
/** Execute every named assertion case explicitly in the intended order. */
void run(void) {
    it_reports_os_file_conflicts_and_never_removes_unplanned_staging_entries();
    it_checks_absolute_paths_literal_metacharacters_and_invalid_endpoints();
    it_reports_usage_and_conflicts_through_the_command_adapter();
    it_accepts_existing_empty_dot_endpoints_without_following_symlinks();
    it_publishes_exact_scaffold_into_new_and_empty_destinations();
    it_refuses_files_nonempty_directories_and_symlink_components();
    it_preserves_destinations_at_every_prepublication_cancellation();
    it_releases_every_failed_allocation_without_publishing();
    it_reports_retained_staging_without_replacing_the_primary_error();
    it_rechecks_user_files_created_before_publication();
    it_runs_the_separate_command_and_serializes_concurrent_initializers();
}
/** Adapt the harness process to its explicit assertion cases and child controls. */
int main(void) { run(); return 0; }
