/** @file
 * @brief Verify complete source-domain admission independently of executable roots.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../compiler/common.c"
#include "delivery/source/domain/domain.h"
#include "../../../../../../tools/project/source/removal/check.h"
static SourceArena exports;
static const DeliverySourceDefinition *definitions;
/** Build an authoritative in-memory catalog; no physical source files exist. */
static void prepare(const char *const *names, const char *const *texts,
    const char *const *configurations, size_t count) {
    setup_frontend(); source_arena_init(&exports, sources.allocator); definitions = NULL;
    ModuleSnapshotEntry entries[8]; assert(count <= 8);
    for (size_t index = 0; index < count; index++) {
        const SourceText *text = NULL;
        SourceStatus added = source_manager_add(&sources, names[index],
            (const unsigned char *)texts[index], strlen(texts[index]), &text);
        assert(added == SOURCE_OK);
        const SourceText *configuration = NULL;
        if (configurations && configurations[index]) {
            SourceStatus configured = source_manager_add(&sources, "captured-module.ini",
                (const unsigned char *)configurations[index], strlen(configurations[index]), &configuration);
            assert(configured == SOURCE_OK);
        }
        entries[index] = (ModuleSnapshotEntry){names[index], text, 1, index + 1, configuration};
    }
    ModuleStatus captured = module_catalog_snapshot(&frontend.project.catalog, entries, count);
    assert(captured == MODULE_OK);
}
/** Export spans borrow the frontend, so release them first and require zero leaks. */
static void finish(void) { source_arena_destroy(&exports); destroy_frontend(); }
/** Retain explicitly captured ABI bytes without creating any filesystem input. */
static const SourceText *captured_abi(const char *path, const char *bytes) {
    const SourceText *text = NULL;
    SourceStatus status = source_manager_add(&sources, path, (const unsigned char *)bytes, strlen(bytes), &text);
    assert(status == SOURCE_OK);
    return text;
}
/** Unused concrete/open/static/constant definitions all contribute edges; draft
 * bytes remain available without activating unfinished bodies or any main. */
static void it_exports_the_complete_domain_without_an_executable_entry(void) {
    const char *names[] = {"//unused", "//marker", "//draft", "//helper", "//element", "//palette"};
    const char *texts[] = {"@!\nvalue\n//element\n", "@...\n", "unfinished !\n",
        "@<\nvalue\n.T\n@!\n", "@!\n", "@=\nitems[]\n//element\n"};
    prepare(names, texts, NULL, 6);
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    if (!checked) { fprintf(stderr, "Domain refused: %s\n", frontend.error.message); }
    assert(checked && !frontend.valid && !frontend.plan_initialized && !frontend.project.entry);
    const DeliverySourceDefinition *item = definitions;
    assert(item && !strcmp(item->identity, "//element")); item = item->next;
    assert(item && item->kind == DELIVERY_SOURCE_PARAMETERIZED); item = item->next;
    assert(item && item->kind == DELIVERY_SOURCE_STATIC); item = item->next;
    assert(item && item->kind == DELIVERY_SOURCE_CONSTANTS && item->dependencies);
    assert(!strcmp(item->dependencies->target, "//element")); item = item->next;
    assert(item && !strcmp(item->identity, "//unused") && item->dependencies && !item->next);
    assert(!strcmp(item->dependencies->target, "//element"));
    assert(item->dependencies->origin.source && frontend.attempted);
    finish();
}
/** A body error in an unused active definition is fatal, unlike a healthy open
 * helper. Never infer that failure means an uninstantiated template. */
static void it_refuses_unused_body_and_path_errors_without_publishing(void) {
    const char *names[] = {"//main", "//unused"};
    const char *bad[] = {"@!\n", "@!\nmissing !\n"};
    const char *absent[] = {"@!\n", "@!\nvalue\n//absent\n"};
    const char *syntax[] = {"@!\n", "@!\n bad\n"};
    const char *const *variants[] = {bad, absent, syntax};
    for (size_t index = 0; index < 3; index++) {
        prepare(names, variants[index], NULL, 2);
        DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
        assert(!checked && definitions == &sentinel && frontend.attempted && frontend.error.message);
        assert(frontend.error.primary.span.source);
        finish();
    }
}
/** Compose the checked whole domain with the original-cut predicate: an unused
 * outside consumer blocks a cyclic removal, while removing that consumer too
 * leaves no surviving incoming edge. No executable root establishes visibility. */
static void it_blocks_original_cycle_removal_for_an_unused_outside_consumer(void) {
    const char *names[] = {"//removed", "//inside", "//unused"};
    const char *texts[] = {"@!\nnext&\n//inside\n", "@!\nback&\n//removed\n",
        "@!\nvalue&\n//removed\n"};
    prepare(names, texts, NULL, 3);
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    assert(checked && !frontend.project.entry);
    const char *removed[] = {"//removed", "//inside", "//unused"};
    const SourceRemovalBlocker *blockers = NULL;
    SourceRemovalStatus status = source_removal_check(&exports, definitions, NULL,
        removed, 2, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_BLOCKED && blockers && !blockers->next);
    assert(!strcmp(blockers->owner->identity, "//unused") && !strcmp(blockers->target, "//removed"));
    assert(blockers->origin.source && blockers->origin.end > blockers->origin.start);
    status = source_removal_check(&exports, definitions, NULL, removed, 3, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_OK && !blockers);
    finish();
}
/** Native source dependencies remain ordinary original edges; sealed companions
 * are consumed without trying to compile nonexistent native body/header paths. */
static void it_admits_captured_native_metadata_without_native_execution(void) {
    const char *names[] = {"//native", "//element"};
    const char *texts[] = {"@C\n", "@!\nvalue\n//native\n"};
    prepare(names, texts, NULL, 2);
    const ModuleSource *native = NULL;
    ModuleStatus found = module_catalog_find(&frontend.project.catalog, "//native", &native);
    assert(found == MODULE_OK);
    const SourceText *abi = NULL;
    SourceStatus added = source_manager_add(&sources, "captured-native.abi",
        (const unsigned char *)"+ initialize\n- destroy\n", strlen("+ initialize\n- destroy\n"), &abi);
    assert(added == SOURCE_OK);
    ModuleStatus sealed = module_catalog_snapshot_native(&frontend.project.catalog, native,
        "nonexistent-native.c", "nonexistent-native.h", abi);
    assert(sealed == MODULE_OK);
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    if (!checked) { fprintf(stderr, "Native domain refused: %s\n", frontend.error.message); }
    assert(checked && definitions && definitions->dependencies);
    assert(!strcmp(definitions->dependencies->target, "//native"));
    assert(definitions->next && definitions->next->kind == DELIVERY_SOURCE_CONCRETE);
    assert(!frontend.inputs.sealed);
    finish();
}
/** Unconditional companion capture does not classify ordinary/draft sources as
 * native. Only production-classified native definitions receive sealed metadata. */
static void it_installs_prepared_companions_only_after_native_classification(void) {
    const char *names[] = {"//native", "//ordinary", "//draft"};
    const char *texts[] = {"@C\n", "@!\n", "unfinished !\n"};
    prepare(names, texts, NULL, 3);
    const SourceText *good = captured_abi("good.abi", "+ initialize\n- destroy\n");
    const SourceText *opaque = captured_abi("opaque.abi", "This must remain unparsed companion data\n");
    DeliverySourceCompanions ordinary = {.identity = "//ordinary", .body = "unused.c", .abi = opaque};
    DeliverySourceCompanions draft = {.next = &ordinary, .identity = "//draft", .abi = opaque};
    DeliverySourceCompanions native = {.next = &draft, .identity = "//native",
        .body = "missing-body.c", .header = "missing-header.h", .abi = good};
    bool checked = delivery_source_domain(&frontend, &exports, &native, &definitions);
    assert(checked);
    const ModuleSource *source = NULL;
    ModuleStatus found = module_catalog_find(&frontend.project.catalog, "//native", &source);
    assert(found == MODULE_OK && source->native_loaded && source->native_abi);
    assert(source->native_abi != good && !strcmp(source->native_c, "missing-body.c"));
    found = module_catalog_find(&frontend.project.catalog, "//ordinary", &source);
    assert(found == MODULE_OK && !source->native_loaded && !source->native_abi && !source->native_c);
    found = module_catalog_find(&frontend.project.catalog, "//draft", &source);
    assert(found == MODULE_OK && !source->native_loaded);
    finish();
}
/** Missing, duplicate or already-bound captured authority never selects a first
 * matching row or falls back to probing physical native paths. */
static void it_refuses_missing_duplicate_and_already_bound_native_capture_authority(void) {
    const char *names[] = {"//native"}; const char *texts[] = {"@C\n"};
    for (size_t variant = 0; variant < 4; variant++) {
        prepare(names, texts, NULL, 1);
        const SourceText *abi = captured_abi("captured.abi", "+ initialize\n- destroy\n");
        DeliverySourceCompanions second = {.identity = "//native", .abi = abi};
        DeliverySourceCompanions first = {.identity = "//native", .abi = abi};
        if (!variant) { first.identity = "//native-other"; }
        else if (variant == 1) { first.next = &second; }
        else if (variant == 2) { first.identity = NULL; }
        else {
            const ModuleSource *source = NULL;
            ModuleStatus status = module_catalog_find(&frontend.project.catalog, "//native", &source);
            assert(status == MODULE_OK);
            status = module_catalog_snapshot_native(&frontend.project.catalog, source, NULL, NULL, abi);
            assert(status == MODULE_OK);
        }
        DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        bool checked = delivery_source_domain(&frontend, &exports, &first, &definitions);
        assert(!checked && frontend.attempted && definitions == &sentinel);
        assert(frontend.error.code == DIAGNOSTIC_CHANGED_INPUT && frontend.error.primary.span.source);
        finish();
    }
}
/** Every classifier, companion-copy, ABI and ordinary-check allocation failure
 * retires safely without publishing a partially checked graph. */
static void it_preserves_output_on_every_prepared_native_domain_allocation_failure(void) {
    const char *names[] = {"//native"}; const char *texts[] = {"@C\n"};
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare(names, texts, NULL, 1);
        const SourceText *abi = captured_abi("captured.abi", "+ initialize\n- destroy\n");
        DeliverySourceCompanions companions = {.identity = "//native", .body = "missing.c", .header = "missing.h", .abi = abi};
        DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool checked = delivery_source_domain(&frontend, &exports, &companions, &definitions);
        if (!failure) { assert(checked); allocations = allocation_count - before; assert(allocations); }
        else { assert(!checked && frontend.attempted && definitions == &sentinel); }
        failure_at = 0; finish();
    }
}
/** Qualified constants preserve compiler-normalized members and exact whole
 * source sites. Simulate a fixed dependency on a checked edge to verify that
 * the transport exporter preserves this typed flag without parsing source text. */
static void it_retains_normalized_constant_members_and_fixed_dependency_sites(void) {
    const char *names[] = {"//main", "//palette", "//element"};
    const char *texts[] = {"@!\nitems[]\n ~chosen[]\n", "@=\nvalues[]\n//element\n", "@!\n"};
    const char *configurations[] = {"~chosen = //palette/values\n", NULL, NULL};
    prepare(names, texts, configurations, 3);
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    if (!checked) { fprintf(stderr, "Constant domain refused: %s\n", frontend.error.message); }
    assert(checked);
    const DeliverySourceDefinition *main = definitions;
    while (main && strcmp(main->identity, "//main")) { main = main->next; }
    assert(main && main->dependencies && !main->dependencies->implicit);
    assert(main->dependencies->member && !strcmp(main->dependencies->member, "values"));
    SourceSpan site = main->dependencies->origin;
    assert(site.end - site.start == strlen("~chosen"));
    assert(!memcmp(source_bytes(site.source) + site.start, "~chosen", strlen("~chosen")));
    const ProjectModule *module = project_find(&frontend.project, "//main");
    assert(module && module->first && !module->first->implicit);
    module->first->implicit = true;
    const DeliverySourceDefinition *again = NULL;
    SourceStatus exported = delivery_source_export(&exports, &frontend.static_inventory, &again);
    assert(exported == SOURCE_OK);
    while (again && strcmp(again->identity, "//main")) { again = again->next; }
    assert(again && again->dependencies->implicit && !strcmp(again->dependencies->member, "values"));
    assert(again->dependencies->member != main->dependencies->member);
    finish();
}
/** Entirely empty or draft-only catalogs need no synthetic executable module. */
static void it_accepts_empty_and_draft_only_domains(void) {
    const char *names[] = {"//draft"}; const char *texts[] = {"unfinished !\n"};
    for (size_t count = 0; count <= 1; count++) {
        prepare(names, texts, NULL, count);
        DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
        assert(checked && !definitions && !frontend.project.entry && !frontend.valid);
        finish();
    }
}
/** Exercise every preparation/parser/check/export allocation with exact fail-N
 * injection, preserving caller output and complete safe destruction. */
static void it_preserves_output_on_every_domain_allocation_failure(void) {
    const char *names[] = {"//main", "//palette", "//element"};
    const char *texts[] = {"@!\nitems[]\n //palette/values[]\n", "@=\nvalues[]\n//element\n", "@!\n"};
    size_t allocations = 0;
    for (size_t ordinal = 0; !ordinal || ordinal <= allocations; ordinal++) {
        prepare(names, texts, NULL, 3);
        DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        size_t before = allocation_count;
        if (ordinal) { failure_at = before + ordinal; }
        bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
        if (!ordinal) { assert(checked); allocations = allocation_count - before; assert(allocations); }
        else { assert(!checked && definitions == &sentinel && frontend.attempted); }
        failure_at = 0; finish();
    }
}
/** Invalid owner state and reuse refuse before changing the completed export. */
static void it_refuses_incomplete_and_already_attempted_frontends(void) {
    setup_frontend(); source_arena_init(&exports, sources.allocator);
    DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
    bool checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    assert(!checked && !frontend.attempted && definitions == &sentinel);
    finish();
    const char *names[] = {"//main"}; const char *texts[] = {"@!\n"};
    prepare(names, texts, NULL, 1);
    checked = delivery_source_domain(&frontend, &exports, NULL, &definitions); assert(checked);
    const DeliverySourceDefinition *previous = definitions;
    checked = delivery_source_domain(&frontend, &exports, NULL, &definitions);
    assert(!checked && definitions == previous);
    checked = delivery_source_domain(NULL, &exports, NULL, &definitions); assert(!checked);
    checked = delivery_source_domain(&frontend, NULL, NULL, &definitions); assert(!checked);
    checked = delivery_source_domain(&frontend, &exports, NULL, NULL); assert(!checked);
    finish();
}
/** Run complete-domain, refusal, empty, ownership and exhaustive allocation cases. */
void run(void) {
    it_exports_the_complete_domain_without_an_executable_entry();
    it_refuses_unused_body_and_path_errors_without_publishing();
    it_accepts_empty_and_draft_only_domains();
    it_retains_normalized_constant_members_and_fixed_dependency_sites();
    it_blocks_original_cycle_removal_for_an_unused_outside_consumer();
    it_admits_captured_native_metadata_without_native_execution();
    it_installs_prepared_companions_only_after_native_classification();
    it_refuses_missing_duplicate_and_already_bound_native_capture_authority();
    it_preserves_output_on_every_prepared_native_domain_allocation_failure();
    it_preserves_output_on_every_domain_allocation_failure();
    it_refuses_incomplete_and_already_attempted_frontends();
}
/** Keep assertions enabled in this component executable. */
int main(void) { run(); return 0; }
