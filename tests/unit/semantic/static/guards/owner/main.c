/** @file
 * @brief Verify scoped symbolic owner shapes preserve all original bindings and lifetime boundaries.
 */
#include "../common.c"

/** Generic owner shape binds each original slot to its existing scoped variable;
 * a later equality can normalize it without changing its lexical environment.
 */
static void it_preserves_generic_and_applied_owner_identity_without_inventing_parameters(void) {
    setup_guards(NULL, NULL);
    const StaticGuardOwner *generic, *again, *applied;
    assert(static_guard_owner(&guards, caller_environment, &generic));
    assert(generic->environment == caller_environment && generic->value->category == STATIC_CATEGORY_TYPE);
    assert(generic->value->as.nominal.capture_count == 2 && generic->value->as.nominal.residual_count == 0);
    assert(!generic->value->ground && caller_module->parameter_count == 2);
    size_t before = allocation_count;
    assert(static_guard_owner(&guards, caller_environment, &again));
    assert(again == generic && allocation_count == before && guards.owner_count == 1);
    StaticTermCapture fixed[2];
    for (size_t ordinal = 0; ordinal < 2; ordinal++) {
        fixed[ordinal] = (StaticTermCapture){.formal = application_formal(caller_module, ordinal), .value = integer_type};
    }
    const StaticTerm *concrete;
    assert(static_term_nominal(&terms, caller_module, fixed, 2, (SourceSpan){0}, &concrete));
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&terms, caller_module, concrete, &environment));
    assert(static_guard_owner(&guards, environment, &applied));
    assert(applied != generic && applied->value == concrete && guards.owner_count == 2);
    for (const StaticTermCapture *capture = generic->value->as.nominal.first_capture; capture; capture = capture->next) {
        assert(capture->value->kind == STATIC_TERM_VARIABLE);
        assert(capture->value->as.variable.environment == caller_environment);
        assert(static_constraint_equate(&solver, capture->value, integer_type, capture->formal->site));
    }
    const StaticTerm *normalized;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, generic->value, &normalized, &outcome));
    assert(outcome == STATIC_CONSTRAINT_COMPLETE && normalized == concrete);
    assert(generic->environment == caller_environment && generic->value != normalized);
    destroy_guards();
}

/** Refuse every observed owner preparation allocation, including failures in
 * borrowed term construction; no owner/result publication survives refusal.
 */
static void it_unwinds_every_generic_and_applied_owner_allocation_refusal(void) {
    for (size_t applied = 0; applied < 2; applied++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_guards(NULL, NULL);
            const StaticTermEnvironment *environment = caller_environment;
            if (applied) {
                const StaticModule *integer = integer_type->as.nominal.origin;
                assert(static_term_environment(&terms, integer, integer_type, &environment));
            }
            const StaticGuardOwner *output = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = static_guard_owner(&guards, environment, &output);
            if (!failure) { assert(success && output); total = allocation_count - before; assert(total > 0); }
            else {
                assert(!success && output == NULL && guards.owner_count == 0 && guards.first_owner == NULL);
                assert(guards.status == STATIC_GUARD_OUT_OF_MEMORY);
            }
            failure_at = 0;
            assert(inventory.valid);
            destroy_guards();
        }
    }
}

/** Foreign/counter errors fail before allocation and stay sticky. */
static void it_refuses_foreign_owners_missing_receivers_and_counter_wrap(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_guards(NULL, NULL);
        StaticTermEnvironment copied = *caller_environment;
        const StaticGuardOwner *output = NULL;
        if (partition == 3) { guards.owner_count = SIZE_MAX; }
        size_t before = allocation_count;
        assert(!static_guard_owner(&guards, partition == 0 ? &copied : partition == 1 ? NULL : caller_environment,
            partition == 2 ? NULL : &output));
        assert(output == NULL && allocation_count == before);
        assert(guards.status == (partition == 3 ? STATIC_GUARD_OUT_OF_MEMORY : STATIC_GUARD_INVALID));
        assert(!static_guard_owner(&guards, caller_environment, &output) && allocation_count == before);
        destroy_guards();
    }
}

/** Execute identity, generic normalization, malformed input and all refusal paths. */
static void run(void) {
    it_preserves_generic_and_applied_owner_identity_without_inventing_parameters();
    it_unwinds_every_generic_and_applied_owner_allocation_refusal();
    it_refuses_foreign_owners_missing_receivers_and_counter_wrap();
}
/** Return success after zero-live-allocation teardown. */
int main(void) { run(); return 0; }
