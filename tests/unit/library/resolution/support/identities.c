/** @file
 * @brief Check complete component keys and propagate forced unequal-key digest collisions.
 */
/** Cosmetic comments preserve ABI identities; actual configuration or field-order changes do not. */
static void it_names_complete_abi_contracts_and_rejects_identity_collisions(void) {
    prepare_project();
    setup_test();
    LibraryBuild first, comments;
    bool valid = resolve(&first, records);
    assert(valid);
    char text[4096];
    int length = snprintf(text, sizeof(text), "# comment does not change ABI\n%s", records);
    assert(length > 0 && (size_t)length < sizeof(text));
    valid = resolve(&comments, text);
    assert(valid && strcmp(first.identity->internal_name, comments.identity->internal_name) == 0);
    library_build_destroy(&comments);
    length = snprintf(text, sizeof(text), "%sc-flag | -m32\n", records);
    assert(length > 0 && (size_t)length < sizeof(text));
    valid = resolve(&comments, text);
    assert(valid && strcmp(first.identity->internal_name, comments.identity->internal_name) != 0);
    library_build_destroy(&comments);
    unsigned char digest[32];
    memcpy(digest, first.identity->digest, sizeof(digest));
    library_build_destroy(&first);
    const SourceText *source = configuration(records);
    LibraryBuild collision;
    library_build_init(&collision, sources.allocator);
    IdentityEncoding encoding;
    IdentityStatus status = identity_encode(&collision.identities.arena,
        (QualifiedIdentity){IDENTITY_RUNTIME, "/test/fixture/v1.0.0/src/unrelated", NULL, 0, {0}}, &encoding);
    assert(status == IDENTITY_OK);
    const IdentityName *injected = NULL;
    status = identity_insert(&collision.identities, encoding, digest, &injected);
    assert(status == IDENTITY_OK);
    valid = library_configuration_parse(&collision, source);
    assert(valid);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT},
        {.path = TEST_STANDARD_ROOT, .kind = MODULE_ROOT_STANDARD}};
    valid = library_build_resolve(&collision, roots, 2);
    assert(!valid && !collision.valid && collision.error.code == DIAGNOSTIC_IDENTITY_COLLISION);
    assert(collision.identities.conflicting == injected);
    library_build_destroy(&collision);
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@<\nmessage text\n//text\nextra\n//text\n@>\nfailure\n//failure\n@!\n");
    length = snprintf(text, sizeof(text), "%sinput | run | extra | message\n", records);
    assert(length > 0 && (size_t)length < sizeof(text));
    valid = resolve(&first, text);
    assert(valid);
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@<\nextra\n//text\nmessage text\n//text\n@>\nfailure\n//failure\n@!\n");
    valid = resolve(&comments, text);
    assert(valid && strcmp(first.identity->internal_name, comments.identity->internal_name) != 0);
    library_build_destroy(&comments);
    library_build_destroy(&first);
    destroy_test();
}

/** Helper identities anchor to an actual canonical export, taking the bytewise
 * minimum independently of explicit export order. Full ABI keys still retain
 * all declarations; this does not assert that changed configurations are equal.
 */
static void it_anchors_component_helpers_to_the_least_canonical_export(void) {
    prepare_project();
    const char *tail = "schema | message | text | //text\n"
        "schema | outcome | failure | //failure\nschema | messages | list | message\n"
        "export | other | /test/fixture/v1.0.0/src/other | read\n"
        "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\n"
        "input | run | message text | message\noutput | run | failure | outcome\n"
        "output | other | messages | messages\n";
    const char *orders[] = {records, tail};
    const char expected[] = "/test/fixture/v1.0.0/src/main::";
    for (size_t index = 0; index < 2; index++) {
        setup_test();
        LibraryBuild build;
        assert(resolve(&build, orders[index]));
        assert(strncmp(build.identity->source_name, expected, sizeof(expected) - 1) == 0);
        library_build_destroy(&build);
        destroy_test();
    }
}
