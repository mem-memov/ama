/** @file
 * @brief Verify original removal safety against exhaustive finite graph cuts and explicit inputs.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/removal/check.h"
static SourceArena report;
static DeliverySourceDefinition nodes[3];
static DeliverySourceDependency edges[9];
static const char *const names[] = {"/a/p/v1.0.0/src/a", "/a/p/v1.0.0/src/b", "/a/p/v1.0.0/tests/check"};
/** Own one immutable original occurrence source and three distinct source categories. */
static void begin(unsigned graph) {
    setup_test(); source_arena_init(&report, sources.allocator);
    const SourceText *text = NULL;
    SourceStatus status = source_manager_add(&sources, "original", (const unsigned char *)"012345678", 9, &text);
    assert(status == SOURCE_OK); memset(nodes, 0, sizeof(nodes)); memset(edges, 0, sizeof(edges));
    for (size_t owner = 0; owner < 3; owner++) {
        nodes[owner].identity = names[owner]; nodes[owner].kind = owner == 1 ? DELIVERY_SOURCE_PARAMETERIZED : DELIVERY_SOURCE_CONCRETE;
        nodes[owner].next = owner + 1 < 3 ? &nodes[owner + 1] : NULL;
        DeliverySourceDependency **tail = &nodes[owner].dependencies;
        for (size_t target = 0; target < 3; target++) {
            size_t index = owner * 3 + target;
            if (!(graph & (1u << index))) { continue; }
            edges[index] = (DeliverySourceDependency){.target = names[target], .origin = {text, index, index + 1}};
            *tail = &edges[index]; tail = &edges[index].next;
        }
    }
}
/** Retire report nodes before original names/spans; require zero live allocations. */
static void finish(void) { source_arena_destroy(&report); destroy_test(); }
/** Compare all 512 directed three-node graphs and all eight removal sets with
 * an independent row/column bit-mask cut, including self edges and every cycle. */
static void it_matches_every_three_vertex_graph_cut_without_runtime_or_kind_filters(void) {
    for (unsigned graph = 0; graph < 512; graph++) {
        for (unsigned removed = 0; removed < 8; removed++) {
            begin(graph); const char *selection[3]; size_t count = 0;
            for (size_t vertex = 0; vertex < 3; vertex++) { if (removed & (1u << vertex)) { selection[count++] = names[vertex]; } }
            unsigned columns = removed | (removed << 3) | (removed << 6);
            unsigned rows = (removed & 1 ? 0u : 7u) | (removed & 2 ? 0u : 56u) | (removed & 4 ? 0u : 448u);
            unsigned expected = graph & columns & rows, observed = 0;
            const SourceRemovalBlocker *blockers = NULL;
            SourceRemovalStatus status = source_removal_check(&report, nodes, NULL, selection, count, NULL, 0, &blockers);
            assert(status == (expected ? SOURCE_REMOVAL_BLOCKED : SOURCE_REMOVAL_OK));
            size_t previous = 0, seen = 0;
            for (const SourceRemovalBlocker *block = blockers; block; block = block->next) {
                assert(block->kind == SOURCE_REMOVAL_MODULE && block->origin.source);
                size_t index = block->origin.start;
                assert(index < 9 && (!seen || index > previous) && block->owner == &nodes[index / 3]);
                assert(!strcmp(block->target, names[index % 3]));
                observed |= 1u << index; previous = index; seen++;
            }
            assert(observed == expected); finish();
        }
    }
}
/** A shared declared asset blocks only surviving owners; target spelling and
 * occurrence are preserved rather than guessed from neighboring files. */
static void it_reports_shared_declared_inputs_and_all_surviving_consumers(void) {
    begin(1u << 3); const char *removed[] = {names[0]}; const char *paths[] = {"snapshot/data.bin"};
    SourceRemovalInputUse second = {.owner = &nodes[2], .path = paths[0]};
    SourceRemovalInputUse first = {.next = &second, .owner = &nodes[0], .path = paths[0]};
    const SourceRemovalBlocker *blockers = NULL;
    SourceRemovalStatus status = source_removal_check(&report, nodes, &first, removed, 1, paths, 1, &blockers);
    assert(status == SOURCE_REMOVAL_BLOCKED && blockers && blockers->owner == &nodes[1] && blockers->kind == SOURCE_REMOVAL_MODULE);
    assert(blockers->next && blockers->next->kind == SOURCE_REMOVAL_INPUT && blockers->next->owner == &nodes[2]);
    assert(!blockers->next->next && !strcmp(blockers->next->target, paths[0]));
    const char *all[] = {names[0], names[1], names[2]};
    status = source_removal_check(&report, nodes, &first, all, 3, paths, 1, &blockers);
    assert(status == SOURCE_REMOVAL_OK && !blockers); finish();
}
/** A same-identity replacement has no input to this original predicate and thus
 * cannot hide a surviving source edge; duplicate removal selections are harmless. */
static void it_preserves_original_blockers_and_handles_duplicate_or_draft_selections(void) {
    begin(1u << 3); const char *removed[] = {names[0], names[0], "/a/p/v1.0.0/src/draft"};
    const SourceRemovalBlocker *blockers = NULL;
    SourceRemovalStatus status = source_removal_check(&report, nodes, NULL, removed, 3, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_BLOCKED && blockers && !blockers->next && blockers->owner == &nodes[1]);
    const char *draft = removed[2];
    status = source_removal_check(&report, nodes, NULL, &draft, 1, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_OK && !blockers); finish();
}
/** Static and constants-only definitions are original consumers even though
 * neither is an executable root; their authored edges still protect targets. */
static void it_reports_non_executable_original_consumers_without_kind_filtering(void) {
    begin((1u << 3) | (1u << 6)); nodes[1].kind = DELIVERY_SOURCE_STATIC; nodes[2].kind = DELIVERY_SOURCE_CONSTANTS;
    const char *removed = names[0]; const SourceRemovalBlocker *blockers = NULL;
    SourceRemovalStatus status = source_removal_check(&report, nodes, NULL, &removed, 1, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_BLOCKED && blockers && blockers->owner == &nodes[1]);
    assert(blockers->next && blockers->next->owner == &nodes[2] && !blockers->next->next); finish();
}
/** Every report allocation can fail without publishing its partial prefix. */
static void it_retains_failed_report_scratch_without_publishing_at_every_allocation(void) {
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        begin((1u << 3) | (1u << 6)); const char *removed = names[0], *path = "snapshot/data.bin";
        SourceRemovalInputUse input = {.owner = &nodes[2], .path = path};
        SourceRemovalBlocker sentinel = {0}; const SourceRemovalBlocker *blockers = &sentinel;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceRemovalStatus status = source_removal_check(&report, nodes, &input, &removed, 1, &path, 1, &blockers);
        if (!failure) { assert(status == SOURCE_REMOVAL_BLOCKED); allocations = allocation_count - before; assert(allocations == 3); }
        else { assert(status == SOURCE_REMOVAL_OUT_OF_MEMORY && blockers == &sentinel); failure_at = 0; }
        finish();
    }
}
/** Foreign input owners and malformed exact sets cannot certify a safe deletion. */
static void it_refuses_invalid_sets_and_foreign_input_owners_without_publishing(void) {
    begin(0); SourceRemovalBlocker sentinel = {0}; const SourceRemovalBlocker *blockers = &sentinel;
    SourceRemovalStatus status = source_removal_check(&report, nodes, NULL, NULL, 1, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_INVALID && blockers == &sentinel);
    DeliverySourceDefinition foreign = {.identity = names[0]};
    SourceRemovalInputUse input = {.owner = &foreign, .path = "snapshot/data.bin"};
    status = source_removal_check(&report, nodes, &input, NULL, 0, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_INVALID && blockers == &sentinel);
    status = source_removal_check(&report, NULL, NULL, NULL, 0, NULL, 0, &blockers);
    assert(status == SOURCE_REMOVAL_OK && !blockers); finish();
}
/** Explicitly exercise finite cuts, input sharing, original-graph and refusal partitions. */
void run(void) {
    it_matches_every_three_vertex_graph_cut_without_runtime_or_kind_filters();
    it_reports_shared_declared_inputs_and_all_surviving_consumers();
    it_preserves_original_blockers_and_handles_duplicate_or_draft_selections();
    it_reports_non_executable_original_consumers_without_kind_filtering();
    it_retains_failed_report_scratch_without_publishing_at_every_allocation();
    it_refuses_invalid_sets_and_foreign_input_owners_without_publishing();
}
/** Finish after all original-source and result owners retire cleanly. */
int main(void) { run(); return 0; }
