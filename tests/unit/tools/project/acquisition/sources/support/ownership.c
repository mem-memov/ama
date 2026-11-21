/** @file
 * @brief Sweep every source-plan allocation refusal and retry without discarding prior metadata.
 * @see [T234 contract](../../../../../../../docs/implementation/acquisition-plan.md)
 */
/** Request, resolution and binding each publish only complete records after allocation. */
static void it_recovers_every_request_resolution_and_binding_allocation_refusal(void) {
    size_t measured = 0;
    for (size_t failure = 0; failure == 0 || failure <= measured; failure++) {
        begin(); acquisition_sources_init(&plan, sources.allocator, NULL);
        ConfigurationDocument owner; scope(&owner, "origin", "tree", "main");
        const ModulePathValue *path = identity("/a/p/v1.0.0/src/m");
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        AcquisitionSourceRequest value = {.resolve_snapshot = true};
        const AcquisitionVersionSource *bound = NULL;
        AcquisitionSourcesStatus status = acquisition_sources_request(&plan, &owner, path, &value, &problem);
        if (status == ACQUISITION_SOURCES_OK) { status = acquisition_snapshot_resolve(&plan, value.snapshot, commit_a, &problem); }
        if (status == ACQUISITION_SOURCES_OK) { status = acquisition_sources_bind(&plan, &value, &bound, &problem); }
        if (!failure) { assert(status == ACQUISITION_SOURCES_OK); measured = allocation_count - before; assert(measured == 9); }
        else {
            assert(status == ACQUISITION_SOURCES_OUT_OF_MEMORY && !bound && !plan.version_count && plan.snapshot_count <= 1);
            failure_at = 0;
            status = acquisition_sources_request(&plan, &owner, path, &value, &problem); assert(status == ACQUISITION_SOURCES_OK);
            resolve(&value, commit_a);
            status = acquisition_sources_bind(&plan, &value, &bound, &problem);
            assert(status == ACQUISITION_SOURCES_OK && plan.snapshot_count == 1 && plan.version_count == 1 && bound);
        }
        acquisition_sources_destroy(&plan); configuration_document_destroy(&owner); finish();
    }
}
