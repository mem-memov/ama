/** @file
 * @brief Compare client root dispositions against exhaustive finite removal choices and original graph safety.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/removal/roots.h"
/** All 27 independent three-root dispositions and eight cuts follow an independent bit-set oracle. */
static void it_checks_every_three_root_disposition_and_removal_set(void) {
    for (unsigned choices = 0; choices < 27; choices++) {
        for (unsigned cut = 0; cut < 8; cut++) {
            begin(0); SourceClientRoot roots[3]; unsigned remaining = choices, expected = 0;
            const char *removed[3]; size_t count = 0;
            for (size_t index = 0; index < 3; index++) {
                unsigned choice = remaining % 3; remaining /= 3;
                roots[index] = (SourceClientRoot){.next = index < 2 ? &roots[index + 1] : NULL,
                    .definition = &nodes[index], .kind = index == 1 ? SOURCE_CLIENT_EXPORT : SOURCE_CLIENT_ENTRY,
                    .action = (SourceClientRootAction)choice, .destination = choice == 2 ? names[(index + 1) % 3] : NULL};
                if (cut & (1u << index)) {
                    removed[count++] = names[index];
                    if (!choice) { expected |= 1u << index; }
                }
            }
            const SourceClientRootBlocker *blockers = NULL;
            SourceRemovalStatus status = source_removal_client_roots(&report, nodes, roots, removed, count, &blockers);
            assert(status == (expected ? SOURCE_REMOVAL_BLOCKED : SOURCE_REMOVAL_OK));
            unsigned observed = 0; size_t previous = 0, seen = 0;
            for (const SourceClientRootBlocker *block = blockers; block; block = block->next) {
                size_t index = (size_t)(block->root - roots);
                assert(index < 3 && (!seen || previous < index));
                observed |= 1u << index; previous = index; seen++;
            }
            assert(observed == expected); finish();
        }
    }
}
/** Explicit client mapping releases that root but never authorizes an outside authored reference. */
static void it_preserves_authored_blockers_after_explicit_client_root_mapping(void) {
    begin(1u << 3); const char *removed = names[0];
    SourceClientRoot root = {.definition = &nodes[0], .kind = SOURCE_CLIENT_ENTRY,
        .action = SOURCE_CLIENT_MAP, .destination = "/a/p/v1.0.0/src/new main"};
    const SourceClientRootBlocker *roots = NULL;
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &roots) == SOURCE_REMOVAL_OK && !roots);
    const SourceRemovalBlocker *edges = NULL;
    assert(source_removal_check(&report, nodes, NULL, &removed, 1, NULL, 0, &edges) == SOURCE_REMOVAL_BLOCKED);
    assert(edges && edges->owner == &nodes[1] && !edges->next); finish();
}
/** Duplicate entry/export occurrences remain distinct; each failed report allocation preserves output. */
static void it_preserves_duplicate_client_roots_and_retires_all_report_allocation_failures(void) {
    for (size_t failure = 0; failure <= 2; failure++) {
        begin(0); const char *removed[] = {names[0], names[0]};
        SourceClientRoot second = {.definition = &nodes[0], .kind = SOURCE_CLIENT_EXPORT};
        SourceClientRoot first = {.next = &second, .definition = &nodes[0], .kind = SOURCE_CLIENT_ENTRY};
        SourceClientRootBlocker sentinel = {0}; const SourceClientRootBlocker *blocks = &sentinel;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceRemovalStatus status = source_removal_client_roots(&report, nodes, &first, removed, 2, &blocks);
        if (!failure) {
            assert(status == SOURCE_REMOVAL_BLOCKED && allocation_count - before == 2);
            assert(blocks->root == &first && blocks->next->root == &second && !blocks->next->next);
        } else { assert(status == SOURCE_REMOVAL_OUT_OF_MEMORY && blocks == &sentinel); }
        failure_at = 0; finish();
    }
}
/** Foreign handles, invalid action/kind/target combinations and incomplete sets never publish reports. */
static void it_refuses_malformed_client_roots_and_preserves_output_on_late_invalidity(void) {
    begin(0); const char *removed = names[0];
    SourceClientRootBlocker sentinel = {0}; const SourceClientRootBlocker *blocks = &sentinel;
    SourceClientRoot root = {.definition = &nodes[0]};
    assert(source_removal_client_roots(NULL, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, NULL) == SOURCE_REMOVAL_INVALID);
    assert(source_removal_client_roots(&report, nodes, &root, NULL, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    const char *bad[] = {NULL, ""};
    for (size_t index = 0; index < 2; index++) {
        assert(source_removal_client_roots(&report, nodes, &root, &bad[index], 1, &blocks) == SOURCE_REMOVAL_INVALID);
    }
    DeliverySourceDefinition foreign = nodes[0]; root.definition = &foreign;
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.definition = &nodes[0]; root.kind = (SourceClientRootKind)-1;
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.kind = (SourceClientRootKind)(SOURCE_CLIENT_EXPORT + 1);
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.kind = SOURCE_CLIENT_ENTRY; root.action = (SourceClientRootAction)-1;
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.action = (SourceClientRootAction)(SOURCE_CLIENT_MAP + 1);
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.action = SOURCE_CLIENT_MAP;
    const char *targets[] = {NULL, "", "./relative", "/a/p/v1.0.0"};
    for (size_t index = 0; index < 4; index++) {
        root.destination = targets[index];
        assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    }
    root.action = SOURCE_CLIENT_KEEP; root.destination = names[1];
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    root.action = SOURCE_CLIENT_DROP;
    assert(source_removal_client_roots(&report, nodes, &root, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    SourceClientRoot first = {.next = &root, .definition = &nodes[0]};
    assert(source_removal_client_roots(&report, nodes, &first, &removed, 1, &blocks) == SOURCE_REMOVAL_INVALID);
    assert(blocks == &sentinel);
    assert(source_removal_client_roots(&report, NULL, NULL, NULL, 0, &blocks) == SOURCE_REMOVAL_OK && !blocks);
    finish();
}
