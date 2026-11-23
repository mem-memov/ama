/** @file
 * @brief Check complete observation freshness independently of graph reachability.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/observation/freshness.h"
#include "../../../../../../tools/project/source/transaction/guard/guard.h"
/** Reordered rows/different text owners and unused timestamps preserve facts;
 * root, membership, precision, physical identity or byte changes do not. */
static void it_compares_exact_original_observation_facts_without_allocation_or_timestamp_guesses(void) {
    ImageAssemblyFixture fixture; image_assembly_setup(&fixture, "/image/from", "/image/copied", false);
    SourceCatalogImageEntry rows[6];
    for (size_t i = 0; i < 5; i++) { rows[i] = fixture.entries[4 - i]; }
    rows[3].content = material_text("/image/from", (const unsigned char *)"from", 4);
    rows[3].metadata.st_mtime++;
    ModuleRoot root = fixture.root; SourceCatalogImage fresh; SourceImageIssue shape;
    assert(source_catalog_image_prepare(&fresh, rows, 5, &root, 1, (SourceImageBounds){0}, &shape) == SOURCE_IMAGE_OK);
    SourceFreshnessIssue issue; size_t calls = allocation_count;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_OK);
    assert(allocation_count == calls && !issue.path && !issue.original && !issue.fresh);
    const SourceText *saved = rows[3].content;
    rows[3].content = material_text("/image/from", (const unsigned char *)"xxxx", 4);
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_CONTENT);
    assert(!strcmp(issue.path, "/image/from") && issue.original && issue.fresh);
    rows[3].content = saved; rows[3].metadata.st_ino += 100;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_IDENTITY);
    rows[3].metadata.st_ino -= 100; rows[3].content = NULL;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_OBSERVATION);
    rows[3].content = saved; rows[4].children_complete = false;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_OBSERVATION);
    rows[4].children_complete = true; root.kind = MODULE_ROOT_STANDARD;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_ROOTS);
    root.kind = MODULE_ROOT_PROJECT; const char *excluded[] = {"/image/elsewhere"}; root.excluded = excluded; root.excluded_count = 1;
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_ROOTS);
    root.excluded = NULL; root.excluded_count = 0;
    rows[5] = (SourceCatalogImageEntry){.path = "/image/ignored.bin"};
    assert(source_catalog_image_prepare(&fresh, rows, 6, &root, 1, (SourceImageBounds){0}, &shape) == SOURCE_IMAGE_OK);
    assert(source_observation_freshness(&fixture.original, &fresh, &issue) == SOURCE_FRESH_MEMBERSHIP);
    assert(!issue.original && issue.fresh == &rows[5]);
    assert(source_observation_freshness(&fresh, &fixture.original, &issue) == SOURCE_FRESH_MEMBERSHIP);
    assert(issue.original == &rows[5] && !issue.fresh);
    SourceCatalogImage empty = {0};
    assert(source_catalog_image_prepare(&empty, NULL, 0, NULL, 0, (SourceImageBounds){0}, &shape) == SOURCE_IMAGE_OK);
    assert(source_observation_freshness(&empty, &empty, &issue) == SOURCE_FRESH_OK);
    assert(source_observation_freshness(NULL, &empty, &issue) == SOURCE_FRESH_INVALID);
    image_assembly_destroy(&fixture);
}
/** New ignored names leave active source admission unchanged but invalidate the
 * prepared boundary; explicitly requested opaque byte/inode changes also refuse. */
static void it_revalidates_real_ignored_membership_opaque_bytes_and_live_inode_replacement(void) {
    setup_test(); fixtures(); file("generated/project/packages/vendor/pkg/v1.0.0/src/payload.blob", "before");
    capture_fixtures(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    const char *payload = source_arena_join(&arena, capture.roots[0].path, "/", "packages/vendor/pkg/v1.0.0/src/payload.blob"); assert(payload);
    SourceObservationRequest request = {payload, SOURCE_OBSERVE_CONTENT, false};
    SourceObservedImage baseline; source_observed_image_init(&baseline, sources.allocator);
    assert(source_observed_image_build(&baseline, &capture, (SourceImageBounds){0}, &request, 1) == SOURCE_OBSERVATION_OK);
    size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceTransactionGuard guard; source_transaction_guard_init(&guard, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceGuardStatus status = source_transaction_guard_acquire(&guard, &baseline, (SourceLeaseControl){0});
        if (!failure) { assert(status == SOURCE_GUARD_OK); total = allocation_count - before; assert(total); }
        else { assert(status != SOURCE_GUARD_OK && !guard.valid); }
        failure_at = 0; assert(source_transaction_guard_destroy(&guard) == 0 && live_allocations == live);
    }
    const SourceFreshnessStatus expected[] = {SOURCE_FRESH_OK, SOURCE_FRESH_MEMBERSHIP, SOURCE_FRESH_CONTENT, SOURCE_FRESH_IDENTITY};
    for (unsigned profile = 0; profile < 4; profile++) {
        if (profile == 1) { file("generated/project/packages/vendor/pkg/v1.0.0/src/extra.blob", "ignored"); }
        if (profile == 2) { file(payload, "after!"); }
        if (profile == 3) { assert(rename(payload, "generated/saved-payload.blob") == 0); file(payload, "before"); }
        SourceTransactionGuard guard; source_transaction_guard_init(&guard, sources.allocator);
        SourceGuardStatus status = source_transaction_guard_acquire(&guard, &baseline, (SourceLeaseControl){0});
        assert(status == (profile ? SOURCE_GUARD_STALE : SOURCE_GUARD_OK));
        assert(guard.freshness_status == expected[profile] && guard.lease.acquired);
        assert(guard.valid == (profile == 0));
        assert(source_transaction_guard_destroy(&guard) == 0);
        if (profile == 1) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/extra.blob") == 0); }
        if (profile == 2) { file(payload, "before"); }
        if (profile == 3) { assert(unlink(payload) == 0 && rename("generated/saved-payload.blob", payload) == 0); }
    }
    const char *new_paths[] = {"generated/project/packages/vendor/pkg/v1.0.0/src/new importer", "generated/project/packages/vendor/pkg/v1.0.0/src/main.c"};
    for (size_t i = 0; i < 2; i++) {
        file(new_paths[i], i ? "opaque companion" : "@!\n");
        SourceTransactionGuard guard; source_transaction_guard_init(&guard, sources.allocator);
        assert(source_transaction_guard_acquire(&guard, &baseline, (SourceLeaseControl){0}) == SOURCE_GUARD_OBSERVATION && !guard.valid && guard.lease.acquired);
        assert(source_transaction_guard_destroy(&guard) == 0 && unlink(new_paths[i]) == 0);
    }
    source_observed_image_destroy(&baseline); assert(unlink(payload) == 0);
    source_arena_destroy(&arena); finish();
}
