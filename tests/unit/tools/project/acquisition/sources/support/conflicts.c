/** @file
 * @brief Reject incompatible per-version origins and foreign snapshot publications exactly.
 * @see [T234 contract](../../../../../../../docs/implementation/acquisition-plan.md)
 */
/** URL bytes, resolved commit and physical directory are independent equality fields. */
static void it_refuses_all_unequal_source_contract_fields_without_replacing_the_prior_version(void) {
    for (int difference = 0; difference < 3; difference++) {
        begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
        ConfigurationDocument first_scope, other_scope;
        scope(&first_scope, "origin", "tree", "main");
        scope(&other_scope, difference == 0 ? "Origin" : "origin", difference == 2 ? "other" : "tree", difference == 1 ? "tag" : "main");
        AcquisitionSourceRequest first = request(&first_scope, "/a/p/v1.0.0/src/one"); resolve(&first, commit_a);
        const AcquisitionVersionSource *accepted = NULL;
        AcquisitionSourcesStatus status = acquisition_sources_bind(&plan, &first, &accepted, &problem); assert(status == ACQUISITION_SOURCES_OK);
        AcquisitionSourceRequest other = request(&other_scope, "/a/p/v1.0.0/src/two"); resolve(&other, difference == 1 ? commit_b : commit_a);
        const AcquisitionVersionSource *output = accepted; size_t before = allocation_count;
        status = acquisition_sources_bind(&plan, &other, &output, &problem);
        assert(status == ACQUISITION_SOURCES_CONFLICT && output == accepted && problem.previous == accepted);
        assert(plan.version_count == 1 && allocation_count == before && problem.issue.span.source == other.origin.source);
        assert(!strcmp(accepted->directory, "tree/v1.0.0") && !strcmp(accepted->snapshot->url, "origin") && !strcmp(accepted->snapshot->commit, commit_a));
        status = acquisition_sources_bind(&plan, &first, &output, &problem); assert(status == ACQUISITION_SOURCES_OK && output == accepted);
        acquisition_sources_destroy(&plan); configuration_document_destroy(&other_scope); configuration_document_destroy(&first_scope); finish();
    }
}
/** Unknown handles and unequal second commit assignments preserve the first source. */
static void it_rejects_foreign_commit_handles_and_preserves_single_assignment(void) {
    begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
    ConfigurationDocument owner; scope(&owner, "origin", ".", "main");
    AcquisitionSourceRequest value = request(&owner, "/a/p/v1.0.0/src/m");
    AcquisitionSnapshot foreign = {0};
    AcquisitionSourcesStatus status = acquisition_snapshot_resolve(&plan, &foreign, commit_a, &problem);
    assert(status == ACQUISITION_SOURCES_INVALID && !value.snapshot->commit);
    status = acquisition_snapshot_resolve(&plan, value.snapshot, "", &problem); assert(status == ACQUISITION_SOURCES_INVALID);
    resolve(&value, commit_a); const char *original = value.snapshot->commit; size_t before = allocation_count;
    status = acquisition_snapshot_resolve(&plan, value.snapshot, commit_b, &problem);
    assert(status == ACQUISITION_SOURCES_CONFLICT && value.snapshot->commit == original && allocation_count == before);
    acquisition_sources_destroy(&plan); configuration_document_destroy(&owner); finish();
}
/** Missing/invalid scope and a real foreign operation cannot publish local indexes. */
static void it_refuses_unmapped_and_foreign_requests_without_growing_indexes(void) {
    begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
    ConfigurationDocument empty, owner;
    ConfigurationStatus parsed = document(&empty, "", CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
    scope(&owner, "origin", ".", "main");
    const ModulePathValue *path = identity("/a/p/v1.0.0/src/m");
    AcquisitionSourceRequest unchanged = {.resolve_snapshot = true};
    size_t before = allocation_count;
    AcquisitionSourcesStatus status = acquisition_sources_request(&plan, &empty, path, &unchanged, &problem);
    assert(status == ACQUISITION_SOURCES_MISSING && unchanged.resolve_snapshot && !unchanged.snapshot && !unchanged.location);
    assert(!plan.snapshot_count && !plan.version_count && allocation_count == before && problem.issue.span.source == path->expression.source);
    status = acquisition_sources_request(&plan, NULL, path, &unchanged, &problem); assert(status == ACQUISITION_SOURCES_INVALID);
    const ModulePathValue *standard = identity("//integer");
    status = acquisition_sources_request(&plan, &owner, standard, &unchanged, &problem); assert(status == ACQUISITION_SOURCES_INVALID);
    AcquisitionSourceRequest own = request(&owner, path->text);
    AcquisitionSources foreign; acquisition_sources_init(&foreign, sources.allocator, NULL);
    AcquisitionSourceRequest other = {0};
    status = acquisition_sources_request(&foreign, &owner, path, &other, &problem); assert(status == ACQUISITION_SOURCES_OK);
    status = acquisition_snapshot_resolve(&foreign, other.snapshot, commit_a, &problem); assert(status == ACQUISITION_SOURCES_OK);
    AcquisitionVersionSource sentinel = {0}; const AcquisitionVersionSource *bound = &sentinel;
    status = acquisition_sources_bind(&plan, &other, &bound, &problem);
    assert(status == ACQUISITION_SOURCES_INVALID && bound == &sentinel && !plan.version_count && !own.snapshot->commit);
    assert(plan.snapshot_count == 1 && foreign.snapshot_count == 1 && !strcmp(other.snapshot->commit, commit_a));
    acquisition_sources_destroy(&foreign); acquisition_sources_destroy(&plan);
    configuration_document_destroy(&owner); configuration_document_destroy(&empty); finish();
}
