/** @file
 * @brief Verify exact binding rejection partitions and original declaration/earlier-binding diagnostics.
 */
/** Require one exact machine-readable note role; ordering is not a declaration identity. */
static const DiagnosticRelated *binding_note(const char *role) {
    const DiagnosticRelated *found = NULL;
    for (size_t index = 0; index < semantic.error.related_count; index++) {
        if (strcmp(semantic.error.related[index].role,role) == 0) {
            assert(!found);
            found = &semantic.error.related[index];
        }
    }
    assert(found);
    return found;
}

/** Existing declarations, all storage suffixes, role visibility, placeholder arity and static copy capability are checked before execution. */
static void it_rejects_binding_role_suffix_arity_and_copy_capability_errors(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nconstant\n/test/fixture/v1.0.0/src/value\n@<\ninput&\n/test/fixture/v1.0.0/src/value\nitems[]\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\nprivate\n/test/fixture/v1.0.0/src/value\n");
    const char *tails[] = {
        "worker !\n input < & value\n",
        "worker !\n items < values[]\n",
        "worker !\n output < value\n",
        "worker !\n private < value\n",
        "worker !\n constant > value\n",
        "worker !\n >\n >\n",
        "worker !\n <\n <\n <\n",
        "worker !\n input& < value\n",
        "worker !\n input& < |literal\n",
        "worker !\n items[] < |literal\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\nvalues[]\n/test/fixture/v1.0.0/src/value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_resolved(text, false);
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/resource\n@>\noutput\n/test/fixture/v1.0.0/src/resource\n@!\n");
    expect_resolved("@!\nworker\n/test/fixture/v1.0.0/src/worker\nresource\n/test/fixture/v1.0.0/src/resource\nworker !\n < resource\n", false);
    expect_resolved("@!\nworker\n/test/fixture/v1.0.0/src/worker\nresource\n/test/fixture/v1.0.0/src/resource\nworker !\n > resource\n", false);
    expect_resolved("@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n <\n >\n", true);
}
/** The later duplicate, the interface declaration and the first binding all retain their distinct original source locations. */
static void it_retains_the_declaration_and_both_binding_sites_for_duplicate_labels(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    const char *tails[] = {
        "worker !\n input < value\n input < value\n",
        "worker !\n output > value\n output > other\n",
        "worker %\n output > value\n output > other\n",
        "worker\n output >> value\n output >> other\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\nother\n/test/fixture/v1.0.0/src/value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        SemanticStatus status = resolve_text(text);
        assert(status == SEMANTIC_INVALID && semantic.error.related_count == 2);
        assert(strstr(semantic.error.message, "more than once") != NULL && diagnostic_valid(&semantic.error));
        const DiagnosticRelated *declaration = binding_note("actual interface declaration");
        const DiagnosticRelated *earlier = binding_note("earlier binding");
        assert(declaration->location.span.source != semantic.error.primary.span.source);
        assert(earlier->location.span.source == semantic.error.primary.span.source);
        assert(earlier->location.span.start < semantic.error.primary.span.start);
        assert(strcmp(earlier->role, "earlier binding") == 0);
        assert(layout_text_is(declaration->location.span, index == 0 ? "input" : "output"));
        destroy_semantic();
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\noutput\n/test/fixture/v1.0.0/src/value\n");
    SemanticStatus status = resolve_text("@!\nvalue\n/test/fixture/v1.0.0/src/value\n/test/fixture/v1.0.0/src/worker\n output >> value\n output >> value\n");
    /* Raw constant-read normalization rejects duplicate labels before solving
     * the namespace origin. It can name both bindings, not an invented formal. */
    assert(status == SEMANTIC_INVALID && semantic.error.related_count == 1);
    assert(diagnostic_valid(&semantic.error) && layout_text_is(semantic.error.primary.span,"output"));
    assert(binding_note("earlier binding")->location.span.source == semantic.error.primary.span.source);
    assert(binding_note("earlier binding")->location.span.start < semantic.error.primary.span.start);
    destroy_semantic();
}
/** Mode/style syntax failures are rejected by the complete loader before namespace/type stages run. */
static void it_rejects_invalid_modes_and_mixed_styles_before_semantic_analysis(void) {
    prepare_types();
    const char *tails[] = {
        "worker %\n <\n", "worker %\n < value\n", "worker !\n >> value\n",
        "worker\n > value\n", "worker\n >>\n", "/test/fixture/v1.0.0/src/worker !\n", "/test/fixture/v1.0.0/src/worker %\n",
        "worker !\n input < value\n > value\n", "worker !\n <\n output > value\n",
        "worker !\n input <\n", "worker !\n > value\n < value\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
        setup_project();
        ProjectStatus status = load_root("generated/program");
        assert(status == PROJECT_SYNTAX_ERROR && !project.valid && diagnostic_valid(&project.error));
        destroy_project();
    }
}
