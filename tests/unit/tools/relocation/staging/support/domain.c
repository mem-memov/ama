/** @file
 * @brief Check empty and newly covered final domains without an invented executable root.
 */
/** Proposed explicit names also serve newly covered sources' original analysis names; no path edits are fabricated. */
static void it_validates_newly_covered_sources_and_empty_domains(void) {
    setup("staged-new-domain");
    folder("project"); folder("selected"); folder("standard");
    file("selected/a","@!\npeer&\n..new.b\n");
    file("selected/b","@!\npeer&\n..new.a\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot added = {.path=path("standard"),.kind=MODULE_ROOT_STANDARD};
    RelocationRequest request = {&root,1,path("selected"),path("standard/new"),&added};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview) && references.edits == NULL);
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    RelocationStagingRequest settings = {.references=&references,.entry="..new.a"};
    assert(relocation_staging_build(&staging,&settings));
    assert(strcmp(staging.entry,"..new.a") == 0);
    assert(project_find(&staging.frontend.project,"..new.a") != NULL);
    for (const RelocationStagedSource *source = staging.first; source != NULL; source = source->next) {
        assert(source->module->before_identity == NULL && source->edit_count == 0);
    }
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();

    setup("staged-empty-domain");
    folder("project"); folder("project/old");
    root = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    relocation_staging_init(&staging,sources.allocator);
    settings = (RelocationStagingRequest){.references=&references};
    assert(relocation_staging_build(&staging,&settings));
    assert(staging.first == NULL && staging.frontend.project.first == NULL && staging.ready);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}
