/** @file
 * @brief Verify newly covered sources join original analysis without namespace precedence.
 */
/** Added roots rewrite helper references, but an initial collision rejects even a unique final registry. */
static void it_analyzes_new_scope_and_rejects_initial_identity_conflicts(void) {
    setup("new-scope");
    folder("project"); folder("project/old"); folder("library");
    file("project/old/a","@!\n");
    file("project/caller","@!\nvalue\n.old.a\n");
    file("library/helper","@!\nvalue\n.old.a\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot added = {.path=path("library"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/old"),path("library/new"),&added};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    size_t edits = 0;
    bool helper = false, caller = false;
    for (const RelocationReference *edge = references.first; edge != NULL; edge = edge->next) {
        assert(edge->target == module(".old.a"));
        if (strcmp(edge->owner->after_identity,".helper") == 0) {
            helper = true;
            assert(edge->owner->before_identity == NULL);
        }
        if (strcmp(edge->owner->after_identity,".caller") == 0) { caller = true; }
    }
    for (const RelocationPathEdit *edit = references.edits; edit != NULL; edit = edit->next) {
        assert(strcmp(edit->replacement,".new.a") == 0);
        edits++;
    }
    assert(helper && caller && edits == 2);
    untouched(&request);
    relocation_references_destroy(&references);
    reset_preview();
    folder("library/old");
    file("library/old/a","@!\n");
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
    folder("project"); folder("selected"); folder("standard");
    file("selected/a","@!\npeer&\n..new.b\n");
    file("selected/b","@!\npeer&\n..new.a\n");
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
    assert(edges == 2 && references.edits == NULL && references.project.entry == NULL);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();

    setup("empty-domain");
    folder("project"); folder("project/old");
    file("project/old/draft","# Draft source without an active section.\n");
    root = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    assert(references.first == NULL && references.edits == NULL && references.project.entry == NULL);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();
}
