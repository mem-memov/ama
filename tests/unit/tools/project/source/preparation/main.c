/** @file
 * @brief Exercise actual parsed deletion through captured originals and the single final operation owner.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/command/source/preparation/internal.h"
/** Create only test-owned fixtures; product preparation must leave their bytes intact. */
static void file(const char *path, const char *bytes) {
    FILE *stream = fopen(path, "wb"); assert(stream);
    assert(fwrite(bytes, 1, strlen(bytes), stream) == strlen(bytes) && fclose(stream) == 0);
}
/** The complete owner is reached from genuine parsed input; no mocked graph,
 * valid flag, fabricated expected permission set or alternate final plan occurs. */
static void it_prepares_parsed_deletion_through_actual_original_and_final_compilers(void) {
    for (unsigned blocked = 0; blocked < 2; blocked++) {
        setup_test();
        assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/scratch", 0700) == 0);
        file("generated/source/removed", "@!\n");
        file("generated/source/retained", blocked ? "@!\nvalue&\n//removed\n" : "@!\n");
        const char *arguments[] = {"ama-project", "delete", "generated/source/removed", "--preview",
            "--standard-root", "generated/source", "--compiler", "../../../../../../ama"};
        ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
        assert(project_source_request_parse(&request, 8, arguments) == PROJECT_SOURCE_REQUEST_OK);
        ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
        ProjectSourcePreparationInputs inputs = (ProjectSourcePreparationInputs){.request = &request, .temporary_parent = "generated/scratch"};
        assert(project_source_preparation_capture(&owner, &inputs) == PROJECT_SOURCE_PREPARATION_OK);
        ProjectSourcePreparationStatus status = project_source_preparation_delete(&owner);
        if (blocked) {
            assert(status == PROJECT_SOURCE_PREPARATION_COMPLETE && owner.complete.status == SOURCE_OPERATION_COMPLETE_PLAN
                && owner.complete.plan.removal_review.consumers && !owner.complete.prepared);
        } else {
            assert(status == PROJECT_SOURCE_PREPARATION_OK && owner.complete.prepared && !owner.complete.sealed);
            assert(project_source_preparation_seal(&owner) == PROJECT_SOURCE_PREPARATION_OK
                && owner.complete.sealed && owner.complete.rebuild.valid);
            assert(owner.complete.plan.capture.view.count == 1 && owner.final.valid);
            assert(owner.complete.roots.first->action == SOURCE_OPERATION_ROOT_KEEP);
        }
        assert(access("generated/source/removed", F_OK) == 0 && access("generated/source/retained", F_OK) == 0);
        FILE *stream = fopen("generated/source/removed", "rb"); assert(stream);
        assert(fgetc(stream) == '@' && fgetc(stream) == '!' && fgetc(stream) == '\n' && fgetc(stream) == EOF && fclose(stream) == 0);
        assert(source_workspace_cleanup(&owner.workspace) == SOURCE_WORKSPACE_OK);
        assert(project_source_preparation_destroy(&owner) == 0); project_source_request_destroy(&request);
        assert(unlink("generated/source/removed") == 0 && unlink("generated/source/retained") == 0
            && rmdir("generated/source") == 0 && rmdir("generated/scratch") == 0);
        destroy_test();
    }
}
/** Removing a semantic root removes its setting while retaining its actual
 * parent only as physical observation; the parent is never a new namespace. */
static void it_drops_an_actually_removed_root_without_indexing_its_observed_parent(void) {
    setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/scratch", 0700) == 0);
    file("generated/source/main", "@!\n");
    const char *arguments[] = {"ama-project", "delete", "generated/source", "--preview",
        "--standard-root", "generated/source", "--compiler", "../../../../../../ama"};
    ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
    assert(project_source_request_parse(&request, 8, arguments) == PROJECT_SOURCE_REQUEST_OK);
    ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
    ProjectSourcePreparationInputs inputs = (ProjectSourcePreparationInputs){.request = &request, .temporary_parent = "generated/scratch"};
    assert(project_source_preparation_capture(&owner, &inputs) == PROJECT_SOURCE_PREPARATION_OK);
    assert(project_source_preparation_delete(&owner) == PROJECT_SOURCE_PREPARATION_OK);
    assert(owner.final_root_count == 0 && owner.complete.plan.capture.image->root_count == 0
        && owner.complete.roots.first->action == SOURCE_OPERATION_ROOT_DROP);
    assert(project_source_preparation_seal(&owner) == PROJECT_SOURCE_PREPARATION_OK && owner.complete.sealed);
    assert(source_workspace_cleanup(&owner.workspace) == SOURCE_WORKSPACE_OK && project_source_preparation_destroy(&owner) == 0);
    project_source_request_destroy(&request);
    assert(unlink("generated/source/main") == 0 && rmdir("generated/source") == 0 && rmdir("generated/scratch") == 0);
    destroy_test();
}
/** Actual malformed source and missing compiler preserve their typed child
 * failure/diagnostics, clean private requests and never alter source bytes. */
static void it_retains_original_capture_and_process_failures_without_final_authority(void) {
    for (unsigned missing = 0; missing < 2; missing++) {
        setup_test(); assert(mkdir("generated/source", 0700) == 0 && mkdir("generated/scratch", 0700) == 0);
        file("generated/source/main", missing ? "@!\n" : "@unknown\n");
        const char *arguments[] = {"ama-project", "delete", "generated/source/main", "--preview",
            "--standard-root", "generated/source", "--compiler", missing ? "./no-such-compiler" : "../../../../../../ama"};
        ProjectSourceRequest request; project_source_request_init(&request, sources.allocator);
        assert(project_source_request_parse(&request, 8, arguments) == PROJECT_SOURCE_REQUEST_OK);
        ProjectSourcePreparation owner; project_source_preparation_init(&owner, sources.allocator);
        ProjectSourcePreparationInputs inputs = (ProjectSourcePreparationInputs){.request = &request, .temporary_parent = "generated/scratch"};
        assert(project_source_preparation_capture(&owner, &inputs) == PROJECT_SOURCE_PREPARATION_ANALYSIS);
        assert(!owner.original.valid && !owner.complete.attempted && !owner.final.attempted);
        assert(owner.original.original.status == SOURCE_COMPILER_CHILD);
        if (missing) { assert(owner.original.original.process.error == ENOENT); }
        if (!missing) { assert(owner.original.original.diagnostics && source_length(owner.original.original.diagnostics)); }
        assert(source_workspace_cleanup(&owner.workspace) == SOURCE_WORKSPACE_OK && project_source_preparation_destroy(&owner) == 0);
        project_source_request_destroy(&request);
        assert(unlink("generated/source/main") == 0 && rmdir("generated/source") == 0 && rmdir("generated/scratch") == 0);
        destroy_test();
    }
}
#include "roots.c"
#include "augmentation.c"
#include "scopes.c"
#include "original.c"
#include "failures.c"
/** Keep the genuine original/final handoff scenario independently runnable. */
void run(void) {
    it_derives_root_settings_from_actual_raw_directory_carries_and_explicit_scopes();
    it_captures_uncovered_existing_final_roots_in_one_original_domain();
    it_unions_equal_root_scopes_by_the_exclusion_intersection_law();
    it_expands_same_path_original_coverage_without_duplicate_root_indexing();
    it_analyzes_augmented_original_inputs_before_preparing_copy_or_move();
    it_retires_every_original_command_capture_allocation_failure_and_preserves_single_attempts();
    it_prepares_parsed_deletion_through_actual_original_and_final_compilers();
    it_drops_an_actually_removed_root_without_indexing_its_observed_parent();
    it_retains_original_capture_and_process_failures_without_final_authority();
}
/** Preserve assertion artifacts; successful leaf cleanup belongs to Make. */
int main(void) { run(); return 0; }
