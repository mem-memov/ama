/** @file
 * @brief Compose current source inventories, immutable nominal values and minimum use checks.
 */
#include "../common.c"
#include "semantic/static/signature/usage/usage.h"
#include "semantic/static/terms/store.h"
#include "semantic/static/applications/plan.h"

/** Every actual formal is introduced by an ordinary runtime name/type pair. */
#define USAGE_CONSTRUCTOR "@<\na\n...key\nb\n...value\nc\n...extra\n@!\n"

/** The independent concrete source gives arguments Type evidence without
 * accidentally making the constructor's own path a scalar argument.
 */
static void prepare_usage(const char *graph, const char *constructor) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/scalar", "@!\n");
    ModuleRoot root = {STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT};
    setup_static_with_roots(graph, constructor, &root, 1);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
}

/** Both source orders impose the same two-member minimum. Three residuals and
 * a partial with two residuals are both legal; an inherited capture stays fixed.
 */
static void it_preserves_additional_residuals_captures_and_actual_admission_order(void) {
    const char *graphs[] = {
        "@...\n...one\n...family\n ...key < .scalar\n...two\n...family\n ...value < .scalar\n",
        "@...\n...two\n...family\n ...value < .scalar\n...one\n...family\n ...key < .scalar\n"
    };
    for (size_t order = 0; order < 2; order++) {
        prepare_usage(graphs[order], USAGE_CONSTRUCTOR);
        const StaticModule *owner = static_module_find(&inventory, project.entry);
        const StaticSymbol *family = static_symbol_find(owner, static_name_site(project.entry, "...family"));
        assert(family->category == STATIC_CATEGORY_DEFERRED && family->signature == NULL);
        assert(owner->constructor_signature == NULL);
        const StaticModule *origin = static_module_find(&inventory, project_find(&project, ".other"));
        const StaticModule *scalar = static_module_find(&inventory, project_find(&project, ".scalar"));
        StaticTermStore terms;
        static_term_store_init(&terms, &inventory, sources.allocator);
        const StaticTerm *plain, *whole, *partial, *missing, *complete;
        assert(static_term_nominal(&terms, scalar, NULL, 0, (SourceSpan){0}, &plain));
        assert(static_term_nominal(&terms, origin, NULL, 0, (SourceSpan){0}, &whole));
        StaticTermCapture captures[] = {
            {.formal = static_symbol_at(origin, STATIC_PARAMETER, 2), .value = plain},
            {.formal = static_symbol_at(origin, STATIC_PARAMETER, 0), .value = plain},
            {.formal = static_symbol_at(origin, STATIC_PARAMETER, 1), .value = plain}
        };
        assert(static_term_nominal(&terms, origin, captures, 1, (SourceSpan){0}, &partial));
        assert(static_term_nominal(&terms, origin, captures, 2, (SourceSpan){0}, &missing));
        assert(static_term_nominal(&terms, origin, captures, 3, (SourceSpan){0}, &complete));
        const StaticTerm *actuals[] = {whole, partial, whole, partial};
        size_t allocations = allocation_count;
        for (size_t index = 0; index < 4; index++) {
            const StaticTerm *actual = actuals[order ? 3 - index : index];
            StaticUsageResult result = static_usage_check(&inventory, family, actual->category, actual->signature);
            assert(result.status == STATIC_USAGE_SATISFIED);
        }
        assert(static_usage_check(&inventory, family, missing->category, missing->signature).status
            == STATIC_USAGE_CONFLICT);
        assert(static_usage_check(&inventory, family, complete->category, complete->signature).status
            == STATIC_USAGE_CONFLICT);
        assert(allocation_count == allocations);
        assert(whole->as.nominal.residual_count == 3 && partial->as.nominal.residual_count == 2);
        assert(partial->as.nominal.first_capture->formal == captures[0].formal);
        assert(partial->as.nominal.first_capture->value == plain);
        assert(partial->signature->count == 2 && family->category == STATIC_CATEGORY_DEFERRED);
        BindingPlan bindings;
        binding_plan_init(&bindings, sources.allocator);
        StaticConstraintSolver solver;
        static_constraint_init(&solver, &terms, sources.allocator);
        StaticApplicationPlan applications;
        static_application_plan_init(&applications, &terms, &solver, &bindings, sources.allocator);
        StaticApplicationInput supplied = {.formal = captures[0].formal, .value = plain};
        const StaticApplicationRecord *record;
        StaticApplicationContext context = {.policy = STATIC_OMIT_PARTIAL};
        assert(static_application_forward(&applications, whole, &supplied, 1, context, &record));
        assert(record->value == partial);
        assert(static_usage_check(&inventory, family, record->value->category,
            record->value->signature).status == STATIC_USAGE_SATISFIED);
        StaticApplicationInput incompatible = {.formal = captures[1].formal, .value = whole};
        const StaticApplicationRecord *unchanged = record;
        assert(!static_application_forward(&applications, whole, &incompatible, 1, context, &unchanged));
        assert(unchanged == record && record->value == partial);
        assert(applications.status == STATIC_APPLY_CONFLICT);
        static_application_plan_destroy(&applications);
        static_constraint_destroy(&solver);
        binding_plan_destroy(&bindings);
        static_term_store_destroy(&terms);
        destroy_static();
    }
}

/** Unresolved operands remain pending; a separate known category conflict wins.
 * Forwarded-only inputs impose no fabricated actual category or member set.
 */
static void it_keeps_pending_and_forwarded_metadata_without_hiding_known_conflicts(void) {
    const char *graphs[] = {
        "@...\n...out\n...family\n ...key < ...argument\n",
        "@...\n...out\n...family\n ...key < ...argument\n...bad\n...family\n ...value < .other\n",
        "@...\n...out\n...family\n"
    };
    for (size_t partition = 0; partition < 3; partition++) {
        prepare_usage(graphs[partition], USAGE_CONSTRUCTOR);
        const StaticModule *owner = static_module_find(&inventory, project.entry);
        const StaticSymbol *family = static_symbol_find(owner, static_name_site(project.entry, "...family"));
        const StaticModule *origin = static_module_find(&inventory, project_find(&project, ".other"));
        StaticUsageResult result = static_usage_check(&inventory, family,
            STATIC_CATEGORY_TEMPLATE, origin->constructor_signature);
        StaticUsageStatus expected[] = {STATIC_USAGE_PENDING, STATIC_USAGE_CONFLICT, STATIC_USAGE_SATISFIED};
        assert(result.status == expected[partition]);
        if (partition == 1) { assert(layout_text_is(result.site, ".other")); }
        if (partition == 2) {
            assert(static_usage_check(&inventory, family, STATIC_CATEGORY_TYPE, NULL).status == STATIC_USAGE_SATISFIED);
            assert(static_usage_check(&inventory, family, STATIC_CATEGORY_DEFERRED, NULL).status == STATIC_USAGE_SATISFIED);
        }
        assert(family->signature == NULL && family->category == STATIC_CATEGORY_DEFERRED);
        destroy_static();
    }
}

/** The current source consumer rejects a missing member before specialization;
 * graph producers with named outputs retain no ordinary constructor identity.
 */
static void it_checks_known_source_applications_and_separates_static_producers(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/scalar", "@!\n");
    ModuleRoot root = {STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT};
    setup_static_with_roots("@...\n...out\n.other\n ...missing < .scalar\n", USAGE_CONSTRUCTOR, &root, 1);
    assert(static_inventory_collect(&inventory) == STATIC_INVALID);
    assert(layout_text_is(inventory.error.primary.span, "...missing"));
    destroy_static();
    setup_static("@...\n.other\n ...out > ...alias\n", "@...\n...out\n...free\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *producer = static_module_find(&inventory, project_find(&project, ".other"));
    assert(producer->parameter_count == 1 && producer->constructor_signature == NULL);
    const StaticModule *owner = static_module_find(&inventory, project.entry);
    assert(owner->first->category == STATIC_CATEGORY_DEFERRED);
    StaticTermStore terms;
    static_term_store_init(&terms, &inventory, sources.allocator);
    const StaticTerm *value = NULL;
    assert(!static_term_nominal(&terms, producer, NULL, 0, (SourceSpan){0}, &value));
    assert(value == NULL);
    static_term_store_destroy(&terms);
    destroy_static();
}

/** Bare forwarding propagates the complete requirement set through aliases,
 * including cycles; passing the same constructor as a Type operand conflicts.
 */
static void it_propagates_requirements_through_aliases_cycles_and_self_operands(void) {
    const char *graphs[] = {
        "@...\n...alias\n...family\n...out\n...alias\n ...key < .scalar\n",
        "@...\n...out\n...alias\n ...key < .scalar\n...alias\n...family\n",
        "@...\n...family\n...alias\n...alias\n...family\n...out\n...alias\n ...key < .scalar\n",
        "@...\n...alias\n...family\n...out\n...family\n ...key < ...alias\n"
    };
    for (size_t partition = 0; partition < 4; partition++) {
        prepare_usage(graphs[partition], USAGE_CONSTRUCTOR);
        const StaticModule *owner = static_module_find(&inventory, project.entry);
        const StaticSymbol *family = static_symbol_find(owner, static_name_site(project.entry, "...family"));
        const StaticSymbol *alias = static_symbol_find(owner, static_name_site(project.entry, "...alias"));
        const StaticModule *origin = static_module_find(&inventory, project_find(&project, ".other"));
        assert(family->usage_group == alias->usage_group);
        StaticUsageResult applied = static_usage_check(&inventory, family,
            STATIC_CATEGORY_TEMPLATE, origin->constructor_signature);
        assert(applied.status == (partition == 3 ? STATIC_USAGE_CONFLICT : STATIC_USAGE_SATISFIED));
        assert(static_usage_check(&inventory, family, STATIC_CATEGORY_TYPE, NULL).status == STATIC_USAGE_CONFLICT);
        destroy_static();
    }
}

/** Run each independent metadata law; counted teardown checks every fixture. */
static void run(void) {
    it_propagates_requirements_through_aliases_cycles_and_self_operands();
    it_preserves_additional_residuals_captures_and_actual_admission_order();
    it_keeps_pending_and_forwarded_metadata_without_hiding_known_conflicts();
    it_checks_known_source_applications_and_separates_static_producers();
}
/** Return success only after all assertions complete. */
int main(void) { run(); return 0; }
