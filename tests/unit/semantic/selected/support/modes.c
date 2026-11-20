/** @file
 * @brief Exhaust parent/child call modes and target forms, rejecting every private-name or suffix reinterpretation.
 */
/** Entry/resume/read parents and children independently select owned/reference scalars and indexed owning/reference lists. */
static void it_accepts_each_parent_child_mode_and_scalar_reference_indexed_form(void) {
    prepare_selection();
    const char *parents[] = {" !", " %", ""};
    const char *children[] = {" !", " %", ""};
    const char *targets[] = {"method", "alias&", "methods[index]", "references&[index]"};
    for (size_t parent = 0; parent < 3; parent++) {
        for (size_t child = 0; child < 3; child++) {
            for (size_t form = 0; form < 4; form++) {
                char text[512];
                int length = snprintf(text, sizeof(text), "@!\nproducer\n/test/fixture/v1.0.0/src/producer\nindex\n/test/fixture/v1.0.0/src/value\n |0\n"
                    "producer%s\n %s%s\n  %s result\n", parents[parent], targets[form], children[child], child == 2 ? ">>" : ">");
                assert(length > 0 && (size_t)length < sizeof(text));
                expect_selected(text, true);
            }
        }
    }
}
/** Selectors never fall back to caller names, constants, parameters, private fields or missing suffixes; indices require caller capabilities. */
static void it_rejects_selector_fallback_wrong_roles_forms_and_caller_index_capabilities(void) {
    prepare_selection();
    const char *targets[] = {"fallback", "constant", "parameter", "private", "method&", "alias",
        "methods !", "methods[] !", "references[index] !", "alias&[] !", "method[index] !"};
    for (size_t index = 0; index < sizeof(targets) / sizeof(*targets); index++) {
        char text[512];
        const char *suffix = index < 6 ? " !" : "";
        int length = snprintf(text, sizeof(text), "@!\nproducer\n/test/fixture/v1.0.0/src/producer\nfallback\n/test/fixture/v1.0.0/src/worker\nindex\n/test/fixture/v1.0.0/src/value\nproducer\n %s%s\n", targets[index], suffix);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_selected(text, false);
    }
    expect_selected("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nindex&\n/test/fixture/v1.0.0/src/value\nproducer\n methods[index] !\n", false);
    expect_selected("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nindex\n/test/fixture/v1.0.0/src/resource\nproducer\n methods[index] !\n", false);
    expect_selected("@!\nproducer\n/test/fixture/v1.0.0/src/producer\nproducer\n methods[absent] !\n", false);
}
/** A recursive reference result has finite syntax depth without consuming the C call stack. */
static void it_resolves_deep_reference_result_trees_iteratively(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/link", "@>\nnext&\n/test/fixture/v1.0.0/src/link\n@!\n");
    char text[70000];
    int length = snprintf(text, sizeof(text), "@!\nchain\n/test/fixture/v1.0.0/src/link\nchain\n");
    assert(length > 0);
    size_t used = (size_t)length;
    for (size_t depth = 1; depth <= 256; depth++) {
        assert(used + depth + 10 < sizeof(text));
        memset(text + used, ' ', depth);
        used += depth;
        const char *target = depth == 256 ? "next& !\n" : "next&\n";
        size_t bytes = strlen(target);
        memcpy(text + used, target, bytes);
        used += bytes;
    }
    text[used] = '\0';
    SemanticStatus status = resolve_selected(text);
    assert(status == SEMANTIC_OK);
    SemanticStatement *parent = selected_call("chain", 0);
    size_t count = 0;
    while (parent->actions != NULL) {
        SemanticStatement *child = parent->actions->operation;
        assert(child != NULL && child->enclosing_operation == parent && parent->actions->next == NULL);
        parent = child;
        count++;
    }
    assert(count == 256);
    destroy_semantic();
}
