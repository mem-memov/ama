/**
 * @file
 * @brief Test retained call-binding grammar, omissions, literal bytes and target-independent parsing.
 */
#include <stdio.h>
#include "../ast-common.c"

/** Verify independent positional direction counters and explicit no-value omission nodes. */
static void it_counts_skipped_inputs_and_outputs_independently(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@!\nworker !\n < first\n < # default\n < third\n >\n > selected\n > # skip\n>\n");
    assert(parsed == PARSE_OK);
    AstNode *call = module.body.first;
    assert(call->as.call.input_count == 3 && call->as.call.output_count == 3);
    AstArgument *argument = call->as.call.first;
    for (size_t index = 0; index < 6; index++, argument = argument->next) {
        assert(argument->position == index % 3);
        bool omitted = index == 1 || index == 3 || index == 5;
        assert(argument->omitted == omitted);
        assert(argument->label == NULL);
        if (omitted) {
            assert(argument->source == NULL && argument->receiver == NULL);
        }
    }
    assert(module.body.last->kind == AST_RETURN && argument == NULL);
    destroy_tree();
}

/** Separate scalar resume output omissions from forbidden resume inputs. */
static void it_permits_output_placeholders_on_scalar_resume(void) {
    const char *valid[] = {"@!\nworker %\n >\n > selected", "@!\nworker !", "@!\nworker %"};
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) {
        setup_tree();
        ParseStatus parsed = parse_text(valid[index]);
        assert(parsed == PARSE_OK);
        destroy_tree();
    }
}

/** Preserve retained physical payload bytes and distinguish empty literal construction from omitted input. */
static void it_preserves_typed_literal_inputs_and_distinguishes_empty_values_from_skips(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@!\nwriter !\n < |Hello # data   \r\n < |\n <\n > result\n"
        "writer !\n 消息 < |@C | quotes \\\"   ");
    assert(parsed == PARSE_OK);
    AstNode *call = module.body.first;
    AstArgument *argument = call->as.call.first;
    assert(call->as.call.input_count == 3 && call->as.call.output_count == 1);
    assert(argument->source->kind == AST_LITERAL_SOURCE);
    assert(layout_text_is(argument->source->payload, "Hello # data   "));
    assert(argument->span.end == argument->source->payload.end);
    assert(layout_text_is(argument->source->span, "|Hello # data   "));
    argument = argument->next;
    assert(!argument->omitted && argument->source->kind == AST_LITERAL_SOURCE);
    assert(argument->source->payload.start == argument->source->payload.end);
    assert(argument->next->omitted && argument->next->source == NULL);
    call = call->next;
    argument = call->as.call.first;
    assert(call->as.call.named && layout_text_is(argument->label->name, "消息"));
    assert(layout_text_is(argument->source->payload, "@C | quotes \\\"   "));
    destroy_tree();
}

/** Verify payload length by source offsets instead of NUL-terminated string interpretation. */
static void it_preserves_embedded_zero_bytes_in_unterminated_input_literals(void) {
    setup_tree();
    const unsigned char bytes[] = "@!\nwriter !\n < |x\0y#  ";
    const unsigned char payload_bytes[] = "x\0y#  ";
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "literal", bytes, sizeof(bytes) - 1, &source);
    assert(added == SOURCE_OK);
    ParseStatus parsed = parser_module_parse(&module, source);
    assert(parsed == PARSE_OK);
    SourceSpan payload = module.body.first->as.call.first->source->payload;
    assert(payload.end - payload.start == sizeof(payload_bytes) - 1);
    assert(memcmp(source_bytes(source) + payload.start, payload_bytes, sizeof(payload_bytes) - 1) == 0);
    assert(payload.end == source_length(source));
    destroy_tree();
}

/** Reject finite malformed mode/style/operand partitions with a valid structured diagnostic. */
static void it_rejects_empty_reads_named_placeholders_invalid_literals_and_mixed_styles(void) {
    const char *invalid[] = {"@!\nworker\n >>", "@!\nworker !\n value <", "@!\nworker !\n value >",
        "@!\nworker !\n <\n value > destination", "@!\nworker %\n <", "@!\nworker !\n >> value",
        "@!\nworker\n >", "@!\nworker\n >> value\n > destination", "@!\nworker !\n >\n <",
        "@!\nworker !\n <|12", "@!\nworker !\n <12", "@!\nworker !\n name< value",
        "@!\nworker\n > > value", "@!\nworker !\n > & item"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        setup_tree();
        ParseStatus parsed = parse_text(invalid[index]);
        assert(parsed == PARSE_INVALID && diagnostic_valid(&module.error.diagnostic));
        destroy_tree();
    }
}

/** Parsing a binding must not resurrect the retired list action interface.
 * This structural tree is still rejected by semantic whole-list execution checks;
 * this case certifies only target-independent named binding recognition.
 */
static void it_parses_binding_structure_without_a_retired_list_dispatch_contract(void) {
    setup_tree();
    ParseStatus parsed = parse_text("@!\nitems[] !\n action < value\n");
    assert(parsed == PARSE_OK);
    AstNode *call = module.body.first;
    assert(call->as.call.target->whole_list && call->as.call.named && call->as.call.input_count == 1);
    assert(layout_text_is(call->as.call.first->label->name, "action"));
    destroy_tree();
}

/** Independent binding-token alphabet: direction, naming style and omission, with retained spelling for finite-model tests. */
typedef struct {
    const char *text;
    int direction;
    bool named;
    bool omitted;
} BindingModel;

/** Independent finite-state oracle for the retained binding mode/style/order core, not full current lexical conformance. */
static bool model_accepts(size_t mode, const BindingModel *tokens, const size_t *sequence, size_t count) {
    if (mode >= 2 && count == 0) {
        return false;
    }
    bool output_seen = false;
    for (size_t index = 0; index < count; index++) {
        BindingModel token = tokens[sequence[index]];
        if (token.named != tokens[sequence[0]].named || (token.direction == 2 && token.omitted)) {
            return false;
        }
        if ((mode == 0 && token.direction == 2) || (mode == 1 && token.direction != 1)
            || (mode >= 2 && token.direction != 2) || (token.direction == 0 && output_seen)) {
            return false;
        }
        output_seen |= token.direction != 0;
    }
    return true;
}

/** Enumerate four modes and every length-zero-through-three sequence over eleven binding forms; compare the independent finite-state model. */
static void it_matches_the_call_mode_language_for_every_short_binding_sequence(void) {
    const BindingModel tokens[] = {
        {" < value\n", 0, false, false}, {" <\n", 0, false, true},
        {" > value\n", 1, false, false}, {" >\n", 1, false, true},
        {" >> value\n", 2, false, false}, {" >>\n", 2, false, true},
        {" input < value\n", 0, true, false}, {" output > value\n", 1, true, false},
        {" output >> value\n", 2, true, false},
        {" < |# data \n", 0, false, false}, {" input < |\n", 0, true, false}
    };
    const size_t token_count = sizeof(tokens) / sizeof(tokens[0]);
    const char *targets[] = {"worker !\n", "worker %\n", "worker\n", "/test/fixture/v1.0.0/src/catalog\n"};
    for (size_t mode = 0; mode < 4; mode++) {
        size_t combinations = 1;
        for (size_t count = 0; count <= 3; count++, combinations *= token_count) {
            for (size_t number = 0; number < combinations; number++) {
                size_t remaining = number;
                size_t sequence[3] = {0};
                char text[256] = "@!\n";
                strcat(text, targets[mode]);
                for (size_t index = 0; index < count; index++) {
                    sequence[index] = remaining % token_count;
                    remaining /= token_count;
                    strcat(text, tokens[sequence[index]].text);
                }
                setup_tree();
                ParseStatus parsed = parse_text(text);
                bool expected = model_accepts(mode, tokens, sequence, count);
                if ((parsed == PARSE_OK) != expected) {
                    fprintf(stderr, "Model disagreement: %s\n", text);
                }
                assert((parsed == PARSE_OK) == expected);
                destroy_tree();
            }
        }
    }
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_parses_binding_structure_without_a_retired_list_dispatch_contract();
    it_counts_skipped_inputs_and_outputs_independently();
    it_permits_output_placeholders_on_scalar_resume();
    it_preserves_typed_literal_inputs_and_distinguishes_empty_values_from_skips();
    it_preserves_embedded_zero_bytes_in_unterminated_input_literals();
    it_rejects_empty_reads_named_placeholders_invalid_literals_and_mixed_styles();
    it_matches_the_call_mode_language_for_every_short_binding_sequence();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
