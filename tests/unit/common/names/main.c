#include "../common.c"

static void it_produces_stable_names_from_complete_identities(void) {
    setup_test();
    char first[GENERATED_NAME_CAPACITY], second[GENERATED_NAME_CAPACITY];
    generated_name(first, NAME_NATIVE, "main", NULL);
    assert(strcmp(first, "ama_native_43d7649607aeb385") == 0);
    generated_name(second, NAME_NATIVE, "other", NULL);
    generated_name(second, NAME_NATIVE, "main", NULL);
    assert(strcmp(first, second) == 0);
    assert(source_identity_hash("shop/item", NULL) != source_identity_hash("shop_item", NULL));
    assert(source_identity_hash("shop/Item", NULL) != source_identity_hash("shop/item", NULL));
    assert(source_identity_hash("a", "bc") != source_identity_hash("ab", "c"));
    assert(source_identity_hash("a", NULL) != source_identity_hash("a", ""));
    generated_name(first, NAME_STATE, "main", NULL);
    generated_name(second, NAME_CALL, "main", NULL);
    assert(strcmp(first, second) != 0);
    destroy_test();
}

static void it_keeps_unicode_identity_and_emits_ascii_identifiers(void) {
    setup_test();
    const char* module = "\u5546\u5e97/\u5546\u54c1";
    char name[GENERATED_NAME_CAPACITY];
    generated_name(name, NAME_FIELD, module, "\u6570\u91cf");
    assert(strcmp(name, "ama_field_b9f21a924b8fd518") == 0);
    for (const char* character = name; *character; character++)
        assert((*character >= 'a' && *character <= 'z') ||
            (*character >= '0' && *character <= '9') || *character == '_');
    assert(source_identity_hash("\u00e9", NULL) != source_identity_hash("e\u0301", NULL));
    destroy_test();
}

static void it_rejects_collisions_without_merging_distinct_entities(void) {
    setup_test();
    bool registered = name_registry_register(&registry, 7, "shop/item", NULL, &error);
    assert(registered);
    registered = name_registry_register(&registry, 7, "shop/item", NULL, &error);
    assert(registered && registry.count == 1);
    registered = name_registry_register(&registry, 7, "shop_item", NULL, &error);
    assert(!registered && error.error_code == ERROR_INVALID && registry.count == 1);
    assert(strstr(error.message, "shop/item") && strstr(error.message, "shop_item"));
    error_clear(&error);
    registered = name_registry_register(&registry, 8, "shop/item", "price", &error);
    assert(registered);
    registered = name_registry_register(&registry, 8, "shop/item", "count", &error);
    assert(!registered && registry.count == 2);
    destroy_test();
}

static void it_grows_the_registry_and_escapes_source_labels(void) {
    setup_test();
    char modules[100][32];
    for (int index = 0; index < 100; index++) {
        snprintf(modules[index], sizeof(modules[index]), "module_%d", index);
        bool registered = name_registry_register(&registry,
            source_identity_hash(modules[index], NULL), modules[index], NULL, &error);
        assert(registered);
    }
    assert(registry.count == 100);
    FILE* output = tmpfile();
    assert(output != NULL);
    emit_source_label(output, "unsafe*/label\n");
    assert_stream_equals(output, "/* Ama: unsafe\\x2a/label\\x0a */\n");
    close_file(output);
    destroy_test();
}

void run(void) {
    it_produces_stable_names_from_complete_identities();
    it_keeps_unicode_identity_and_emits_ascii_identifiers();
    it_rejects_collisions_without_merging_distinct_entities();
    it_grows_the_registry_and_escapes_source_labels();
}

int main(void) {
    run();
    printf("unit/common/names: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
