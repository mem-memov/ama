/** @file
 * @brief Verify concrete lifecycle field order, reference copying and distinct ordinary/continuation copy policies.
 * @see [T187 contract](../../../../../docs/implementation/specialized-planning.md)
 */

/** Owned fields clone, references copy, and only continuation recipes preserve positions; cleanup uses concrete reverse fields. */
static void it_preserves_concrete_lifecycle_ownership_and_explicit_saved_position_modes(void) {
    setup_specializations();
    const char *names[] = {"first", "second"};
    for (size_t binding = 0; binding < 2; binding++) {
        PlannedType *worker = fixture_field(frontend.plan.entry, names[binding])->type;
        const StorageField *field = worker->first;
        for (const LifeNode *node = worker->create.first; node; node = node->next) {
            assert(node->type == worker && node->failure == worker->create.failure);
            if (node->operation != LIFE_CONSTRUCT_FIELD) { continue; }
            assert(node->field == field && node->site.source == field->symbol->site.source);
            field = field->next;
        }
        assert(!field);
        field = worker->last;
        for (const LifeNode *node = worker->destroy.first; node; node = node->next) {
            if (node->operation != LIFE_DESTROY_FIELD) { continue; }
            assert(node->type == worker && node->field == field);
            field = field->previous;
        }
        assert(!field && worker->destroy.accumulates_errors);
        const LifeProcedure *copies[] = {&worker->clone, &worker->continuation};
        for (size_t mode = 0; mode < 2; mode++) {
            size_t owned = 0, references = 0, cleared = 0, preserved = 0;
            for (const LifeNode *node = copies[mode]->first; node; node = node->next) {
                assert(node->type == worker && node->failure == copies[mode]->failure);
                if (node->operation == LIFE_CLONE_OWNED || node->operation == LIFE_COPY_REFERENCE) {
                    assert(node->field && node->field->symbol->owner == worker->type);
                    bool reference = node->field->form == STORAGE_REFERENCE || node->field->form == STORAGE_REFERENCE_LIST;
                    assert((node->operation == LIFE_COPY_REFERENCE) == reference);
                }
                owned += node->operation == LIFE_CLONE_OWNED;
                references += node->operation == LIFE_COPY_REFERENCE;
                cleared += node->operation == LIFE_CLEAR_POSITIONS;
                preserved += node->operation == LIFE_PRESERVE_POSITIONS;
            }
            assert(owned == 4 && references == 2 && cleared == (mode == 0) && preserved == (mode == 1));
            assert(copies[mode]->failure->operation == LIFE_UNWIND_PARTIAL);
            assert(copies[mode]->failure->next->operation == LIFE_RELEASE_CLONE_MAP);
            assert(copies[mode]->failure->next->next->operation == LIFE_FAILURE);
        }
    }
    destroy_frontend();
}
