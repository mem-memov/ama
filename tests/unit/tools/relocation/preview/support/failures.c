/** @file
 * @brief Exercise namespace collisions, physical aliases, required anchors and exhaustive allocation rollback.
 */
/** Physical absence of D cannot hide a logical collision in another root of the same family. */
static void it_rejects_collisions_aliases_and_orphan_plus_directories(void) {
    setup("collisions");
    package("left", "fixture"); package("left", "target");
    folder("right"); folder("right/packages"); folder("right/packages/test");
    folder("right/packages/test/target");
    file("left/packages/test/fixture/v1.0.0/src/value","@!\n");
    file("left/packages/test/target/v1.0.0/src/value","@!\n");
    ModuleRoot roots[] = {{.path=path("left"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("right"),.kind=MODULE_ROOT_PROJECT}};
    RelocationRequest request = {roots,2,path("left/packages/test/fixture/v1.0.0"),
        path("right/packages/test/target/v1.0.0"),NULL};
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_COLLISION);
    untouched(&request);
    reset_preview();
    roots[1] = roots[0];
    request.destination = path("left/final");
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_ALIAS);
    untouched(&request);
    finish();

    setup("anchors");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old+");
    file("project/packages/test/fixture/v1.0.0/src/old","unfinished definition\n"); file("project/packages/test/fixture/v1.0.0/src/old+/child","@!\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/packages/test/fixture/v1.0.0/src/old+"),path("project/packages/test/fixture/v1.0.0/src/new+"),NULL};
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_ORPHAN);
    untouched(&request);
    reset_preview();
    file("project/packages/test/fixture/v1.0.0/src/new","unfinished definition\n");
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module("/test/fixture/v1.0.0/src/old/child")->after_identity,"/test/fixture/v1.0.0/src/new/child") == 0);
    assert(strcmp(module("/test/fixture/v1.0.0/src/old")->after_identity,"/test/fixture/v1.0.0/src/old") == 0);
    assert(!module("/test/fixture/v1.0.0/src/old")->active && !module("/test/fixture/v1.0.0/src/new")->active);
    untouched(&request);
    finish();
}

/** Syntax errors in unused modules block preview; incomplete draft bodies retain their ordinary exclusion. */
static void it_rejects_invalid_unused_active_sources_but_accepts_draft_bodies(void) {
    setup("invalid-unused");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/value","@!\n"); file("project/packages/test/fixture/v1.0.0/src/unused","@!\n ?\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_INVALID_SOURCE);
    assert(preview.diagnostic.primary.span.source != NULL && !preview.ready);
    untouched(&request);
    reset_preview();
    file("project/packages/test/fixture/v1.0.0/src/unused","unfinished definition\n /test/fixture/v1.0.0/src/missing\n");
    assert(relocation_preview_build(&preview,&request) && !module("/test/fixture/v1.0.0/src/unused")->active);
    untouched(&request);
    finish();
}

/** Fail each actual measured production allocation; every failure preserves source paths and releases its complete prefix. */
static void it_releases_every_failed_preparation_prefix_without_touching_files(void) {
    setup("allocation-failures");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/value","@!\n"); file("project/packages/test/fixture/v1.0.0/src/draft","unfinished definition\n /test/fixture/v1.0.0/src/missing\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    allocation_count = 0;
    assert(relocation_preview_build(&preview,&request));
    size_t measured = allocation_count;
    assert(measured != 0);
    relocation_preview_destroy(&preview);
    assert(live_allocations == 0);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        allocation_count = 0;
        failure_at = ordinal;
        relocation_preview_init(&preview,sources.allocator);
        assert(!relocation_preview_build(&preview,&request) && !preview.ready);
        assert(preview.status == RELOCATION_OUT_OF_MEMORY);
        untouched(&request);
        relocation_preview_destroy(&preview);
        assert(live_allocations == 0);
    }
    failure_at = 0;
    relocation_preview_init(&preview,sources.allocator);
    finish();
}

/** An IO cause cannot be replaced by an allocation error while constructing its diagnostic. */
static void it_preserves_io_diagnostics_without_allocating_during_failure_reporting(void) {
    setup("io-diagnostic");
    const char **names = NULL;
    size_t count = 99;
    size_t before = allocation_count;
    failure_at = before+1;
    assert(!relocation_entries(&preview,path("missing"),&names,&count));
    assert(preview.status == RELOCATION_IO_ERROR && preview.diagnostic.code == DIAGNOSTIC_IO_FAILURE);
    assert(names == NULL && count == 99 && allocation_count == before);
    failure_at = 0;
    finish();
}
