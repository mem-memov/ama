/** @file
 * @brief Compose resolved source cycles and original alias scopes with delivery work scheduling.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../../../../compiler/common.c"
#include "../../../../../../tools/project/acquisition/closure/dependencies/expand.h"
#include "delivery/source/export.h"
static AcquisitionRoles roles;
static SourceArena exports;
static const DeliverySourceDefinition *definitions;
/** Query plain exported names independently of compiler registry internals. */
static const DeliverySourceDefinition *find_definition(const char *identity) {
    for (const DeliverySourceDefinition *item = definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    return NULL;
}
/** Real source scopes resolve aliases; reference fields form a finite source cycle. */
static void prepare(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nnext&\n~target\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.ini", "~target = /test/fixture/v1.0.0/src/other\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/other", "@!\nback&\n/test/fixture/v1.0.0/src/main\n");
    setup_frontend(); acquisition_roles_init(&roles, sources.allocator);
    source_arena_init(&exports, sources.allocator); definitions = NULL;
    bool checked = check_project();
    if (!checked) { fprintf(stderr, "Frontend refused dependency fixture: %s\n", frontend.error.message); }
    assert(checked);
    SourceStatus exported = delivery_source_export(&exports, &frontend.static_inventory, &definitions);
    assert(exported == SOURCE_OK);
}
/** Preserve borrowed compiler source evidence until after queue retirement. */
static void finish(void) { acquisition_roles_destroy(&roles); source_arena_destroy(&exports); destroy_frontend(); }
/** Discover a closed actual cycle in FIFO order without expanding either identity twice. */
static void it_expands_actual_alias_resolved_cycle_once_with_original_requirement_spans(void) {
    prepare();
    const AcquisitionRoleRecord *selected = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/test/fixture/v1.0.0/src/main", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &selected); assert(status == ACQUISITION_ROLE_OK);
    const AcquisitionRoleRecord *current; size_t count = 0;
    while ((current = acquisition_roles_take(&roles))) {
        count++; assert(count <= 2);
        const DeliverySourceDefinition *definition = find_definition(current->identity); assert(definition);
        status = acquisition_dependencies_expand(&roles, current, definition, NULL, NULL);
        assert(status == ACQUISITION_ROLE_OK);
    }
    assert(count == 2 && selected->reasons->requester && selected->reasons->next);
    const AcquisitionRoleRecord *other = roles.records; assert(other != selected);
    assert(!strcmp(other->identity, "/test/fixture/v1.0.0/src/other"));
    SourceSpan origin = other->reasons->origin;
    assert(origin.source == frontend.project.entry->source->text);
    assert(origin.end - origin.start == strlen("~target") && !memcmp(source_bytes(origin.source) + origin.start, "~target", strlen("~target")));
    assert(other->reasons->requester == selected);
    status = acquisition_dependencies_expand(&roles, selected, find_definition("/test/fixture/v1.0.0/src/main"), NULL, NULL);
    assert(status == ACQUISITION_ROLE_OK && !acquisition_roles_take(&roles));
    finish();
}
/** Static producers, their nested arguments and constant namespaces remain real
 * source dependencies even though no runtime call executes those graph nodes. */
static void it_expands_actual_static_and_constant_edges_without_unrelated_sources(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main.ini", "");
    file("generated/project/packages/test/fixture/v1.0.0/src/main",
        "@!\nvalue\n./assembly\n .export\nfirst[]\n ./palette/values[]\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly",
        "@...\n.export\n./box\n .element < ./element\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\nvalue\n.element\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/element", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nvalues[]\n./element\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@!\n");
    setup_frontend(); acquisition_roles_init(&roles, sources.allocator);
    source_arena_init(&exports, sources.allocator); definitions = NULL;
    bool checked = check_project();
    if (!checked) { fprintf(stderr, "Frontend refused dependency fixture: %s\n", frontend.error.message); }
    assert(checked);
    SourceStatus exported = delivery_source_export(&exports, &frontend.static_inventory, &definitions);
    assert(exported == SOURCE_OK);
    const AcquisitionRoleRecord *selected = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/test/fixture/v1.0.0/src/main", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &selected); assert(status == ACQUISITION_ROLE_OK);
    const AcquisitionRoleRecord *current; size_t count = 0; unsigned observed = 0;
    static const char *const expected[] = {"main", "assembly", "box", "element", "palette"};
    while ((current = acquisition_roles_take(&roles))) {
        count++; assert(count <= 5);
        const char *name = strrchr(current->identity, '/') + 1;
        for (size_t index = 0; index < 5; index++) { if (!strcmp(name, expected[index])) { observed |= 1u << index; } }
        const DeliverySourceDefinition *definition = find_definition(current->identity); assert(definition);
        status = acquisition_dependencies_expand(&roles, current, definition, NULL, NULL);
        assert(status == ACQUISITION_ROLE_OK);
    }
    assert(count == 5 && observed == 31);
    assert(find_definition("/test/fixture/v1.0.0/src/assembly")->kind == DELIVERY_SOURCE_STATIC);
    assert(find_definition("/test/fixture/v1.0.0/src/box")->kind == DELIVERY_SOURCE_PARAMETERIZED);
    assert(find_definition("/test/fixture/v1.0.0/src/palette")->kind == DELIVERY_SOURCE_CONSTANTS);
    finish();
}
/** A foreign/mismatched handle must refuse before dereferencing absent metadata. */
static void it_refuses_foreign_and_mismatched_records_without_expanding(void) {
    prepare(); const AcquisitionRoleRecord *selected = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/test/fixture/v1.0.0/src/other", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &selected); assert(status == ACQUISITION_ROLE_OK);
    AcquisitionRoleRecord foreign = {0};
    status = acquisition_dependencies_expand(&roles, &foreign, find_definition("/test/fixture/v1.0.0/src/main"), NULL, NULL);
    assert(status == ACQUISITION_ROLE_INVALID);
    status = acquisition_dependencies_expand(&roles, selected, find_definition("/test/fixture/v1.0.0/src/main"), NULL, NULL);
    assert(status == ACQUISITION_ROLE_INVALID && selected->reasons && !selected->reasons->next);
    finish();
}
/** Keep checked edges external without mutating their original binding. */
static bool retain_external(void *context, const DeliverySourceDefinition *definition,
    const DeliverySourceDependency *edge) {
    assert(definition && edge && edge->target);
    (*(size_t *)context)++;
    return false;
}
/** Explicit external membership allocates nothing; foreign owners never reach policy. */
static void it_retains_external_edges_without_allocating_or_consulting_policy_for_foreign_owners(void) {
    prepare();
    const AcquisitionRoleRecord *selected = NULL;
    AcquisitionRoleStatus status = acquisition_roles_add(&roles,
        "/test/fixture/v1.0.0/src/main", ACQUISITION_ACTIVE,
        ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &selected);
    assert(status == ACQUISITION_ROLE_OK);
    const DeliverySourceDefinition *definition = find_definition(selected->identity);
    const DeliverySourceDependency *edge = definition->dependencies;
    assert(edge && !edge->next);
    const char *target = edge->target;
    size_t calls = 0, before = allocation_count;
    status = acquisition_dependencies_expand(&roles, selected, definition, retain_external, &calls);
    assert(status == ACQUISITION_ROLE_OK && calls == 1 && allocation_count == before);
    assert(roles.records == selected && !selected->next && edge->target == target);
    assert(!strcmp(target, "/test/fixture/v1.0.0/src/other"));
    AcquisitionRoleRecord foreign = *selected;
    status = acquisition_dependencies_expand(&roles, &foreign, definition, retain_external, &calls);
    assert(status == ACQUISITION_ROLE_INVALID && calls == 1 && allocation_count == before);
    finish();
}
/** Each failed edge insertion remains owned and cannot publish a target record. */
static void it_retires_failed_actual_dependency_edge_allocations(void) {
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare(); const AcquisitionRoleRecord *selected = NULL;
        AcquisitionRoleStatus status = acquisition_roles_add(&roles, "/test/fixture/v1.0.0/src/main", ACQUISITION_ACTIVE,
            ACQUISITION_SELECTED, NULL, (SourceSpan){0}, &selected); assert(status == ACQUISITION_ROLE_OK);
        const AcquisitionRoleRecord *claimed = acquisition_roles_take(&roles); assert(claimed == selected);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        status = acquisition_dependencies_expand(&roles, selected, find_definition("/test/fixture/v1.0.0/src/main"), NULL, NULL);
        if (!failure) { assert(status == ACQUISITION_ROLE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == ACQUISITION_ROLE_OUT_OF_MEMORY && roles.records == selected && !roles.pending); failure_at = 0; }
        finish();
    }
}
/** Run actual resolved source composition, handle validation and memory refusals. */
void run(void) {
    it_expands_actual_alias_resolved_cycle_once_with_original_requirement_spans();
    it_refuses_foreign_and_mismatched_records_without_expanding();
    it_expands_actual_static_and_constant_edges_without_unrelated_sources();
    it_retires_failed_actual_dependency_edge_allocations();
    it_retains_external_edges_without_allocating_or_consulting_policy_for_foreign_owners();
}
/** Finish after all counted queue/frontend owners retire. */
int main(void) { run(); return 0; }
