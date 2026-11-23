/** @file
 * @brief Exercise explicit O/U/F settings with actual promotion and existing/absent root relocation plans.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/roots/roots.h"
#include <stdint.h>
/** Use the real directory relation as the independent carry witness in fixtures. */
static bool setting_carried(const SourceOperationPlan *plan, const char *old, const char *now) {
    for (const SourceFileMapping *row = plan->inputs.files->first; row; row = row->next) {
        if (row->original->directory && !strcmp(row->source, old) && !strcmp(row->destination, now)) { return true; }
    }
    return false;
}
/** Independently capture inconsistent O facts, then restore host fixture bytes.
 * The gate must compare immutable captures rather than recapture newer host data.
 * Both O and U are genuine owners; no validity flags or catalog records change. */
static void check_root_settings_original_bytes(const SourceOperationPlan *plan) {
    const SourceRootCapture *augmented = plan->inputs.observed->original;
    const SourceCapturedUnit *unit = augmented->units; assert(unit && unit->source->version);
    for (unsigned configuration = 0; configuration < 2; configuration++) {
        const SourceText *old = configuration ? unit->source->version->configuration : unit->source->text;
        const char *path = source_path(old);
        file(path, configuration ? "; independently captured configuration\n" : "@!\n# independently captured source\n");
        SourceRootCapture different; source_root_capture_init(&different, sources.allocator);
        assert(source_root_capture_build(&different, augmented->roots, 1) == SOURCE_CAPTURE_OK);
        permission_source_file(path, old);
        SourceOperationRootDisposition original = {SOURCE_OPERATION_ROOT_DROP, SIZE_MAX};
        SourceOperationRootAddition final = {0, SIZE_MAX};
        SourceOperationRootsInputs inputs = {.configured = &different, .plan = plan, .originals = &original,
            .original_count = 1, .additions = &final, .addition_count = 1};
        SourceOperationRoots owner; source_operation_roots_init(&owner, sources.allocator);
        assert(source_operation_roots_build(&owner, &inputs) == SOURCE_OPERATION_ROOTS_ORIGINAL
            && !owner.valid && owner.failed_path && !strcmp(owner.failed_path, path));
        source_operation_roots_destroy(&owner); source_root_capture_destroy(&different);
    }
}
/** Actual projected images have one final root. Configured O is checked both as
 * the identical U owner and as an independently captured earlier subset of U. */
static void check_operation_root_settings(const SourceOperationPlan *plan, const SourceCompilerClient *compiler) {
    assert(plan->prepared && plan->inputs.final_root_count == 1);
    const SourceRootCapture *augmented = plan->inputs.observed->original;
    assert(augmented->root_count && augmented->root_count <= 2);
    SourceRootCapture configured; source_root_capture_init(&configured, sources.allocator);
    assert(source_root_capture_build(&configured, augmented->roots, 1) == SOURCE_CAPTURE_OK);
    for (unsigned profile = 0; profile < 2; profile++) {
        const SourceRootCapture *original = profile ? &configured : augmented;
        SourceOperationRootDisposition requests[2];
        for (size_t i = 0; i < original->root_count; i++) {
            requests[i] = (SourceOperationRootDisposition){
                !strcmp(original->roots[i].path, plan->inputs.final_roots[0].path) ? SOURCE_OPERATION_ROOT_KEEP : SOURCE_OPERATION_ROOT_CARRY, 0};
            assert(requests[i].action == SOURCE_OPERATION_ROOT_KEEP
                || setting_carried(plan, original->roots[i].path, plan->inputs.final_roots[0].path));
        }
        SourceOperationRootsInputs inputs = {.configured = original, .plan = plan,
            .originals = requests, .original_count = original->root_count};
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceOperationRoots result; source_operation_roots_init(&result, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            SourceOperationRootsStatus status = source_operation_roots_build(&result, &inputs);
            if (!failure) {
                assert(status == SOURCE_OPERATION_ROOTS_OK && result.valid);
                total = allocation_count - before; assert(total);
                assert(result.first && result.first->original == original->roots && result.first->final == plan->inputs.final_roots);
                size_t calls = allocation_count;
                assert(source_operation_roots_build(&result, &inputs) == SOURCE_OPERATION_ROOTS_INVALID && result.valid && allocation_count == calls);
            } else { assert(status == SOURCE_OPERATION_ROOTS_MEMORY && !result.valid); }
            failure_at = 0; source_operation_roots_destroy(&result); assert(live_allocations == baseline);
        }
        SourceOperationRoots accepted; source_operation_roots_init(&accepted, sources.allocator);
        assert(source_operation_roots_build(&accepted, &inputs) == SOURCE_OPERATION_ROOTS_OK);
        check_operation_rebuild_intentions(&accepted, compiler); source_operation_roots_destroy(&accepted);
        for (unsigned bad = 0; bad < 5; bad++) {
            SourceOperationRootsInputs invalid = inputs;
            SourceOperationRootAddition extra = {0, SIZE_MAX};
            if (bad == 0) { invalid.original_count--; }
            if (bad == 1) { invalid.originals = NULL; }
            if (bad == 2) { invalid.additions = &extra; invalid.addition_count = 1; }
            if (bad == 3) { invalid.additions = &extra; invalid.addition_count = SIZE_MAX; }
            if (bad == 4) { invalid.plan = NULL; }
            SourceOperationRoots result; source_operation_roots_init(&result, sources.allocator);
            assert(source_operation_roots_build(&result, &invalid) != SOURCE_OPERATION_ROOTS_OK && !result.valid);
            source_operation_roots_destroy(&result);
        }
        for (size_t i = 0; i < original->root_count; i++) { requests[i] = (SourceOperationRootDisposition){SOURCE_OPERATION_ROOT_DROP, SIZE_MAX}; }
        SourceOperationRoots result; source_operation_roots_init(&result, sources.allocator);
        assert(source_operation_roots_build(&result, &inputs) == SOURCE_OPERATION_ROOTS_ACCOUNTING && !result.valid);
        source_operation_roots_destroy(&result);
        SourceOperationRootAddition explicit = {0, SIZE_MAX}; inputs.additions = &explicit; inputs.addition_count = 1;
        source_operation_roots_init(&result, sources.allocator);
        assert(source_operation_roots_build(&result, &inputs) == SOURCE_OPERATION_ROOTS_OK && result.valid);
        assert(result.last->addition && result.last->explicit_destination && result.last->action == SOURCE_OPERATION_ROOT_ADD
            && !result.last->original && result.last->final == plan->inputs.final_roots);
        source_operation_roots_destroy(&result);
    }
    source_root_capture_destroy(&configured);
    check_root_settings_original_bytes(plan);
    assert(source_operation_roots_build(NULL, NULL) == SOURCE_OPERATION_ROOTS_INVALID);
}
