/** @file
 * @brief Reject broken replacement groups and release all partial lowering allocations.
 */
/** Operand provenance, single construction choice and phase completeness are independently checked before publication. */
static void it_rejects_damaged_replacement_recipes_and_partial_commit_groups(void) {
    for (size_t shape = 0; shape < 12; shape++) {
        prepare_transfers();
        PlanStatus status = plan_text(transfer_main);
        assert(status == PLAN_OK);
        ReplacementPlan *recipe = plan.entry->replacements;
        while (recipe->context != REPLACE_PARAMETER || recipe->binding == NULL) { recipe = recipe->allocated_next; }
        switch (shape) {
            case 0: recipe->construction = REPLACEMENT_DECLARED_DEFAULT; break;
            case 1: recipe->source = NULL; break;
            case 2: recipe->transfer = TRANSFER_HANDLE; break;
            case 3: recipe->first = recipe->commit; break;
            case 4: recipe->last = recipe->first; break;
            case 5: recipe->commit = recipe->last; break;
            case 6: phase_of(recipe, CONTROL_PREPARE_REPLACEMENT)->replacement = NULL; break;
            case 7: phase_of(recipe, CONTROL_PREPARE_REPLACEMENT)->next = phase_of(recipe, CONTROL_RELEASE_VALUES); break;
            case 8: phase_of(recipe, CONTROL_COMMIT_REPLACEMENT)->alternative = plan.entry->complete; break;
            case 9: find_control(plan.entry, CONTROL_EVALUATE_INPUT)->next = find_control(plan.entry, CONTROL_PREFLIGHT_INPUTS)->next; break;
            case 10: find_control(plan.entry, CONTROL_SET_RETURN)->resume = find_control(plan.entry, CONTROL_RELEASE_TARGET); break;
            case 11: plan.entry->entry = phase_of(recipe, CONTROL_PREPARE_REPLACEMENT); break;
        }
        size_t before = allocation_count;
        bool valid = program_plan_validate(&plan);
        assert(!valid && !plan.valid && plan.status == PLAN_INVALID && diagnostic_valid(&plan.error));
        assert(before == allocation_count);
        destroy_plan();
    }
}
/** Every replacement recipe and phase-node allocation is covered by a fail-at-N complete plan build. */
static void it_unwinds_every_ordered_transfer_lowering_allocation(void) {
    prepare_transfers();
    PlanStatus status = plan_text(transfer_main);
    assert(status == PLAN_OK);
    program_plan_destroy(&plan);
    size_t live_before = live_allocations;
    program_plan_init(&plan, &semantic, sources.allocator);
    size_t before = allocation_count;
    status = program_plan_build(&plan);
    assert(status == PLAN_OK);
    size_t count = allocation_count - before;
    program_plan_destroy(&plan);
    assert(live_allocations == live_before);
    for (size_t failure = 1; failure <= count; failure++) {
        program_plan_init(&plan, &semantic, sources.allocator);
        failure_at = allocation_count + failure;
        status = program_plan_build(&plan);
        assert(status == PLAN_OUT_OF_MEMORY && !plan.valid && diagnostic_valid(&plan.error));
        program_plan_destroy(&plan);
        assert(live_allocations == live_before);
        failure_at = 0;
    }
    destroy_semantic();
}
