/** @file
 * @brief Check complete owner and source correspondence through specialized logical storage.
 * @see [T187 contract](../../../../../docs/implementation/specialized-planning.md)
 */

/** Equal keys share one plan, different bindings have distinct plans, and each owned instance keeps its own slot. */
static void it_preserves_full_specialization_identity_and_all_storage_forms_without_static_runtime_objects(void) {
    setup_specializations();
    ProgramPlan *plan = &frontend.plan;
    StorageField *first = fixture_field(plan->entry, "first");
    StorageField *second = fixture_field(plan->entry, "second");
    StorageField *again = fixture_field(plan->entry, "again");
    assert(first->type == again->type && first->type != second->type);
    assert(first != again && first->slot != again->slot && first->form == STORAGE_OWNED && again->form == STORAGE_OWNED);
    assert(first->type->type->module == second->type->type->module);
    PlannedType *workers[] = {first->type, second->type};
    PlannedType *scalars[] = {fixture_field(plan->entry, "number")->type, fixture_field(plan->entry, "text")->type};
    const char *names[] = {"input", "output", "owned", "reference", "items", "references"};
    StorageForm forms[] = {STORAGE_OWNED, STORAGE_OWNED, STORAGE_OWNED, STORAGE_REFERENCE, STORAGE_OWNING_LIST, STORAGE_REFERENCE_LIST};
    for (size_t binding = 0; binding < 2; binding++) {
        PlannedType *worker = workers[binding];
        assert(worker->requires_pool && worker->stable_slots && worker->service_storage);
        assert(worker->field_count == 6 && worker->cell_count == 1 && worker->frame_count == 1);
        for (size_t index = 0; index < 6; index++) {
            StorageField *field = fixture_field(worker, names[index]);
            assert(field->type == scalars[binding] && field->form == forms[index] && field->slot == index);
            assert(field->symbol->owner == worker->type && field->symbol->original);
            assert(field->symbol->name.source == worker->type->module->source->text);
        }
        assert(fixture_field(worker, "input")->symbol->role_ordinal == 0);
        assert(fixture_field(worker, "output")->symbol->role_ordinal == 0);
        assert(worker->cells->symbol->type == scalars[binding]->type);
    }
    size_t worker_count = 0, pool_count = 0;
    const SemanticType *semantic = frontend.semantic.first;
    for (PlannedType *type = plan->first; type; type = type->next) {
        assert(type->type == semantic && semantic->module->syntax.sections.kind != MODULE_STATIC);
        assert(semantic->specialization && semantic->specialization->context->registry_use->candidate->state == STATIC_CANDIDATE_CHECKED);
        worker_count += type->type->module == first->type->type->module;
        pool_count += type->requires_pool;
        semantic = semantic->next;
    }
    assert(!semantic && worker_count == 2 && pool_count == plan->context_storage.pools);
    size_t shared = 0;
    for (const SharedStorage *storage = plan->shared_first; storage; storage = storage->next) {
        if (storage->constant->symbol->owner->module != first->type->type->module) { continue; }
        assert(storage->immutable && !storage->service_storage && storage->form == STORAGE_OWNING_LIST);
        assert(storage->value_type == scalars[0] || storage->value_type == scalars[1]);
        shared++;
    }
    assert(shared == 2);
    destroy_frontend();
}
