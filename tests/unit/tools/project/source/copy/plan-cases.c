/** @file
 * @brief Compose complete image/configuration plans and exhaust every preparation allocation failure.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/plan/plan.h"
/** A source cycle can be moved only as one reviewed union. The final source
 * snapshot changes, so a client for the earlier copy-only image must refuse. */
static void check_copy_plan_removals(SourceArena *arena, const SourceCopyPlanInputs *baseline) {
    SourceRemovalSelection first, second, merged;
    source_removal_selection_init(&first, sources.allocator); source_removal_selection_init(&second, sources.allocator);
    source_removal_selection_init(&merged, sources.allocator);
    assert(source_removal_selection_build(&first, &tree, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    SourceTreeSelection other; source_tree_selection_init(&other, sources.allocator);
    const ModuleSource *other_source = NULL;
    assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/other", &other_source) == MODULE_OK);
    assert(source_tree_selection_read(&other, &capture, source_path(other_source->text)) == SOURCE_TREE_OK);
    assert(source_removal_selection_build(&second, &other, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    const SourceRemovalSelection *parts[] = {&first, &second};
    assert(source_removal_selection_merge(&merged, parts, 2) == SOURCE_REMOVAL_SELECTION_OK);
    size_t count = baseline->change_count + merged.path_count;
    SourceImageChange *changes = source_arena_allocate(arena, count * sizeof(*changes)); assert(changes);
    memcpy(changes, baseline->changes, baseline->change_count * sizeof(*changes));
    size_t index = baseline->change_count;
    for (const SourceRemovalSelectedFile *file = merged.selected; file; file = file->next) {
        changes[index++] = (SourceImageChange){.kind = SOURCE_IMAGE_REMOVE, .entry = {.path = file->file->file.path}};
    }
    SourceCopyPlanInputs inputs = *baseline; inputs.changes = changes; inputs.change_count = count; inputs.removal = &merged;
    SourceCopyPlan plan; source_copy_plan_init(&plan, sources.allocator);
    SourceCopyPlanStatus status = source_copy_plan_build(&plan, &inputs);
    if (status != SOURCE_COPY_PLAN_OK) { fprintf(stderr, "move plan status=%d path=%s review=%d\n", status, plan.failed_path ? plan.failed_path : "(none)", plan.removal_status); }
    assert(status == SOURCE_COPY_PLAN_OK);
    FILE *request = fopen("generated/final-move-plan.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &plan.capture.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/final-move-plan.json", &plan.capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_plan_prove(&plan, &client, &proof) == SOURCE_COPY_PROOF_OK);
    source_compiler_client_destroy(&client); source_copy_plan_destroy(&plan);
    inputs.change_count--; source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_REMOVAL && !plan.prepared);
    source_copy_plan_destroy(&plan); inputs.change_count = count;
    SourceClientRoot root = {.definition = copy_definition(main_identity)};
    inputs.clients = &root; source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_REMOVAL && plan.removal_status == SOURCE_REMOVAL_BLOCKED && plan.removal_review.clients);
    source_copy_plan_destroy(&plan); inputs.clients = NULL; inputs.removal = &first;
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_REMOVAL && !plan.prepared);
    source_copy_plan_destroy(&plan);
    index = baseline->change_count;
    for (const SourceRemovalSelectedFile *file = first.selected; file; file = file->next) {
        changes[index++] = (SourceImageChange){.kind = SOURCE_IMAGE_REMOVE, .entry = {.path = file->file->file.path}};
    }
    inputs.change_count = index; source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_REMOVAL && plan.removal_status == SOURCE_REMOVAL_BLOCKED && plan.removal_review.consumers);
    source_copy_plan_destroy(&plan);
    source_removal_selection_destroy(&merged); source_removal_selection_destroy(&second);
    source_tree_selection_destroy(&other); source_removal_selection_destroy(&first);
}

/** Independent original selections determine every retained version's expected
 * roots; the initialized new clone-only version must have the empty root set. */
static void check_copy_plan(SourceArena *arena, const SourceObservedImage *observed,
    const SourceCloneMap *map, const SourceFileMap *files, const SourceGraphPreparedText *texts,
    const SourceCopyContextOmissions *contexts, const SourceImageCapture *prepared,
    const SourceCompilerClient *client, const SourceImageChange *changes, size_t change_count) {
    SourcePackageEnvironment original, candidate;
    source_package_environment_init(&original, sources.allocator); source_package_environment_init(&candidate, sources.allocator);
    assert(source_package_environment_build(&original, &capture.catalog) == SOURCE_ENVIRONMENT_OK);
    assert(source_package_environment_build(&candidate, &prepared->catalog) == SOURCE_ENVIRONMENT_OK);
    size_t count = 0;
    for (const SourcePackageScope *scope = candidate.first; scope; scope = scope->next) { count++; }
    SourceCopyConfigurationExpectation *expected = source_arena_allocate(arena, count * sizeof(*expected)); assert(expected);
    size_t index = 0;
    for (const SourcePackageScope *scope = candidate.first; scope; scope = scope->next) {
        expected[index] = (SourceCopyConfigurationExpectation){.environment = &candidate, .scope = scope};
        const SourcePackageScope *old = original.first;
        while (old && strcmp(old->version->logical_path, scope->version->logical_path)) { old = old->next; }
        if (old) {
            SourceSelectionRootsIssue issue;
            assert(source_copy_selected_roots(arena, old->selection, candidate.view, NULL, false, NULL, 0,
                &expected[index].roots, &issue) == SOURCE_SELECTION_ROOTS_OK);
        }
        index++;
    }
    SourceCopyPlanInputs inputs = {.copy = &copy, .observed = observed, .mains = map, .files = files,
        .texts = texts, .contexts = contexts, .changes = changes, .change_count = change_count,
        .configurations = expected, .configuration_count = count};
    check_copy_plan_removals(arena, &inputs);
    SourceCopyPlan plan; source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_OK && plan.prepared);
    SourceCopyProofIssue proof;
    assert(source_copy_plan_prove(&plan, client, &proof) == SOURCE_COPY_PROOF_OK);
    assert(source_copy_plan_prove(&plan, &analysis.original, &proof) == SOURCE_COPY_PROOF_INPUTS);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_INVALID && plan.prepared);
    source_copy_plan_destroy(&plan);
    size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_copy_plan_init(&plan, sources.allocator); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCopyPlanStatus status = source_copy_plan_build(&plan, &inputs);
        if (!failure) { assert(status == SOURCE_COPY_PLAN_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_COPY_PLAN_OK && !plan.prepared); }
        failure_at = 0; source_copy_plan_destroy(&plan); assert(live_allocations == live);
    }
    inputs.configuration_count = count - 1;
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_CONFIGURATION && !plan.prepared);
    assert(source_copy_plan_prove(&plan, client, &proof) == SOURCE_COPY_PROOF_INVALID);
    source_copy_plan_destroy(&plan); inputs.configuration_count = count;
    SourceCopyConfigurationExpectation saved = expected[count - 1]; expected[count - 1] = expected[0];
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_CONFIGURATION && !plan.prepared);
    source_copy_plan_destroy(&plan); expected[count - 1] = saved;
    SourceImageChange illegal = {.kind = SOURCE_IMAGE_REMOVE, .entry = {.path = source_path(tree.units[0]->source->text)}};
    inputs.changes = &illegal; inputs.change_count = 1;
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_REMOVAL && !plan.prepared);
    source_copy_plan_destroy(&plan); inputs.changes = changes; inputs.change_count = change_count;
    SourceCopyClosure policy = copy; policy.mode = SOURCE_COPY_KEEP_EXTERNAL;
    SourceCopyContextOmissions policy_context = *contexts; policy_context.copy = &policy;
    inputs.copy = &policy; inputs.contexts = &policy_context;
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_MODE && !plan.prepared);
    source_copy_plan_destroy(&plan);
    source_package_environment_destroy(&candidate); source_package_environment_destroy(&original);
}
