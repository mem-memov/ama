/** @file
 * @brief Check complete source scope and byte-preserving read-only physical inventory.
 */
/** External unused importers and drafts are included; companions/assets are inventoried without parsing. */
static void it_discovers_all_sources_and_preserves_opaque_accompanying_bytes(void) {
    setup("scope");
    folder("project"); folder("project/old"); folder("imports");
    file("project/old/value","@!\n");
    file("project/old/draft","unfinished definition\n .missing\n");
    file("imports/unused","@!\nvalue\n.old.value\n");
    const unsigned char opaque[] = {0,255,13,9};
    bytes("project/old/value.c",opaque,sizeof(opaque));
    bytes("project/old/image.png",opaque,sizeof(opaque));
    assert(symlink(".",path("project/old/loop.dat")) == 0);
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("imports"),.kind=MODULE_ROOT_PROJECT}};
    RelocationRequest request = {roots,2,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request) && preview.ready);
    const RelocationModule *value = module(".old.value"), *draft = module(".old.draft"), *unused = module(".unused");
    assert(value != NULL && value->active && strcmp(value->after_identity,".new.value") == 0);
    assert(draft != NULL && !draft->active && strcmp(draft->after_identity,".new.draft") == 0);
    assert(unused != NULL && unused->active && strcmp(unused->after_identity,".unused") == 0);
    assert(strcmp(value->after_path+strlen(value->after_path)-strlen("/new/value"),"/new/value") == 0);
    size_t entries = 0;
    for (const RelocationEntry *entry = preview.entries; entry != NULL; entry = entry->next) { entries++; }
    assert(entries == 6);
    assert(source_length(draft->text) == strlen("unfinished definition\n .missing\n"));
    assert(memcmp(source_bytes(draft->text),"unfinished definition\n .missing\n",source_length(draft->text)) == 0);
    unsigned char observed[sizeof(opaque)];
    FILE *stream = fopen(path("project/old/value.c"),"rb");
    assert(stream != NULL && fread(observed,1,sizeof(observed),stream) == sizeof(observed));
    assert(fclose(stream) == 0 && memcmp(observed,opaque,sizeof(opaque)) == 0);
    untouched(&request);
    finish();
}

/** Maintenance does not require an executable entry or silently create a synthetic default module. */
static void it_previews_constants_only_and_empty_source_domains_without_an_entry(void) {
    setup("constants-only");
    folder("project"); folder("project/old");
    file("project/old/value","@C\n");
    file("project/old/constants","@=\nflag\n.old.value\n |1\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/old"),path("project/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    assert(module(".old.constants")->active && module(".old.value")->active);
    untouched(&request);
    finish();

    setup("empty-domain");
    folder("container"); folder("container/root");
    root = (ModuleRoot){.path=path("container/root"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("container"),path("new container"),NULL};
    assert(relocation_preview_build(&preview,&request) && preview.modules == NULL);
    assert(preview.after_root_count == 1);
    untouched(&request);
    finish();
}
