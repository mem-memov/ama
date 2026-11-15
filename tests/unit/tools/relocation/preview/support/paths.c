/** @file
 * @brief Verify endpoint meaning, physical traversal and explicit rejection of aliases/merging.
 */
/** D denotes the final directory itself; missing traversal components cannot be normalized away. */
static void it_uses_exact_destination_and_rejects_unsafe_endpoints(void) {
    setup("paths");
    folder("project"); folder("project/old"); folder("project/oldish");
    file("project/old/value","@!\n");
    file("project/oldish/unchanged","@!\n");
    ModuleRoot root = {.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,path("project/old"),path("project/old/new"),NULL};
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_DESCENDANT);
    untouched(&request);
    reset_preview();
    file("project/existing","opaque");
    request.destination = path("project/existing");
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_DESTINATION_EXISTS);
    reset_preview();
    request.destination = path("project/missing/../new");
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_INVALID_PATH);
    reset_preview();
    request.destination = path("outside");
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_UNCOVERED);
    untouched(&request);
    reset_preview();
    assert(symlink("old",path("project/alias")) == 0);
    request.source = path("project/alias");
    request.destination = path("project/new");
    assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_UNSUPPORTED_PATH);
    assert(unlink(path("project/alias")) == 0);
    reset_preview();
    request.source = path("project/./old");
    request.destination = path("project/новый раздел");
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module(".old.value")->after_identity,".новый раздел.value") == 0);
    assert(strcmp(module(".oldish.unchanged")->after_identity,".oldish.unchanged") == 0);
    assert(strstr(module(".old.value")->after_path,"/новый раздел/value") != NULL);
    assert(strstr(module(".old.value")->after_path,"/новый раздел/old/") == NULL);
    untouched(&request);
    finish();
}

/** Relative roots/endpoints use an exactly grown cwd buffer, and preflight detects unavailable parent writes. */
static void it_resolves_long_working_directories_and_checks_required_parent_permissions(void) {
    setup("long-working-directory");
    char component[161];
    memset(component,'a',sizeof(component)-1);
    component[sizeof(component)-1] = 0;
    folder(component);
    const char *old = source_arena_join(&fixture_paths,component,"/","old");
    const char *value = source_arena_join(&fixture_paths,old,"/","value");
    assert(old != NULL && value != NULL);
    folder(old); file(value,"@!\n");
    char original[4096];
    assert(getcwd(original,sizeof(original)) != NULL);
    assert(chdir(path(component)) == 0);
    ModuleRoot root = {.path=".",.kind=MODULE_ROOT_PROJECT};
    RelocationRequest request = {&root,1,"old","new",NULL};
    assert(relocation_preview_build(&preview,&request));
    assert(strcmp(module(".old.value")->after_identity,".new.value") == 0);
    untouched(&request);
    assert(chdir(original) == 0);
    finish();

    setup("parent-permissions");
    folder("project"); folder("project/old"); folder("project/destination");
    file("project/old/value","@!\n");
    root = (ModuleRoot){.path=path("project"),.kind=MODULE_ROOT_PROJECT};
    request = (RelocationRequest){&root,1,path("project/old"),path("project/destination/new"),NULL};
    assert(chmod(path("project/destination"),0500) == 0);
    if (access(path("project/destination"),W_OK|X_OK) != 0) {
        assert(!relocation_preview_build(&preview,&request) && preview.status == RELOCATION_READ_ONLY);
        untouched(&request);
    }
    assert(chmod(path("project/destination"),0700) == 0);
    finish();
}
