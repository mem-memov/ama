/** @file
 * @brief Refuse cross-capture IR substitutions and every concrete planning allocation without publishing partial plans.
 * @see [T187 contract](../../../../../docs/implementation/specialized-planning.md)
 */
#include <stdint.h>

/** Select a real fixture operation for deliberate corruption, never fabricate a valid control graph. */
static ControlNode *fixture_control(PlannedType *type, ControlOperation operation) {
    for (ControlNode *node = type->controls; node; node = node->allocated_next) {
        if (node->operation == operation) { return node; }
    }
    assert(!"Missing expected fixture control operation");
    return NULL;
}

/** Shared original source sites cannot make a symbol, body or local cell from another capture interchangeable. */
static void it_rejects_cross_capture_storage_control_lifecycle_and_scoped_cell_operands(void) {
    for (size_t shape = 0; shape < 10; shape++) {
        setup_specializations();
        ProgramPlan *plan = &frontend.plan;
        PlannedType *first = fixture_field(plan->entry, "first")->type;
        PlannedType *second = fixture_field(plan->entry, "second")->type;
        ControlNode *entry = fixture_control(plan->entry, CONTROL_ENTER_INSTANCE);
        PlannedType *other = entry->callee == first ? second : first;
        switch (shape) {
            case 0: fixture_field(first, "owned")->type = fixture_field(second, "owned")->type; break;
            case 1: entry->callee = other; break;
            case 2: entry->next = other->entry; break;
            case 3: first->create.first->field = second->create.first->field; break;
            case 4: first->cells->symbol = second->cells->symbol; break;
            case 5: first->layout_ordinal = second->layout_ordinal; break;
            case 6: plan->entry = first; break;
            case 7: plan->constant_startup = NULL; break;
            case 8: first->stable_slots = false; break;
            case 9: fixture_field(first, "output")->symbol = fixture_field(second, "output")->symbol; break;
        }
        size_t before = allocation_count;
        assert(!program_plan_validate(plan) && !plan->valid && plan->status == PLAN_INVALID);
        assert(diagnostic_valid(&plan->error) && before == allocation_count);
        destroy_frontend();
    }
}

/** Reuse immutable admitted semantics while independently refusing every actual plan allocation; teardown restores the exact live-owner baseline. */
static void it_unwinds_every_concrete_planning_allocation_and_refuses_selector_exhaustion(void) {
    setup_specializations();
    ProgramPlan *plan = &frontend.plan;
    program_plan_destroy(plan);
    size_t baseline = live_allocations;
    program_plan_init(plan, &frontend.semantic, sources.allocator);
    size_t before = allocation_count;
    assert(program_plan_build(plan) == PLAN_OK);
    size_t total = allocation_count - before;
    assert(total);
    program_plan_destroy(plan);
    assert(live_allocations == baseline);
    for (size_t cut = 1; cut <= total; cut++) {
        program_plan_init(plan, &frontend.semantic, sources.allocator);
        failure_at = allocation_count + cut;
        assert(program_plan_build(plan) == PLAN_OUT_OF_MEMORY && !plan->valid);
        assert(diagnostic_valid(&plan->error) && plan->error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        failure_at = 0;
        program_plan_destroy(plan);
        assert(live_allocations == baseline && frontend.semantic.constants_valid && frontend.specializations.closure.closed);
    }
    program_plan_init(plan, &frontend.semantic, sources.allocator);
    plan->next_selector = SIZE_MAX;
    assert(program_plan_build(plan) == PLAN_OUT_OF_MEMORY && !plan->valid);
    assert(plan->error.code == DIAGNOSTIC_OUT_OF_MEMORY && plan->next_selector == SIZE_MAX);
    printf("concrete planning allocation refusal positions: %zu\n", total);
    destroy_frontend();
}
