/** @file
 * @brief Check section and body static compositions with native syntax and failure cleanup.
 */
/** Runtime declarations and static interface entries retain separate ordered
 * owners, including nested foreign signatures and native section syntax.
 */
static void it_composes_static_interfaces_with_runtime_declarations_in_both_implementations(void) {
    const char *markers[] = {"@!", "@C"};
    for (size_t index = 0; index < 2; index++) {
        char text[1024];
        int written = snprintf(text, sizeof(text),
            "@<\n...factory\n ...key\n ...value factory\n  ...element\n"
            "input\n...key\n...other\n@>\n...family\n.dictionary\n"
            " ...key < ...key\nvalue[]\n...other\n%s\n", markers[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        setup_tree();
        assert(parse_text(text) == PARSE_OK);
        AstNode *formal = module.parameters.first;
        assert(formal->kind == AST_STATIC_FORMAL);
        AstStaticFormal *signature = formal->as.static_formal->first;
        assert(layout_text_is(signature->name, "...key") && signature->first == NULL);
        assert(layout_text_is(signature->next->first->name, "...element"));
        assert(signature->next == formal->as.static_formal->last);
        AstNode *input = formal->next;
        assert(input->kind == AST_DECLARATION);
        assert(input->as.declaration.type->kind == AST_TYPE_PLACEHOLDER);
        assert(input->next->kind == AST_STATIC_FORMAL && input->next == module.parameters.last);
        AstNode *result = module.results.first;
        assert(result->kind == AST_STATIC_RESULT);
        assert(layout_text_is(result->as.static_result.name, "...family"));
        assert(layout_text_is(result->as.static_result.expression->first->input->head, "...key"));
        assert(result->next->kind == AST_DECLARATION && result->next == module.results.last);
        assert(result->next->as.declaration.name->whole_list);
        destroy_tree();
    }
}

/** Static outputs and runtime constant reads share an expression without
 * sharing their action lists; an ordinary initializer remains a runtime source.
 */
static void it_composes_projections_constant_reads_and_initialized_type_expressions(void) {
    setup_tree();
    assert(parse_text("@!\n.factory\n ... < ..integer\n ... > ...alias\n"
        "...alias\n default >> result\nvalue\n.container\n"
        " ... < ...alias\n result\n") == PARSE_OK);
    AstNode *projection = module.body.first;
    assert(projection->kind == AST_STATIC_PROJECTION);
    assert(projection->as.static_projection->last->kind == AST_STATIC_OUTPUT);
    AstNode *read = projection->next;
    assert(read->kind == AST_CALL && read->as.call.mode == AST_CONSTANT_READ);
    assert(read->as.call.namespace_expression->kind == AST_TYPE_PLACEHOLDER);
    assert(read->as.call.first->kind == AST_SAVED_OUTPUT);
    AstNode *declaration = read->next;
    assert(declaration == module.body.last && declaration->kind == AST_DECLARATION);
    assert(declaration->as.declaration.type->first->kind == AST_STATIC_INPUT);
    assert(declaration->as.declaration.first != NULL && declaration->as.declaration.first->next == NULL);
    destroy_tree();
}

/** Section context fixes the production; incomplete definitions, hanging
 * markers, comment-only signatures and malformed nesting cannot change it.
 */
static void it_rejects_incomplete_static_interfaces_and_execution_suffixes(void) {
    const char *invalid[] = {
        "@<\n...factory <\n@!\n", "@<\n...factory\n  ...key\n@!\n",
        "@<\n...factory\n # comment\n@!\n", "@>\n...result\n@!\n",
        "@>\n...result\n .type\n@!\n", "@=\n...formal\n.type\n",
        "@!\n...type !\n", "@!\n...type %\n", "@!\n.factory\n ... < .type\n",
        "@!\n.factory\n ... > ...alias\n:\n", "@!\n.factory\n default >> result\n ... < .type\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_tree();
        assert(parse_text(invalid[index]) == PARSE_INVALID);
        assert(diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** A complete interface/body parse owns every intermediate expression, result,
 * signature and operation frame, including failures after partial publication.
 */
static void it_unwinds_every_composed_static_module_allocation_failure(void) {
    const char *text = "@<\n...family\n ...key\n ...nested\n  ...value\n"
        "input\n...key\n@>\n...result\n...family\n ...key < .type\n"
        "@!\n.factory\n ... < .type\n ... > ...alias\n...alias\n zero >> value\n";
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_tree();
        const SourceText *source;
        assert(source_manager_add(&sources, "static-module", (const unsigned char *)text,
            strlen(text), &source) == SOURCE_OK);
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        ParseStatus status = parser_module_parse(&module, source);
        if (failure == 0) {
            assert(status == PARSE_OK && module.valid);
            total = allocation_count - before;
            assert(total != 0);
        } else {
            assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
            assert(module.error.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        }
        destroy_tree();
    }
}

/** Finite signatures use explicit frames independently of expression frames;
 * this source has 600 proper child levels and no semantic name lookup.
 */
static void it_parses_deep_signatures_without_c_recursion(void) {
    size_t depth = 600, size = 4 + (depth + 1) * 8 + depth * (depth + 1) / 2 + 4;
    char *text = malloc(size);
    assert(text != NULL);
    size_t used = 0;
    memcpy(text + used, "@<\n", 3); used += 3;
    for (size_t indentation = 0; indentation <= depth; indentation++) {
        memset(text + used, ' ', indentation); used += indentation;
        memcpy(text + used, "...type\n", 8); used += 8;
    }
    memcpy(text + used, "@!\n", 4); used += 4;
    assert(used <= size);
    setup_tree();
    assert(parse_text(text) == PARSE_OK);
    AstStaticFormal *formal = module.parameters.first->as.static_formal;
    for (size_t indentation = 0; indentation < depth; indentation++) {
        assert(formal->first != NULL && formal->first == formal->last);
        formal = formal->first;
    }
    assert(formal->first == NULL);
    destroy_tree();
    free(text);
}

/** Output-only interface heads retain their source structure in implicit,
 * explicitly declared and positional-skip forms; category introduction remains
 * a semantic operation rather than parser lookup or synthesized formal syntax.
 */
static void it_preserves_output_only_interface_heads_and_explicit_type_controls(void) {
    const char *outputs[] = {"...element type > ...received element type", "... >"};
    for (size_t explicit = 0; explicit < 2; explicit++) {
        for (size_t output = 0; output < 2; output++) {
            char text[512];
            int length = snprintf(text, sizeof(text), "@<\n%ssource&\n...source type\n %s\n@!\n",
                explicit ? "...source type\n" : "", outputs[output]);
            assert(length > 0 && (size_t)length < sizeof(text));
            setup_tree();
            assert(parse_text(text) == PARSE_OK);
            AstNode *input = module.parameters.first;
            if (explicit) {
                assert(input->kind == AST_STATIC_FORMAL && input->as.static_formal->first == NULL);
                input = input->next;
            }
            assert(input == module.parameters.last && input->kind == AST_DECLARATION);
            AstTypeExpression *expression = input->as.declaration.type;
            assert(expression->kind == AST_TYPE_PLACEHOLDER && expression->first == expression->last);
            assert(expression->first->kind == AST_STATIC_OUTPUT && expression->first->input == NULL);
            assert(expression->first->omitted == (output == 1));
            destroy_tree();
        }
    }
}
