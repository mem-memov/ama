/** @file
 * @brief Check independent final dependency preservation and exact source layout.
 */
/** Cycles and external importers keep their resolved targets while CRLF, comments and draft payloads stay exact. */
static void it_validates_final_cycles_and_preserves_all_nonpath_bytes(void) {
    setup("staged-cycles");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old"); package("imports", "imports");
    file("project/packages/test/fixture/v1.0.0/src/old/a","# .old.b comment\r\n@!\r\npeer&\r\n/test/fixture/v1.0.0/src/old/b");
    file("project/packages/test/fixture/v1.0.0/src/old/b","@!\npeer&\n/test/fixture/v1.0.0/src/old/a\n");
    file("project/packages/test/fixture/v1.0.0/src/old/draft","unfinished\n.old.a\n");
    file("imports/packages/test/imports/v1.0.0/src/unused","@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("imports"),.kind=MODULE_ROOT_PROJECT}};
    RelocationRequest request = {roots,2,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/longer name"),NULL};
    bool discovered = relocation_preview_build(&preview,&request);
    if (!discovered) { fprintf(stderr,"Preview %d: %s (%s)\n",preview.status,preview.diagnostic.message == NULL ? "" : preview.diagnostic.message,preview.path == NULL ? "" : preview.path); }
    assert(discovered);
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references}) && staging.ready);
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/a")->text,"# .old.b comment\r\n@!\r\npeer&\r\n/test/fixture/v1.0.0/src/longer name/b");
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/b")->text,"@!\npeer&\n/test/fixture/v1.0.0/src/longer name/a\n");
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/draft")->text,"unfinished\n.old.a\n");
    exact(prepared(&staging,"/test/imports/v1.0.0/src/unused")->text,"@!\nvalue\n/test/fixture/v1.0.0/src/longer name/a\n");
    assert(project_find(&staging.frontend.project,"/test/fixture/v1.0.0/src/longer name/a") != NULL);
    assert(staging.frontend.project.entry == NULL);
    assert(!relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references}) && staging.ready);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    request.source = path("project");
    request.destination = path("moved project");
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview) && references.edits == NULL);
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry="/test/fixture/v1.0.0/src/old/a"}));
    exact(prepared(&staging,"/test/fixture/v1.0.0/src/old/a")->text,"# .old.b comment\r\n@!\r\npeer&\r\n/test/fixture/v1.0.0/src/old/b");
    assert(strcmp(staging.entry,"/test/fixture/v1.0.0/src/old/a") == 0);
    assert(strcmp(preview.after_roots[0].path,preview.destination) == 0);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}
