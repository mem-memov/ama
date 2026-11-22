/** @file
 * @brief Verify simultaneous clone/survivor binding laws on independent finite graph fixtures.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/graph/bindings.h"
#include "../../../../../../tools/project/source/graph/sites.h"
/** Use genuine original handles; fixture exports below test only the binding predicate. */
static void binding_map(bool second) {
    source_clone_map_init(&map, sources.allocator, &capture);
    const SourceCloneMapping *row = NULL;
    assert(source_clone_map_add(&map, original("/vendor/pkg/v1.0.0/src/parent/child"), "/mapped/a", "/vendor/target/v2.0.0/src/a", &row) == SOURCE_CLONE_OK);
    if (second) { assert(source_clone_map_add(&map, original("/vendor/pkg/v1.0.0/src/parentish"), "/mapped/b", "/vendor/target/v2.0.0/src/b", &row) == SOURCE_CLONE_OK); }
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
}
/** All self-recursive removal/copy subsets and closed two-source cycles preserve standard edges. */
static void it_preserves_simultaneous_cycle_bindings_for_every_clone_and_survivor_subset(void) {
    const char *a = "/vendor/pkg/v1.0.0/src/parent/child", *b = "/vendor/pkg/v1.0.0/src/parentish";
    for (unsigned second = 0; second < 2; second++) {
        for (unsigned removed = 0; removed < 4; removed++) {
            for (unsigned cycle = 0; cycle < 2; cycle++) {
                /* A checked final cycle can retain both originals, or remove both
                 * when both copies exist. Other cuts are removal blockers, not
                 * successful complete final graph fixtures. */
                if (cycle && removed && (removed != 3 || !second)) { continue; }
                prepare(); binding_map(second != 0);
                DeliverySourceDependency external = {.target = "//standard", .implicit = true};
                DeliverySourceDependency edges[] = {{.target = cycle ? b : a, .next = &external, .member = "choice"}, {.target = cycle ? a : b}};
                DeliverySourceDefinition old[] = {{.identity = a, .dependencies = &edges[0]},
                    {.identity = b, .dependencies = &edges[1]}, {.identity = "//standard"}};
                old[0].next = &old[1]; old[1].next = &old[2];
                DeliverySourceDependency copied_edges[] = {{.target = cycle ? second ? "/vendor/target/v2.0.0/src/b" : b : "/vendor/target/v2.0.0/src/a", .next = &external, .member = "choice"},
                    {.target = cycle ? "/vendor/target/v2.0.0/src/a" : "/vendor/target/v2.0.0/src/b"}};
                DeliverySourceDefinition copies[] = {{.identity = "/vendor/target/v2.0.0/src/a", .dependencies = &copied_edges[0]},
                    {.identity = "/vendor/target/v2.0.0/src/b", .dependencies = &copied_edges[1]}};
                DeliverySourceDefinition survivors[] = {old[0], old[1], old[2]};
                DeliverySourceDefinition *final = &copies[0];
                copies[0].next = second ? &copies[1] : &survivors[2]; copies[1].next = &survivors[2];
                if (!(removed & 2)) { survivors[1].next = final; final = &survivors[1]; }
                if (!(removed & 1)) { survivors[0].next = final; final = &survivors[0]; }
                const char *deleted[2]; size_t count = 0;
                if (removed & 1) { deleted[count++] = a; }
                if (removed & 2) { deleted[count++] = b; }
                SourceBindingsIssue issue = {0}; size_t before = allocation_count; failure_at = before + 1;
                assert(source_graph_bindings(old, final, &map, deleted, count, &issue) == SOURCE_BINDINGS_OK);
                assert(!issue.original && !issue.final && !issue.expected && allocation_count == before);
                failure_at = 0; source_clone_map_destroy(&map); finish();
            }
        }
    }
}
/** Explicit member/implicit/multiplicity/category distinctions and old-importer targets are immutable. */
static void it_refuses_each_changed_clone_edge_field_and_surviving_original_redirection(void) {
    const char *a = "/vendor/pkg/v1.0.0/src/parent/child";
    for (unsigned changed = 0; changed < 8; changed++) {
        prepare(); binding_map(false);
        DeliverySourceDependency old_edge = {.target = a, .member = "value"};
        DeliverySourceDefinition old = {.identity = a, .dependencies = &old_edge};
        DeliverySourceDependency copy_edge = {.target = "/vendor/target/v2.0.0/src/a", .member = "value"};
        DeliverySourceDependency extra = copy_edge;
        DeliverySourceDefinition copy = {.identity = "/vendor/target/v2.0.0/src/a", .dependencies = &copy_edge};
        DeliverySourceDefinition survivor = old; survivor.next = &copy;
        DeliverySourceDependency changed_survivor = old_edge;
        if (changed == 0) { copy_edge.target = a; }
        if (changed == 1) { copy_edge.member = NULL; }
        if (changed == 2) { copy_edge.member = "other"; }
        if (changed == 3) { copy_edge.implicit = true; }
        if (changed == 4) { copy.dependencies = NULL; }
        if (changed == 5) { copy_edge.next = &extra; }
        if (changed == 6) { copy.kind = DELIVERY_SOURCE_PARAMETERIZED; }
        if (changed == 7) { changed_survivor.target = "/vendor/target/v2.0.0/src/a"; survivor.dependencies = &changed_survivor; }
        SourceBindingsIssue issue = {0};
        assert(source_graph_bindings(&old, &survivor, &map, NULL, 0, &issue)
            == (changed == 6 ? SOURCE_BINDINGS_KIND : SOURCE_BINDINGS_EDGE));
        assert(issue.original == &old && issue.final == (changed == 7 ? &survivor : &copy));
        if (changed != 6) { assert(issue.original_edge || issue.final_edge); }
        source_clone_map_destroy(&map); finish();
    }
}
/** Missing/extra definitions and nominal overlap are independent of edge checks. */
static void it_refuses_missing_extra_and_survivor_clone_identity_collisions(void) {
    const char *a = "/vendor/pkg/v1.0.0/src/parent/child";
    for (unsigned changed = 0; changed < 5; changed++) {
        prepare(); source_clone_map_init(&map, sources.allocator, &capture);
        const SourceCloneMapping *row = NULL;
        const ModuleSource *input = original(changed == 4 ? "/vendor/pkg/v1.0.0/src/parent" : a);
        assert(source_clone_map_add(&map, input, "/mapped/a", changed == 3 ? a : "/vendor/target/v2.0.0/src/a", &row) == SOURCE_CLONE_OK);
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        DeliverySourceDefinition copy = {.identity = changed == 3 ? a : "/vendor/target/v2.0.0/src/a"};
        DeliverySourceDefinition old = {.identity = a}, survivor = old, extra = {.identity = "//unexpected"};
        survivor.next = &copy;
        const DeliverySourceDefinition *final = &survivor;
        if (changed == 0) { final = &copy; }
        if (changed == 1) { survivor.next = NULL; }
        if (changed == 2) { copy.next = &extra; }
        SourceBindingsIssue issue = {0};
        SourceBindingsStatus expected = changed < 2 ? SOURCE_BINDINGS_MISSING
            : changed == 3 ? SOURCE_BINDINGS_COLLISION : SOURCE_BINDINGS_EXTRA;
        assert(source_graph_bindings(&old, final, &map, NULL, 0, &issue) == expected);
        assert(issue.expected);
        if (changed == 3) {
            const char *removed[] = {a};
            assert(source_graph_bindings(&old, &copy, &map, removed, 1, &issue) == SOURCE_BINDINGS_OK);
        }
        source_clone_map_destroy(&map); finish();
    }
}
/** Empty checked domains work, while unsealed relations and malformed array interfaces refuse. */
static void it_checks_binding_laws_without_allocating_and_rejects_invalid_owners(void) {
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    SourceBindingsIssue issue = {0};
    assert(source_graph_bindings(NULL, NULL, &map, NULL, 0, &issue) == SOURCE_BINDINGS_INVALID);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    size_t before = allocation_count; failure_at = before + 1;
    assert(source_graph_bindings(NULL, NULL, &map, NULL, 0, &issue) == SOURCE_BINDINGS_OK);
    assert(source_graph_bindings(NULL, NULL, NULL, NULL, 0, &issue) == SOURCE_BINDINGS_INVALID);
    assert(source_graph_bindings(NULL, NULL, &map, NULL, 1, &issue) == SOURCE_BINDINGS_INVALID);
    const char *removed[] = {NULL};
    assert(source_graph_bindings(NULL, NULL, &map, removed, 1, &issue) == SOURCE_BINDINGS_INVALID);
    assert(source_graph_bindings(NULL, NULL, &map, NULL, 0, NULL) == SOURCE_BINDINGS_INVALID);
    assert(allocation_count == before); failure_at = 0; source_clone_map_destroy(&map); finish();
}
/** A replacement that reuses an identifier cannot stand in for a retained original target. */
static void it_refuses_retained_targets_removed_by_replacement_even_when_the_final_name_matches(void) {
    const char *a = "/vendor/pkg/v1.0.0/src/parent/child", *b = "/vendor/pkg/v1.0.0/src/parentish";
    for (unsigned survivor = 0; survivor < 2; survivor++) {
        prepare(); source_clone_map_init(&map, sources.allocator, &capture);
        const SourceCloneMapping *row = NULL;
        assert(source_clone_map_add(&map, original(a), source_path(original(b)->text), b, &row) == SOURCE_CLONE_OK);
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        DeliverySourceDependency old_edge = {.target = b}, copy_edge = {.target = b};
        DeliverySourceDefinition old_target = {.identity = b}, old = {.identity = a, .dependencies = &old_edge, .next = &old_target};
        DeliverySourceDefinition copy = {.identity = b, .dependencies = &copy_edge}, kept = {.identity = a, .dependencies = &old_edge, .next = &copy};
        const char *removed[] = {b, a};
        SourceBindingsIssue issue = {0};
        assert(source_graph_bindings(&old, survivor ? &kept : &copy, &map, removed, survivor ? 1 : 2, &issue) == SOURCE_BINDINGS_REMOVED_TARGET);
        assert(issue.original == &old && issue.original_edge == &old_edge && !strcmp(issue.expected, b));
        assert(issue.final_edge && !strcmp(issue.final_edge->target, b));
        assert((issue.clone != NULL) == !survivor);
        SourceSitesIssue sites = {0};
        assert(source_graph_sites(&old, survivor ? &kept : &copy, &map, removed,
            survivor ? 1 : 2, NULL, &sites) == SOURCE_SITES_BINDINGS);
        assert(sites.binding_status == SOURCE_BINDINGS_REMOVED_TARGET);
        source_clone_map_destroy(&map); finish();
    }
}
/** A mapped self-reference remains valid when relocation preserves its complete identity. */
static void it_accepts_identity_preserving_mapped_self_references_after_original_removal(void) {
    prepare(); const char *a = "/vendor/pkg/v1.0.0/src/parent/child";
    source_clone_map_init(&map, sources.allocator, &capture); const SourceCloneMapping *row = NULL;
    assert(source_clone_map_add(&map, original(a), "/moved/a", a, &row) == SOURCE_CLONE_OK);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    DeliverySourceDependency old_edge = {.target = a}, copied_edge = {.target = a};
    DeliverySourceDefinition old = {.identity = a, .dependencies = &old_edge}, copied = {.identity = a, .dependencies = &copied_edge};
    const char *removed[] = {a}; SourceBindingsIssue issue = {0};
    assert(source_graph_bindings(&old, &copied, &map, removed, 1, &issue) == SOURCE_BINDINGS_OK);
    source_clone_map_destroy(&map); finish();
}
