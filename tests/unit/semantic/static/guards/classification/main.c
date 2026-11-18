/** @file
 * @brief Compose actual source applications with reuse, partial metadata and producer guard classification.
 */
#include "../common.c"
#include "semantic/static/bindings/normalize.h"

/** No capability search can select a constructor for the unknown application. */
static const char *caller_source = "@<\n...family\n ...x\n...type\n@>\n...partial\n...family\n"
    "...concrete partial\n.other\n@!\nanchor&\n..integer\nreuse&\n...type\n"
    "producer&\n...type\n ... >\nunknown&\n...family\n ...x < ..integer\n";

/** Select original declaration/result syntax rather than foreign signature entries. */
static const AstTypeExpression *expression(const StaticModule *module, const char *name) {
    for (const StaticOccurrence *site = module->first_occurrence; site; site = site->next) {
        if (!site->root || site->receiver != NULL || site->statement == NULL) { continue; }
        if (site->statement->kind == AST_DECLARATION &&
            layout_text_is(site->statement->as.declaration.name->name, name)) { return site->expression; }
        if (site->statement->kind == AST_STATIC_RESULT &&
            layout_text_is(site->statement->as.static_result.name, name)) { return site->expression; }
    }
    assert(false); return NULL;
}

/** Unknown-interface T173 requests are re-normalized by the T177 adapter when
 * actual origin information arrives; one source-written RHS uses integer.
 */
static const StaticApplicationRecord *prepare(const StaticTermEnvironment *environment,
    const AstTypeExpression *expression, const StaticTerm *head) {
    BindingApplication *requests;
    assert(binding_normalize_static(&binding_plan, NULL, expression, &requests));
    StaticTermOperand operand = {.request = requests->first, .value = integer_type};
    const StaticTermApplication *source;
    assert(static_term_application(&terms, &binding_plan, requests, environment, head,
        requests->input_count ? &operand : NULL, requests->input_count, &source));
    const StaticApplicationRecord *record;
    assert(static_application_source(&applications, source, &record));
    return record;
}

/** Unapplied constructors remain metadata, unknown applied heads remain Pending,
 * and plain supplied Types add no edge even when their nominal origin is unknown.
 */
static void it_distinguishes_partial_forwarding_unknown_application_and_plain_type_reuse(void) {
    setup_guards(caller_source, "@<\n...x\n@>\n...result\n..integer\n@!\n");
    const StaticCreationPath *root;
    assert(static_creation_root(&guards, caller_environment, &root));
    const char *names[] = {"...partial", "...concrete partial", "unknown", "reuse", "producer"};
    StaticCreationOutcome expected[] = {STATIC_CREATION_METADATA, STATIC_CREATION_METADATA,
        STATIC_CREATION_PENDING, STATIC_CREATION_REUSE, STATIC_CREATION_PENDING};
    for (size_t partition = 0; partition < 5; partition++) {
        const StaticTerm *head = partition == 1 ? target_constructor :
            application_symbol(partition < 3 ? "...family" : "...type");
        const StaticApplicationRecord *record = prepare(caller_environment,
            expression(caller_module, names[partition]), head);
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome;
        assert(static_creation_enter(&guards, root, record, &output, &outcome));
        assert(outcome == expected[partition] && output == NULL && guards.path_count == 1);
    }
    assert(caller_module->parameter_count == 2);
    destroy_guards();
}

/** An ordinary closed supplied Type's explicit output phase creates a producer
 * dependency. Ordinary self edges are not repeated-template violations; the
 * separate static-definition graph still has to validate actual output equations.
 */
static void it_requires_ordinary_supplied_type_producers_without_banning_ordinary_source_reuse(void) {
    setup_guards(caller_source, "@>\n...result\n..integer\n@!\nself&\n.other\n ... >\n");
    const StaticCreationPath *root, *producer, *again;
    assert(static_creation_root(&guards, caller_environment, &root));
    const StaticTerm *type = application_symbol("...type");
    assert(static_constraint_equate(&solver, type, target_constructor, expression(caller_module, "producer")->span));
    const StaticApplicationRecord *record = prepare(caller_environment, expression(caller_module, "producer"), type);
    StaticCreationOutcome outcome;
    assert(static_creation_enter(&guards, root, record, &producer, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && producer->owner->value == target_constructor);
    assert(producer->parent == root && producer->application->context.requests->output_count == 1);
    const StaticApplicationRecord *self = prepare(producer->owner->environment,
        expression(target_module, "self"), target_constructor);
    assert(static_creation_enter(&guards, producer, self, &again, &outcome));
    assert(outcome == STATIC_CREATION_ENTERED && again->parent == producer);
    assert(again->owner == producer->owner && guards.path_count == 3);
    destroy_guards();
}

/** Execute actual T173/T175/T176/T177 handoffs for every edge-classification role. */
static void run(void) {
    it_distinguishes_partial_forwarding_unknown_application_and_plain_type_reuse();
    it_requires_ordinary_supplied_type_producers_without_banning_ordinary_source_reuse();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
