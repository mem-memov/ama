/** @file
 * @brief Compose complete image/configuration plans and exhaust every preparation allocation failure.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/operation/plan/plan.h"
#include "../../../../../../tools/project/source/operation/material/material.h"
#include "material-review/failures.c"
#include "material-review/membership.c"
#include "material-review/fixture.c"
#include "../../../../../../tools/project/source/transaction/document/document.h"
/** Durable image records retain the same material/compiler proof after the
 * temporary JSON text expires; decoded data has no independent mutation authority. */
static void check_recorded_copy_plan(SourceArena *arena, const SourceOperationPlan *plan,
    const SourceCompilerClient *client, const char *const *removed, size_t removed_count) {
    FILE *stream = tmpfile(); assert(stream);
    assert(source_image_document_write(stream, plan->image));
    long length = ftell(stream); assert(length >= 0 && fseek(stream, 0, SEEK_SET) == 0);
    unsigned char *bytes = source_arena_allocate(arena, (size_t)length); assert(bytes);
    assert(fread(bytes, 1, (size_t)length, stream) == (size_t)length && fclose(stream) == 0);
    SourceManager record; source_manager_init(&record, sources.allocator);
    const SourceText *text = NULL;
    assert(source_manager_add(&record, "recorded-plan.json", bytes, (size_t)length, &text) == SOURCE_OK);
    SourceImageDocument document; source_image_document_init(&document, sources.allocator);
    assert(source_image_document_read(&document, text) == SOURCE_JSON_OK);
    source_manager_destroy(&record);
    SourceImageCapture decoded; source_image_capture_init(&decoded, sources.allocator);
    assert(source_image_capture_build(&decoded, &document.image) == SOURCE_IMAGE_CAPTURE_OK);
    SourceCopyProofIssue issue;
    const SourceOperationPlanInputs *inputs = &plan->inputs;
    assert(source_copy_prove(arena, inputs->original, inputs->observed, inputs->mains,
        inputs->files, inputs->texts, inputs->contexts, &decoded, client, removed,
        removed_count, &issue) == SOURCE_COPY_PROOF_OK);
    source_image_capture_destroy(&decoded); source_image_document_destroy(&document);
}
/** A source cycle can be moved only as one reviewed union. The final source
 * snapshot changes, so a client for the earlier copy-only image must refuse. */
static void check_copy_plan_removals(SourceArena *arena, const SourceOperationPlanInputs *baseline) {
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
    SourceOperationPlanInputs inputs = *baseline; inputs.changes = changes; inputs.change_count = count; inputs.removal = &merged;
    SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
    SourceOperationPlanStatus status = source_operation_plan_build(&plan, &inputs);
    if (status != SOURCE_OPERATION_PLAN_OK) { fprintf(stderr, "move plan status=%d path=%s review=%d\n", status, plan.failed_path ? plan.failed_path : "(none)", plan.removal_status); }
    /* main.payload is a sibling companion, not a descendant of the main
     * file or main+ directory, so it is its own seventh mutation root. */
    assert(status == SOURCE_OPERATION_PLAN_OK && plan.mutations.count == 7);
    bool payload_root = false;
    for (size_t i = 0; i < plan.mutations.count; i++) {
        const char *name = plan.mutations.roots[i].name;
        if (!strcmp(name, "main.payload")) { payload_root = true; }
    }
    assert(payload_root);
    FILE *request = fopen("generated/final-move-plan.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &plan.capture.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/final-move-plan.json", &plan.capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_operation_plan_prove(&plan, &client, &proof) == SOURCE_COPY_PROOF_OK);
    check_recorded_copy_plan(arena, &plan, &client, merged.identities, merged.identity_count);
    source_compiler_client_destroy(&client); source_operation_plan_destroy(&plan);
    inputs.change_count--; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_REMOVAL && !plan.prepared);
    source_operation_plan_destroy(&plan); inputs.change_count = count;
    SourceClientRoot root = {.definition = copy_definition(main_identity)};
    inputs.clients = &root; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_REMOVAL && plan.removal_status == SOURCE_REMOVAL_BLOCKED && plan.removal_review.clients);
    source_operation_plan_destroy(&plan); inputs.clients = NULL; inputs.removal = &first;
    source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_REMOVAL && !plan.prepared);
    source_operation_plan_destroy(&plan);
    index = baseline->change_count;
    for (const SourceRemovalSelectedFile *file = first.selected; file; file = file->next) {
        changes[index++] = (SourceImageChange){.kind = SOURCE_IMAGE_REMOVE, .entry = {.path = file->file->file.path}};
    }
    inputs.change_count = index; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_REMOVAL && plan.removal_status == SOURCE_REMOVAL_BLOCKED && plan.removal_review.consumers);
    source_operation_plan_destroy(&plan);
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
    SourceOperationConfigurationExpectation *expected = source_arena_allocate(arena, count * sizeof(*expected)); assert(expected);
    size_t index = 0;
    for (const SourcePackageScope *scope = candidate.first; scope; scope = scope->next) {
        expected[index] = (SourceOperationConfigurationExpectation){.environment = &candidate, .scope = scope};
        const SourcePackageScope *old = original.first;
        while (old && strcmp(old->version->logical_path, scope->version->logical_path)) { old = old->next; }
        if (old) {
            SourceSelectionRootsIssue issue;
            assert(source_copy_selected_roots(arena, old->selection, candidate.view, NULL, false, NULL, 0,
                &expected[index].roots, &issue) == SOURCE_SELECTION_ROOTS_OK);
        }
        index++;
    }
    SourceOperationPlanInputs inputs = {.original = copy.expansion.analysis, .final_roots = observed->image.roots, .final_root_count = observed->image.root_count, .copy = &copy, .observed = observed, .mains = map, .files = files,
        .texts = texts, .contexts = contexts, .changes = changes, .change_count = change_count,
        .configurations = expected, .configuration_count = count};
    check_copy_plan_removals(arena, &inputs);
    SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_OK && plan.prepared);
    assert(plan.mutations.count == 1 && !plan.mutations.roots[0].before_count && plan.mutations.roots[0].after_count);
    SourceCopyProofIssue proof;
    assert(source_operation_plan_prove(&plan, client, &proof) == SOURCE_COPY_PROOF_OK);
    check_recorded_copy_plan(arena, &plan, client, NULL, 0);
    check_copy_material_review(&plan);
    assert(source_copy_prove(&plan.arena, inputs.original, inputs.observed, map, files, texts,
        NULL, &plan.capture, client, NULL, 0, &proof) == SOURCE_COPY_PROOF_INVALID);
    assert(source_operation_plan_prove(&plan, &analysis.original, &proof) == SOURCE_COPY_PROOF_INPUTS);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_INVALID && plan.prepared);
    source_operation_plan_destroy(&plan);
    size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_operation_plan_init(&plan, sources.allocator); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceOperationPlanStatus status = source_operation_plan_build(&plan, &inputs);
        if (!failure) { assert(status == SOURCE_OPERATION_PLAN_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_OPERATION_PLAN_OK && !plan.prepared); }
        failure_at = 0; source_operation_plan_destroy(&plan); assert(live_allocations == live);
    }
    inputs.configuration_count = count - 1;
    source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_CONFIGURATION && !plan.prepared);
    assert(source_operation_plan_prove(&plan, client, &proof) == SOURCE_COPY_PROOF_INVALID);
    source_operation_plan_destroy(&plan); inputs.configuration_count = count;
    SourceOperationConfigurationExpectation saved = expected[count - 1]; expected[count - 1] = expected[0];
    source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_CONFIGURATION && !plan.prepared);
    source_operation_plan_destroy(&plan); expected[count - 1] = saved;
    SourceImageChange illegal = {.kind = SOURCE_IMAGE_REMOVE, .entry = {.path = source_path(tree.units[0]->source->text)}};
    inputs.changes = &illegal; inputs.change_count = 1;
    source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_REMOVAL && !plan.prepared);
    source_operation_plan_destroy(&plan); inputs.changes = changes; inputs.change_count = change_count;
    SourceCopyClosure policy = copy; policy.mode = SOURCE_COPY_KEEP_EXTERNAL;
    SourceCopyContextOmissions policy_context = *contexts; policy_context.copy = &policy;
    inputs.copy = &policy; inputs.contexts = &policy_context;
    source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_MODE && !plan.prepared);
    source_operation_plan_destroy(&plan);
    source_package_environment_destroy(&candidate); source_package_environment_destroy(&original);
}
