/** @file
 * @brief Validate truncation and allocation boundaries across complete physical/lexical/structural parsing.
 */
/** This complete example combines sections, raw literal bytes, references, selected modes, loops and handlers. */
static const char *const composed_program =
    "@=\nцвет\n..палитра\n |красный # raw  \n"
    "@<\nвход\n.тип\n@>\nself&\n.main\n &\n@!\n"
    "новая переменная\n вход\n"
    "root !\n < |raw # @!  \n method& %\n  condition[index]\n  {\n"
    "   items[item&]\n   {\n    %\n    ^\n   }\n"
    "   ready\n   ?\n   :\n    >\n  }\n :\n  fallback !\n:\n >\n";

/** Every byte prefix either forms valid bounded syntax or reports a valid original-byte diagnostic, including partial UTF-8. */
static void it_checks_every_byte_prefix_of_a_composed_unicode_program(void) {
    size_t length = strlen(composed_program);
    char *prefix = malloc(length + 1);
    assert(prefix != NULL);
    for (size_t end = 0; end <= length; end++) {
        memcpy(prefix, composed_program, end);
        prefix[end] = 0;
        setup_tree();
        ParseStatus status = parse_text(prefix);
        if (status == PARSE_OK) { assert(module.valid); check_tree(prefix); }
        else { assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic)); }
        destroy_tree();
    }
    free(prefix);
}
/** Allocation failure anywhere from source admission through warnings never publishes a valid partial module. */
static void it_unwinds_every_composed_source_to_ast_allocation_failure(void) {
    setup_tree();
    ParseStatus status = parse_text(composed_program);
    assert(status == PARSE_OK);
    check_tree(composed_program);
    size_t total = allocation_count;
    destroy_tree();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_tree();
        failure_at = failure;
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)composed_program,
            strlen(composed_program), &source);
        if (added == SOURCE_OK) {
            status = parser_module_parse(&module, source);
            assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
            assert(module.error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        } else { assert(added == SOURCE_OUT_OF_MEMORY); }
        destroy_tree();
    }
}
