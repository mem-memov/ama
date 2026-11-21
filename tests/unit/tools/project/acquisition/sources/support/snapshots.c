/** @file
 * @brief Check exact snapshot keys, healthy pending and commit-based version identity.
 * @see [T234 contract](../../../../../../../docs/implementation/acquisition-plan.md)
 */
/** Different module/test requests share one unresolved source and one later contract. */
static void it_coalesces_exact_snapshot_requests_and_waits_for_explicit_resolution(void) {
    begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
    ConfigurationDocument owner; scope(&owner, "origin", "tree", "main");
    AcquisitionSourceRequest first = request(&owner, "/a/p/v1.0.0/src/one");
    AcquisitionSourceRequest second = request(&owner, "/a/p/v1.0.0/tests/two");
    assert(first.resolve_snapshot && !second.resolve_snapshot && first.snapshot == second.snapshot);
    assert(plan.snapshot_count == 1 && !first.snapshot->commit);
    AcquisitionVersionSource sentinel = {0}; const AcquisitionVersionSource *bound = &sentinel;
    AcquisitionSourcesStatus status = acquisition_sources_bind(&plan, &second, &bound, &problem);
    assert(status == ACQUISITION_SOURCES_PENDING && bound == &sentinel && !plan.version_count && !first.snapshot->commit);
    resolve(&first, commit_a);
    size_t before = allocation_count; resolve(&second, commit_a); assert(allocation_count == before);
    status = acquisition_sources_bind(&plan, &first, &bound, &problem); assert(status == ACQUISITION_SOURCES_OK);
    const AcquisitionVersionSource *again = NULL;
    before = allocation_count;
    status = acquisition_sources_bind(&plan, &second, &again, &problem);
    assert(status == ACQUISITION_SOURCES_OK && again == bound && plan.version_count == 1 && allocation_count == before);
    assert(!strcmp(bound->version, "/a/p/v1.0.0") && !strcmp(bound->directory, "tree/v1.0.0"));
    assert(!strcmp(bound->snapshot->url, "origin") && !strcmp(bound->snapshot->commit, commit_a));
    /* A different logical version shares the snapshot but retains its own contract. */
    AcquisitionSourceRequest other = request(&owner, "/a/p/v2.0.0/src/one");
    status = acquisition_sources_bind(&plan, &other, &again, &problem);
    assert(status == ACQUISITION_SOURCES_OK && again != bound && plan.version_count == 2 && plan.snapshot_count == 1);
    before = allocation_count;
    status = acquisition_sources_bind(&plan, &first, &again, &problem);
    assert(status == ACQUISITION_SOURCES_OK && again == bound && allocation_count == before);
    acquisition_sources_destroy(&plan); configuration_document_destroy(&owner); finish();
}
/** Default branch is not guessed to be HEAD; equal observed commits may still join. */
static void it_distinguishes_default_revision_and_aliases_equal_resolved_commits(void) {
    begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
    ConfigurationDocument owners[3];
    scope(&owners[0], "origin", "tree", NULL); scope(&owners[1], "origin", "tree/./", "HEAD"); scope(&owners[2], "origin", "tree/empty/..", "main");
    const AcquisitionVersionSource *first = NULL;
    AcquisitionSourceRequest initial = request(&owners[0], "/a/p/v1.0.0/src/one");
    assert(initial.resolve_snapshot && !initial.snapshot->revision); resolve(&initial, commit_a);
    AcquisitionSourcesStatus status = acquisition_sources_bind(&plan, &initial, &first, &problem); assert(status == ACQUISITION_SOURCES_OK);
    for (size_t index = 1; index < 3; index++) {
        AcquisitionSourceRequest value = request(&owners[index], "/a/p/v1.0.0/src/two");
        assert(value.resolve_snapshot && value.snapshot != initial.snapshot && value.snapshot->revision);
        const AcquisitionVersionSource *bound = NULL;
        status = acquisition_sources_bind(&plan, &value, &bound, &problem);
        assert(status == ACQUISITION_SOURCES_PENDING && !bound && !value.snapshot->commit && plan.version_count == 1);
        resolve(&value, commit_a);
        status = acquisition_sources_bind(&plan, &value, &bound, &problem);
        assert(status == ACQUISITION_SOURCES_OK && bound == first);
    }
    AcquisitionSourceRequest repeated = request(&owners[0], "/a/p/v1.0.0/tests/again");
    assert(!repeated.resolve_snapshot && repeated.snapshot == initial.snapshot && !repeated.snapshot->revision);
    size_t before = allocation_count; resolve(&repeated, commit_a); assert(allocation_count == before);
    assert(plan.snapshot_count == 3 && plan.version_count == 1);
    acquisition_sources_destroy(&plan);
    for (size_t index = 0; index < 3; index++) { configuration_document_destroy(&owners[index]); }
    finish();
}
