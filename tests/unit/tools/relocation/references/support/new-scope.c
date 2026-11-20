/** @file
 * @brief Verify newly covered sources join original analysis without namespace precedence.
 */
/** Added roots rewrite helper references, but an initial collision rejects even a unique final registry. */
static void it_analyzes_new_scope_and_rejects_initial_identity_conflicts(void) {
    setup("new-scope");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old"); package("library", "library");
    file("project/packages/test/fixture/v1.0.0/src/old/a","@!\n");
    file("project/packages/test/fixture/v1.0.0/src/caller","@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
    file("library/packages/test/library/v1.0.0/src/helper","@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot added = {.path=path("library"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("library/packages/test/library/v1.0.0/src/new"),&added};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    size_t edits = 0;
    bool helper = false, caller = false;
    for (const RelocationReference *edge = references.first; edge != NULL; edge = edge->next) {
        assert(edge->target == module("/test/fixture/v1.0.0/src/old/a"));
        if (strcmp(edge->owner->after_identity,"/test/library/v1.0.0/src/helper") == 0) {
            helper = true;
            assert(edge->owner->before_identity == NULL);
        }
        if (strcmp(edge->owner->after_identity,"/test/fixture/v1.0.0/src/caller") == 0) { caller = true; }
    }
    for (const RelocationPathEdit *edit = references.edits; edit != NULL; edit = edit->next) {
        assert(strcmp(edit->replacement,"/test/library/v1.0.0/src/new/a") == 0);
        edits++;
    }
    assert(helper && caller && edits == 2);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();

    setup("initial-version-conflict");
    package("project", "fixture"); package("library", "fixture");
    folder("library/packages/test/new");
    file("project/packages/test/fixture/v1.0.0/src/a", "@!\n");
    file("library/packages/test/fixture/v1.0.0/src/a", "@!\n");
    root = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    added = (ModuleRoot){.path=path("library"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/packages/test/fixture/v1.0.0"),
        path("library/packages/test/new/v1.0.0"),&added};
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(!relocation_references_build(&references,&preview));
    assert(!references.ready && references.status == RELOCATION_INVALID_REFERENCE);
    assert(references.error.code == DIAGNOSTIC_AMBIGUOUS_MODULE);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();
}

/** Newly covered selected-tree files use explicit proposed names and can form a complete entry-free domain. */
static void it_analyzes_newly_covered_selected_sources_and_empty_domains(void) {
    setup("selected-new-scope");
    package("project", "fixture"); folder("selected"); folder("standard");
    file("selected/a","@!\npeer&\n//new/b\n");
    file("selected/b","@!\npeer&\n//new/a\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot added = {.path=path("standard"),.kind=MODULE_ROOT_STANDARD};
    RelocationRequest request = {&root,1,path("selected"),path("standard/new"),&added};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    size_t edges = 0;
    for (const RelocationReference *edge = references.first; edge != NULL; edge = edge->next) {
        assert(edge->owner->before_identity == NULL && edge->target->before_identity == NULL);
        edges++;
    }
    assert(edges == 2 && references.edits == NULL && references.frontend.project.entry == NULL);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();

    setup("empty-domain");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old");
    file("project/packages/test/fixture/v1.0.0/src/old/draft","# Draft source without an active section.\n");
    root = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/packages/test/fixture/v1.0.0/src/old"),path("project/packages/test/fixture/v1.0.0/src/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    assert(references.first == NULL && references.edits == NULL && references.frontend.project.entry == NULL);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();
}
