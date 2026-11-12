/**
 * @file
 * @brief Check ordered structural syntax trees, source spans and iterative nested-body ownership.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <stdio.h>
#include "../ast-common.c"

/**
 * Assert that the component builds typed declarations and preserves each initializer source
 * form. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_builds_typed_declarations_and_preserves_each_initializer_source_form(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@=\n价格\n.商店.金额\n |12 # data  \r\n@<\ntarget&\n.shop.item\n &owned\n"
        "@>\nitems&[]\n.shop.item\n target&\n &owned\n@!\nowned\n.shop.item\n %original\n"
        "copy\n.shop.item\n .catalog.default\nlist[]\n.shop.item\n first\n second\n");
    assert(parsed == PARSE_OK && module.valid && module.sections.kind == MODULE_AMA);
    AstNode *constant = module.constants.first;
    assert(layout_text_is(constant->as.declaration.name->name, "价格"));
    AstSource *literal = constant->as.declaration.first->source;
    assert(literal->kind == AST_LITERAL_SOURCE && layout_text_is(literal->payload, "12 # data  "));
    assert(module.parameters.first->as.declaration.name->reference);
    assert(module.results.first->as.declaration.name->whole_list);
    assert(module.results.first->as.declaration.initializer_count == 2);
    assert(module.body.first->as.declaration.first->source->kind == AST_CONTINUATION_SOURCE);
    AstSource *qualified = module.body.first->next->as.declaration.first->source;
    assert(qualified->kind == AST_CONSTANT_SOURCE);
    assert(layout_text_is(qualified->module_path, ".catalog") && layout_text_is(qualified->member, "default"));
    assert(module.body.last->as.declaration.initializer_count == 2);
    destroy_tree();
}

/**
 * Assert that the component preserves embedded nul in literal payload without string
 * interpretation. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_preserves_embedded_nul_in_literal_payload_without_string_interpretation(void) {
    setup_tree();
    const unsigned char bytes[] = {'@', '!', '\n', 'a', '\n', '.', 't', '\n', ' ', '|', 'x', 0, 'y', '\n'};
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "literal", bytes, sizeof(bytes), &source);
    assert(added == SOURCE_OK);
    ParseStatus parsed = parser_module_parse(&module, source);
    assert(parsed == PARSE_OK);
    SourceSpan payload = module.body.first->as.declaration.first->source->payload;
    assert(payload.end - payload.start == 3);
    assert(memcmp(source_bytes(payload.source) + payload.start, bytes + 10, 3) == 0);
    destroy_tree();
}

/**
 * Assert that the component represents assignments nested indices calls reads and resume. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_represents_assignments_nested_indices_calls_reads_and_resume(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@!\ndestination\n source\nreference&\n &items[positions[index]]\n"
        "items[position]!\n value < input\n result > answer\n:\n fallback!\n"
        "target&%\n > answer\nsensor\n result >> reading\n.catalog\n default >> copy\n");
    assert(parsed == PARSE_OK);
    AstNode *node = module.body.first;
    assert(node->kind == AST_ASSIGNMENT);
    node = node->next;
    AstAccess *access = node->as.assignment.source->access;
    assert(node->as.assignment.source->kind == AST_ACQUIRE_SOURCE);
    assert(layout_text_is(access->index->index->name, "index"));
    node = node->next;
    assert(node->kind == AST_CALL && node->as.call.mode == AST_ENTRY_CALL);
    assert(node->as.call.named && node->as.call.input_count == 1 && node->as.call.output_count == 1);
    assert(node->as.call.alternative.first->kind == AST_CALL);
    node = node->next;
    assert(node->as.call.mode == AST_RESUME_CALL && node->as.call.target->reference);
    node = node->next;
    assert(node->as.call.mode == AST_RETAINED_READ);
    node = node->next;
    assert(node->as.call.mode == AST_CONSTANT_READ && layout_text_is(node->as.call.namespace_path, ".catalog"));
    destroy_tree();
}

/**
 * Assert that the component builds branches loops and control without a symbol table. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_builds_branches_loops_and_control_without_a_symbol_table(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@!\nready\n# adjacent comment\n?\n active\n {\n  worker!\n  %\n  ^\n }\n:\n >\n");
    assert(parsed == PARSE_OK);
    AstNode *branch = module.body.first;
    assert(branch->kind == AST_BRANCH && branch->as.control.body.first->kind == AST_LOOP);
    AstNode *loop = branch->as.control.body.first;
    assert(loop->as.control.body.first->next->kind == AST_SUSPEND);
    assert(loop->as.control.body.last->kind == AST_BREAK);
    assert(branch->as.control.alternative.first->kind == AST_RETURN);
    destroy_tree();
}

/**
 * Assert that the component rejects invalid accesses layout and incomplete active declarations.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_invalid_accesses_layout_and_incomplete_active_declarations(void) {
    const char *invalid[] = {"@=\nname", "@!\nname\n.type&", "@!\nname\n  source",
        "@!\nname\n |literal", "@!\nref&\n &other&", "@!\nx\n items[0]",
        "@!\nx\n %items[]", "@!\nitems[]%", "@!\n.type!", "@!\ncondition\n\n?",
        "@!\ncondition&\n?", "@!\ncondition\n{\n action!", "@!\nworker !",
        "@!\nx\n values[indices&]", "@!\nx\n .member", "@!\nname\n.type\n  |wrong"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        ParseStatus parsed = parse_text(invalid[index]);
        if (parsed == PARSE_OK) {
            fprintf(stderr, "Unexpected acceptance: %s\n", invalid[index]);
        }
        assert(parsed == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/**
 * Assert that the component parses deep control and indices without recursive c activations. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_parses_deep_control_and_indices_without_recursive_c_activations(void) {
    const size_t depth = 512;
    size_t capacity = depth * depth + depth * 12 + 32;
    char *text = malloc(capacity);
    assert(text != NULL);
    size_t length = 0;
    memcpy(text, "@!\n", 3);
    length = 3;
    for (size_t level = 0; level < depth; level++) {
        memset(text + length, ' ', level);
        length += level;
        memcpy(text + length, "ready\n", 6);
        length += 6;
        memset(text + length, ' ', level);
        length += level;
        memcpy(text + length, "?\n", 2);
        length += 2;
    }
    memset(text + length, ' ', depth);
    length += depth;
    text[length++] = '%';
    text[length++] = '\n';
    text[length] = 0;
    setup_tree();
    ParseStatus parsed = parse_text(text);
    assert(parsed == PARSE_OK);
    AstNode *node = module.body.first;
    for (size_t level = 0; level < depth; level++) {
        assert(node->kind == AST_BRANCH);
        node = node->as.control.body.first;
    }
    assert(node->kind == AST_SUSPEND);
    destroy_tree();
    memcpy(text, "@!\nx\n ", 6);
    length = 6;
    for (size_t level = 0; level < depth; level++) {
        memcpy(text + length, "values[", 7);
        length += 7;
    }
    memcpy(text + length, "position", 8);
    length += 8;
    memset(text + length, ']', depth);
    length += depth;
    text[length] = 0;
    setup_tree();
    parsed = parse_text(text);
    assert(parsed == PARSE_OK);
    AstAccess *access = module.body.first->as.assignment.source->access;
    for (size_t level = 0; level < depth; level++) {
        assert(access->index != NULL);
        access = access->index;
    }
    assert(layout_text_is(access->name, "position"));
    destroy_tree();
    free(text);
}

/**
 * Assert that the component cleans every failed allocation in composed tree construction. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_cleans_every_failed_allocation_in_composed_tree_construction(void) {
    const char *text = "@=\nconstant\n.type\n |x\n@!\nready\n?\n worker!\n  < |# bytes\t  \n  > result\n:\n target\n  %source\n";
    setup_tree();
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "failure", (const unsigned char *)text, strlen(text), &source);
    assert(added == SOURCE_OK);
    allocation_count = 0;
    ParseStatus parsed = parser_module_parse(&module, source);
    assert(parsed == PARSE_OK);
    size_t total = allocation_count;
    destroy_tree();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_tree();
        added = source_manager_add(&sources, "failure", (const unsigned char *)text, strlen(text), &source);
        assert(added == SOURCE_OK);
        allocation_count = 0;
        failure_at = failure;
        parsed = parser_module_parse(&module, source);
        assert(parsed == PARSE_OUT_OF_MEMORY && !module.valid);
        destroy_tree();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_builds_typed_declarations_and_preserves_each_initializer_source_form();
    it_preserves_embedded_nul_in_literal_payload_without_string_interpretation();
    it_represents_assignments_nested_indices_calls_reads_and_resume();
    it_builds_branches_loops_and_control_without_a_symbol_table();
    it_rejects_invalid_accesses_layout_and_incomplete_active_declarations();
    it_parses_deep_control_and_indices_without_recursive_c_activations();
    it_cleans_every_failed_allocation_in_composed_tree_construction();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
