/**
 * @file
 * @brief Test typed storage/lifecycle/control correspondence and failed-plan cleanup.
 */
#include "../common.c"

/** Self-looking copies retain all selection/check/prepare/commit edges; runtime decides identity. */
static void it_keeps_runtime_replacement_checks_for_identically_spelled_assignments(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nvalue\n.value\nitems[]\n.value\n |3\nindex\n.value\n |0\n"
        "value\n value\nitems[index]\n items[index]\n");
    assert(status == PLAN_OK);
    const ControlOperation expected[] = {CONTROL_SELECT_RECEIVER, CONTROL_SELECT_SOURCE,
        CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT, CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    ControlNode *node = plan.entry->entry;
    for (size_t assignment = 0; assignment < 2; assignment++) {
        for (size_t index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
            assert(node != NULL && node->operation == expected[index]);
            assert(node->statement->transfer == TRANSFER_CLONE);
            node = node->next;
        }
    }
    assert(node == plan.entry->complete);
    destroy_plan();
}

/** Keep immutable constants outside instance slots/service masks while parameter/result/state roles remain explicit. */
static void it_separates_shared_interface_persistent_and_service_storage(void) {
    prepare_types();
    PlanStatus status = plan_text("@=\nconstant\n.value\n |shared\n@<\nparameter\n.value\n"
        "@>\nresult\n.value\n@!\npersistent\n.value\nreferences&[]\n.value\n");
    assert(status == PLAN_OK && plan.valid && plan.entry->field_count == 4);
    StorageField *field = plan.entry->first;
    assert(field->symbol->role == SYMBOL_PARAMETER && field->slot == 0);
    assert(field->next->symbol->role == SYMBOL_RESULT);
    assert(plan.entry->last->form == STORAGE_REFERENCE_LIST);
    assert(plan.entry->service_storage == (STORAGE_GENERATION | STORAGE_BUSY | STORAGE_POSITION
        | STORAGE_RETURN_LINK | STORAGE_REFUSAL | STORAGE_ACQUISITION
        | STORAGE_CONSTRUCTING_IDENTITY | STORAGE_OWNERSHIP));
    assert(plan.constant_startup->symbol->role == SYMBOL_CONSTANT);
    assert(plan.shared_first->constant == plan.constant_startup);
    assert(plan.shared_first->immutable && plan.shared_first->service_storage == 0);
    assert(plan.shared_first->value_type != NULL);
    assert(plan.entry->entry == plan.entry->complete);
    for (field = plan.entry->first; field != NULL; field = field->next) {
        assert(field->symbol->role != SYMBOL_CONSTANT && field->type != NULL);
    }
    destroy_plan();
}

/** Check selected native constructors, field ordering, cleanup edges and explicit continuation-position mode. */
static void it_constructs_fields_before_payload_and_destroys_payload_before_reverse_fields(void) {
    prepare_types();
    native_fixture("worker", "+ initialize\n- destroy\n| literal\n= clone\n");
    file("generated/program/worker", "@<\nfirst\n.value\nsecond&\n.resource\n@C\n");
    PlanStatus status = plan_text("@!\nworker\n.worker\n");
    assert(status == PLAN_OK);
    PlannedType *worker = program_plan_type(&plan, semantic_find_type(&semantic, ".worker"));
    LifeNode *node = worker->create.first;
    assert(node->operation == LIFE_CONSTRUCT_FIELD && node->field == worker->first);
    assert(node->next->operation == LIFE_CONSTRUCT_FIELD && node->next->field == worker->last);
    node = node->next->next;
    assert(node->operation == LIFE_CONSTRUCT_NATIVE && node->binding == &worker->type->abi.bindings[ABI_DEFAULT]);
    node = worker->literal.first->next->next;
    assert(node->operation == LIFE_CONSTRUCT_NATIVE && node->binding == &worker->type->abi.bindings[ABI_LITERAL]);
    node = worker->destroy.first;
    assert(worker->destroy.accumulates_errors && node->operation == LIFE_DESTROY_NATIVE);
    assert(node->next->field == worker->last && node->next->next->field == worker->first);
    assert(worker->create.first->failure == worker->create.failure);
    assert(worker->create.failure->operation == LIFE_UNWIND_PARTIAL);
    assert(worker->clone.first->operation == LIFE_PREFLIGHT_SOURCE);
    assert(worker->clone.first->next->operation == LIFE_BEGIN_CLONE_MAP);
    bool clears = false;
    bool preserves = false;
    for (node = worker->clone.first; node != NULL; node = node->next) {
        clears |= node->operation == LIFE_CLEAR_POSITIONS;
        assert(node->operation != LIFE_PRESERVE_POSITIONS);
    }
    for (node = worker->continuation.first; node != NULL; node = node->next) {
        preserves |= node->operation == LIFE_PRESERVE_POSITIONS;
    }
    assert(clears && preserves);
    destroy_plan();
}

/** Verify resolved text-order input selection precedes exact callee declaration-order installation and return linking. */
static void it_installs_inputs_in_declaration_order_after_source_order_evaluation(void) {
    prepare_types();
    file("generated/program/worker", "@<\nfirst\n.value\nsecond\n.value\n@>\nresult\n.value\n@!\n");
    PlanStatus status = plan_text("@!\nworker\n.worker\nvalue\n.value\nworker !\n"
        " second < value\n first < |literal\n result > value\n");
    assert(status == PLAN_OK);
    ControlNode *node = plan.entry->entry;
    assert(node->operation == CONTROL_ACQUIRE_INSTANCE);
    node = node->next;
    assert(node->operation == CONTROL_EVALUATE_INPUT && node->binding->position == 1);
    node = node->next;
    assert(node->operation == CONTROL_INSTALL_PARAMETER && node->position == 0);
    assert(node->binding->transfer == TRANSFER_LITERAL);
    node = node->next;
    assert(node->operation == CONTROL_INSTALL_PARAMETER && node->position == 1);
    node = node->next;
    assert(node->operation == CONTROL_INITIALIZE_RESULT);
    node = node->next;
    assert(node->operation == CONTROL_SET_RETURN);
    assert(node->resume->operation == CONTROL_TRANSFER_OUTPUT);
    assert(node->resume->next->operation == CONTROL_RELEASE_TARGET);
    assert(node->resume->next->next == plan.entry->complete);
    assert(node->next->operation == CONTROL_ENTER_INSTANCE);
    assert(node->next->next == node->next->callee->entry);
    destroy_plan();
}

/** Retained reads omit execution/reset and resumes omit input construction while retaining explicit output continuation. */
static void it_reads_without_reset_and_resumes_without_input_installation(void) {
    prepare_types();
    file("generated/program/worker", "@>\nresult\n.value\n@!\n");
    PlanStatus status = plan_text("@!\nworker\n.worker\nvalue\n.value\nworker\n >> value\nworker %\n > value\n");
    assert(status == PLAN_OK);
    ControlNode *read = plan.entry->entry;
    assert(read->operation == CONTROL_ACQUIRE_INSTANCE);
    assert(read->next->operation == CONTROL_TRANSFER_OUTPUT);
    ControlNode *resume = read->next->next->next;
    assert(resume->operation == CONTROL_ACQUIRE_INSTANCE);
    assert(resume->next->operation == CONTROL_SET_RETURN);
    assert(resume->next->next->operation == CONTROL_RESUME_INSTANCE);
    assert(resume->next->resume->operation == CONTROL_TRANSFER_OUTPUT);
    assert(find_control(plan.entry, CONTROL_INSTALL_PARAMETER) == NULL);
    destroy_plan();
}

/** Check refusal snapshot/handler edges separately from capture-before-cleanup fatal paths, including empty alternatives. */
static void it_keeps_refusal_alternatives_distinct_from_technical_failure_even_when_empty(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nworker\n.worker\nworker !\n:\n");
    assert(status == PLAN_OK);
    ControlNode *acquire = plan.entry->entry;
    assert(acquire->alternative->operation == CONTROL_REFUSAL_SNAPSHOT);
    assert(acquire->alternative->next == plan.entry->complete);
    assert(acquire->failure == plan.entry->failure);
    assert(plan.entry->failure->operation == CONTROL_CAPTURE_ERROR);
    assert(plan.entry->failure->next->operation == CONTROL_CLEANUP_FAILURE);
    destroy_plan();
    status = plan_text("@!\nworker\n.worker\nworker !\n");
    assert(status == PLAN_OK && plan.entry->entry->alternative == plan.entry->failure);
    destroy_plan();
}

/** Verify retained control successors, loop exits and saved resume labels without recursive C control frames. */
static void it_lowers_loop_break_branch_yield_and_early_return_to_explicit_edges(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\ncondition\n.value\ncondition\n{\n condition\n ?\n  %\n :\n  ^\n}\n>\n");
    assert(status == PLAN_OK);
    ControlNode *loop = plan.entry->entry;
    ControlNode *branch = loop->next;
    assert(loop->operation == CONTROL_CONDITION && branch->operation == CONTROL_CONDITION);
    assert(loop->alternative == plan.entry->complete && branch->alternative == plan.entry->complete);
    assert(branch->next->operation == CONTROL_SAVE_POSITION);
    assert(branch->next->next == plan.entry->yield && branch->next->resume == loop);
    destroy_plan();
}

/** Check every assignment phase in order and preserve source spans/type metadata for emission. */
static void it_retains_receiver_source_preflight_prepare_commit_order_for_assignment(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nfirst\n.value\nsecond\n.value\nfirst\n second\n");
    assert(status == PLAN_OK);
    const ControlOperation expected[] = {CONTROL_SELECT_RECEIVER, CONTROL_SELECT_SOURCE,
        CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT,
        CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    ControlNode *node = plan.entry->entry;
    for (size_t index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
        assert(node != NULL && node->operation == expected[index] && source_span_valid(node->site));
        node = node->next;
    }
    assert(node == plan.entry->complete);
    destroy_plan();
}

/** Enumerate each measured plan allocation failure and require safely destructible partial storage/lifecycle/control records. */
static void it_cleans_partial_lifecycle_storage_and_control_plans_on_every_allocation_failure(void) {
    prepare_types();
    const char *text = "@!\nworker\n.worker\nvalue\n.value\nworker !\n:\n value\n  value\n%\n";
    PlanStatus status = plan_text(text);
    assert(status == PLAN_OK);
    size_t baseline = allocation_count;
    program_plan_destroy(&plan);
    program_plan_init(&plan, &semantic, sources.allocator);
    size_t before = allocation_count;
    status = program_plan_build(&plan);
    assert(status == PLAN_OK);
    size_t count = allocation_count - before;
    assert(baseline > count && count > 0);
    destroy_plan();
    for (size_t failure = 1; failure <= count; failure++) {
        SemanticStatus resolved = resolve_text(text);
        assert(resolved == SEMANTIC_OK);
        resolved = semantic_relations(&semantic);
        assert(resolved == SEMANTIC_OK);
        resolved = semantic_constants(&semantic);
        assert(resolved == SEMANTIC_OK);
        program_plan_init(&plan, &semantic, sources.allocator);
        failure_at = allocation_count + failure;
        status = program_plan_build(&plan);
        assert(status == PLAN_OUT_OF_MEMORY && !plan.valid && diagnostic_valid(&plan.error));
        destroy_plan();
    }
}

/** Build a deep retained control model with explicit lowering work instead of recursive compiler activations. */
static void it_lowers_deep_branch_nesting_without_recursive_c_activations(void) {
    prepare_types();
    const size_t depth = 128;
    size_t capacity = depth * depth * 2 + depth * 20 + 128;
    char *text = malloc(capacity);
    assert(text != NULL);
    int written = snprintf(text, capacity, "@!\ncondition\n.value\n");
    assert(written > 0);
    size_t used = (size_t)written;
    for (size_t level = 0; level < depth; level++) {
        written = snprintf(text + used, capacity - used, "%*scondition\n%*s?\n",
            (int)level, "", (int)level, "");
        assert(written > 0 && (size_t)written < capacity - used);
        used += (size_t)written;
    }
    written = snprintf(text + used, capacity - used, "%*s%%\n", (int)depth, "");
    assert(written > 0 && (size_t)written < capacity - used);
    PlanStatus status = plan_text(text);
    assert(status == PLAN_OK);
    ControlNode *node = plan.entry->entry;
    for (size_t level = 0; level < depth; level++) {
        assert(node->operation == CONTROL_CONDITION && node->alternative == plan.entry->complete);
        node = node->next;
    }
    assert(node->operation == CONTROL_SAVE_POSITION && node->resume == plan.entry->complete);
    destroy_plan();
    free(text);
}

/** Patch cyclic type-level calls only after entries exist, retaining instance selection and explicit return sites. */
static void it_links_same_type_reentry_through_instance_selection_and_return_metadata(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nother&\n.main\nother& !\n");
    assert(status == PLAN_OK);
    ControlNode *acquire = plan.entry->entry;
    assert(acquire->operation == CONTROL_ACQUIRE_INSTANCE);
    assert(acquire->alternative == plan.entry->failure);
    assert(acquire->next->operation == CONTROL_SET_RETURN);
    ControlNode *jump = acquire->next->next;
    assert(jump->operation == CONTROL_ENTER_INSTANCE && jump->callee == plan.entry);
    assert(jump->next == plan.entry->entry);
    assert(acquire->next->resume->operation == CONTROL_RELEASE_TARGET);
    destroy_plan();
}

/** Reject selector exhaustion without wrap and preserve zero as an absent position. */
static void it_reserves_zero_for_absent_control_and_source_positions(void) {
    prepare_types();
    PlanStatus status = plan_text("@!\nvalue\n.value\n");
    assert(status == PLAN_OK);
    for (PlannedType *type = plan.first; type != NULL; type = type->next) {
        for (ControlNode *node = type->controls; node != NULL; node = node->allocated_next) { assert(node->selector != 0); }
    }
    destroy_plan();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_keeps_runtime_replacement_checks_for_identically_spelled_assignments();
    it_links_same_type_reentry_through_instance_selection_and_return_metadata();
    it_reserves_zero_for_absent_control_and_source_positions();
    it_separates_shared_interface_persistent_and_service_storage();
    it_constructs_fields_before_payload_and_destroys_payload_before_reverse_fields();
    it_installs_inputs_in_declaration_order_after_source_order_evaluation();
    it_reads_without_reset_and_resumes_without_input_installation();
    it_keeps_refusal_alternatives_distinct_from_technical_failure_even_when_empty();
    it_lowers_loop_break_branch_yield_and_early_return_to_explicit_edges();
    it_retains_receiver_source_preflight_prepare_commit_order_for_assignment();
    it_lowers_deep_branch_nesting_without_recursive_c_activations();
    it_cleans_partial_lifecycle_storage_and_control_plans_on_every_allocation_failure();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
