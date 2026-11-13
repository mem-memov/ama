/**
 * @file
 * @brief Exhaust diagnostic location boundaries and verify atomic failed adaptation.
 */

/** Every half-open bounded byte interval, including EOF, is valid; malformed ranges never become renderable. */
static void it_validates_all_primary_and_related_byte_intervals_including_eof(void) {
    setup_test();
    size_t length = source_length(source);
    Diagnostic diagnostic = {.code = DIAGNOSTIC_SYNTAX, .message = "Invalid input"};
    DiagnosticRelated related = {.role = "declaration"};
    diagnostic.related = &related;
    diagnostic.related_count = 1;
    for (size_t start = 0; start <= length + 1; start++) {
        for (size_t end = 0; end <= length + 1; end++) {
            bool expected = start <= end && end <= length;
            diagnostic.primary.span = (SourceSpan){source, start, end};
            related.location.span = (SourceSpan){source, 0, 0};
            assert(diagnostic_valid(&diagnostic) == expected);
            diagnostic.primary.span = (SourceSpan){source, 0, 0};
            related.location.span = (SourceSpan){source, start, end};
            assert(diagnostic_valid(&diagnostic) == expected);
        }
    }
    diagnostic.primary.span = (SourceSpan){0};
    related.location.span = (SourceSpan){0};
    assert(diagnostic_valid(&diagnostic));
    diagnostic.primary.span.end = 1;
    assert(!diagnostic_valid(&diagnostic));
    destroy_test();
}

/** Every failure code has a unique fixed machine name; absent and invalid prose/roles are rejected without inference. */
static void it_rejects_invalid_code_and_required_text_domains(void) {
    Diagnostic diagnostic = {.message = "Invalid input"};
    for (int code = 0; code < DIAGNOSTIC_CODE_COUNT; code++) {
        diagnostic.code = (DiagnosticCode)code;
        assert(diagnostic_valid(&diagnostic));
        const char *name = diagnostic_code_name(diagnostic.code);
        assert(name != NULL && strcmp(name, "invalid-diagnostic") != 0);
        for (int previous = 0; previous < code; previous++) {
            assert(strcmp(name, diagnostic_code_name((DiagnosticCode)previous)) != 0);
        }
    }
    const int invalid[] = {-1, DIAGNOSTIC_CODE_COUNT, 1000};
    for (size_t index = 0; index < 3; index++) {
        diagnostic.code = (DiagnosticCode)invalid[index];
        assert(!diagnostic_valid(&diagnostic));
        assert(strcmp(diagnostic_code_name(diagnostic.code), "invalid-diagnostic") == 0);
    }
    diagnostic.code = DIAGNOSTIC_SYNTAX;
    diagnostic.message = "";
    assert(!diagnostic_valid(&diagnostic));
    diagnostic.message = NULL;
    assert(!diagnostic_valid(&diagnostic));
    assert(!diagnostic_valid(NULL));
}

/** Both adapter outputs remain byte-identical when invalid inputs fail validation before publication. */
static void it_publishes_neither_diagnostic_nor_related_note_on_invalid_adaptation(void) {
    setup_test();
    Diagnostic output = {.code = DIAGNOSTIC_SYNTAX, .message = "sentinel"};
    unsigned char before[sizeof(output)];
    memcpy(before, &output, sizeof(output));
    DiagnosticRelated note = {.role = "sentinel"};
    unsigned char note_before[sizeof(note)];
    memcpy(note_before, &note, sizeof(note));
    ModuleIssue issues[] = {
        {.status = MODULE_OK},
        {.status = (ModuleStatus)-1},
        {.status = (ModuleStatus)1000},
        {.status = MODULE_INVALID_UTF8, .source = source, .offset = source_length(source)},
        {.status = MODULE_INVALID_UTF8, .path = "wrong", .source = source, .offset = 0}
    };
    for (size_t index = 0; index < sizeof(issues) / sizeof(issues[0]); index++) {
        bool adapted = diagnostic_from_module(&issues[index], &note, &output);
        assert(!adapted);
        assert(memcmp(&output, before, sizeof(output)) == 0);
        assert(memcmp(&note, note_before, sizeof(note)) == 0);
    }
    IdentityName previous = {.source_name = ".old", .declaration = {source, 0, 0}};
    bool adapted = diagnostic_from_identity_collision(&previous, "..new",
        (SourceSpan){source, 0, source_length(source) + 1}, &note, &output);
    assert(!adapted);
    assert(memcmp(&output, before, sizeof(output)) == 0);
    assert(memcmp(&note, note_before, sizeof(note)) == 0);
    destroy_test();
}
