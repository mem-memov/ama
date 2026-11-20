/** @file
 * @brief Verify that qualified constant module prefixes exclude member names, storage suffixes and literal/comment bytes.
 */
/** Native companions are opaque presence/ABI fixtures; analysis never compiles or executes their C body. */
static void constant_value_fixture(void) {
    file("project/packages/test/fixture/v1.0.0/src/old/number","@C\n");
    file("project/packages/test/fixture/v1.0.0/src/old/number.h","/* Opaque native payload declaration. */\n");
    file("project/packages/test/fixture/v1.0.0/src/old/number.c","/* Opaque .old.names.default value text. */\n");
    file("project/packages/test/fixture/v1.0.0/src/old/number.abi","+ initialize\n- destroy\n| literal\n= clone\n");
}
/** Prefix edits stop before the final slash/member, leaving every non-path byte available for exact staged preservation. */
static void it_preserves_constant_members_suffixes_comments_and_literal_bytes(void) {
    setup("qualified-constants");
    package("project", "fixture"); folder("project/packages/test/fixture/v1.0.0/src/old"); package("imports", "imports"); folder("standard");
    constant_value_fixture();
    file("project/packages/test/fixture/v1.0.0/src/old/names","@=\ndefault value\n/test/fixture/v1.0.0/src/old/number\n |.old.names.literal bytes\nvalues[]\n/test/fixture/v1.0.0/src/old/number\n |1\n |2\n");
    file("imports/packages/test/imports/v1.0.0/src/unused","# .old.names.default value is a comment\n@!\nvalue\n/test/fixture/v1.0.0/src/old/number\n /test/fixture/v1.0.0/src/old/names/default value\nvalues[]\n/test/fixture/v1.0.0/src/old/number\nvalues[]\n /test/fixture/v1.0.0/src/old/names/values[]\n");
    ModuleRoot roots[] = {{.path=path("project"),.kind=MODULE_ROOT_PROJECT},
        {.path=path("imports"),.kind=MODULE_ROOT_PROJECT}, {.path=path("standard"),.kind=MODULE_ROOT_STANDARD}};
    RelocationRequest request = {roots,3,path("project/packages/test/fixture/v1.0.0/src/old"),path("standard/new"),NULL};
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    bool built = relocation_references_build(&references,&preview);
    if (!built) { fprintf(stderr,"References: %s\n",references.error.message == NULL ? "No source diagnostic" : references.error.message); }
    assert(built);
    size_t qualified = 0, edits = 0;
    for (const RelocationPathEdit *edit = references.edits; edit != NULL; edit = edit->next) {
        const unsigned char *source = source_bytes(edit->original.source);
        size_t length = edit->original.end-edit->original.start;
        assert(length == strlen("/test/fixture/v1.0.0/src/old/names") || length == strlen("/test/fixture/v1.0.0/src/old/number"));
        assert(strncmp(edit->replacement,"//new/",6) == 0);
        if (length == strlen("/test/fixture/v1.0.0/src/old/names")) {
            assert(source[edit->original.end] == '/');
            size_t remaining = source_length(edit->original.source)-edit->original.end;
            assert((remaining >= 14 && memcmp(source+edit->original.end,"/default value",14) == 0)
                || (remaining >= 9 && memcmp(source+edit->original.end,"/values[]",9) == 0));
            qualified++;
        }
        assert(source[edit->original.start-1] != '|');
        edits++;
    }
    assert(qualified == 2 && edits == 6);
    untouched(&request);
    relocation_references_destroy(&references);
    finish();
}
