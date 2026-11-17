/**
 * @file
 * @brief Compose checked lexical names, root paths, declaration regions and owned AST lifetime.
 */
#include <stdio.h>
#include "../ast-common.c"

/** Verify all four storage forms and complete names through each real physical ending convention. */
static void it_builds_all_storage_forms_with_multiword_names_and_standard_root_types(void) {
    const char *text = "@=\ndefault value\n..integer\n |1\n@<\ninput value\n..integer\n default value\n"
        "@>\nresult value&\n..integer\n &input value\n@!\nowned value\n..integer\n |2\n"
        "reference value&\n..integer\n &owned value\nowned values[]\n..integer\n |3\n |4\n"
        "references&[]\n..integer\n &owned value\n";
    const char *names[] = {"owned value", "reference value", "owned values", "references"};
    const bool references[] = {false,true,false,true};
    const bool lists[] = {false,false,true,true};
    for (size_t ending = 0; ending < 3; ending++) {
        char bytes[1024];
        size_t length = 0;
        for (const char *cursor = text; *cursor != 0; cursor++) {
            if (*cursor == '\n' && ending == 1) { bytes[length++] = '\r'; }
            bytes[length++] = *cursor;
        }
        if (ending == 2) { length--; }
        bytes[length] = 0;
        setup_tree();
        ParseStatus status = parse_text(bytes);
        assert(status == PARSE_OK && module.valid);
        assert(layout_text_is(module.constants.first->as.declaration.name->name, "default value"));
        assert(layout_text_is(module.parameters.first->as.declaration.name->name, "input value"));
        assert(layout_text_is(module.results.first->as.declaration.name->name, "result value"));
        AstNode *node = module.body.first;
        for (size_t index = 0; index < 4; index++) {
            assert(node != NULL && node->kind == AST_DECLARATION);
            assert(layout_text_is(node->as.declaration.name->name, names[index]));
            assert(layout_text_is(node->as.declaration.type->head, "..integer"));
            assert(node->as.declaration.name->reference == references[index]);
            assert(node->as.declaration.name->whole_list == lists[index]);
            assert(node->as.declaration.initializer_count == (index == 2 ? 2 : 1));
            node = node->next;
        }
        assert(node == NULL);
        destroy_tree();
    }
}

/** Layout at the declaration and initializer depth is distinct; a parent comment closes the initializer region. */
static void it_keeps_declaration_and_initializer_comments_in_their_exact_regions(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\nname value\n# before type\n..integer\n # before initializer\n |0\n"
        "# initializer region is closed\nother name\n..integer\n");
    assert(status == PARSE_OK);
    AstNode *first = module.body.first;
    assert(first != NULL && first->as.declaration.initializer_count == 1);
    assert(first->next != NULL && first->next->as.declaration.initializer_count == 0);
    assert(first->next->next == NULL);
    destroy_tree();
    const char *invalid[] = {
        "@=\n # comment cannot invent declarations\n",
        "@!\nname\n # wrong depth before type\n..integer\n",
        "@!\nname\n..integer\n# initializer closed\n |0\n",
        "@!\nname\n..integer\n  # skipped initializer depth\n |0\n",
        "@C\n # native layout is at zero\n",
        " # no root body\n@!\n",
        "@!\n # no child body\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && !module.valid && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Drafts skip incomplete structure, while their lexical spacing and annotation shape remain strict and classification is atomic. */
static void it_shares_draft_activation_and_lexical_validation_without_parsing_unfinished_declarations(void) {
    const char *drafts[] = {"@<\nunfinished[\n", "# @!\n |@C # payload\n", "name with spaces\n"};
    for (size_t index = 0; index < 3; index++) {
        setup_tree();
        ParseStatus status = parse_text(drafts[index]);
        assert(status == PARSE_OK && module.sections.kind == MODULE_DRAFT && module.body.first == NULL);
        destroy_tree();
    }
    const char *invalid[] = {"@unknown", " @!", "@! extra", "unfinished  name", "unfinished ", "  ", "@!\nunfinished  name"};
    for (size_t index = 0; index < 7; index++) {
        setup_tree();
        ParseStatus status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && diagnostic_valid(&module.error.diagnostic));
        bool sentinel = index != 6;
        ParseError error;
        bool classified = parser_module_classify(&module.layout, &sentinel, &error);
        assert(!classified && sentinel == (index != 6) && diagnostic_valid(&error.diagnostic));
        destroy_tree();
    }
}

/** Fail every allocation of a composed complete-name declaration parse and release all partial owners. */
static void it_cleans_each_allocation_failure_in_complete_name_declaration_construction(void) {
    const char *text = "@=\nconstant value\n..integer\n |1\n@!\nvalues[]\n..integer\n constant value\n |2\n";
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

/** Execute every declaration/region/activation acceptance scenario explicitly. */
void run(void) {
    it_builds_all_storage_forms_with_multiword_names_and_standard_root_types();
    it_keeps_declaration_and_initializer_comments_in_their_exact_regions();
    it_shares_draft_activation_and_lexical_validation_without_parsing_unfinished_declarations();
    it_cleans_each_allocation_failure_in_complete_name_declaration_construction();
}
/** Return success after every asserted scenario. */
int main(void) { run(); return EXIT_SUCCESS; }
