/** @file
 * @brief Verify exact original/prepared occurrence handoffs across independently owned captures.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/graph/sites.h"
/** Correct deep-owned sites pass; missing, shifted, foreign and contradictory proofs refuse. */
static void it_requires_exact_prepared_site_provenance_after_binding_admission(void) {
    const char *a = "/vendor/pkg/v1.0.0/src/parent/child";
    for (unsigned choice = 0; choice < 11; choice++) {
        prepare(); binding_map(false);
        const SourceText *old_text = original(a)->text, *edit_input = old_text;
        if (choice == 9) {
            assert(source_manager_add(&sources, source_path(old_text), source_bytes(old_text), source_length(old_text), &edit_input) == SOURCE_OK);
        }
        SourceTextEdits text_edits; source_text_edits_init(&text_edits, sources.allocator, edit_input);
        SourceSpan replace = {edit_input, 0, choice == 4 ? 2 : 1};
        assert(source_text_edits_add(&text_edits, replace, (const unsigned char *)"long", 4) == SOURCE_EDITS_OK);
        const SourceText *result = old_text;
        if (choice != 6) { assert(source_text_edits_apply(&text_edits, &sources, "/mapped/a", &result) == SOURCE_EDITS_OK); }
        const SourceText *deep = NULL;
        assert(source_manager_add(&sources, choice == 3 ? "/wrong/a" : source_path(result),
            source_bytes(result), source_length(result), &deep) == SOURCE_OK);
        assert(deep != result);
        DeliverySourceDependency old_edge = {.target = a, .origin = {old_text, choice == 4 ? 1 : 0, source_length(old_text)}};
        DeliverySourceDefinition old = {.identity = a, .dependencies = &old_edge};
        DeliverySourceDependency copy_edge = {.target = choice == 8 ? a : "//a", .origin = {deep, choice == 2 ? 1 : 0, source_length(deep)}};
        DeliverySourceDefinition copy = {.identity = "//a", .dependencies = &copy_edge};
        DeliverySourceDependency survivor_edge = old_edge;
        if (choice == 7) { survivor_edge.origin.start = 1; }
        DeliverySourceDefinition survivor = old; survivor.dependencies = &survivor_edge; survivor.next = &copy;
        SourceGraphPreparedText duplicate = {.edits = &text_edits}, prepared = {.edits = &text_edits};
        if (choice == 5) { prepared.next = &duplicate; }
        SourceSitesIssue issue = {0}; size_t before = allocation_count; failure_at = before + 1;
        SourceSitesStatus expected = choice == 0 ? SOURCE_SITES_OK
            : choice == 1 || choice == 9 ? SOURCE_SITES_MISSING
            : choice == 5 || choice == 6 || choice == 10 ? SOURCE_SITES_INVALID
            : choice == 8 ? SOURCE_SITES_BINDINGS : SOURCE_SITES_CHANGED;
        assert(source_graph_sites(&old, &survivor, &map, NULL, 0, choice == 1 ? NULL : &prepared,
            choice == 10 ? NULL : &issue) == expected);
        assert(allocation_count == before);
        if (choice == 0) { assert(!issue.prepared && !issue.original_edge && !issue.binding.original); }
        if (choice == 7) { assert(issue.final_edge == &survivor_edge && !issue.prepared); }
        failure_at = 0; source_text_edits_destroy(&text_edits); source_clone_map_destroy(&map); finish();
    }
}
