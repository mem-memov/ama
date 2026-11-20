/** @file
 * @brief Check syntactic body emptiness, negative-only branches, lexical iterator cells and static checking of skipped paths.
 */
/** Comments do not create a body, declarations do, and a colon-only branch keeps its explicit negative polarity. */
static void it_retains_branch_polarity_and_syntactic_empty_body_warnings(void) {
    prepare_control();
    SemanticStatus status = resolve_control("@!\ncondition\n.value\ncondition\n?\n:\ncondition\n:\ncondition\n{\n # empty\n}\n"
        "condition\n{\n declaration\n .value\n}\n");
    assert(status == SEMANTIC_OK);
    assert(!control_statement(AST_BRANCH, 0)->negative_branch && control_statement(AST_BRANCH, 1)->negative_branch);
    assert(control_statement(AST_LOOP, 0)->empty_body && !control_statement(AST_LOOP, 1)->empty_body);
    size_t warnings = 0;
    for (const AstWarning *warning = semantic_entry()->module->syntax.warnings; warning != NULL; warning = warning->next) {
        assert(warning->diagnostic.code == DIAGNOSTIC_EMPTY_BODY && diagnostic_valid(&warning->diagnostic));
        warnings++;
    }
    assert(warnings == 4);
    destroy_semantic();
}
/** Indexed truth and foreach have distinct roles; nested compatible binders reuse cells and disjoint scopes remain independent. */
static void it_distinguishes_foreach_binding_from_indexed_truth_and_reuses_exact_lexical_cells(void) {
    prepare_control();
    SemanticStatus status = resolve_control("@!\nitems[]\n.value\nindex\n.value\n |0\nitems[index]\n{\n}\n"
        "items[item&]\n{\n items[item&]\n {\n  saved&\n   item&\n }\n}\nitems[item&]\n{\n}\n");
    assert(status == SEMANTIC_OK);
    assert(control_statement(AST_LOOP, 0)->traversal == NULL && control_statement(AST_LOOP, 0)->condition.access->index != NULL);
    SemanticStatement *outer = control_statement(AST_FOREACH, 0);
    SemanticStatement *inner = control_statement(AST_FOREACH, 1);
    SemanticStatement *later = control_statement(AST_FOREACH, 2);
    assert(outer->traversal->introduced && !inner->traversal->introduced && later->traversal->introduced);
    assert(outer->traversal->binding == inner->traversal->binding && later->traversal->binding != outer->traversal->binding);
    assert(inner->enclosing_loop == outer && later->enclosing_loop == NULL);
    SemanticStatement *copy = control_statement(AST_ASSIGNMENT, 0);
    assert(copy->source->access->symbol == outer->traversal->binding && copy->access->symbol->inferred && !copy->access->symbol->local);
    assert(later->empty_body && !outer->empty_body);
    destroy_semantic();
    expect_control("@!\nitems&[]\n.resource\nitems&[item&]\n{\n item& !\n}\nitem&\n.resource\n", true);
}
/** Empty/default-false paths still require valid names/types, and a new header never supplies a break target to its own handlers. */
static void it_checks_skipped_paths_predicate_capabilities_and_header_handler_exits(void) {
    prepare_control();
    const char *invalid[] = {
        "@!\nitems[]\n.value\nitems[item&]\n{\n missing !\n}\n",
        "@!\ncondition\n.value\n |0\ncondition\n?\n destination\n  missing\n",
        "@!\ncondition&\n.value\ncondition\n{\n}\n",
        "@!\ncondition\n.resource\ncondition\n:\n",
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) { expect_control(invalid[index], false); }
    expect_control_parse_error("@!\nroot\n.producer\nroot !\n child !\n  condition\n  {\n  }\n :\n  ^\n");
    expect_control_parse_error("@!\nroot\n.producer\nroot !\n child !\n  condition\n  {\n  }\n:\n ^\n");
    file("generated/program/worker", "@>\ncondition&\n.value\n@!\n");
    expect_control("@!\nroot\n.producer\nroot !\n child !\n  condition\n  {\n  }\n", false);
}
