/** @file
 * @brief Verify static lexical contradictions, foreign identities and actual interface arities.
 */
/** Known and unknown origins both reject lexical contradictions. Unknown
 * foreign labels and arities require actual metadata; no output-name search is
 * performed, and every error retains a real original request position.
 */
static void it_rejects_static_mode_order_duplicate_and_actual_arity_errors(void) {
    const char *bodies[] = {
        ".other\n ...a < ...type\n ... >\n",
        ".other\n ...a < ...type\n ...out > ...received\n",
        ".other\n ...a < ...type\n ...a < ...type\n ...out > ...received\n",
        ".other\n ...out > ...one\n ...out > ...two\n",
        ".other\n ...unknown < ...type\n ...out > ...received\n",
        ".other\n ...unknown > ...received\n",
        ".other\n ... <\n ... <\n ... <\n ... >\n",
        ".other\n ... >\n ... >\n ... >\n"
    };
    for (size_t index = 0; index < sizeof(bodies) / sizeof(*bodies); index++) {
        for (size_t actual = 0; actual < 2; actual++) {
            setup_requests(bodies[index]);
            BindingInterface *view = actual != 0 ? request_interface() : NULL;
            BindingApplication *output = NULL;
            const AstTypeExpression *expression = request_head(".other")->expression;
            AstTypeExpression reordered_expression;
            AstStaticBinding reordered[2];
            if (index == 1) {
                /* The parser already rejects this source order. Clone only the
                 * request chain to test the normalizer's independent boundary;
                 * the accepted project AST and original source stay immutable. */
                reordered_expression = *expression;
                reordered[0] = *expression->first;
                reordered[1] = *expression->first->next;
                reordered[1].next = &reordered[0];
                reordered[0].next = NULL;
                reordered_expression.first = &reordered[1];
                reordered_expression.last = &reordered[0];
                expression = &reordered_expression;
            }
            bool success = binding_normalize_static(&binding_plan, view, expression, &output);
            bool expected_success = actual == 0 && index >= 4;
            assert(success == expected_success);
            if (success) { assert(output->pending && output->first->slot == NULL); }
            else {
                assert(output == NULL && binding_plan.first_application == NULL && binding_plan.status == BINDING_INVALID);
                assert(diagnostic_valid(&binding_plan.error));
                assert(binding_plan.error.primary.span.source == project.entry->source->text);
                if (index == 2 || index == 3) {
                    assert(binding_plan.error.related_count == (actual != 0 ? 2 : 1));
                    if (actual != 0) {
                        assert(binding_plan.related[1].location.span.source == project_find(&project, ".other")->source->text);
                    }
                    assert(binding_plan.related[0].location.span.start < binding_plan.error.primary.span.start);
                }
            }
            destroy_binding();
        }
    }
}
