/**
 * @file
 * @brief Test retained exact transfer/capability rules, call typing and allocation failures.
 */
#include "../typed-common.c"
#include "support/root-identities.c"
#include "support/self.c"
#include "support/capabilities.c"

/** Same spelling cannot exempt syntactic owned copies from COPY-06 capability checks. */
static void it_requires_owned_copy_capability_even_for_statically_identical_storage(void) {
    prepare_types();
    const char *invalid[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/resource\nvalue\n value\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/resource\nitems[]\n items[]\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/resource\nindex\n/test/fixture/v1.0.0/src/value\n |0\nitems[index]\n items[index]\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        SemanticStatus status = resolve_text(invalid[index]);
        assert(status == SEMANTIC_INVALID && strstr(semantic.error.message, "independent copying") != NULL);
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nresource\n/test/fixture/v1.0.0/src/resource\n");
    expect_resolved("@!\nvalue\n/test/fixture/v1.0.0/src/worker\nvalue\n value\n", false);
    const char *valid[] = {
        "@!\nvalue&\n/test/fixture/v1.0.0/src/resource\nvalue&\n value&\n",
        "@!\nitems&[]\n/test/fixture/v1.0.0/src/resource\nitems&[]\n items&[]\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(valid[0]); index++) { expect_resolved(valid[index], true); }
}

/** Enumerate all four source/destination forms and equal/unequal nominal identities, with no implicit conversion. */
static void it_applies_the_complete_nominal_storage_matrix_without_conversions(void) {
    prepare_types();
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t destination = 0; destination < 4; destination++) {
        for (size_t source = 0; source < 4; source++) {
            for (size_t different = 0; different < 2; different++) {
                char text[512];
                int length = snprintf(text, sizeof(text),
                    "@!\nsource%s\n/test/fixture/v1.0.0/src/value\ndestination%s\n/test/fixture/v1.0.0/src/%s\ndestination%s\n source%s\n",
                    forms[source], forms[destination], different ? "other" : "value",
                    forms[destination], forms[source]);
                assert(length > 0 && (size_t)length < sizeof(text));
                expect_resolved(text, destination == source && !different);
            }
        }
    }
}

/** Partition retained literal/reference/continuation initialization and visibility failures without bypassing type capabilities. */
static void it_checks_initialization_literals_reference_formation_and_continuation_copy(void) {
    prepare_types();
    const char *valid[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\n |12 # bytes  \nlist[]\n/test/fixture/v1.0.0/src/value\n |first\n |second\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\n & value\ncopy\n/test/fixture/v1.0.0/src/value\n % reference&\n",
        "@!\nvalue&\n/test/fixture/v1.0.0/src/resource\ncopy&\n/test/fixture/v1.0.0/src/resource\n value&\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(valid[0]); index++) {
        expect_resolved(valid[index], true);
    }
    const char *invalid[] = {
        "@!\nfirst\n/test/fixture/v1.0.0/src/value\n second\nsecond\n/test/fixture/v1.0.0/src/value\n",
        "@!\nreference&\n/test/fixture/v1.0.0/src/value\n |literal\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/resource\n |literal\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/resource\ncopy\n/test/fixture/v1.0.0/src/resource\n value\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nvalue\n % value\n",
        "@=\nreference&\n/test/fixture/v1.0.0/src/value\n@!\n",
        "@=\nconstant\n/test/fixture/v1.0.0/src/value\n@!\nreference&\n/test/fixture/v1.0.0/src/value\n & constant\n",
        "@=\nconstant\n/test/fixture/v1.0.0/src/value\n@!\nconstant\n constant\n",
        "@=\nconstant\n/test/fixture/v1.0.0/src/resource\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/second\nsecond\n/test/fixture/v1.0.0/src/value\n@!\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        expect_resolved(invalid[index], false);
    }
}

/** Check exact callee fields, omissions, duplicate/unknown labels and mode-specific source/receiver compatibility. */
static void it_resolves_calls_bindings_defaults_and_saved_constant_reads(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\nreference&\n/test/fixture/v1.0.0/src/value\nitems[]\n/test/fixture/v1.0.0/src/value\n"
        "@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    const char *prefix = "@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\n";
    const char *tails[] = {
        "worker !\n input < |payload\n output > value\n",
        "worker !\n < |payload\n <\n <\n >\nworker\n >> value\n",
        "worker %\n > value\n",
        "worker !\n < value\n < & value\n",
        "worker !\n input < value\n input < value\n",
        "worker !\n reference& < |payload\n",
        "worker !\n items[] < |payload\n",
        "worker !\n unknown < value\n",
        "worker !\n <\n <\n <\n <\n",
        "worker !\n >\n >\n",
        "worker !\n reference < & value\n",
        "worker !\n input < absent\n"
    };
    for (size_t index = 0; index < sizeof(tails) / sizeof(tails[0]); index++) {
        char text[768];
        int length = snprintf(text, sizeof(text), "%s%s", prefix, tails[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        expect_resolved(text, index < 4);
    }
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nfirst\n/test/fixture/v1.0.0/src/value\n |one\nsecond\n/test/fixture/v1.0.0/src/value\n first\n@!\n");
    expect_resolved("@!\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/first\n/test/fixture/v1.0.0/src/worker\n second >> value\n", true);
    expect_resolved("@!\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/absent\n", false);
    expect_resolved("@!\nvalue\n/test/fixture/v1.0.0/src/value\n/test/fixture/v1.0.0/src/worker\n first >> value\n first >> value\n", false);
}

/** Reject borrowed/list/noncapable conditions and indices even if an owned child has the required protocol. */
static void it_requires_native_condition_and_index_capabilities_without_inheritance(void) {
    prepare_types();
    expect_resolved("@!\nindex\n/test/fixture/v1.0.0/src/value\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[index]\n?\n >\n", true);
    expect_resolved("@!\nindex\n/test/fixture/v1.0.0/src/resource\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[index] !\n", false);
    expect_resolved("@!\ncondition\n/test/fixture/v1.0.0/src/resource\ncondition\n?\n >\n", false);
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nfield\n/test/fixture/v1.0.0/src/value\n");
    expect_resolved("@!\ncondition\n/test/fixture/v1.0.0/src/worker\ncondition\n?\n >\n", false);
}

/** Verify text-order typed bindings retain their independent interface ordinals and untouched literal payload. */
static void it_keeps_resolved_bindings_and_literal_bytes_in_source_order(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nfirst\n/test/fixture/v1.0.0/src/value\nsecond\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    SemanticStatus status = resolve_text("@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\n"
        "worker !\n second < |last # data  \n first < value\n output > value\n");
    assert(status == SEMANTIC_OK);
    SemanticStatement *call = semantic_entry()->last_statement;
    assert(call->bindings->position == 1 && call->bindings->next->position == 0);
    assert(call->bindings->transfer == TRANSFER_LITERAL);
    assert(layout_text_is(call->bindings->source->syntax->payload, "last # data  "));
    assert(call->bindings->field->type == call->bindings->source->type);
    destroy_semantic();
}

/** Whole-list execution/read cannot reach retired positional bindings. */
static void it_rejects_whole_list_execution_and_retained_results(void) {
    prepare_types();
    const char *operations[] = {"items[] !\n", "items[]\n >> value\n", "items[] !\n action < value\n"};
    for (size_t index = 0; index < sizeof(operations) / sizeof(*operations); index++) {
        char text[512];
        int length = snprintf(text, sizeof(text), "@!\nvalue\n/test/fixture/v1.0.0/src/value\nitems[]\n/test/fixture/v1.0.0/src/resource\n%s", operations[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        SemanticStatus status = resolve_text(text);
        assert(status == SEMANTIC_INVALID);
        assert(diagnostic_valid(&semantic.error) && semantic.error.code == DIAGNOSTIC_TYPE_MISMATCH);
        SourceSpan site = semantic.error.primary.span;
        assert(site.end-site.start >= 7 && memcmp(source_bytes(site.source)+site.start,"items[]",7) == 0);
        destroy_semantic();
    }
}

/** Inject each measured semantic allocation failure into a real loaded/ABI-checked project and retain valid failure context. */
static void it_releases_every_partial_resolved_value_binding_and_access(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    const char *text = "@!\nvalue\n/test/fixture/v1.0.0/src/value\n |initial\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < |input\n > value\n";
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
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

/** Explicit later declarations are visible to body operations, while creation-time initializers still require an earlier initialized source. */
static void it_separates_complete_explicit_visibility_from_initializer_order(void) {
    prepare_types();
    const char *valid[] = {
        "@!\ndestination\n/test/fixture/v1.0.0/src/value\ndestination\n later\nlater\n/test/fixture/v1.0.0/src/value\n |7\n",
        "@!\nready\n/test/fixture/v1.0.0/src/value\n |0\ndestination\n/test/fixture/v1.0.0/src/value\ndestination\n later\nready\n?\n later\n /test/fixture/v1.0.0/src/value\n  |7\n",
        "@!\nworker !\nworker\n/test/fixture/v1.0.0/src/worker\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(valid[0]); index++) {
        SemanticStatus status = resolve_text(valid[index]);
        assert(status == SEMANTIC_OK && semantic.resolved);
        destroy_semantic();
    }
    SemanticStatus status = resolve_text("@!\ndestination\n/test/fixture/v1.0.0/src/value\n later\nlater\n/test/fixture/v1.0.0/src/value\n |7\n");
    assert(status == SEMANTIC_INVALID && strstr(semantic.error.message, "not initialized yet") != NULL);
    assert(semantic.error.related_count == 1 && diagnostic_valid(&semantic.error));
    assert(semantic.error.primary.span.start < semantic.error.related[0].location.span.start);
    destroy_semantic();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_retains_the_owned_copy_witness_and_native_seed();
    it_keeps_root_kind_identity_in_the_complete_transfer_matrix();
    it_types_lexical_self_in_initializers_assignments_and_explicit_inputs();
    it_rejects_incompatible_self_destinations_without_adopting_their_types();
    it_unwinds_every_allocation_while_typing_self_sources();
    it_separates_complete_explicit_visibility_from_initializer_order();
    it_requires_owned_copy_capability_even_for_statically_identical_storage();
    it_releases_every_partial_resolved_value_binding_and_access();
    it_applies_the_complete_nominal_storage_matrix_without_conversions();
    it_checks_initialization_literals_reference_formation_and_continuation_copy();
    it_resolves_calls_bindings_defaults_and_saved_constant_reads();
    it_requires_native_condition_and_index_capabilities_without_inheritance();
    it_keeps_resolved_bindings_and_literal_bytes_in_source_order();
    it_rejects_whole_list_execution_and_retained_results();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
