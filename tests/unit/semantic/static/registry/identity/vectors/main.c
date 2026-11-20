/** @file
 * @brief Verify independent specialization descriptor/hash vectors and original zero-input names.
 */
#include "../common.c"

/** The descriptor and SHA-256 name were independently derived from the specified
 * field schema and version-two outer framing with Python struct/hashlib.
 */
static void it_encodes_a_complete_owner_with_a_counted_postorder_value_table(void) {
    setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    const StaticTermEnvironment *environment = key_ordinary(caller_module, &input, 1);
    StaticKeyEncoding key = key_encode(&destination, environment);
    const char expected[] = "AMA-STATIC-KEY:1;O129:/test/fixture/v1.0.0/src/main1:5:.typeT0:1:T29://integer0:0:";
    assert(key.length == sizeof(expected) - 1 && strcmp(key.descriptor, expected) == 0);
    assert(key.qualified.category == IDENTITY_TYPE && key.qualified.member_count == 2);
    assert(strcmp(key.qualified.members[0], "ama-static-key-v1") == 0);
    IdentityRegistry names;
    identity_registry_init(&names, sources.allocator);
    const IdentityName *name, *again;
    assert(static_key_register(&terms, environment, &names, &name) == STATIC_KEY_OK);
    assert(strcmp(name->internal_name,
        "ama_t_5681405da57314bf48000df36d8a22ff1c6f855858b59f2c7f5488a5e8c67282") == 0);
    assert(identity_register(&names, key.qualified, &again) == IDENTITY_OK && name == again);
    identity_registry_destroy(&names);
    source_arena_destroy(&destination);
    destroy_guards();
}

/** UTF-8 byte lengths preserve full Unicode formal/type names. The descriptor
 * and digest are independent vectors, not values copied from the implementation.
 */
static void it_preserves_unicode_formals_and_origins_with_a_stable_name_vector(void) {
    fixture_package_prepare(STATIC_SCOPE_ROOT_DIRECTORY, "fixture");
    file("generated/static scope/packages/test/fixture/v1.0.0/src/\xd1\x82\xd0\xb8\xd0\xbf", "@!\n");
    setup_guards("@<\nvalue\n.\xd0\xba\xd0\xbb\xd1\x8e\xd1\x87\n@!\nanchor&\n//integer\nunicode&\n/test/fixture/v1.0.0/src/\xd1\x82\xd0\xb8\xd0\xbf\n",
        "@!\n");
    const StaticModule *module = static_module_find(&inventory,
        project_find(&project, "/test/fixture/v1.0.0/src/\xd1\x82\xd0\xb8\xd0\xbf"));
    const StaticTerm *value;
    assert(module && static_term_nominal(&terms, module, NULL, 0, (SourceSpan){0}, &value));
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = value};
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    const StaticTermEnvironment *environment = key_ordinary(caller_module, &input, 1);
    StaticKeyEncoding key = key_encode(&destination, environment);
    const char expected[] = "AMA-STATIC-KEY:1;O129:/test/fixture/v1.0.0/src/main1:9:.ключT0:1:T131:/test/fixture/v1.0.0/src/тип0:0:";
    assert(key.length == sizeof(expected) - 1 && strcmp(key.descriptor, expected) == 0);
    IdentityRegistry names;
    identity_registry_init(&names, sources.allocator);
    const IdentityName *name;
    assert(static_key_register(&terms, environment, &names, &name) == STATIC_KEY_OK);
    assert(strcmp(name->internal_name,
        "ama_t_bd9940f2567119a67877dc8909bfc95e58f074edd887efc9280557e878095612") == 0);
    identity_registry_destroy(&names);
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Ordinary zero-input names reuse existing source identities; graph producers
 * retain a distinct compiler-only domain even with an empty input map.
 */
static void it_preserves_zero_input_type_names_and_separates_static_producer_names(void) {
    char ordinary_name[71] = {0};
    for (size_t producer = 0; producer < 2; producer++) {
        setup_guards(producer ? "@...\n.anchor\n//integer\n" : "@!\nanchor&\n//integer\n", "@!\n");
        SourceArena destination;
        source_arena_init(&destination, sources.allocator);
        const StaticTermEnvironment *environment = key_environment(caller_environment);
        StaticKeyEncoding key = key_encode(&destination, environment);
        assert(strcmp(key.descriptor, producer ? "AMA-STATIC-KEY:1;P129:/test/fixture/v1.0.0/src/main0:0:" :
            "AMA-STATIC-KEY:1;O129:/test/fixture/v1.0.0/src/main0:0:") == 0);
        IdentityRegistry names;
        identity_registry_init(&names, sources.allocator);
        const IdentityName *name;
        assert(static_key_register(&terms, environment, &names, &name) == STATIC_KEY_OK);
        if (producer) {
            assert(key.qualified.category == IDENTITY_RUNTIME && key.qualified.member_count == 2);
            assert(strcmp(ordinary_name, name->internal_name) != 0);
        } else {
            assert(key.qualified.category == IDENTITY_TYPE && key.qualified.member_count == 0);
            QualifiedIdentity base = {.category = IDENTITY_TYPE, .module_path = "/test/fixture/v1.0.0/src/main"};
            const IdentityName *existing;
            assert(identity_register(&names, base, &existing) == IDENTITY_OK && existing == name);
            memcpy(ordinary_name, name->internal_name, sizeof(ordinary_name));
        }
        identity_registry_destroy(&names);
        source_arena_destroy(&destination);
        destroy_guards();
    }
}

/** Execute independent vectors and original zero-input naming. */
static void run(void) {
    it_encodes_a_complete_owner_with_a_counted_postorder_value_table();
    it_preserves_unicode_formals_and_origins_with_a_stable_name_vector();
    it_preserves_zero_input_type_names_and_separates_static_producer_names();
}
/** Return success only after counted teardown. */
int main(void) { run(); return 0; }
