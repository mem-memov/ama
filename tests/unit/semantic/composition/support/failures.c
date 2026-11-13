/** @file
 * @brief Preserve exact first-failure stage prefixes and diagnostics, and fail every allocation of complete semantic analysis.
 */
/** A failed stage prevents later attempts and keeps original declaration/ABI/cycle/alias source context consumable. */
static void it_preserves_the_first_failed_stage_and_original_diagnostic_context(void) {
    const char *invalid[] = {
        "@!\nvalue\n.value\nvalue&\n.value\n",
        "@!\nvalue\n.value\n",
        "@!\nnew value\n missing\n",
        "@!\nsource\n.value\ndestination\n.other\ndestination\n source\n",
        "@!\nrecursive\n.main\n",
        "@!\nvalue\n.value\nalias&\n &value\nvalue\n % alias&\n"
    };
    for (size_t shape = 0; shape < sizeof(invalid) / sizeof(*invalid); shape++) {
        prepare_composition();
        if (shape == 1) { file("generated/program/value.abi", "+ initialize\n- destroy\n= clone\n= duplicate\n"); }
        load_composition(invalid[shape]);
        semantic_program_init(&semantic, &project, sources.allocator);
        SemanticStatus status = semantic_analyze(&semantic);
        assert(status == SEMANTIC_INVALID && !semantic.constants_valid && diagnostic_valid(&semantic.error));
        assert(semantic.names_attempted);
        assert(semantic.native_attempted == (shape != 0));
        assert(semantic.inference_attempted == (shape > 1));
        assert(semantic.resolve_attempted == (shape > 2));
        assert(semantic.relations_attempted == (shape == 4));
        assert(!semantic.constants_attempted && source_span_valid(semantic.error.primary.span));
        SemanticProgram staged;
        semantic_program_init(&staged, &project, sources.allocator);
        SemanticStatus staged_status = analyze_staged(&staged);
        assert(staged_status == status && !staged.constants_valid);
        assert(staged.names_attempted == semantic.names_attempted && staged.native_attempted == semantic.native_attempted);
        assert(staged.inference_attempted == semantic.inference_attempted && staged.resolve_attempted == semantic.resolve_attempted);
        assert(staged.relations_attempted == semantic.relations_attempted && staged.constants_attempted == semantic.constants_attempted);
        compare_diagnostics(&semantic.error, &staged.error);
        semantic_program_destroy(&staged);
        if (shape == 1) { assert(strstr(source_path(semantic.error.primary.span.source), "value.abi") != NULL); }
        if (shape == 3) { assert(semantic.error.related_count == 2 && strcmp(semantic.error.subject, ".value") == 0); }
        if (shape == 4) { assert(semantic.error.related_count > 0 && strstr(semantic.error.message, "cycle") != NULL); }
        if (shape == 5) { assert(semantic.error.related_count == 2 && layout_text_is(semantic.error.related[1].location.span, "&value")); }
        destroy_semantic();
    }
}
/** Each independent semantic allocation failure returns out-of-memory, preserves diagnostics and releases all partial owners. */
static void it_unwinds_every_allocation_of_complete_semantic_composition(void) {
    prepare_composition();
    load_composition(composed_main);
    semantic_program_init(&semantic, &project, sources.allocator);
    size_t before = allocation_count;
    SemanticStatus status = semantic_analyze(&semantic);
    assert(status == SEMANTIC_OK);
    size_t count = allocation_count - before;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        load_composition(composed_main);
        semantic_program_init(&semantic, &project, sources.allocator);
        failure_at = allocation_count + failure;
        status = semantic_analyze(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.constants_valid && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
