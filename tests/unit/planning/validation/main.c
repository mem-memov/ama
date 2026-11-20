/** @file
 * @brief Corrupt completed typed IR deliberately and require rejection before publication.
 */
#include "../common.c"
/** Validate cyclic executable edges through inventories without executing or allocating. */
static void it_validates_cyclic_ir_without_allocation_or_changing_published_identity(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nother&\n/test/fixture/v1.0.0/src/main\nother& !\n");
    assert(status == PLAN_OK);
    size_t before = allocation_count;
    bool valid = program_plan_validate(&plan);
    assert(valid && plan.valid && before == allocation_count);
    assert(find_control(plan.entry, CONTROL_ENTER_INSTANCE)->next == plan.entry->entry);
    destroy_plan();
}
/** Independent mutations cover exact nominal/storage identity, source maps, operations and all graph edge families. */
static void it_rejects_unresolved_or_foreign_ir_operands_and_revokes_publication(void) {
    for (size_t shape = 0; shape < 26; shape++) {
        prepare_types();
        file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nparameter\n/test/fixture/v1.0.0/src/value\n@>\nresult\n/test/fixture/v1.0.0/src/value\n@!\n");
        PlanStatus status = plan_text("@!\nworker\n/test/fixture/v1.0.0/src/worker\nvalue\n/test/fixture/v1.0.0/src/value\nworker !\n < value\n > value\n:\n%\n");
        assert(status == PLAN_OK);
        PlannedType foreign_type = {0};
        ControlNode foreign_control = {0};
        LifeNode foreign_life = {0};
        ControlNode *acquire = find_control(plan.entry, CONTROL_ACQUIRE_INSTANCE);
        ControlNode *connection = find_control(plan.entry, CONTROL_SET_RETURN);
        switch (shape) {
            case 0: plan.entry = &foreign_type; break;
            case 1: plan.entry->first->slot++; break;
            case 2: plan.entry->first->type = &foreign_type; break;
            case 3: plan.entry->first->form = STORAGE_REFERENCE; break;
            case 4: acquire->next = &foreign_control; break;
            case 5: acquire->alternative = &foreign_control; break;
            case 6: acquire->failure = plan.entry->complete; break;
            case 7: connection->resume = &foreign_control; break;
            case 8: connection->callee = &foreign_type; break;
            case 9: acquire->selector = 0; break;
            case 10: acquire->selector = connection->selector; break;
            case 11: acquire->statement = NULL; break;
            case 12: acquire->site = (SourceSpan){0}; break;
            case 13: acquire->operation = (ControlOperation)999; break;
            case 14: plan.entry->create.first->failure = &foreign_life; break;
            case 15: plan.entry->create.first->field = NULL; break;
            case 16: plan.entry->create.first->type = &foreign_type; break;
            case 17: plan.entry->create.first->operation = (LifeOperation)999; break;
            case 18: find_control(plan.entry, CONTROL_INSTALL_PARAMETER)->field = NULL; break;
            case 19: find_control(plan.entry, CONTROL_INSTALL_PARAMETER)->value_type = semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/main"); break;
            case 20: find_control(plan.entry, CONTROL_INITIALIZE_RESULT)->position++; break;
            case 21: find_control(plan.entry, CONTROL_INITIALIZE_RESULT)->value_form = STORAGE_REFERENCE; break;
            case 22: find_control(plan.entry, CONTROL_ENTER_INSTANCE)->callee = plan.entry->first->next->type; break;
            case 23: find_control(plan.entry, CONTROL_TRANSFER_OUTPUT)->value_type = NULL; break;
            case 24: find_control(plan.entry, CONTROL_EVALUATE_INPUT)->field = NULL; break;
            case 25: find_control(plan.entry, CONTROL_TRANSFER_OUTPUT)->position++; break;
        }
        size_t before = allocation_count;
        bool valid = program_plan_validate(&plan);
        assert(!valid && !plan.valid && plan.status == PLAN_INVALID && diagnostic_valid(&plan.error));
        assert(before == allocation_count);
        destroy_plan();
    }
}
/** Native hook operands must be exact registered ABI members, not same-spelled invented descriptors. */
static void it_rejects_unregistered_native_hooks_and_foreign_lifecycle_endpoints(void) {
    for (size_t shape = 0; shape < 2; shape++) {
        prepare_types();
        PlanStatus status = plan_text("@!\nvalue\n/test/fixture/v1.0.0/src/value\n");
        assert(status == PLAN_OK);
        PlannedType *value = program_plan_type(&plan, semantic_find_type(&semantic, "/test/fixture/v1.0.0/src/value"));
        AbiBinding invented = value->type->abi.bindings[ABI_DEFAULT];
        LifeNode foreign = {0};
        if (shape == 0) { value->create.first->binding = &invented; }
        else { value->create.failure = &foreign; }
        bool valid = program_plan_validate(&plan);
        assert(!valid && !plan.valid && plan.status == PLAN_INVALID);
        destroy_plan();
    }
}
/** Shared initialization, service-free storage and required lifecycle endpoints are explicit IR obligations. */
static void it_rejects_incomplete_shared_and_required_procedure_inventories(void) {
    for (size_t shape = 0; shape < 7; shape++) {
        prepare_types();
        PlanStatus status = plan_text("@=\nshared\n/test/fixture/v1.0.0/src/value\n |1\n@!\nvalue\n/test/fixture/v1.0.0/src/value\n");
        assert(status == PLAN_OK);
        switch (shape) {
            case 0: plan.shared_first->immutable = false; break;
            case 1: plan.shared_first->service_storage = STORAGE_BUSY; break;
            case 2: plan.constant_cleanup = NULL; break;
            case 3: plan.shared_first->value_type = plan.entry; break;
            case 4: plan.entry->create.first = NULL; break;
            case 5: plan.entry->clone.first = NULL; break;
            case 6: plan.entry->requires_pool = false; break;
        }
        bool valid = program_plan_validate(&plan);
        assert(!valid && !plan.valid && plan.status == PLAN_INVALID);
        destroy_plan();
    }
}
/** Run independent contract cases through the normal component harness. */
void run(void) {
    it_rejects_incomplete_shared_and_required_procedure_inventories();
    it_validates_cyclic_ir_without_allocation_or_changing_published_identity();
    it_rejects_unresolved_or_foreign_ir_operands_and_revokes_publication();
    it_rejects_unregistered_native_hooks_and_foreign_lifecycle_endpoints();
}
/** Return success only after all explicit assertion cases complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
