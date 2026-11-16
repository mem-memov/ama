/** @file
 * @brief Check native/asset byte capture, literal symlinks and qualified constant rewrites without native execution.
 */
/** Locate an inventoried physical suffix independently from any module identity. */
static const RelocationStagedFile *captured(const RelocationStaging *staging, const char *suffix) {
    size_t length = strlen(suffix);
    for (const RelocationStagedFile *item = staging->files; item != NULL; item = item->next) {
        size_t total = strlen(item->before);
        if (total >= length && strcmp(item->before+total-length,suffix) == 0) { return item; }
    }
    return NULL;
}
/** Native C/header inputs are opaque semantic-presence fixtures, not executable native implementations. */
static void it_captures_native_assets_links_and_validates_qualified_constants(void) {
    setup("staged-native");
    folder("project"); folder("project/old"); folder("imports"); folder("standard");
    file("project/old/number","@C\n");
    file("project/old/number.c","/* .old.number remains opaque */\n");
    file("project/old/number.h","/* Opaque native header */\n");
    file("project/old/number.abi","+ initialize\n- destroy\n| literal\n= clone\n");
    file("project/old/names","@=\ndefault value\n.old.number\n |.old.names.literal bytes\nvalues[]\n.old.number\n |1\n |2\n");
    file("imports/unused","# .old.names.default value stays\n@!\nvalue\n.old.number\n .old.names.default value\nvalues[]\n.old.number\nvalues[]\n .old.names.values[]\n");
    const unsigned char asset[] = {0,255,1,13,10,0};
    bytes("project/old/asset.bin",asset,sizeof(asset));
    char link_target[180];
    memset(link_target,'x',sizeof(link_target)-1);
    link_target[sizeof(link_target)-1] = 0;
    assert(symlink(link_target,path("project/old/long.link")) == 0);
    assert(symlink("cycle.link",path("project/old/cycle.link")) == 0);
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("imports"),.kind=MODULE_ROOT_PROJECT},{.path=path("standard"),.kind=MODULE_ROOT_STANDARD}};
    RelocationRequest request = {roots,3,path("project/old"),path("standard/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references}));
    exact(prepared(&staging,".unused")->text,"# .old.names.default value stays\n@!\nvalue\n..new.number\n ..new.names.default value\nvalues[]\n..new.number\nvalues[]\n ..new.names.values[]\n");
    exact(prepared(&staging,".old.names")->text,"@=\ndefault value\n..new.number\n |.old.names.literal bytes\nvalues[]\n..new.number\n |1\n |2\n");
    const RelocationStagedFile *binary = captured(&staging,"/asset.bin");
    assert(binary != NULL && binary->replacement == binary->original);
    assert(source_length(binary->original) == sizeof(asset));
    assert(memcmp(source_bytes(binary->original),asset,sizeof(asset)) == 0);
    exact(captured(&staging,"/number.c")->replacement,"/* .old.number remains opaque */\n");
    exact(captured(&staging,"/long.link")->replacement,link_target);
    exact(captured(&staging,"/cycle.link")->replacement,"cycle.link");
    const ModuleSource *native = NULL;
    assert(module_catalog_find(&staging.project.catalog,"..new.number",&native) == MODULE_OK);
    assert(native->native_loaded && strstr(native->native_c,"/standard/new/number.c") != NULL);
    assert(strstr(source_path(native->native_abi),"/standard/new/number.abi") != NULL);
    struct stat metadata;
    assert(lstat(native->native_c,&metadata) != 0 && errno == ENOENT);
    untouched(&request);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    finish();
}
