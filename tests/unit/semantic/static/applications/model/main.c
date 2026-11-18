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
    setup_applications(NULL, "@<\n...a\n...b\n...c\n@!\n");
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

/** Use positional requests and reversed declared foreign order. The actual
 * constructor's source order still controls positions; skips leave exact slots
 * open. Every retained d binding survives all 27 fixed/residual combinations.
 */
static void it_recovers_all_partial_maps_using_actual_order_instead_of_signature_order(void) {
    for (size_t model = 0; model < 27; model++) {
        char caller[512];
        int length = snprintf(caller, sizeof(caller), "@<\n...family\n ...c\n ...b\n ...a\n"
            "@>\n...result\n...family\n");
        assert(length > 0 && (size_t)length < sizeof(caller));
        size_t digits = model;
        for (size_t slot = 0; slot < 3; slot++, digits /= 3) {
            size_t state = digits % 3;
            const char *line = state == 0 ? " ... <\n" : state == 1 ? " ... < ..integer\n" :
                " ... < .other\n  ...a < ..integer\n  ...b < ..integer\n"
                "  ...c < ..integer\n  ...d < ..integer\n";
            size_t added = strlen(line);
            assert((size_t)length + added < sizeof(caller));
            memcpy(caller + length, line, added + 1); length += (int)added;
        }
        assert((size_t)length + strlen("@!\nanchor&\n..integer\n") < sizeof(caller));
        strcpy(caller + length, "@!\nanchor&\n..integer\n");
        setup_applications(caller, "@<\n...a\n...b\n...c\n...d\n@!\n");
        StaticTermCapture full[4];
        for (size_t slot = 0; slot < 4; slot++) {
            full[slot] = (StaticTermCapture){.formal = application_formal(target_module, slot), .value = integer_type};
        }
        const StaticTerm *other_type;
        assert(static_term_nominal(&terms, target_module, full, 4, (SourceSpan){0}, &other_type));
        const AstTypeExpression *expression = NULL;
        for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
            if (site->root && site->receiver == NULL && site->statement->kind == AST_STATIC_RESULT) {
                expression = site->expression; break;
            }
        }
        assert(expression);
        BindingApplication *requests;
        assert(binding_normalize_static(&binding_plan, NULL, expression, &requests));
        StaticTermOperand operands[3];
        StaticTermCapture captures[4] = {full[3]};
        size_t count = 0;
        digits = model;
        for (const BindingRequest *request = requests->first; request; request = request->next, digits /= 3) {
            size_t state = digits % 3;
            if (state == 0) { assert(request->omitted); continue; }
            const StaticTerm *value = state == 1 ? integer_type : other_type;
            operands[count] = (StaticTermOperand){.request = request, .value = value};
            captures[count + 1] = (StaticTermCapture){.formal = application_formal(target_module, request->source_ordinal), .value = value};
            count++;
        }
        const StaticTerm *actual, *expected;
        assert(static_term_nominal(&terms, target_module, captures, count + 1, expression->span, &actual));
        assert(static_term_nominal(&terms, target_module, full + 3, 1, expression->span, &expected));
        const StaticTermApplication *source;
        assert(static_term_application(&terms, &binding_plan, requests, caller_environment,
            application_symbol("...family"), operands, count, &source));
        const StaticApplicationRecord *record;
        assert(static_application_recover(&applications, source, actual, &record));
        assert(record->constructor == expected);
        assert(record->value == actual);
        assert(record->first->formal == application_formal(target_module, 0));
        assert(record->last->formal == application_formal(target_module, 2));
        destroy_applications();
    }
}

/** Execute complete finite map and positional recovery domains. */
static void run(void) {
    it_matches_all_three_slot_maps_permutations_and_staged_partitions();
    it_recovers_all_partial_maps_using_actual_order_instead_of_signature_order();
}
/** Return success after counted complete-domain teardown. */
int main(void) { run(); return 0; }
