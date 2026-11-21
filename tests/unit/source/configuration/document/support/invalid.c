/**
 * @file
 * @brief Refuse unknown, duplicate, incomplete and profile-inappropriate INI records.
 * Each refusal keeps an actual original source interval and leaves valid false.
 */
/** Structural profile and exact table failures are not repaired into a different valid input. */
static void it_rejects_malformed_profile_records_and_duplicate_tables(void) {
    const struct { const char *text; ConfigurationProfile profile; } cases[] = {
        {"vendor = acme", CONFIGURATION_PACKAGE},
        {"[unknown]", CONFIGURATION_PACKAGE},
        {"[aliases]\n[aliases]", CONFIGURATION_PACKAGE},
        {"[dependencies]\n[dependencies]", CONFIGURATION_PACKAGE},
        {"[aliases]\n~a = //x\n~a = //y", CONFIGURATION_PACKAGE},
        {"[aliases]\na = //x", CONFIGURATION_PACKAGE},
        {"[aliases]\n~a = ~b", CONFIGURATION_PACKAGE},
        {"[aliases]\n~a = .type", CONFIGURATION_PACKAGE},
        {"[aliases]\n~a  = //x", CONFIGURATION_PACKAGE},
        {"[aliases]\n~a = //x ", CONFIGURATION_PACKAGE},
        {"[dependencies]\n1 = /acme/pkg/v1.0.0/src/item", CONFIGURATION_PACKAGE},
        {"[dependencies]\n//integer", CONFIGURATION_PACKAGE},
        {"[dependencies]\n~shared/*/../item", CONFIGURATION_PACKAGE},
        {"[dependencies]\n/acme/*\n/acme/*", CONFIGURATION_PACKAGE},
        {"[repository /acme]", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = remote", CONFIGURATION_PACKAGE},
        {"[repository /acme]\ndirectory = .", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = remote\n[aliases]", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = .\nurl = y", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = .\ndirectory = other", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = .\nrevision = a\nrevision = b", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = .\nbranch = main", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = .\n[repository /acme]", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = /absolute", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nurl = x\ndirectory = a/../../b", CONFIGURATION_PACKAGE},
        {"[repository /acme]\nnot an assignment", CONFIGURATION_PACKAGE},
        {";comment ", CONFIGURATION_PACKAGE},
        {"#not an INI comment", CONFIGURATION_PACKAGE},
        {"[aliases]", CONFIGURATION_MODULE},
        {"[dependencies]", CONFIGURATION_MODULE},
        {"[repository /acme]", CONFIGURATION_MODULE},
        {"~a = //x\n~a = //x", CONFIGURATION_MODULE},
        {"~a = value", CONFIGURATION_MODULE},
        {"~a = .placeholder", CONFIGURATION_MODULE},
        {"/acme/pkg/v1.0.0/src/item", CONFIGURATION_MODULE},
        {"", (ConfigurationProfile)17}
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        setup_test();
        assert(parse(cases[index].text, cases[index].profile) == CONFIGURATION_INVALID);
        assert(document.attempted && !document.valid && document.status == CONFIGURATION_INVALID);
        assert(source_span_valid(document.issue.span) && document.issue.span.source == document.source);
        assert(document.issue.message != NULL && document.issue.expected != NULL);
        assert(memcmp(source_bytes(document.source), cases[index].text, strlen(cases[index].text)) == 0);
        finish();
    }
}

/** Byte offsets remain exact through CRLF and international preceding entries. */
static void it_reports_original_duplicate_unknown_and_incomplete_record_sites(void) {
    const struct { const char *text; const char *prefix; } cases[] = {
        {"[aliases]\r\n~名字 = //x\r\n~名字 = //y", "[aliases]\r\n~名字 = //x\r\n"},
        {"[aliases]\n[aliases]", "[aliases]\n"},
        {"[repository /acme]\nurl = x\ndirectory = .\nunknown = x", "[repository /acme]\nurl = x\ndirectory = .\n"},
        {"[repository /acme]\nurl = x", ""},
        {"[dependencies]\n/acme/*/../x", "[dependencies]\n"}
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        setup_test();
        assert(parse(cases[index].text, CONFIGURATION_PACKAGE) == CONFIGURATION_INVALID);
        assert(document.issue.span.start == strlen(cases[index].prefix));
        finish();
    }
}
