/**
 * @file
 * @brief Test narrow publication phases using only bytes, allocation and diagnostics.
 *
 * This binary does not link or construct a frontend, project, semantic graph or
 * NativeScaffold. The companion publication leaf tests their real composition.
 */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "native/scaffold/output/publication/publication.h"

static SourceArena arena;
static Diagnostic error;
static ScaffoldPublicationIO io;

/** Establish only the two records required by the filesystem children. */
static void start(void) {
    setup_test();
    source_arena_init(&arena, sources.allocator);
    error = (Diagnostic){0};
    io = (ScaffoldPublicationIO){&arena, &error};
}

/** Destroy temporary paths/bytes and require balanced allocations. */
static void finish(void) {
    source_arena_destroy(&arena);
    destroy_test();
}

/** Compare actual bytes against independent expected content, including NUL. */
static void require_bytes(const char *path, const unsigned char *expected, size_t length) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    for (size_t index = 0; index < length; index++) {
        int byte = fgetc(input);
        assert(byte != EOF && (unsigned char)byte == expected[index]);
    }
    int tail = fgetc(input);
    int failed = ferror(input);
    int closed = fclose(input);
    assert(tail == EOF && failed == 0 && closed == 0);
}

/** A preflight policy may supply a specific failure without a compiler context. */
static bool reject_owned(void *context, const ScaffoldArtifact *artifact) {
    (void)artifact;
    Diagnostic *record = context;
    if (record != NULL) {
        *record = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY, .message = "ownership allocation failure"};
    }
    return false;
}

/** Stage/publish byte-exact executable output and remove every owned temporary. */
static void it_publishes_exact_binary_bytes_and_executable_mode_without_a_frontend(void) {
    start();
    const unsigned char expected[] = {'n', 0, 'w'};
    const SourceText *content = NULL;
    SourceStatus added = source_manager_add(&sources, "prepared", expected, sizeof(expected), &content);
    assert(added == SOURCE_OK);
    ScaffoldArtifact artifact = {.path = "generated/phase-output", .content = content, .executable = true};
    ScaffoldPublicationPolicy policy = {.owns = reject_owned};
    bool permitted = scaffold_publication_preflight(&io, &artifact, &policy);
    assert(permitted && !artifact.existed);
    bool staged = scaffold_publication_stage(&io, &artifact);
    assert(staged);
    bool published = scaffold_publication_artifact(&io, &artifact);
    assert(published && artifact.published);
    scaffold_publication_cleanup(&io, &artifact, false);
    require_bytes(artifact.path, expected, sizeof(expected));
    struct stat metadata;
    int found = lstat(artifact.path, &metadata);
    assert(found == 0 && (metadata.st_mode & 0777) == 0700);
    found = lstat(artifact.stage, &metadata);
    assert(found == -1 && errno == ENOENT);
    finish();
}

/** Preserve a policy's precise first error, and skip policy under explicit overwrite. */
static void it_keeps_ownership_diagnostics_and_bypasses_only_explicit_overwrite(void) {
    start();
    const char *path = "generated/phase-output";
    ScaffoldArtifact artifact = {.path = path};
    ScaffoldPublicationPolicy policy = {.context = &error, .owns = reject_owned};
    bool permitted = scaffold_publication_preflight(&io, &artifact, &policy);
    assert(!permitted && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
    assert(strcmp(error.message, "ownership allocation failure") == 0);
    error = (Diagnostic){0};
    policy.context = NULL;
    permitted = scaffold_publication_preflight(&io, &artifact, &policy);
    assert(!permitted && error.code == DIAGNOSTIC_INVALID_PATH);
    error = (Diagnostic){0};
    policy.overwrite = true;
    permitted = scaffold_publication_preflight(&io, &artifact, &policy);
    assert(permitted && artifact.existed && error.message == NULL);
    finish();
}

/** A prepared replacement can restore exact old bytes after a changed prefix. */
static void it_restores_replaced_bytes_and_releases_backups(void) {
    start();
    const unsigned char original[] = {'n', 0, 'w'};
    const unsigned char replacement[] = {'o', 't', 'h', 'e', 'r'};
    const SourceText *content = NULL;
    SourceStatus added = source_manager_add(&sources, "replacement", replacement, sizeof(replacement), &content);
    assert(added == SOURCE_OK);
    ScaffoldArtifact artifact = {.path = "generated/phase-output", .content = content};
    ScaffoldPublicationPolicy policy = {.overwrite = true};
    bool permitted = scaffold_publication_preflight(&io, &artifact, &policy);
    assert(permitted && artifact.existed);
    bool staged = scaffold_publication_stage(&io, &artifact);
    assert(staged && artifact.backup != NULL);
    bool published = scaffold_publication_artifact(&io, &artifact);
    assert(published);
    require_bytes(artifact.path, replacement, sizeof(replacement));
    scaffold_publication_cleanup(&io, &artifact, true);
    assert(!artifact.published);
    require_bytes(artifact.path, original, sizeof(original));
    struct stat metadata;
    int found = lstat(artifact.backup, &metadata);
    assert(found == -1 && errno == ENOENT);
    finish();
}

/** Directory/temporary failures record ownership-safe diagnostics before publication. */
static void it_checks_directory_temporary_and_allocation_failures(void) {
    start();
    bool created = scaffold_publication_directories(&io, "generated/phase-dir/child");
    assert(created);
    created = scaffold_publication_directories(&io, "generated/phase-output/child");
    assert(!created && error.code == DIAGNOSTIC_UNSUPPORTED_PATH);
    error = (Diagnostic){0};
    int descriptor = -1;
    char *temporary = scaffold_publication_temporary(&io,
        "generated/missing-parent/file", ".stage-XXXXXX", &descriptor);
    assert(temporary == NULL && descriptor == -1 && error.code == DIAGNOSTIC_IO_FAILURE);
    error = (Diagnostic){0};
    failure_at = allocation_count + 1;
    created = scaffold_publication_directories(&io, "generated/allocation-failure");
    assert(!created && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
    finish();
}

/** A failed lock attempt cannot unlink an existing lock; first diagnostics persist. */
static void it_acquires_and_releases_only_its_own_lock(void) {
    start();
    ScaffoldPublicationLock first, second;
    bool acquired = scaffold_publication_lock(&io, "generated/phase-dir", &first);
    assert(acquired && first.acquired);
    acquired = scaffold_publication_lock(&io, "generated/phase-dir", &second);
    assert(!acquired && !second.acquired);
    bool released = scaffold_publication_unlock(&io, &second);
    assert(released);
    struct stat metadata;
    int found = lstat(first.path, &metadata);
    assert(found == 0);
    released = scaffold_publication_unlock(&io, &first);
    assert(released && !first.acquired);
    finish();
}

/** Exercise standalone phase contracts with no unrelated compiler state. */
void run(void) {
    it_publishes_exact_binary_bytes_and_executable_mode_without_a_frontend();
    it_keeps_ownership_diagnostics_and_bypasses_only_explicit_overwrite();
    it_restores_replaced_bytes_and_releases_backups();
    it_checks_directory_temporary_and_allocation_failures();
    it_acquires_and_releases_only_its_own_lock();
}
int main(void) {
    run();
    return EXIT_SUCCESS;
}
