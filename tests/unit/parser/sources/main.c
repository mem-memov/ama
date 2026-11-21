/**
 * @file
 * @brief Check source kinds and uniform assignments without declaration lookup or inferred type decisions.
 */
#include <stdio.h>
#include "../ast-common.c"

/** Existing and unknown names produce the same assignment node shape; indices and source modes remain explicit. */
static void it_builds_uniform_assignments_without_classifying_first_use_receivers(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\nexisting target\n//value\n |0\n"
        "existing target\n source value\nnew target\n source value\n"
        "owned values[current position]\n /test/fixture/v1.0.0/src/palette/default value\n"
        "reference slots&[positions[next position]]\n source reference&\n");
    assert(status == PARSE_OK);
    const char *names[] = {"existing target", "new target", "owned values", "reference slots"};
    AstNode *node = module.body.first->next;
    for (size_t index = 0; index < 4; index++) {
        assert(node != NULL && node->kind == AST_ASSIGNMENT);
        assert(layout_text_is(node->as.assignment.receiver->name, names[index]));
        assert((node->as.assignment.receiver->index != NULL) == (index >= 2));
        node = node->next;
    }
    assert(node == NULL);
    AstNode *first = module.body.first->next;
    AstNode *second = first->next;
    assert(first->as.assignment.source->kind == AST_VALUE_SOURCE);
    assert(second->as.assignment.source->kind == AST_VALUE_SOURCE);
    assert(layout_text_is(first->as.assignment.source->access->name, "source value"));
    assert(layout_text_is(second->as.assignment.source->access->name, "source value"));
    destroy_tree();
}

/** Every permitted source spelling retains its own tag and original qualified component/storage views. */
static void it_preserves_indexed_qualified_reference_list_and_continuation_source_modes(void) {
    const char *forms[] = {"owned values[positions[next position]]", "reference slots&[current position]",
        "& owned values[current position]", "% owned value", "% source reference&", "values[]",
        "references&[]", "/test/fixture/v1.0.0/src/palette/default value", "//palette/default values[]", "&"};
    const AstSourceKind expected[] = {AST_VALUE_SOURCE,AST_VALUE_SOURCE,AST_ACQUIRE_SOURCE,
        AST_CONTINUATION_SOURCE,AST_CONTINUATION_SOURCE,AST_VALUE_SOURCE,AST_VALUE_SOURCE,
        AST_CONSTANT_SOURCE,AST_CONSTANT_SOURCE,AST_SELF_REFERENCE_SOURCE};
    for (size_t index = 0; index < 10; index++) {
        char text[256];
        int written = snprintf(text, sizeof(text), "@!\nreceiver\n %s\n", forms[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        setup_tree();
        ParseStatus status = parse_text(text);
        assert(status == PARSE_OK);
        AstSource *source = module.body.first->as.assignment.source;
        assert(source->kind == expected[index] && layout_text_is(source->span, forms[index]));
        if (index == 8) {
            assert(source->constant_list && layout_text_is(source->qualified_path, "//palette/default values"));
        }
        if (index == 9) { assert(source->access == NULL && source->qualified_path.source == NULL); }
        destroy_tree();
    }
}

/** Raw constant operands retain aliases and reductions without a guessed member split. */
static void it_retains_whole_constant_aliases_and_relative_operands_for_scoped_resolution(void) {
    const char *forms[] = {"~complete", "~complete[]", "./ghost/../palette/red",
        "../palette/red", "./ghost/..", "//single component"};
    const char *paths[] = {"~complete", "~complete", "./ghost/../palette/red",
        "../palette/red", "./ghost/..", "//single component"};
    for (size_t index = 0; index < sizeof(forms) / sizeof(*forms); index++) {
        char text[256];
        int length = snprintf(text, sizeof(text), "@!\nreceiver\n %s\n", forms[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        setup_tree();
        assert(parse_text(text) == PARSE_OK);
        const AstSource *source = module.body.first->as.assignment.source;
        assert(source->kind == AST_CONSTANT_SOURCE && source->constant_list == (index == 1));
        assert(layout_text_is(source->qualified_path, paths[index]));
        assert(source->qualified_path.source == source->span.source);
        destroy_tree();
    }
}

/** A standalone ampersand is a source, never a receiver/target/index or continuation operand. */
static void it_restricts_self_reference_to_source_positions_without_inventing_a_name(void) {
    setup_tree();
    ParseStatus status = parse_text("@>\nresult&\n/test/fixture/v1.0.0/src/main\n &\n@!\nworker !\n < |&\n < &\n");
    assert(status == PARSE_OK);
    assert(module.results.first->as.declaration.first->source->kind == AST_SELF_REFERENCE_SOURCE);
    assert(module.body.first->as.call.first->source->kind == AST_LITERAL_SOURCE);
    assert(layout_text_is(module.body.first->as.call.first->source->payload, "&"));
    assert(module.body.first->as.call.first->next->source->kind == AST_SELF_REFERENCE_SOURCE);
    destroy_tree();
    const char *invalid[] = {"@!\n&\n source", "@!\n& !", "@!\n&\n?", "@!\nreceiver\n values[&]",
        "@!\nreceiver\n % &", "@!\nworker !\n > &", "@!\nreceiver\n & reference&"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Literal assignment remains forbidden, and a parent comment cannot reopen a receiver's source region. */
static void it_rejects_literal_assignments_and_parent_comment_reopening_with_exact_sites(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\nreceiver\n |raw # bytes  \n");
    assert(status == PARSE_INVALID && module.error.expected == EXPECT_COPY_SOURCE);
    assert(module.error.diagnostic.primary.span.start == strlen("@!\nreceiver\n "));
    assert(module.error.diagnostic.help != NULL);
    destroy_tree();
    setup_tree();
    status = parse_text("@!\nreceiver\n # source region\n source value\n");
    assert(status == PARSE_OK && module.body.first->kind == AST_ASSIGNMENT);
    destroy_tree();
    const char *invalid[] = {"@!\nreceiver\n# closed\n source value", "@!\nreceiver\n  # skipped depth\n source value",
        "@!\nreceiver\n //only/module/", "@!\nreceiver\n % values[]", "@!\nreceiver\n & values[]"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Fail every source/AST allocation while constructing a self-reference source and preserve cleanup ownership. */
static void it_cleans_every_allocation_failure_in_self_reference_assignment_construction(void) {
    const char *text = "@!\nreference slots&[positions[current position]]\n &\n";
    setup_tree();
    ParseStatus status = parse_text(text);
    assert(status == PARSE_OK);
    size_t count = allocation_count;
    destroy_tree();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_tree();
        failure_at = failure;
        const SourceText *source = NULL;
        SourceStatus added = source_manager_add(&sources, "module", (const unsigned char *)text, strlen(text), &source);
        if (added == SOURCE_OK) {
            status = parser_module_parse(&module, source);
            assert(status == PARSE_OUT_OF_MEMORY && !module.valid);
        } else { assert(added == SOURCE_OUT_OF_MEMORY); }
        destroy_tree();
    }
}

/** T222 preserves complete owned operands and rejects every missing or non-ASCII separator. */
static void it_requires_one_ascii_space_after_reference_acquisition(void) {
    const char *valid[] = {"& item", "& item state", "& items[current position]",
        "& items[positions[next position]]"};
    const char *names[] = {"item", "item state", "items", "items"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        char text[256];
        int written = snprintf(text, sizeof(text), "@!\nreceiver&\n %s\n", valid[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        setup_tree();
        assert(parse_text(text) == PARSE_OK);
        AstSource *source = module.body.first->as.assignment.source;
        assert(source->kind == AST_ACQUIRE_SOURCE);
        assert(layout_text_is(source->span, valid[index]));
        assert(layout_text_is(source->access->name, names[index]));
        assert((source->access->index != NULL) == (index >= 2));
        destroy_tree();
    }
    const char *invalid[] = {"&item", "&item state", "&items[position]", "&  item",
        "&\titem", "&\xc2\xa0item", "&\n item", "& ", "& reference&", "& items[]"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        char text[256];
        int written = snprintf(text, sizeof(text), "@!\nreceiver&\n %s\n", invalid[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        setup_tree();
        assert(parse_text(text) == PARSE_INVALID);
        assert(diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Run all receiver/source-position and failure scenarios explicitly. */
void run(void) {
    it_retains_whole_constant_aliases_and_relative_operands_for_scoped_resolution();
    it_requires_one_ascii_space_after_reference_acquisition();
    it_builds_uniform_assignments_without_classifying_first_use_receivers();
    it_preserves_indexed_qualified_reference_list_and_continuation_source_modes();
    it_restricts_self_reference_to_source_positions_without_inventing_a_name();
    it_rejects_literal_assignments_and_parent_comment_reopening_with_exact_sites();
    it_cleans_every_allocation_failure_in_self_reference_assignment_construction();
}
/** Return success after all assertions complete. */
int main(void) { run(); return EXIT_SUCCESS; }
