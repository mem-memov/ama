/** @file
 * @brief Verify exact binding rejection partitions and original declaration/earlier-binding diagnostics.
 */
/** Existing declarations, all storage suffixes, role visibility, placeholder arity and static copy capability are checked before execution. */
static void it_rejects_binding_role_suffix_arity_and_copy_capability_errors(void) {
    prepare_types();
    file("generated/program/worker", "@=\nconstant\n.value\n@<\ninput&\n.value\nitems[]\n.value\n@>\noutput\n.value\n@!\nprivate\n.value\n");
    const char *tails[] = {
        "worker !\n input < &value\n",
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
        int length = snprintf(text, sizeof(text), "@!\nworker\n.worker\nvalue\n.value\nvalues[]\n.value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_resolved(text, false);
    }
    file("generated/program/worker", "@<\ninput\n.resource\n@>\noutput\n.resource\n@!\n");
    expect_resolved("@!\nworker\n.worker\nresource\n.resource\nworker !\n < resource\n", false);
    expect_resolved("@!\nworker\n.worker\nresource\n.resource\nworker !\n > resource\n", false);
    expect_resolved("@!\nworker\n.worker\nworker !\n <\n >\n", true);
}
/** The later duplicate, the interface declaration and the first binding all retain their distinct original source locations. */
static void it_retains_the_declaration_and_both_binding_sites_for_duplicate_labels(void) {
    prepare_types();
    file("generated/program/worker", "@<\ninput\n.value\n@>\noutput\n.value\n@!\n");
    const char *tails[] = {
        "worker !\n input < value\n input < value\n",
        "worker !\n output > value\n output > other\n",
        "worker %\n output > value\n output > other\n",
        "worker\n output >> value\n output >> other\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n.worker\nvalue\n.value\nother\n.value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        SemanticStatus status = resolve_text(text);
        assert(status == SEMANTIC_INVALID && semantic.error.related_count == 2);
        assert(strstr(semantic.error.message, "more than once") != NULL && diagnostic_valid(&semantic.error));
        const DiagnosticRelated *declaration = &semantic.error.related[0];
        const DiagnosticRelated *earlier = &semantic.error.related[1];
        assert(declaration->location.span.source != semantic.error.primary.span.source);
        assert(earlier->location.span.source == semantic.error.primary.span.source);
        assert(earlier->location.span.start < semantic.error.primary.span.start);
        assert(strcmp(earlier->role, "earlier binding") == 0);
        assert(layout_text_is(declaration->location.span, index == 0 ? "input\n.value" : "output\n.value"));
        destroy_semantic();
    }
    file("generated/program/worker", "@=\noutput\n.value\n");
    SemanticStatus status = resolve_text("@!\nvalue\n.value\n.worker\n output >> value\n output >> value\n");
    assert(status == SEMANTIC_INVALID && semantic.error.related_count == 2);
    assert(layout_text_is(semantic.error.related[0].location.span, "output\n.value"));
    assert(strcmp(semantic.error.related[1].role, "earlier binding") == 0);
    destroy_semantic();
}
/** Mode/style syntax failures are rejected by the complete loader before namespace/type stages run. */
static void it_rejects_invalid_modes_and_mixed_styles_before_semantic_analysis(void) {
    prepare_types();
    const char *tails[] = {
        "worker %\n <\n", "worker %\n < value\n", "worker !\n >> value\n",
        "worker\n > value\n", "worker\n >>\n", ".worker !\n", ".worker %\n",
        "worker !\n input < value\n > value\n", "worker !\n <\n output > value\n",
        "worker !\n input <\n", "worker !\n > value\n < value\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(*tails); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nworker\n.worker\nvalue\n.value\n%s", tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        file("generated/program/main", text);
        setup_project();
        ProjectStatus status = load_root("generated/program");
        assert(status == PROJECT_SYNTAX_ERROR && !project.valid && diagnostic_valid(&project.error));
        destroy_project();
    }
}
