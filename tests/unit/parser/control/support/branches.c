/** @file
 * @brief Check written-marker identity and parsed-statement emptiness independently of lowering.
 */
/** All three branch forms retain exactly the bodies actually written. */
static void it_distinguishes_written_positive_negative_and_omitted_bodies(void) {
    const char *forms[] = {"@!\nready\n?\n", "@!\nready\n:\n", "@!\nready\n?\n:\n"};
    for (size_t index = 0; index < 3; index++) {
        setup_tree();
        AstNode *node = parsed(forms[index]);
        assert(node->kind == AST_BRANCH && node->next == NULL);
        assert((node->as.control.body_marker.source != NULL) == (index != 1));
        assert((node->as.control.alternative_marker.source != NULL) == (index != 0));
        size_t warnings = 0;
        for (AstWarning *warning = module.warnings; warning != NULL; warning = warning->next) {
            assert(warning->diagnostic.code == DIAGNOSTIC_EMPTY_BODY);
            assert(diagnostic_valid(&warning->diagnostic));
            warnings++;
        }
        assert(warnings == (index == 2 ? 2 : 1));
        destroy_tree();
    }
    setup_tree();
    AstNode *node = parsed("@!\nready\n# same depth\n:\n %\nnext !\n");
    assert(node->as.control.body.first == NULL);
    assert(node->as.control.alternative.first->kind == AST_SUSPEND);
    assert(node->next->kind == AST_CALL && module.warnings == NULL);
    destroy_tree();
}
/** Empty handlers warn at their own markers in physical order; declarations prevent warnings. */
static void it_counts_declarations_as_statements_and_keeps_warning_marker_order(void) {
    setup_tree();
    AstNode *node = parsed("@!\nready\n?\n local\n /test/fixture/v1.0.0/src/value\n:\n # empty\nroot !\n child !\n :\n:\n");
    assert(node->as.control.body.first->kind == AST_DECLARATION);
    size_t previous = 0, count = 0;
    for (AstWarning *warning = module.warnings; warning != NULL; warning = warning->next) {
        SourceSpan marker = warning->diagnostic.primary.span;
        assert(marker.start > previous && source_bytes(marker.source)[marker.start] == ':');
        previous = marker.start;
        count++;
    }
    assert(count == 3);
    destroy_tree();
    setup_tree();
    parsed("@!\n");
    assert(module.warnings == NULL);
    destroy_tree();
}
