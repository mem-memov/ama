/** @file
 * @brief Verify simultaneous identity mapping over cycles and importers outside the moved tree.
 */
/** A reference cycle requires no rewrite order, and unchanged identities yield no replacement bytes. */
static void it_maps_cycles_and_external_importers_without_textual_substitution(void) {
    setup("cycles");
    folder("project"); folder("project/old"); folder("imports");
    file("project/old/a","# .old.b remains comment\r\n@!\r\npeer&\r\n.old.b\r\n");
    file("project/old/b","@!\npeer&\n.old.a\n");
    file("imports/unused","@!\nfirst\n.old.a\n");
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("imports"),.kind=MODULE_ROOT_PROJECT}};
    RelocationRequest request = {roots,2,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview) && references.ready);
    size_t edges = 0, edits = 0;
    for (const RelocationReference *edge = references.first; edge != NULL; edge = edge->next) {
        assert(!edge->implicit && edge->owner != NULL && edge->target != NULL);
        edges++;
    }
    for (const RelocationPathEdit *edit = references.edits; edit != NULL; edit = edit->next) {
        assert(edit->original.end-edit->original.start == strlen(".old.a"));
        assert(memcmp(source_bytes(edit->original.source)+edit->original.start,".old.",5) == 0);
        assert(strncmp(edit->replacement,".new.",5) == 0);
        assert(edit->target == module(edit->replacement[5] == 'a' ? ".old.a" : ".old.b"));
        edits++;
    }
    assert(edges == 3 && edits == 3);
    untouched(&request);
    relocation_references_destroy(&references);
    reset_preview();
    request.source = path("project"); request.destination = path("moved project");
    assert(relocation_preview_build(&preview,&request));
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview) && references.edits == NULL);
    edges = 0;
    for (const RelocationReference *edge = references.first; edge != NULL; edge = edge->next) { edges++; }
    assert(edges == 3);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();
}
