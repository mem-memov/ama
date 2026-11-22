/** @file
 * @brief Verify direct test selection from real concrete/open/static/native definitions.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../../../../compiler/common.c"
#include "../../../../../../tools/project/acquisition/closure/tests/roots.h"
#include "delivery/source/export.h"
#include "delivery/source/entry.h"
static SourceArena output;
static const DeliverySourceDefinition *definitions;
/** Prepare real definitions; helper is valid but has an unconstrained own formal. */
static void prepare(void) {
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/sort");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/sort/child");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/sorted");
    file("generated/project/packages/test/fixture/v1.0.0/src/element", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/check",
        "@!\nvalue\n/test/fixture/v1.0.0/src/element\nworker\n./helper\nworker !\n < value\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/another", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/helper", "@<\nvalue\n.T\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/marker", "@...\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/child/check", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sorted/check", "@!\n");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/only helpers");
    file("generated/project/packages/test/fixture/v1.0.0/tests/only helpers/helper", "@<\nvalue\n.T\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/native", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/native.h", "#error Must not compile native C during test discovery\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/native.c", "#error Must not execute native C during test discovery\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/native.abi", "+ initialize\n- destroy\n");
    setup_frontend(); source_arena_init(&output, sources.allocator); definitions = NULL;
}
/** Check the whole explicitly required definition set without trial executable roots. */
static bool check(void) {
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&frontend.project.catalog, &root, 1); assert(status == MODULE_OK);
    static const char *const required[] = {
        "/test/fixture/v1.0.0/tests/sort/check", "/test/fixture/v1.0.0/tests/sort/another",
        "/test/fixture/v1.0.0/tests/sort/helper", "/test/fixture/v1.0.0/tests/sort/marker",
        "/test/fixture/v1.0.0/tests/sort/child/check", "/test/fixture/v1.0.0/tests/sorted/check",
        "/test/fixture/v1.0.0/tests/only helpers/helper", "/test/fixture/v1.0.0/tests/sort/native"};
    bool checked = compiler_frontend_check_snapshot(&frontend, required, sizeof(required) / sizeof(required[0]));
    if (!checked) { return false; }
    SourceStatus exported = delivery_source_export(&output, &frontend.static_inventory, &definitions);
    assert(exported == SOURCE_OK); return true;
}
/** Destroy projection before its borrowed real inventory/project. */
static void finish(void) { source_arena_destroy(&output); destroy_frontend(); }
/** Three direct concrete tests count; open, static, descendants and prefix-neighbors do not. */
static void it_selects_immediate_runtime_candidates_before_exact_entry_classification(void) {
    prepare(); bool checked = check(); assert(checked);
    const AcquisitionTestRoot *roots = NULL;
    AcquisitionTestRootsStatus status = acquisition_test_roots(&output, definitions,
        "/test/fixture/v1.0.0/tests/sort", &roots);
    assert(status == ACQUISITION_TEST_ROOTS_OK && roots && roots->next && roots->next->next && roots->next->next->next && !roots->next->next->next->next);
    assert(!strcmp(roots->definition->identity, "/test/fixture/v1.0.0/tests/sort/another"));
    assert(!strcmp(roots->next->definition->identity, "/test/fixture/v1.0.0/tests/sort/check"));
    assert(roots->next->next->definition->kind == DELIVERY_SOURCE_PARAMETERIZED);
    assert(!strcmp(roots->next->next->next->definition->identity, "/test/fixture/v1.0.0/tests/sort/native"));
    finish();
}
/** Descendant-only or absent directories cannot silently satisfy mandatory testing. */
static void it_reports_missing_candidates_and_preserves_parameterized_candidates_for_probing(void) {
    prepare(); bool checked = check(); assert(checked);
    AcquisitionTestRoot sentinel = {0}; const AcquisitionTestRoot *roots = &sentinel;
    AcquisitionTestRootsStatus status = acquisition_test_roots(&output, definitions,
        "/test/fixture/v1.0.0/tests/absent", &roots);
    assert(status == ACQUISITION_TEST_ROOTS_MISSING && roots == &sentinel);
    status = acquisition_test_roots(&output, definitions,
        "/test/fixture/v1.0.0/tests/only helpers", &roots);
    assert(status == ACQUISITION_TEST_ROOTS_OK && roots && !roots->next && roots->definition->kind == DELIVERY_SOURCE_PARAMETERIZED);
    finish();
}
/** Normal compiler errors remain fatal before any successful root projection. */
static void it_preserves_actual_helper_errors_instead_of_skipping_failed_compilation(void) {
    prepare();
    file("generated/project/packages/test/fixture/v1.0.0/tests/sort/helper", "@<\nvalue\n.T\n@!\nmissing !\n");
    bool checked = check(); assert(!checked && frontend.error.message);
    finish();
}
/** Every selected-root allocation can fail atomically, retaining safely owned scratch. */
static void it_preserves_output_on_every_root_projection_allocation_failure(void) {
    prepare(); bool checked = check(); assert(checked);
    const AcquisitionTestRoot *roots = NULL; size_t before = allocation_count;
    AcquisitionTestRootsStatus status = acquisition_test_roots(&output, definitions,
        "/test/fixture/v1.0.0/tests/sort", &roots); assert(status == ACQUISITION_TEST_ROOTS_OK);
    size_t count = allocation_count - before; assert(count == 4);
    for (size_t ordinal = 1; ordinal <= count; ordinal++) {
        SourceArena failing; source_arena_init(&failing, sources.allocator);
        AcquisitionTestRoot sentinel = {0}; roots = &sentinel; failure_at = allocation_count + ordinal;
        status = acquisition_test_roots(&failing, definitions,
            "/test/fixture/v1.0.0/tests/sort", &roots);
        assert(status == ACQUISITION_TEST_ROOTS_OUT_OF_MEMORY && roots == &sentinel);
        failure_at = 0; source_arena_destroy(&failing);
    }
    finish();
}
/** Export failure cannot publish partially copied compiler metadata to a tool. */
static void it_preserves_export_output_on_every_allocation_failure_and_invalid_inventory(void) {
    prepare(); bool checked = check(); assert(checked);
    SourceArena probe; source_arena_init(&probe, sources.allocator);
    const DeliverySourceDefinition *exported = NULL; size_t before = allocation_count;
    SourceStatus status = delivery_source_export(&probe, &frontend.static_inventory, &exported);
    assert(status == SOURCE_OK && exported); size_t count = allocation_count - before;
    source_arena_destroy(&probe); assert(count);
    for (size_t ordinal = 1; ordinal <= count; ordinal++) {
        SourceArena failing; source_arena_init(&failing, sources.allocator);
        DeliverySourceDefinition sentinel = {0}; exported = &sentinel;
        failure_at = allocation_count + ordinal;
        status = delivery_source_export(&failing, &frontend.static_inventory, &exported);
        assert(status == SOURCE_OUT_OF_MEMORY && exported == &sentinel);
        failure_at = 0; source_arena_destroy(&failing);
    }
    StaticInventory invalid = {0}; DeliverySourceDefinition sentinel = {0}; exported = &sentinel;
    status = delivery_source_export(&output, &invalid, &exported);
    assert(status == SOURCE_INVALID_ARGUMENT && exported == &sentinel);
    finish();
}
/** Original formals do not imply openness: a typed constant default closes the
 * independent root. A genuinely pending root is Open, and a real binder error
 * remains Invalid rather than becoming an ignored helper. */
static void it_classifies_grounded_formals_pending_roots_and_real_errors_by_typed_entry_evidence(void) {
    prepare();
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nvalue\n./element\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/grounded", "@>\nresult\n.T\n ./palette/value\n@!\n");
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    bool checked = compiler_frontend_check(&frontend, &root, 1, "/test/fixture/v1.0.0/src/grounded");
    assert(checked && delivery_source_entry_state(&frontend) == DELIVERY_ENTRY_EXECUTABLE);
    const StaticModule *module = static_module_find(&frontend.static_inventory, frontend.project.entry);
    assert(module && module->parameter_count == 1);
    CompilerFrontend probe; compiler_frontend_init(&probe, sources.allocator);
    checked = compiler_frontend_check(&probe, &root, 1, "/test/fixture/v1.0.0/tests/only helpers/helper");
    assert(!checked && delivery_source_entry_state(&probe) == DELIVERY_ENTRY_OPEN);
    compiler_frontend_destroy(&probe);
    file("generated/project/packages/test/fixture/v1.0.0/src/invalid", "@!\nmissing !\n");
    compiler_frontend_init(&probe, sources.allocator);
    checked = compiler_frontend_check(&probe, &root, 1, "/test/fixture/v1.0.0/src/invalid");
    assert(!checked && delivery_source_entry_state(&probe) == DELIVERY_ENTRY_INVALID && probe.error.message);
    compiler_frontend_destroy(&probe);
    file("generated/project/packages/test/fixture/v1.0.0/src/other", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/conflict",
        "@>\nresult\n.T\n ./palette/value\n@!\nother\n./other\nresult\nother\n");
    compiler_frontend_init(&probe, sources.allocator);
    checked = compiler_frontend_check(&probe, &root, 1, "/test/fixture/v1.0.0/src/conflict");
    assert(!checked && delivery_source_entry_state(&probe) == DELIVERY_ENTRY_INVALID && probe.error.message);
    compiler_frontend_destroy(&probe);
    file("generated/project/packages/test/fixture/v1.0.0/src/open producer", "@>\nresult\n.T\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unresolved",
        "@!\nproducer\n./open producer\nproducer !\n > fresh\n");
    compiler_frontend_init(&probe, sources.allocator);
    checked = compiler_frontend_check(&probe, &root, 1, "/test/fixture/v1.0.0/src/unresolved");
    assert(!checked && probe.specialization_initialized && probe.specializations.closure.status == STATIC_CLOSURE_UNRESOLVED);
    module = static_module_find(&probe.static_inventory, probe.project.entry); assert(module && !module->parameter_count);
    assert(delivery_source_entry_state(&probe) == DELIVERY_ENTRY_INVALID);
    compiler_frontend_destroy(&probe);
    file("generated/project/packages/test/fixture/v1.0.0/src/grounded unresolved",
        "@>\nresult\n.T\n ./palette/value\n@!\nproducer\n./open producer\nproducer !\n > fresh\n");
    compiler_frontend_init(&probe, sources.allocator);
    checked = compiler_frontend_check(&probe, &root, 1, "/test/fixture/v1.0.0/src/grounded unresolved");
    assert(!checked && probe.specializations.closure.status == STATIC_CLOSURE_UNRESOLVED);
    module = static_module_find(&probe.static_inventory, probe.project.entry); assert(module && module->parameter_count == 1);
    assert(delivery_source_entry_state(&probe) == DELIVERY_ENTRY_INVALID);
    compiler_frontend_destroy(&probe); finish();
}
/** Run semantic role, required-test, invalid-helper and resource partitions explicitly. */
void run(void) {
    it_selects_immediate_runtime_candidates_before_exact_entry_classification();
    it_reports_missing_candidates_and_preserves_parameterized_candidates_for_probing();
    it_preserves_actual_helper_errors_instead_of_skipping_failed_compilation();
    it_preserves_output_on_every_root_projection_allocation_failure();
    it_preserves_export_output_on_every_allocation_failure_and_invalid_inventory();
    it_classifies_grounded_formals_pending_roots_and_real_errors_by_typed_entry_evidence();
}
/** Exit only after complete counted-owner teardown. */
int main(void) { run(); return 0; }
