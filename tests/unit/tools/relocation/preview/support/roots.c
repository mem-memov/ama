/** @file
 * @brief Check carried namespace roots, explicit destination families and roots below the new directory.
 */
/** A container moves both root locations while preserving family, exclusions and full logical identity. */
static void it_projects_carried_roots_and_explicit_destination_root_kinds(void) {
    setup("carried-roots");
    folder("bundle"); folder("bundle/project"); folder("bundle/library"); folder("bundle/project/ignored");
    file("bundle/project/value","@!\n"); file("bundle/library/value","@!\n");
    file("bundle/project/ignored/invalid","@!\n ?\n");
    const char *excluded[] = {"ignored"};
    ModuleRoot roots[] = {{path("bundle/project"),excluded,1,MODULE_ROOT_PROJECT},
        {.path=path("bundle/library"),.kind=MODULE_ROOT_STANDARD}};
    RelocationRequest request = {roots,2,path("bundle"),path("relocated"),NULL};
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module(".value")->after_identity,".value") == 0);
    assert(strcmp(module("..value")->after_identity,"..value") == 0);
    assert(preview.after_root_count == 2 && strstr(preview.after_roots[0].path,"/relocated/project") != NULL);
    assert(preview.after_roots[0].kind == MODULE_ROOT_PROJECT && preview.after_roots[1].kind == MODULE_ROOT_STANDARD);
    roots[0].kind = MODULE_ROOT_STANDARD;
    excluded[0] = "changed";
    assert(preview.before_roots[0].kind == MODULE_ROOT_PROJECT);
    assert(strcmp(preview.after_roots[0].excluded[0],"ignored") == 0);
    untouched(&request);
    finish();

    setup("explicit-root");
    folder("project"); folder("project/old");
    file("project/old/значение","@!\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot destination = {.path=path("new root"),.kind=MODULE_ROOT_STANDARD};
    request = (RelocationRequest){&root,1,path("project/old"),path("new root"),&destination};
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module(".old.значение")->after_identity,"..значение") == 0);
    assert(preview.after_roots[1].kind == MODULE_ROOT_STANDARD);
    untouched(&request);
    finish();
}

/** Existing roots can change a moved module's family; an added root contributes all its pre-existing files to scope. */
static void it_moves_between_existing_root_kinds_and_includes_destination_sources(void) {
    setup("cross-kind");
    folder("project"); folder("project/old"); folder("standard");
    file("project/old/value","@!\n"); file("standard/existing","@!\n");
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("standard"),.kind=MODULE_ROOT_STANDARD}};
    RelocationRequest request = {roots,2,path("project/old"),path("standard/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module(".old.value")->after_identity,"..new.value") == 0);
    assert(strcmp(module("..existing")->after_identity,"..existing") == 0);
    untouched(&request);
    reset_preview();
    ModuleRoot duplicate = {.path=roots[0].path,.kind=MODULE_ROOT_STANDARD};
    request.destination_root = &duplicate;
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_ALIAS);
    untouched(&request);
    finish();

    setup("added-existing-root");
    folder("project"); folder("project/old"); folder("standard");
    file("project/old/value","@!\n"); file("standard/helper","@!\nvalue\n.old.value\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    ModuleRoot destination = {.path=path("standard"),.kind=MODULE_ROOT_STANDARD};
    request = (RelocationRequest){&root,1,path("project/old"),path("standard/new"),&destination};
    assert(relocation_preview_build(&preview,&request));
    bool found = false;
    for (const RelocationModule *item = preview.modules; item != NULL; item = item->next) {
        if (strcmp(item->after_identity,"..helper") == 0) { found = true; assert(item->active); }
    }
    assert(found);
    assert(strcmp(module(".old.value")->after_identity,"..new.value") == 0);
    untouched(&request);
    finish();
}
