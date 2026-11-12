/**
 * @file
 * @brief Verify publication phases and real rollback after filesystem-operation failures.
 *
 * Fixtures use the real frontend, scaffold model and filesystem publisher. Only
 * exact selected link/rename calls fail; successful calls are never simulated.
 */
#include <dirent.h>
#include "../common.c"
#include "native/scaffold/output/publication/publication.h"
#include "support/faults.c"

/** Borrow old file bytes until the current scaffold/fixture owners are destroyed. */
typedef struct SavedFile {
    struct SavedFile *next;
    const char *path;
    const SourceText *content;
} SavedFile;

/** Build a simple native interface at an explicit fresh/refresh destination. */
static void prepare_publication(const char *destination, bool changed) {
    prepare_native();
    file("generated/project/main", changed ? "@<\nchanged\n.value\n@C\n" : "@<\noriginal\n.value\n@C\n");
    begin_scaffold();
    bool built = native_scaffold_build(&scaffold, (ScaffoldOptions){
        .directory = destination, .sdk = TEST_SDK, .compiler = "gcc"});
    assert(built);
}

/** Snapshot exact current destination bytes, independently of the manifest digest. */
static SavedFile *save_file(SavedFile *next, const char *path) {
    SavedFile *saved = source_arena_allocate(&scaffold.arena, sizeof(*saved));
    assert(saved != NULL);
    const SourceText *text = NULL;
    SourceStatus loaded = source_manager_load(&sources, path, &text);
    assert(loaded == SOURCE_OK);
    *saved = (SavedFile){next, path, text};
    return saved;
}

/** Snapshot each existing artifact and the already published manifest. */
static SavedFile *save_outputs(void) {
    SavedFile *saved = NULL;
    for (ScaffoldArtifact *artifact = scaffold.first; artifact != NULL; artifact = artifact->next) {
        saved = save_file(saved, artifact->path);
    }
    const char *manifest = source_arena_join(&scaffold.arena, scaffold.directory, "/", ".ama-native-outputs");
    assert(manifest != NULL);
    return save_file(saved, manifest);
}

/** Read the actual post-operation file rather than a cached source-manager view. */
static void require_bytes(const char *path, const SourceText *expected) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    const unsigned char *bytes = source_bytes(expected);
    for (size_t index = 0; index < source_length(expected); index++) {
        int actual = fgetc(input);
        assert(actual != EOF && (unsigned char)actual == bytes[index]);
    }
    int tail = fgetc(input);
    int failed = ferror(input);
    int closed = fclose(input);
    assert(tail == EOF && failed == 0 && closed == 0);
}

/** No lock/stage/backup may remain after ordinary failure or success cleanup. */
static void require_clean_directory(const SavedFile *saved) {
    DIR *directory = opendir(scaffold.directory);
    assert(directory != NULL);
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        bool expected = false;
        for (const SavedFile *file = saved; file != NULL; file = file->next) {
            const char *name = strrchr(file->path, '/');
            assert(name != NULL);
            expected = expected || strcmp(name + 1, entry->d_name) == 0;
        }
        assert(expected);
    }
    int closed = closedir(directory);
    assert(closed == 0);
}

/** Run one real fresh/refresh transaction with selected one-shot failure ordinals. */
static void exercise_failure(const char *destination, bool refresh, size_t link_failure,
    size_t rename_failure, size_t *links, size_t *renames) {
    if (refresh) {
        prepare_publication(destination, false);
        bool published = native_scaffold_publish(&scaffold);
        assert(published);
        end_scaffold();
    }
    prepare_publication(destination, refresh);
    SavedFile *saved = refresh ? save_outputs() : NULL;
    publication_faults.enabled = true;
    publication_faults.links = publication_faults.renames = 0;
    publication_faults.fail_link = link_failure;
    publication_faults.fail_rename = rename_failure;
    publication_faults.fail_second_rename = 0;
    bool published = native_scaffold_publish(&scaffold);
    publication_faults.enabled = false;
    if (links != NULL) { *links = publication_faults.links; }
    if (renames != NULL) { *renames = publication_faults.renames; }
    if (link_failure == 0 && rename_failure == 0) {
        assert(published && scaffold.published);
    } else {
        assert(!published && !scaffold.published && scaffold.error.code == DIAGNOSTIC_IO_FAILURE);
        for (SavedFile *file = saved; file != NULL; file = file->next) {
            require_bytes(file->path, file->content);
        }
        if (!refresh) {
            for (ScaffoldArtifact *artifact = scaffold.first; artifact != NULL; artifact = artifact->next) {
                struct stat metadata;
                int found = lstat(artifact->path, &metadata);
                assert(found == -1 && errno == ENOENT);
            }
        }
        require_clean_directory(saved);
    }
    end_scaffold();
}

/** Every exclusive-publication failure removes all successfully published new files. */
static void it_rolls_back_each_new_file_publication_failure(void) {
    size_t links = 0;
    exercise_failure("generated/fresh-baseline", false, 0, 0, &links, NULL);
    assert(links > 1);
    for (size_t failure = 1; failure <= links; failure++) {
        char destination[128];
        int length = snprintf(destination, sizeof(destination), "generated/fresh-%zu", failure);
        assert(length > 0 && (size_t)length < sizeof(destination));
        exercise_failure(destination, false, failure, 0, NULL, NULL);
    }
}

/** Backup and replacement failures restore exact old bytes, including the ledger. */
static void it_preserves_all_prior_outputs_on_backup_and_replacement_failure(void) {
    size_t links = 0, renames = 0;
    exercise_failure("generated/refresh-baseline", true, 0, 0, &links, &renames);
    assert(links > 1 && renames > 1);
    for (size_t failure = 1; failure <= links; failure++) {
        char destination[128];
        int length = snprintf(destination, sizeof(destination), "generated/backup-%zu", failure);
        assert(length > 0 && (size_t)length < sizeof(destination));
        exercise_failure(destination, true, failure, 0, NULL, NULL);
    }
    for (size_t failure = 1; failure <= renames; failure++) {
        char destination[128];
        int length = snprintf(destination, sizeof(destination), "generated/replace-%zu", failure);
        assert(length > 0 && (size_t)length < sizeof(destination));
        exercise_failure(destination, true, 0, failure, NULL, NULL);
    }
}

/** Restore failure preserves the actual old backup and reports manual recovery. */
static void it_retains_old_backup_when_rollback_itself_fails(void) {
    const char *destination = "generated/failed-restore";
    prepare_publication(destination, false);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    end_scaffold();
    prepare_publication(destination, true);
    SavedFile *saved = save_outputs();
    ScaffoldArtifact *first = scaffold.first;
    const SourceText *old = NULL;
    for (SavedFile *file = saved; file != NULL; file = file->next) {
        if (strcmp(file->path, first->path) == 0) { old = file->content; }
    }
    assert(old != NULL);
    publication_faults.enabled = true;
    publication_faults.links = publication_faults.renames = 0;
    publication_faults.fail_link = 0;
    publication_faults.fail_rename = 2;
    publication_faults.fail_second_rename = 3;
    published = native_scaffold_publish(&scaffold);
    publication_faults.enabled = false;
    assert(!published && !scaffold.published && scaffold.error.cause != NULL);
    assert(first->published && first->backup != NULL);
    require_bytes(first->backup, old);
    SavedFile retained = {saved, first->backup, old};
    require_clean_directory(&retained);
    end_scaffold();
}

/** A failed second acquisition/release cannot remove another caller's lock. */
static void it_owns_only_its_exclusive_lock_and_reports_release_failure(void) {
    prepare_publication("generated/lock-domain", false);
    ScaffoldPublicationIO io = {&scaffold.arena, &scaffold.error};
    bool created = scaffold_publication_directories(&io, scaffold.directory);
    assert(created);
    ScaffoldPublicationLock first, second;
    bool locked = scaffold_publication_lock(&io, scaffold.directory, &first);
    assert(locked && first.acquired);
    locked = scaffold_publication_lock(&io, scaffold.directory, &second);
    assert(!locked && !second.acquired);
    bool released = scaffold_publication_unlock(&io, &second);
    assert(released && first.acquired);
    struct stat metadata;
    int found = lstat(first.path, &metadata);
    assert(found == 0 && S_ISREG(metadata.st_mode));
    int removed = unlink(first.path);
    assert(removed == 0);
    released = scaffold_publication_unlock(&io, &first);
    assert(!released && scaffold.error.code == DIAGNOSTIC_IO_FAILURE);
    end_scaffold();
}

/** The public guard requires readiness and cannot republish a completed owner. */
static void it_requires_readiness_and_rejects_a_second_publication_attempt(void) {
    prepare_native();
    begin_scaffold();
    bool published = native_scaffold_publish(&scaffold);
    assert(!published && !scaffold.publish_attempted && !scaffold.published);
    end_scaffold();
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    published = native_scaffold_publish(&scaffold);
    assert(published && scaffold.publish_attempted && scaffold.published);
    published = native_scaffold_publish(&scaffold);
    assert(!published && scaffold.published && scaffold.error.code == DIAGNOSTIC_INVALID_PATH);
    for (ScaffoldArtifact *artifact = scaffold.first; artifact != NULL; artifact = artifact->next) {
        require_bytes(artifact->path, artifact->content);
    }
    end_scaffold();
}

/** Exercise all phases independently and through the real transaction. */
void run(void) {
    it_requires_readiness_and_rejects_a_second_publication_attempt();
    it_rolls_back_each_new_file_publication_failure();
    it_preserves_all_prior_outputs_on_backup_and_replacement_failure();
    it_retains_old_backup_when_rollback_itself_fails();
    it_owns_only_its_exclusive_lock_and_reports_release_failure();
}
int main(void) {
    run();
    return EXIT_SUCCESS;
}
