/** @file
 * @brief Prove nearest-first context selection through actual original and final compiler certificates.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * chooses the original ancestor by position, even when another ancestor produces
 * well-typed final code. No command-specific or mocked certificate is accepted.
 */
#include "../../../../../../tools/project/source/copy/members/units.h"
#include "../../../../../../tools/project/source/copy/contexts/ancestors.h"
#include "../../../../../../src/delivery/source/snapshot/write.h"
/** Borrow a real catalog main; failed lookup is an invalid fixture. */
static const ModuleSource *find(const SourceRootCapture *capture, const char *identity) {
    const ModuleSource *source = NULL;
    ModuleStatus status = module_catalog_find(&capture->catalog, identity, &source);
    assert(status == MODULE_OK && source); return source;
}
/** Both independently compiled candidates have valid topology, but only the
 * nearest-parent relation may acquire the complete operation seal. Companion
 * bytes independently distinguish the actual copied definitions. */
static void check_context_positions(bool deep) {
    setup_test();
    int made = mkdir("generated/source", 0700); assert(!made);
    made = mkdir("generated/source/g+", 0700); assert(!made);
    made = mkdir("generated/source/g+/folder", 0700); assert(!made);
    made = mkdir("generated/source/g+/folder/a+", 0700); assert(!made);
    made = mkdir("generated/scratch", 0700); assert(!made);
    file("generated/source/g", "@!\n"); file("generated/source/g.tag", "grandparent");
    file("generated/source/g+/folder/a", "@!\n"); file("generated/source/g+/folder/a.tag", "nearest parent");
    file("generated/source/g+/folder/a+/child", "@!\n");
    SourceAllocator allocator = sources.allocator;
    const char *arguments[] = {"ama-project", "copy", "generated/source/g+/folder/a+/child", deep ? "generated/source/b+/inner+/copy" : "generated/source/b+/copy", "--preview",
        "--standard-root", "generated/source", "--compiler", "../../../../../../ama"};
    ProjectSourceRequest request; project_source_request_init(&request, allocator);
    ProjectSourceRequestStatus parsed = project_source_request_parse(&request, (int)(sizeof(arguments)/sizeof(*arguments)), arguments);
    assert(parsed == PROJECT_SOURCE_REQUEST_OK);
    ProjectSourcePreparation original; project_source_preparation_init(&original, allocator);
    ProjectSourcePreparationInputs capture = {.request = &request, .temporary_parent = "generated/scratch"};
    ProjectSourcePreparationStatus captured = project_source_preparation_capture(&original, &capture);
    assert(captured == PROJECT_SOURCE_PREPARATION_OK);
    SourceCopyClosure copy; source_copy_closure_init(&copy, allocator, original.original_capture, &original.original, NULL);
    SourceCopyClosureStatus closed = source_copy_closure_build(&copy, original.selected, SOURCE_COPY_KEEP_EXTERNAL);
    assert(closed == SOURCE_COPY_CLOSURE_OK);
    SourceObservedImage observed; source_observed_image_init(&observed, allocator);
    SourceObservationStatus read = source_observed_image_build(&observed, original.original_capture, (SourceImageBounds){0}, NULL, 0);
    assert(read == SOURCE_OBSERVATION_OK);
    const ModuleSource *original_child = find(original.original_capture, "//g/folder/a/child");
    const ModuleSource *nearest = find(original.original_capture, "//g/folder/a");
    const ModuleSource *outer = find(original.original_capture, "//g");
    const SourceCapturedUnit *units = original.original_capture->units;
    size_t position = SIZE_MAX, before_queries = allocation_count;
    assert(source_copy_context_position(units, nearest, original_child, &position) && position == 0);
    assert(source_copy_context_position(units, outer, original_child, &position) && position == 1);
    assert(source_copy_context_ancestor(units, original_child, 0)->source == nearest);
    assert(source_copy_context_ancestor(units, original_child, 1)->source == outer);
    assert(!source_copy_context_ancestor(units, original_child, 2));
    ModuleSource uninitialized = {0}; position = SIZE_MAX;
    assert(!source_copy_context_position(units, &uninitialized, original_child, &position) && position == SIZE_MAX);
    assert(!source_copy_context_position(units, nearest, &uninitialized, &position) && position == SIZE_MAX);
    assert(!source_copy_context_position(NULL, &uninitialized, &uninitialized, &position) && position == SIZE_MAX);
    assert(!source_copy_context_ancestor(units, &uninitialized, 0));
    ModuleSource foreign = *nearest; position = SIZE_MAX;
    assert(!source_copy_context_position(units, &foreign, original_child, &position) && position == SIZE_MAX);
    assert(!source_copy_context_position(units, nearest, nearest, &position) && position == SIZE_MAX);
    assert(!source_copy_context_position(units, nearest, original_child, NULL));
    assert(!source_copy_context_ancestor(units, &foreign, 0) && !source_copy_context_ancestor(units, NULL, 0));
    assert(allocation_count == before_queries);
    check_context_parent_failures(&original, &copy, &observed);
    for (unsigned choice = 0; choice < 3; choice++) {
        bool grand = choice == 1, automatic = choice == 2;
        SourceArena arena; source_arena_init(&arena, allocator);
        SourceCloneMap mains; source_clone_map_init(&mains, allocator, original.original_capture);
        const SourceCloneMapping *row = NULL;
        SourceCloneStatus mapping = source_clone_map_add(&mains, find(original.original_capture, "//g/folder/a/child"), original.destination.path, deep ? "//b/inner/copy" : "//b/copy", &row);
        assert(mapping == SOURCE_CLONE_OK);
        if (automatic) {
            SourceCopyParentInputs parents = {.copy = &copy, .mains = &mains, .observed = &observed,
                .roots = original.root_settings.final, .root_count = original.root_settings.final_count};
            SourceCopyParentIssue parent_issue;
            assert(source_copy_context_place(&parents, &parent_issue) == SOURCE_COPY_PARENT_OK);
        } else {
            const char *path = source_arena_join(&arena, original.original_capture->roots[0].path, "/", "b"); assert(path);
            mapping = source_clone_map_add(&mains, find(original.original_capture, (grand != deep) ? "//g" : "//g/folder/a"), path, "//b", &row);
            assert(mapping == SOURCE_CLONE_OK);
            if (deep) {
                const char *inner = source_arena_join(&arena, original.original_capture->roots[0].path, "/", "b+/inner"); assert(inner);
                mapping = source_clone_map_add(&mains, find(original.original_capture, grand ? "//g" : "//g/folder/a"), inner, "//b/inner", &row);
                assert(mapping == SOURCE_CLONE_OK);
            }
        }
        mapping = source_clone_map_seal(&mains); assert(mapping == SOURCE_CLONE_OK);
        SourceCopyContextOmissions contexts; source_copy_context_omissions_init(&contexts, allocator, &copy, &mains);
        SourceFileMap files; source_file_map_init(&files, allocator);
        SourceFileMapStatus raw = source_file_map_tree(&files, &original.placement); assert(raw == SOURCE_FILE_MAP_OK);
        SourceCopyUnitsIssue issue;
        SourceCopyUnitsStatus units = source_copy_map_unit_members(&files, &copy, &mains, &contexts, &issue); assert(units == SOURCE_COPY_UNITS_OK);
        SourceCopyContextStatus omitted = source_copy_context_omissions_seal(&contexts); assert(omitted == SOURCE_COPY_CONTEXT_OK);
        raw = source_file_map_seal(&files); assert(raw == SOURCE_FILE_MAP_OK);
        SourceOperationIntent intent = {.kind = SOURCE_OPERATION_COPY, .source = original.selected, .placement = &original.placement};
        SourceOperationWritesInputs writes = {.copy = &copy, .observed = &observed, .mains = &mains, .files = &files};
        SourceOperationCompleteInputs inputs = {.writes = &writes, .intent = &intent, .original = &original.original,
            .configured = &original.configured, .final_roots = original.root_settings.final, .final_root_count = original.root_settings.final_count,
            .contexts = &contexts, .root_originals = original.root_settings.originals, .root_original_count = original.configured.root_count};
        SourceOperationComplete complete; source_operation_complete_init(&complete, allocator);
        SourceOperationCompleteStatus prepared = source_operation_complete_prepare(&complete, &inputs);
        assert(prepared == (grand ? SOURCE_OPERATION_COMPLETE_MATERIAL : SOURCE_OPERATION_COMPLETE_OK));
        if (grand) { assert(complete.material.status == SOURCE_OPERATION_MATERIAL_CONTEXT && !complete.prepared); }
        const char *request_path = grand ? "generated/grand.json" : "generated/nearest.json";
        FILE *output = fopen(request_path, "wb"); assert(output);
        bool encoded = delivery_source_snapshot_write(output, &complete.plan.capture.view); int saved = fclose(output); assert(encoded && !saved);
        SourceCompilerClient final; source_compiler_client_init(&final, allocator);
        SourceCompilerStatus compiled = source_compiler_client_run(&final, request.compiler, request_path, &complete.plan.capture.view, NULL);
        assert(compiled == SOURCE_COMPILER_OK);
        SourceCopyContextIssue context_issue; size_t before = allocation_count;
        assert(source_copy_context_copies_check(&contexts, &observed, &complete.plan.capture, &context_issue)
            == (grand ? SOURCE_COPY_CONTEXT_UNNECESSARY : SOURCE_COPY_CONTEXT_OK));
        assert(allocation_count == before);
        SourceOperationCompleteStatus sealed = source_operation_complete_seal(&complete, &final);
        assert(sealed == (grand ? SOURCE_OPERATION_COMPLETE_INVALID : SOURCE_OPERATION_COMPLETE_OK));
        assert(complete.sealed == !grand);
        if (!grand) { assert(complete.proof_status == SOURCE_COPY_PROOF_OK); }
        const char *tag = source_arena_join(&arena, original.original_capture->roots[0].path, "/", "b.tag"); assert(tag);
        const SourceText *actual = NULL;
        for (size_t i = 0; i < complete.plan.image->count; i++) {
            if (!strcmp(complete.plan.image->entries[i].path, tag)) { actual = complete.plan.image->entries[i].content; }
        }
        const char *expected = (grand != deep) ? "grandparent" : "nearest parent";
        assert(actual && source_length(actual) == strlen(expected) && !memcmp(source_bytes(actual), expected, strlen(expected)));
        source_operation_complete_destroy(&complete); source_compiler_client_destroy(&final);
        source_file_map_destroy(&files); source_copy_context_omissions_destroy(&contexts); source_clone_map_destroy(&mains); source_arena_destroy(&arena);
    }
    source_observed_image_destroy(&observed); source_copy_closure_destroy(&copy);
    SourceWorkspaceStatus cleaned = source_workspace_cleanup(&original.workspace); assert(cleaned == SOURCE_WORKSPACE_OK);
    int retired = project_source_preparation_destroy(&original); assert(!retired);
    project_source_request_destroy(&request);
    const char *files[] = {"g", "g.tag", "g+/folder/a", "g+/folder/a.tag", "g+/folder/a+/child"};
    for (size_t i = 0; i < sizeof(files) / sizeof(*files); i++) {
        char path[128]; assert(snprintf(path, sizeof(path), "generated/source/%s", files[i]) > 0);
        assert(unlink(path) == 0);
    }
    assert(rmdir("generated/source/g+/folder/a+") == 0 && rmdir("generated/source/g+/folder") == 0
        && rmdir("generated/source/g+") == 0
        && rmdir("generated/source") == 0 && rmdir("generated/scratch") == 0);
    destroy_test();
}

/** A shortened chain must use its nearest original; a two-parent chain must
 * preserve both positions and reject swapped parents, despite valid typing.
 * The ordinary folder between original parents must consume no position. */
static void it_matches_missing_parent_clones_by_position_before_sealing(void) {
    check_context_positions(false);
    check_context_positions(true);
}
