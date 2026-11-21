/**
 * @file
 * @brief Exhaust allocation refusals and byte truncations across composed strict INI parsing.
 * The real source owner outlives both complete and partially constructed records.
 */
static const char complete_configuration[] =
    "[aliases]\n~own = ./src\n~名字 = //item\n"
    "[dependencies]\n/acme/pkg/v1.0.0/src/item\n~own/helper\n"
    "[repository /acme]\nurl = remote\ndirectory = .\n"
    "[repository /acme/pkg]\nrevision = main\ndirectory = packages\nurl = nested";

/** Every successfully allocated record has the same destructor after any later allocation refusal. */
static void it_cleans_every_document_allocation_refusal_without_touching_source(void) {
    setup_test();
    SourceSpan input = configuration_test_text(complete_configuration);
    configuration_document_init(&document, sources.allocator);
    size_t before = allocation_count;
    assert(configuration_document_parse(&document, input.source, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    size_t count = allocation_count - before;
    assert(count == 6);
    finish();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_test();
        input = configuration_test_text(complete_configuration);
        size_t source_allocations = live_allocations;
        configuration_document_init(&document, sources.allocator);
        failure_at = allocation_count + failure;
        assert(configuration_document_parse(&document, input.source, CONFIGURATION_PACKAGE) == CONFIGURATION_OUT_OF_MEMORY);
        assert(!document.valid && document.issue.span.source == input.source && source_span_valid(document.issue.span));
        assert(memcmp(source_bytes(input.source), complete_configuration, strlen(complete_configuration)) == 0);
        configuration_document_destroy(&document);
        assert(live_allocations == source_allocations);
        destroy_test();
    }
}

/** Every byte prefix either forms a complete bounded document or returns an original bounded issue. */
static void it_retains_bounded_diagnostics_for_every_byte_prefix(void) {
    size_t length = strlen(complete_configuration);
    for (size_t end = 0; end <= length; end++) {
        setup_test();
        SourceSpan input = configuration_test_span((const unsigned char *)complete_configuration, end);
        configuration_document_init(&document, sources.allocator);
        ConfigurationStatus status = configuration_document_parse(&document, input.source, CONFIGURATION_PACKAGE);
        assert(status == CONFIGURATION_OK || status == CONFIGURATION_INVALID);
        if (status == CONFIGURATION_OK) { assert(document.valid); }
        else { assert(!document.valid && source_span_valid(document.issue.span)); }
        finish();
    }
}

/** An invalid later comment is globally refused before any earlier valid alias allocates a record. */
static void it_checks_complete_encoding_before_constructing_any_records(void) {
    setup_test();
    SourceSpan input = configuration_test_text("[aliases]\n~a = //item\n;bad\xff");
    configuration_document_init(&document, sources.allocator);
    size_t before = allocation_count;
    assert(configuration_document_parse(&document, input.source, CONFIGURATION_PACKAGE) == CONFIGURATION_INVALID);
    assert(allocation_count == before && document.aliases == NULL && !document.valid);
    assert(document.issue.span.start == strlen("[aliases]\n~a = //item\n;bad"));
    finish();
    setup_test();
    configuration_document_init(&document, sources.allocator);
    assert(configuration_document_parse(&document, NULL, CONFIGURATION_PACKAGE) == CONFIGURATION_INVALID);
    assert(!document.valid && document.issue.span.source == NULL);
    finish();
}
