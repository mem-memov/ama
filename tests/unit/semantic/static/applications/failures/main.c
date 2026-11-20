/** @file
 * @brief Verify failed application preparation preserves publications and releases partial ownership.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** Source-backed graph maps and ordinary input/body fields exercise different
 * allocation paths without mixing static and runtime module sections.
 */
static const char *caller_source = "@...\n.result\n.family\n .b < .element\n"
    ".integer type\n//integer\n";
static const char *field_source = "@<\nfamily value\n.family\nelement value\n.element\n"
    "input&\n/test/fixture/v1.0.0/src/other\n@!\nanchor&\n//integer\nfield&\n/test/fixture/v1.0.0/src/other\n";
static const char *target_source = "@<\nfirst argument\n.a\nsecond argument\n.b\n@!\n";

/** Find an exact original declaration or static result, with no foreign name lookup. */
static const AstTypeExpression *expression(const char *name) {
    for (const StaticOccurrence *site = caller_module->first_occurrence; site; site = site->next) {
        if (!site->root || site->receiver != NULL || site->statement == NULL) { continue; }
        if (name == NULL && site->statement->kind == AST_STATIC_RESULT) { return site->expression; }
        if (name != NULL && site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Prepared requests and actuals precede failure injection; only the operation
 * under examination consumes the swept allocation boundary.
 */
static void it_unwinds_every_source_recovery_and_generated_input_allocation_refusal(void) {
    for (size_t partition = 0; partition < 6; partition++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            const char *graph = partition == 5
                ? "@...\n.result\n.family\n .b < .element\n.bad\n.family\n .missing < //integer\n"
                : caller_source;
            setup_applications(partition < 2 || partition >= 4 ? graph : field_source, target_source);
            const StaticTerm *head = application_symbol(".family");
            StaticTermCapture captures[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
                {.formal = application_formal(target_module, 1), .value = integer_type}};
            const StaticTerm *actual;
            assert(static_term_nominal(&terms, target_module, captures, partition == 4 ? 1 : 2, (SourceSpan){0}, &actual));
            const StaticTermApplication *source = NULL;
            StaticApplicationContext context = {0};
            if (partition < 2 || partition >= 4) {
                BindingApplication *requests;
                assert(binding_normalize_static(&binding_plan, NULL, expression(NULL), &requests));
                StaticTermOperand input = {.request = requests->first, .value = application_symbol(".element")};
                if (partition == 1) {
                    const StaticTerm *partial;
                    assert(static_term_nominal(&terms, target_module, captures, 1, (SourceSpan){0}, &partial));
                    assert(static_constraint_equate(&solver, head, partial, expression(NULL)->span));
                }
                assert(static_term_application(&terms, &binding_plan, requests, caller_environment,
                    head, &input, 1, &source));
            } else {
                assert(static_application_context(&applications, caller_environment,
                    expression(partition == 2 ? "field" : "input"), &context));
            }
            const StaticApplicationRecord *output = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = partition == 0 || partition >= 4 ? static_application_recover(&applications, source, actual, &output) :
                partition == 1 ? static_application_source(&applications, source, &output) :
                static_application_forward(&applications, target_constructor, NULL, 0, context, &output);
            if (!failure) {
                assert(success == (partition < 4));
                if (partition < 4) { assert(output != NULL); }
                else { assert(output == NULL && applications.status == STATIC_APPLY_CONFLICT && applications.count == 0); }
                total = allocation_count - before;
                assert(total > 0);
            } else {
                assert(!success && output == NULL && applications.count == 0);
                assert(applications.status == STATIC_APPLY_OUT_OF_MEMORY);
            }
            failure_at = 0;
            assert(inventory.valid && caller_module->parameter_count == 2 && target_module->parameter_count == 2);
            destroy_applications();
        }
    }
}

/** Publication counters fail before allocation and cannot wrap into a valid
 * record. A failed owner stays failed and cannot change a caller's receiver.
 */
static void it_guards_application_counter_and_sticky_failure_without_allocation(void) {
    setup_applications(caller_source, target_source);
    applications.count = SIZE_MAX;
    size_t before = allocation_count;
    const StaticApplicationRecord *output = NULL;
    assert(!static_application_forward(&applications, target_constructor, NULL, 0,
        (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &output));
    assert(output == NULL && allocation_count == before && applications.status == STATIC_APPLY_OUT_OF_MEMORY);
    assert(!static_application_forward(&applications, target_constructor, NULL, 0,
        (StaticApplicationContext){.policy = STATIC_OMIT_PARTIAL}, &output));
    assert(allocation_count == before);
    destroy_applications();
}

/** Execute all observed resource partitions and checked counter guards. */
static void run(void) {
    it_unwinds_every_source_recovery_and_generated_input_allocation_refusal();
    it_guards_application_counter_and_sticky_failure_without_allocation();
}
/** Return success after counted refusal teardown. */
int main(void) { run(); return 0; }
