#include "../typed-common.c"

static void it_applies_the_complete_nominal_storage_matrix_without_conversions(void) {
    prepare_types();
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t destination = 0; destination < 4; destination++) {
        for (size_t source = 0; source < 4; source++) {
            for (size_t different = 0; different < 2; different++) {
                char text[512];
                int length = snprintf(text, sizeof(text),
                    "@!\nsource%s\n.value\ndestination%s\n.%s\ndestination%s\n source%s\n",
                    forms[source], forms[destination], different ? "other" : "value",
                    forms[destination], forms[source]);
                assert(length > 0 && (size_t)length < sizeof(text));
                expect_resolved(text, destination == source && !different);
            }
        }
    }
}

static void it_checks_initialization_literals_reference_formation_and_continuation_copy(void) {
    prepare_types();
    const char *valid[] = {
        "@!\nvalue\n.value\n |12 # bytes  \nlist[]\n.value\n |first\n |second\n",
        "@!\nvalue\n.value\nreference&\n.value\n &value\ncopy\n.value\n %reference&\n",
        "@!\nvalue\n.resource\nvalue\n value\n",
        "@!\nvalue&\n.resource\ncopy&\n.resource\n value&\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(valid[0]); index++) {
        expect_resolved(valid[index], true);
    }
    const char *invalid[] = {
        "@!\n^\n",
        "@!\nfirst\n.value\n second\nsecond\n.value\n",
        "@!\nreference&\n.value\n |literal\n",
        "@!\nvalue\n.resource\n |literal\n",
        "@!\nvalue\n.resource\ncopy\n.resource\n value\n",
        "@!\nvalue\n.value\nvalue\n %value\n",
        "@=\nreference&\n.value\n@!\n",
        "@=\nconstant\n.value\n@!\nreference&\n.value\n &constant\n",
        "@=\nconstant\n.value\n@!\nconstant\n constant\n",
        "@=\nconstant\n.resource\n@!\n",
        "@=\nfirst\n.value\n .main.second\nsecond\n.value\n@!\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        expect_resolved(invalid[index], false);
    }
}

static void it_resolves_calls_bindings_defaults_and_saved_constant_reads(void) {
    prepare_types();
    file("generated/program/worker", "@<\ninput\n.value\nreference&\n.value\nitems[]\n.value\n"
        "@>\noutput\n.value\n@!\n");
    const char *prefix = "@!\nworker\n.worker\nvalue\n.value\n";
    const char *tails[] = {
        "worker!\n input < |payload\n output > value\n",
        "worker!\n < |payload\n <\n <\n >\nworker\n >> value\n",
        "worker%\n > value\n",
        "worker!\n < value\n < &value\n",
        "worker!\n input < value\n input < value\n",
        "worker!\n reference& < |payload\n",
        "worker!\n items[] < |payload\n",
        "worker!\n unknown < value\n",
        "worker!\n <\n <\n <\n <\n",
        "worker!\n >\n >\n",
        "worker!\n reference < &value\n",
        "worker!\n input < absent\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(tails[0]); index++) {
        char text[768];
        int length = snprintf(text, sizeof(text), "%s%s", prefix, tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_resolved(text, index < 4);
    }
    file("generated/program/worker", "@=\nfirst\n.value\n |one\nsecond\n.value\n first\n@!\n");
    expect_resolved("@!\nvalue\n.value\n .worker.first\n.worker\n second >> value\n", true);
    expect_resolved("@!\nvalue\n.value\n .worker.absent\n", false);
    expect_resolved("@!\nvalue\n.value\n.worker\n first >> value\n first >> value\n", false);
}

static void it_requires_native_condition_and_index_capabilities_without_inheritance(void) {
    prepare_types();
    expect_resolved("@!\nindex\n.value\nitems[]\n.value\nitems[index]\n?\n >\n", true);
    expect_resolved("@!\nindex\n.resource\nitems[]\n.value\nitems[index]!\n", false);
    expect_resolved("@!\ncondition\n.resource\ncondition\n?\n >\n", false);
    file("generated/program/worker", "@!\nfield\n.value\n");
    expect_resolved("@!\ncondition\n.worker\ncondition\n?\n >\n", false);
}

static void it_keeps_resolved_bindings_and_literal_bytes_in_source_order(void) {
    prepare_types();
    file("generated/program/worker", "@<\nfirst\n.value\nsecond\n.value\n@>\noutput\n.value\n@!\n");
    SemanticStatus status = resolve_text("@!\nworker\n.worker\nvalue\n.value\n"
        "worker!\n second < |last # data  \n first < value\n output > value\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *call = semantic.first->last_statement;
    assert(call->bindings->position == 1 && call->bindings->next->position == 0);
    assert(call->bindings->transfer == TRANSFER_LITERAL);
    assert(layout_text_is(call->bindings->source->syntax->payload, "last # data  "));
    assert(call->bindings->field->type == call->bindings->source->type);
    destroy_semantic();
}

/** Whole-list execution/read cannot reach retired positional bindings. */
static void it_rejects_whole_list_execution_and_retained_results(void) {
    prepare_types();
    const char *operations[] = {"items[]!\n", "items[]\n >> value\n"};
    for (size_t index = 0; index < sizeof(operations) / sizeof(*operations); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nvalue\n.value\nitems[]\n.resource\n%s", operations[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        SemanticStatus status = resolve_text(text);
        assert(status == SEMANTIC_INVALID);
        assert(strstr(semantic.error.message, "whole list") != NULL);
        destroy_semantic();
    }
}

static void it_releases_every_partial_resolved_value_binding_and_access(void) {
    prepare_types();
    file("generated/program/worker", "@<\ninput\n.value\n@>\noutput\n.value\n@!\n");
    const char *text = "@!\nvalue\n.value\n |initial\nworker\n.worker\nworker!\n < |input\n > value\n";
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        file("generated/program/main", text);
        setup_semantic();
        status = load_semantic("generated/program");
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) {
            destroy_semantic();
            continue;
        }
        failure_at = failure;
        status = semantic_resolve(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved);
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}

void run(void) {
    it_releases_every_partial_resolved_value_binding_and_access();
    it_applies_the_complete_nominal_storage_matrix_without_conversions();
    it_checks_initialization_literals_reference_formation_and_continuation_copy();
    it_resolves_calls_bindings_defaults_and_saved_constant_reads();
    it_requires_native_condition_and_index_capabilities_without_inheritance();
    it_keeps_resolved_bindings_and_literal_bytes_in_source_order();
    it_rejects_whole_list_execution_and_retained_results();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
