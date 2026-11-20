/** @file
 * @brief Exhaust finite disjoint-map composition and residual-set recovery models.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** Every permutation of three independent original slot identities. */
static const size_t orders[][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};

/** Each of three slots is absent, integer or a distinct closed nominal Type.
 * Compare all input permutations and every prefix split with the independently
 * constructed original capture map, not with another application path alone.
 */
static void it_matches_all_three_slot_maps_permutations_and_staged_partitions(void) {
    setup_applications(NULL, "@<\nfirst\n.a\nsecond\n.b\nthird\n.c\n@!\n");
    StaticTermCapture caller_fixed[] = {{.formal = application_formal(caller_module, 0), .value = integer_type},
        {.formal = application_formal(caller_module, 1), .value = integer_type}};
    const StaticTerm *other_type;
    assert(static_term_nominal(&terms, caller_module, caller_fixed, 2, (SourceSpan){0}, &other_type));
    for (size_t model = 0; model < 27; model++) {
        size_t digits = model, count = 0;
        StaticTermCapture map[3];
        StaticApplicationInput inputs[3];
        for (size_t slot = 0; slot < 3; slot++, digits /= 3) {
            size_t state = digits % 3;
            if (state == 0) { continue; }
            map[count++] = (StaticTermCapture){.formal = application_formal(target_module, slot),
                .value = state == 1 ? integer_type : other_type};
        }
        const StaticTerm *expected;
        assert(static_term_nominal(&terms, target_module, map, count, (SourceSpan){0}, &expected));
        for (size_t permutation = 0; permutation < 6; permutation++) {
            size_t written = 0;
            for (size_t index = 0; index < 3; index++) {
                const StaticSymbol *formal = application_formal(target_module, orders[permutation][index]);
                for (size_t capture = 0; capture < count; capture++) {
                    if (map[capture].formal == formal) {
                        inputs[written++] = (StaticApplicationInput){.formal = formal, .value = map[capture].value};
                    }
                }
            }
            assert(written == count);
            for (size_t split = 0; split <= count; split++) {
                const StaticApplicationRecord *first, *last;
                StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
                assert(static_application_forward(&applications, target_constructor, inputs, split, context, &first));
                assert(static_application_forward(&applications, first->value, inputs + split, count - split, context, &last));
                assert(last->value == expected);
                assert(expected->as.nominal.capture_count == count && expected->as.nominal.residual_count == 3 - count);
            }
        }
    }
    destroy_applications();
}

/** Each a/b/c is independently residual, supplied, or already captured; d is
 * always retained and unused. Enumerate every named source permutation against
 * independently built B and B-minus-P maps, including empty P and partial R.
 */
static void it_recovers_all_named_partial_maps_and_permutations_without_signature_declarations(void) {
    const char *names[] = {".a", ".b", ".c"};
    for (size_t model = 0; model < 27; model++) {
        size_t states[3], digits = model;
        for (size_t slot = 0; slot < 3; slot++, digits /= 3) { states[slot] = digits % 3; }
        for (size_t permutation = 0; permutation < 6; permutation++) {
            char caller[384];
            int used = snprintf(caller, sizeof(caller), "@...\n.result\n.family\n");
            assert(used > 0 && (size_t)used < sizeof(caller));
            for (size_t index = 0; index < 3; index++) {
                size_t slot = orders[permutation][index];
                if (states[slot] != 1) { continue; }
                int added = snprintf(caller + used, sizeof(caller) - (size_t)used,
                    " %s < //integer\n", names[slot]);
                assert(added > 0 && (size_t)added < sizeof(caller) - (size_t)used);
                used += added;
            }
            int added = snprintf(caller + used, sizeof(caller) - (size_t)used,
                ".integer type\n//integer\n");
            assert(added > 0 && (size_t)added < sizeof(caller) - (size_t)used);
            setup_applications(caller, "@<\nfirst\n.a\nsecond\n.b\nthird\n.c\nfourth\n.d\n@!\n");
            StaticTermCapture full[4];
            for (size_t slot = 0; slot < 4; slot++) {
                full[slot] = (StaticTermCapture){.formal = application_formal(target_module, slot), .value = integer_type};
            }
            const StaticTerm *other_type;
            assert(static_term_nominal(&terms, target_module, full, 4, (SourceSpan){0}, &other_type));
            StaticTermCapture actual_map[4] = {full[3]}, preceding_map[4] = {full[3]};
            size_t actual_count = 1, preceding_count = 1;
            for (size_t slot = 0; slot < 3; slot++) {
                if (states[slot] == 0) { continue; }
                StaticTermCapture capture = {.formal = application_formal(target_module, slot),
                    .value = states[slot] == 1 ? integer_type : other_type};
                actual_map[actual_count++] = capture;
                if (states[slot] == 2) { preceding_map[preceding_count++] = capture; }
            }
            const StaticTerm *actual, *expected;
            assert(static_term_nominal(&terms, target_module, actual_map, actual_count, (SourceSpan){0}, &actual));
            assert(static_term_nominal(&terms, target_module, preceding_map, preceding_count, (SourceSpan){0}, &expected));
            const AstTypeExpression *expression = caller_module->first_occurrence->expression;
            assert(layout_text_is(expression->head, ".family"));
            BindingApplication *requests;
            assert(binding_normalize_static(&binding_plan, NULL, expression, &requests));
            StaticTermOperand operands[3] = {{0}};
            size_t count = 0;
            for (const BindingRequest *request = requests->first; request; request = request->next) {
                assert(request->named && !request->omitted && count < 3);
                operands[count++] = (StaticTermOperand){.request = request, .value = integer_type};
            }
            const StaticTermApplication *source;
            assert(static_term_application(&terms, &binding_plan, requests, caller_environment,
                application_symbol(".family"), operands, count, &source));
            const StaticApplicationRecord *record;
            assert(static_application_recover(&applications, source, actual, &record));
            assert(record->constructor == expected && record->value == actual);
            assert(expected->as.nominal.capture_count == preceding_count);
            assert(expected->as.nominal.residual_count == 4 - preceding_count);
            assert(actual->as.nominal.capture_count == actual_count && actual->as.nominal.residual_count == 4 - actual_count);
            assert(record->input_count == count && expected->as.nominal.last_capture->formal == full[3].formal);
            assert(expected->as.nominal.last_capture->value == integer_type);
            destroy_applications();
        }
    }
}

/** Execute complete finite map and named recovery domains. */
static void run(void) {
    it_matches_all_three_slot_maps_permutations_and_staged_partitions();
    it_recovers_all_named_partial_maps_and_permutations_without_signature_declarations();
}
/** Return success after counted complete-domain teardown. */
int main(void) { run(); return 0; }
