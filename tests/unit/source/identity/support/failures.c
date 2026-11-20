/**
 * @file
 * @brief Preserve original identity inputs and registered names through malformed requests and every new encoding allocation failure.
 */

/** Invalid raw insertion metadata must not index a category table or publish a name. */
static void it_rejects_invalid_raw_key_shape_without_publishing_or_dereferencing_missing_data(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    unsigned char bytes[] = {1,2}, digest[32] = {0};
    IdentityEncoding encoding = {bytes, sizeof(bytes), "shape fixture", IDENTITY_TYPE, {0}};
    const IdentityName *output = NULL;
    encoding.category = (IdentityCategory)0;
    assert(identity_insert(&registry, encoding, digest, &output) == IDENTITY_INVALID);
    encoding.category = IDENTITY_TYPE;
    encoding.bytes = NULL;
    assert(identity_insert(&registry, encoding, digest, &output) == IDENTITY_INVALID);
    encoding.bytes = bytes;
    encoding.length = 0;
    assert(identity_insert(&registry, encoding, digest, &output) == IDENTITY_INVALID);
    encoding.length = sizeof(bytes);
    encoding.source_name = NULL;
    assert(identity_insert(&registry, encoding, digest, &output) == IDENTITY_INVALID);
    encoding.source_name = "shape fixture";
    assert(identity_insert(&registry, encoding, NULL, &output) == IDENTITY_INVALID);
    assert(identity_insert(&registry, encoding, digest, NULL) == IDENTITY_INVALID);
    assert(output == NULL && registry.first == NULL && allocation_count == 0);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Root-aware member encodings copy borrowed spelling and preserve earlier records at every allocation failure. */
static void it_preserves_existing_names_through_every_root_aware_registration_allocation_failure(void) {
    size_t count = 0;
    for (size_t selected = 0; selected <= count; selected++) {
        setup_test();
        IdentityRegistry registry;
        identity_registry_init(&registry, sources.allocator);
        const IdentityName *prior = register_namespace_vector(&registry, 0);
        const char *members[] = {"field name","0","read"};
        QualifiedIdentity identity = {IDENTITY_RUNTIME, "//магазин/товар данные", members, 3, {0}};
        size_t before = allocation_count;
        const IdentityName *output = prior;
        failure_at = selected == 0 ? 0 : before + selected;
        IdentityStatus status = identity_register(&registry, identity, &output);
        failure_at = 0;
        if (selected == 0) {
            count = allocation_count - before;
            assert(status == IDENTITY_OK && output != prior);
            assert(strcmp(output->source_name, "//магазин/товар данные::field name::0::read") == 0);
        } else { assert(status == IDENTITY_OUT_OF_MEMORY && output == prior && registry.first == prior); }
        assert(register_namespace_vector(&registry, 0) == prior);
        identity_registry_destroy(&registry);
        destroy_test();
    }
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    char path[] = "//item state";
    char member[] = "field name";
    const char *members[] = {member};
    QualifiedIdentity identity = {IDENTITY_STATE_MEMBER, path, members, 1, {0}};
    const IdentityName *name = NULL;
    assert(identity_register(&registry, identity, &name) == IDENTITY_OK);
    memset(path, 'x', sizeof(path) - 1);
    memset(member, 'x', sizeof(member) - 1);
    assert(strcmp(name->source_name, "//item state::field name") == 0);
    const char *original_members[] = {"field name"};
    identity.module_path = "//item state";
    identity.members = original_members;
    const IdentityName *again = NULL;
    assert(identity_register(&registry, identity, &again) == IDENTITY_OK && again == name);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Many discriminator parts must not retain one allocation per successive display prefix. */
static void it_builds_many_member_displays_with_constant_allocation_count(void) {
    const size_t counts[] = {0,1,32};
    for (size_t selected = 0; selected < 3; selected++) {
        setup_test();
        SourceArena arena;
        source_arena_init(&arena, sources.allocator);
        const char *members[32];
        char expected[sizeof("//module name") + 32 * (sizeof("::entry name") - 1)];
        strcpy(expected, "//module name");
        for (size_t index = 0; index < counts[selected]; index++) {
            members[index] = "entry name";
            strcat(expected, "::entry name");
        }
        QualifiedIdentity identity = {IDENTITY_RUNTIME, "//module name", members, counts[selected], {0}};
        IdentityEncoding encoding;
        assert(identity_encode(&arena, identity, &encoding) == IDENTITY_OK);
        assert(strcmp(encoding.source_name, expected) == 0 && allocation_count <= 2);
        assert(encoding.bytes[7] == 2 && encoding.bytes[8] == IDENTITY_RUNTIME && encoding.bytes[9] == MODULE_ROOT_STANDARD);
        source_arena_destroy(&arena);
        destroy_test();
    }
}
