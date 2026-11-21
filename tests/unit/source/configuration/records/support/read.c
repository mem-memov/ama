/**
 * @file
 * @brief Check exact INI record categories, original value spans and refusal publication.
 * The parent leaf supplies original SourceText fixtures and counted ownership.
 */
/** Known headers, comments and raw assignments never process later equals/semicolon/hash data. */
static void it_recognizes_exact_records_and_preserves_raw_value_views(void) {
    const struct {
        const char *text;
        ConfigurationRecordKind kind;
        const char *key;
        const char *value;
    } cases[] = {
        {"", CONFIGURATION_BLANK, NULL, NULL},
        {"; ignored=;#", CONFIGURATION_COMMENT, NULL, NULL},
        {"[dependencies]", CONFIGURATION_DEPENDENCIES, NULL, NULL},
        {"[aliases]", CONFIGURATION_ALIASES, NULL, NULL},
        {"[repository /acme/shop tools]", CONFIGURATION_REPOSITORY, "/acme/shop tools", NULL},
        {"~name = //item", CONFIGURATION_ASSIGNMENT, "~name", "//item"},
        {"url = https://host/x;y?z=a=b#raw", CONFIGURATION_ASSIGNMENT, "url", "https://host/x;y?z=a=b#raw"},
        {"revision = \"$HOME\"", CONFIGURATION_ASSIGNMENT, "revision", "\"$HOME\""},
        {"/acme/*/v2.4.14-*/src/mod1", CONFIGURATION_SELECTOR, NULL, "/acme/*/v2.4.14-*/src/mod1"},
        {"#not an INI comment", CONFIGURATION_SELECTOR, NULL, "#not an INI comment"}
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        setup_test();
        SourceSpan text = configuration_test_text(cases[index].text);
        ConfigurationRecord record;
        ConfigurationIssue issue = {0};
        size_t before = allocation_count;
        failure_at = allocation_count + 1;
        assert(configuration_record_read(text, &record, &issue));
        assert(record.kind == cases[index].kind && record.span.source == text.source);
        assert(record.span.start == 0 && record.span.end == strlen(cases[index].text));
        if (cases[index].key == NULL) { assert(record.key.source == NULL); }
        else { assert(configuration_text_is(record.key, cases[index].key)); }
        if (cases[index].value == NULL) { assert(record.value.source == NULL); }
        else { assert(configuration_text_is(record.value, cases[index].value)); }
        assert(allocation_count == before && issue.message == NULL);
        destroy_test();
    }
}

/** Refusal leaves the complete caller record unchanged and identifies an original bounded interval. */
static void it_refuses_malformed_headers_and_assignment_joins_without_publication(void) {
    const char *invalid[] = {"[other]", "[Aliases]", "[aliases] ; comment", "[repository //acme]",
        "[repository /acme/v1.2.3]", "[repository /acme/]", "[repository /acme//pkg]",
        "[repository ~acme]", "[repository /acme/.]", "a= b", "a =b", "a  = b", "a =  b",
        " = b", "a =", "a = ", "= x", "a =\xc2\xa0x", "a = \xc2\xa0x"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        SourceSpan text = configuration_test_text(invalid[index]);
        ConfigurationRecord record = {.kind = CONFIGURATION_COMMENT, .span = text};
        unsigned char saved[sizeof(record)];
        memcpy(saved, &record, sizeof(record));
        ConfigurationIssue issue = {0};
        assert(!configuration_record_read(text, &record, &issue));
        assert(memcmp(saved, &record, sizeof(record)) == 0);
        assert(source_span_valid(issue.span) && issue.span.source == text.source);
        assert(issue.message != NULL && issue.expected != NULL);
        destroy_test();
    }
    ConfigurationRecord record = {0};
    ConfigurationIssue issue = {0};
    assert(!configuration_record_read((SourceSpan){0}, &record, &issue));
    assert(issue.message != NULL);
}
