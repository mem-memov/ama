/** @file
 * @brief Own real removal observations and empty copy relations for the shared operation plan.
 * @see [T283 contract](../../../../../../../docs/implementation/source-operations.md#t283-entry-review-and-execution)
 */
#include "../../../../../../../tools/project/source/operation/plan/plan.h"
#include "../../../../../../../tools/project/source/removal/selection/material.h"
/** Retain every initialized failure prefix; the plan borrows all other fields. */
typedef struct {
    SourceArena arena;
    SourceObservedImage observed;
    SourceCloneMap mains;
    SourceFileMap files;
    SourcePackageEnvironment expected;
    SourceOperationPlan plan;
} RemovalPlanFixture;
/** Prepare actual read-only facts without invoking a removal-specific planner.
 * These fixture configurations have no explicit selectors; assert that premise
 * independently before supplying their exact empty expected permission sets. */
static bool removal_plan_fixture_build(RemovalPlanFixture *fixture, SourceRemovalSelection *selected) {
    memset(fixture, 0, sizeof(*fixture)); source_arena_init(&fixture->arena, sources.allocator);
    source_observed_image_init(&fixture->observed, sources.allocator);
    source_clone_map_init(&fixture->mains, sources.allocator, &capture);
    source_file_map_init(&fixture->files, sources.allocator);
    source_package_environment_init(&fixture->expected, sources.allocator);
    source_operation_plan_init(&fixture->plan, sources.allocator);
    if (source_clone_map_seal(&fixture->mains) != SOURCE_CLONE_OK
        || source_file_map_seal(&fixture->files) != SOURCE_FILE_MAP_OK) { return false; }
    SourceRemovalMaterial material;
    if (source_removal_material_prepare(&fixture->arena, selected, &material) != SOURCE_REMOVAL_MATERIAL_OK) { return false; }
    if (source_observed_image_build(&fixture->observed, &capture, (SourceImageBounds){0},
        material.requests, material.count) != SOURCE_OBSERVATION_OK) { return false; }
    if (source_package_environment_build(&fixture->expected, &capture.catalog) != SOURCE_ENVIRONMENT_OK) { return false; }
    size_t count = 0;
    for (const SourcePackageScope *scope = fixture->expected.first; scope; scope = scope->next) { count++; }
    SourceOperationConfigurationExpectation *expectations = count ? source_arena_allocate(&fixture->arena, count * sizeof(*expectations)) : NULL;
    if (count && !expectations) { return false; }
    size_t index = 0;
    for (const SourcePackageScope *scope = fixture->expected.first; scope; scope = scope->next) {
        assert(!scope->selection->count);
        expectations[index++] = (SourceOperationConfigurationExpectation){.environment = &fixture->expected, .scope = scope};
    }
    SourceOperationPlanInputs inputs = {.original = &analysis,
        .final_roots = capture.roots, .final_root_count = capture.root_count,
        .observed = &fixture->observed, .mains = &fixture->mains, .files = &fixture->files,
        .changes = material.changes, .change_count = material.count, .removal = selected,
        .configurations = expectations, .configuration_count = count};
    return source_operation_plan_build(&fixture->plan, &inputs) == SOURCE_OPERATION_PLAN_OK;
}
/** Retire the plan before every borrowed map/observation/configuration owner. */
static void removal_plan_fixture_destroy(RemovalPlanFixture *fixture) {
    source_operation_plan_destroy(&fixture->plan);
    source_package_environment_destroy(&fixture->expected);
    source_file_map_destroy(&fixture->files); source_clone_map_destroy(&fixture->mains);
    source_observed_image_destroy(&fixture->observed); source_arena_destroy(&fixture->arena);
}
/** Inject every allocation in the actual shared final-proof entry. Fresh child
 * plans make retained successful/failed proof scratch observable at teardown. */
static void removal_plan_proof_failures(const SourceOperationPlan *prepared, const SourceCompilerClient *client) {
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
        assert(source_operation_plan_build(&plan, &prepared->inputs) == SOURCE_OPERATION_PLAN_OK);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCopyProofIssue issue;
        SourceCopyProofStatus status = source_operation_plan_prove(&plan, client, &issue);
        if (!failure) { assert(status == SOURCE_COPY_PROOF_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_COPY_PROOF_OK); }
        failure_at = 0; source_operation_plan_destroy(&plan); assert(live_allocations == baseline);
    }
}
