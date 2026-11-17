/**
 * @file
 * @brief Verify exact binding separators, complete labels and independent directional positions.
 */
#include "../ast-common.c"
#include "parser/calls/bindings/internal.h"

/** Exercise the narrow binding parser after physical reading, independently of the module-wide token checker. */
static bool binding(const char *text, AstArgument *argument) {
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "binding", (const unsigned char *)text, strlen(text), &source);
    assert(added == SOURCE_OK);
    LayoutStatus read = layout_read(&module.layout, source);
    assert(read == LAYOUT_OK);
    Parser parser = {&module, module.layout.first, NULL, false};
    *argument = (AstArgument){.span = module.layout.first->code};
    if (module.layout.first->kind == LINE_INPUT_LITERAL) { argument->span.end = module.layout.first->physical.end; }
    return parser_binding(&parser, argument);
}

/** Exact component separators are mandatory even before the enclosing lexical prepass is applied. */
static void it_rejects_extra_or_missing_binding_separators_without_trimming_them(void) {
    const char *invalid[] = {"input name  < source", "input name< source", "<  source", ">  receiver",
        "output name  > receiver", ">>  receiver", ">>receiver", "input name <", "output name >", ">>"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        AstArgument argument;
        bool accepted = binding(invalid[index], &argument);
        assert(!accepted && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Complete Unicode declaration labels include compact reference/list suffixes and preserve empty literal payloads. */
static void it_preserves_complete_labels_and_empty_literal_versus_omitted_input(void) {
    const char *labels[] = {"消息 内容 < |", "消息 内容& < &owned value", "消息 内容[] < values[]", "消息 内容&[] < references&[]"};
    for (size_t index = 0; index < 4; index++) {
        setup_tree();
        AstArgument argument;
        bool accepted = binding(labels[index], &argument);
        assert(accepted && argument.label != NULL && !argument.omitted);
        assert(layout_text_is(argument.label->name, "消息 内容"));
        assert(argument.label->reference == (index == 1 || index == 3));
        assert(argument.label->whole_list == (index >= 2));
        if (index == 0) {
            assert(argument.source->kind == AST_LITERAL_SOURCE);
            assert(argument.source->payload.start == argument.source->payload.end);
        }
        destroy_tree();
    }
    setup_tree();
    AstArgument argument;
    bool accepted = binding("<   # omitted input", &argument);
    assert(accepted && argument.omitted && argument.source == NULL && argument.receiver == NULL);
    destroy_tree();
}

/** Real call ASTs preserve source text order and independent input/output ordinals for multiword named labels. */
static void it_composes_named_binding_phases_and_independent_direction_positions(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\nworker !\n first input < source value\n second input < |\n"
        " first output > new value\n second output > references&[current position]\n");
    assert(status == PARSE_OK);
    AstNode *call = module.body.first;
    assert(call->as.call.named && call->as.call.outputs_started);
    assert(call->as.call.input_count == 2 && call->as.call.output_count == 2);
    const char *names[] = {"first input", "second input", "first output", "second output"};
    AstArgument *argument = call->as.call.first;
    for (size_t index = 0; index < 4; index++) {
        assert(argument != NULL && layout_text_is(argument->label->name, names[index]));
        assert(argument->position == index % 2);
        assert(argument->kind == (index < 2 ? AST_INPUT : AST_OUTPUT));
        argument = argument->next;
    }
    assert(argument == NULL);
    destroy_tree();
}

/** A parent comment closes the binding region; another deeper binding cannot reopen it. */
static void it_keeps_binding_comments_in_the_exact_call_region(void) {
    setup_tree();
    ParseStatus status = parse_text("@!\nworker !\n # argument region\n <\n >\n# parent region\n:\n # handler region\n >\n");
    assert(status == PARSE_OK && module.body.first->as.call.has_alternative);
    assert(module.body.first->as.call.input_count == 1 && module.body.first->as.call.output_count == 1);
    destroy_tree();
    const char *invalid[] = {"@!\nworker !\n# closed bindings\n < value", "@!\nworker !\n  # skipped level\n < value",
        "@!\nworker %\n <", "@!\nworker\n >", "@!\n..palette\n >>", "@!\nworker !\n > value\n < source"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        status = parse_text(invalid[index]);
        assert(status == PARSE_INVALID && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Resume, retained instance reads and Standard constant reads preserve their distinct target/storage metadata. */
static void it_preserves_resume_retained_and_standard_constant_read_modes(void) {
    const char *texts[] = {"@!\nworker& %\n >\n > new result\n",
        "@!\nreferences&[current position]\n result name >> new value\n",
        "@!\n..цветовая палитра\n first value >> new value\n second values[] >> new values[]\n"};
    const AstCallMode modes[] = {AST_RESUME_CALL,AST_RETAINED_READ,AST_CONSTANT_READ};
    const size_t counts[] = {2,1,2};
    for (size_t index = 0; index < 3; index++) {
        setup_tree();
        ParseStatus status = parse_text(texts[index]);
        assert(status == PARSE_OK);
        AstNode *call = module.body.first;
        assert(call->as.call.mode == modes[index] && call->as.call.input_count == 0);
        assert(call->as.call.output_count == counts[index] && call->as.call.outputs_started);
        if (index == 0) { assert(call->as.call.target->reference && call->as.call.first->omitted); }
        if (index == 1) { assert(call->as.call.target->reference && call->as.call.target->index != NULL); }
        if (index == 2) {
            assert(call->as.call.target == NULL && layout_text_is(call->as.call.namespace_expression->head, "..цветовая палитра"));
            assert(call->as.call.first->next->label->whole_list);
        }
        destroy_tree();
    }
    setup_tree();
    ParseStatus status = parse_text("@!\nworker !\n");
    assert(status == PARSE_OK && !module.body.first->as.call.outputs_started);
    assert(module.body.first->as.call.input_count == 0 && module.body.first->as.call.output_count == 0);
    destroy_tree();
}

/** Fail every allocation of named-label, literal/list-source and receiver construction in a real call. */
static void it_unwinds_every_allocation_failure_in_named_call_construction(void) {
    const char *text = "@!\nworker !\n input name < |first\n references&[] < supplied&[]\n output name > receiver\n";
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

/** Execute direct component and real AST phase/region cases. */
void run(void) {
    it_rejects_extra_or_missing_binding_separators_without_trimming_them();
    it_preserves_complete_labels_and_empty_literal_versus_omitted_input();
    it_composes_named_binding_phases_and_independent_direction_positions();
    it_keeps_binding_comments_in_the_exact_call_region();
    it_preserves_resume_retained_and_standard_constant_read_modes();
    it_unwinds_every_allocation_failure_in_named_call_construction();
}
/** Return success after all explicit assertions. */
int main(void) { run(); return EXIT_SUCCESS; }
