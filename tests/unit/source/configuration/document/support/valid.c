/**
 * @file
 * @brief Check valid complete INI profiles and ordering-independent typed contents.
 * Observations compare original bytes and independently expected table cardinality.
 */
/** Both profiles permit empty/comment-only inputs; empty package tables invent no records. */
static void it_accepts_empty_profiles_and_empty_optional_sections(void) {
    const char *empty[] = {"", "\n", "\r\n", ";comment", ";comment\r\n\r\n"};
    for (size_t profile = 0; profile < 2; profile++) {
        for (size_t index = 0; index < sizeof(empty) / sizeof(*empty); index++) {
            setup_test();
            assert(parse(empty[index], (ConfigurationProfile)profile) == CONFIGURATION_OK);
            assert(document.valid && document.attempted && document.issue.message == NULL);
            assert(document.aliases == NULL && document.selectors == NULL && document.repositories == NULL);
            finish();
        }
    }
    setup_test();
    assert(parse("[aliases]\r\n[dependencies]", CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    assert(document.valid && document.aliases == NULL && document.selectors == NULL);
    finish();
}

/** Alias targets stay raw and repository values retain semicolon/equals/quotes/environment-looking data. */
static void it_preserves_exact_alias_selector_and_raw_repository_values(void) {
    setup_test();
    const char *source = "[aliases]\r\n~own code = ./src\r\n~名字 = //values\r\n"
        "[dependencies]\r\n/acme/*/v2.4.14-*/src/mod1\r\n~own code/extra\r\n"
        "[repository /acme]\r\nrevision = \"$HOME\"\r\n"
        "directory = legacy (C)/code\r\nurl = https://host/x;y?z=a=b#raw";
    assert(parse(source, CONFIGURATION_PACKAGE) == CONFIGURATION_OK && document.valid);
    assert(text_is(document.aliases->name, "~own code") && text_is(document.aliases->target, "./src"));
    assert(text_is(document.aliases->next->name, "~名字") && document.aliases->next->next == NULL);
    assert(text_is(document.selectors->pattern, "/acme/*/v2.4.14-*/src/mod1"));
    assert(text_is(document.selectors->next->pattern, "~own code/extra") && document.selectors->next->next == NULL);
    assert(text_is(document.repositories->prefix, "/acme"));
    assert(text_is(document.repositories->revision, "\"$HOME\""));
    assert(text_is(document.repositories->directory, "legacy (C)/code"));
    assert(text_is(document.repositories->url, "https://host/x;y?z=a=b#raw"));
    assert(document.repositories->next == NULL);
    assert(memcmp(source_bytes(document.source), source, strlen(source)) == 0);
    finish();
    setup_test();
    assert(parse("~data = ~shared/items\n~Data = ../../other", CONFIGURATION_MODULE) == CONFIGURATION_OK);
    assert(text_is(document.aliases->name, "~data") && text_is(document.aliases->target, "~shared/items"));
    assert(text_is(document.aliases->next->name, "~Data") && document.aliases->next->next == NULL);
    assert(document.repositories == NULL && document.selectors == NULL);
    finish();
    setup_test();
    assert(parse("~x = //a\n~x long = //b\n~\xc3\xa9 = //c\n~e\xcc\x81 = //d", CONFIGURATION_MODULE) == CONFIGURATION_OK);
    const char *names[] = {"~x", "~x long", "~\xc3\xa9", "~e\xcc\x81"};
    const ConfigurationAlias *alias = document.aliases;
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        assert(alias != NULL && text_is(alias->name, names[index]));
        alias = alias->next;
    }
    assert(alias == NULL);
    finish();
}

/** Every ordering of three sections and three repository fields yields the same logical entries. */
static void it_accepts_all_section_and_repository_field_permutations(void) {
    const unsigned permutations[][3] = {{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}};
    const char *fields[] = {"url = remote;part=a=b\n", "directory = .\n", "revision = main\n"};
    for (size_t section_order = 0; section_order < 6; section_order++) {
        for (size_t field_order = 0; field_order < 6; field_order++) {
            char repository[256], text[512];
            int written = snprintf(repository, sizeof(repository), "[repository /acme]\n%s%s%s",
                fields[permutations[field_order][0]], fields[permutations[field_order][1]], fields[permutations[field_order][2]]);
            assert(written > 0 && (size_t)written < sizeof(repository));
            const char *sections[] = {"[aliases]\n~own = ./src\n", "[dependencies]\n/acme/pkg/v1.0.0/src/item\n", repository};
            written = snprintf(text, sizeof(text), "%s%s%s", sections[permutations[section_order][0]],
                sections[permutations[section_order][1]], sections[permutations[section_order][2]]);
            assert(written > 0 && (size_t)written < sizeof(text));
            setup_test();
            assert(parse(text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
            assert(document.aliases != NULL && document.aliases->next == NULL && text_is(document.aliases->target, "./src"));
            assert(document.selectors != NULL && document.selectors->next == NULL);
            assert(document.repositories != NULL && document.repositories->next == NULL);
            assert(text_is(document.repositories->url, "remote;part=a=b"));
            assert(text_is(document.repositories->directory, ".") && text_is(document.repositories->revision, "main"));
            finish();
        }
    }
}

/** Deep raw paths remain bounded views, with no fixed record buffer or recursive scan limit. */
static void it_accepts_long_records_without_fixed_buffers_or_recursive_path_scanning(void) {
    const size_t components = 2048;
    const char prefix[] = "~deep = //";
    size_t length = sizeof(prefix) - 1 + components * 5 + 4;
    char *text = malloc(length + 1);
    assert(text != NULL);
    memcpy(text, prefix, sizeof(prefix) - 1);
    size_t offset = sizeof(prefix) - 1;
    for (size_t index = 0; index < components; index++) {
        memcpy(text + offset, "node/", 5);
        offset += 5;
    }
    memcpy(text + offset, "leaf", 5);
    assert(strlen(text) == length);
    setup_test();
    SourceSpan input = configuration_test_text(text);
    configuration_document_init(&document, sources.allocator);
    size_t before = allocation_count;
    assert(configuration_document_parse(&document, input.source, CONFIGURATION_MODULE) == CONFIGURATION_OK);
    assert(allocation_count == before + 1 && document.aliases->next == NULL);
    SourceSpan target = document.aliases->target;
    assert(target.end - target.start == 2 + components * 5 + 4);
    assert(memcmp(source_bytes(target.source) + target.start, text + sizeof(prefix) - 3,
        target.end - target.start) == 0);
    finish();
    free(text);
}
