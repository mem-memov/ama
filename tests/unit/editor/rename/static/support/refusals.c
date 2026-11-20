/** @file
 * @brief Exhaust every measured candidate-allocation refusal for captured static formal renaming.
 * @see [T192 contract](../../../../../../docs/implementation/static-editor-bindings.md)
 */
/** Refusing each actual allocation must preserve original bytes and retire every candidate prefix. */
static void it_unwinds_every_static_candidate_correspondence_allocation(void) {
    fixture();
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/box",".slot",".renamed slot"));
    size_t before = allocation_count;
    assert(validate());
    size_t total = allocation_count-before;
    assert(total > 0);
    destroy_rename();
    for (size_t refusal = 1; refusal <= total; refusal++) {
        setup_rename();
        assert(prepare("/test/fixture/v1.0.0/src/box",".slot",".renamed slot"));
        failure_at = allocation_count+refusal;
        bool valid = editor_rename_validate(&rename_plan);
        if (valid || rename_plan.diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY) {
            fprintf(stderr,"Static candidate allocation %zu/%zu: valid=%u code=%u message=%s\n",refusal,total,
                (unsigned)valid,(unsigned)rename_plan.diagnostic.code,
                rename_plan.diagnostic.message ? rename_plan.diagnostic.message : "none");
        }
        assert(!valid && !rename_plan.validated && rename_plan.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        failure_at = 0;
        const ModuleSource *box = NULL;
        assert(module_catalog_find(&analysis.frontend.project.catalog,"/test/fixture/v1.0.0/src/box",&box) == MODULE_OK);
        assert(strstr((const char *)source_bytes(box->text),".slot") != NULL);
        assert(strstr((const char *)source_bytes(box->text),".renamed slot") == NULL);
        destroy_rename();
    }
    fprintf(stderr,"Static candidate checked %zu allocation refusal points\n",total);
}
