/** @file
 * @brief Retain original binding diagnostics and release every failed analysis prefix.
 */
/** Unresolved original types cannot be repaired by guessing their proposed final spelling. */
static void it_rejects_unresolved_original_bindings_and_conflicting_spans(void) {
    setup("unresolved");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/a","@!\npeer&\n/test/fixture/v1.0.0/src/missing\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(!relocation_references_build(&references,&preview) && !references.ready);
    assert(references.status == RELOCATION_INVALID_REFERENCE && references.error.code == DIAGNOSTIC_MISSING_MODULE);
    untouched(&request);
    relocation_references_destroy(&references);
    const SourceText *text = NULL;
    assert(source_manager_add(&sources,"overlap",(const unsigned char *)"abcdef ghijkl",13,&text) == SOURCE_OK);
    RelocationModule first = {.after_identity="/test/fixture/v1.0.0/src/new/a"}, second = {.after_identity="/test/fixture/v1.0.0/src/new/b"};
    relocation_references_init(&references,sources.allocator);
    SourceSpan exact = {text,0,6};
    assert(relocation_reference_edit(&references,exact,&first));
    assert(relocation_reference_edit(&references,exact,&first) && references.edits->next == NULL);
    assert(!relocation_reference_edit(&references,exact,&second));
    assert(references.status == RELOCATION_REFERENCE_CONFLICT && references.error.related_count == 1);
    relocation_references_destroy(&references);
    relocation_references_init(&references,sources.allocator);
    assert(relocation_reference_edit(&references,exact,&first));
    assert(!relocation_reference_edit(&references,(SourceSpan){text,5,13},&second));
    assert(references.status == RELOCATION_REFERENCE_CONFLICT);
    relocation_references_destroy(&references);
    finish();
}

/** All counted graph/ABI/parser/copy allocations can fail without touching original files or leaking a failed owner. */
static void it_releases_every_failed_reference_analysis_prefix(void) {
    setup("reference-allocation-failures");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/a","@!\npeer&\n/test/fixture/v1.0.0/src/old/b\n");
    file("project/packages/test/fixture/v1.0.0/src/old/b","@!\npeer&\n/test/fixture/v1.0.0/src/old/a\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    size_t preview_allocations = live_allocations;
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    allocation_count = 0;
    assert(relocation_references_build(&references,&preview));
    size_t measured = allocation_count;
    relocation_references_destroy(&references);
    assert(live_allocations == preview_allocations);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        allocation_count = 0;
        failure_at = ordinal;
        relocation_references_init(&references,sources.allocator);
        assert(!relocation_references_build(&references,&preview) && !references.ready);
        assert(references.status == RELOCATION_OUT_OF_MEMORY);
        relocation_references_destroy(&references);
        assert(live_allocations == preview_allocations && preview.ready);
        untouched(&request);
    }
    failure_at = 0;
    finish();
}
