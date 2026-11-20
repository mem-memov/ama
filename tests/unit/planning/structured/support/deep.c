/** @file
 * @brief Verify complete deep lowering uses finite lexical records rather than recursive compiler or runtime activations.
 */
/** Selected source paths and nested reused foreach scopes remain bounded by their 128 written sites through full IR validation. */
static void it_lowers_deep_operation_paths_and_traversal_scopes_without_recursive_compiler_calls(void) {
    const size_t depth = 128;
    size_t capacity = depth * depth * 3 + depth * 48 + 256;
    char *text = malloc(capacity);
    assert(text != NULL);
    for (size_t shape = 0; shape < 2; shape++) {
        prepare_structured_types();
        file("generated/program/packages/test/fixture/v1.0.0/src/node", "@>\nnext&\n/test/fixture/v1.0.0/src/node\n@!\n%\n");
        int written = snprintf(text, capacity, "%s", shape == 0 ? "@!\nroot\n/test/fixture/v1.0.0/src/node\nroot !\n" : "@!\nitems[]\n/test/fixture/v1.0.0/src/value\n");
        assert(written > 0);
        size_t used = (size_t)written;
        for (size_t level = 0; level < depth; level++) {
            written = shape == 0 ? snprintf(text + used, capacity - used, "%*snext& !\n", (int)(level + 1), "")
                : snprintf(text + used, capacity - used, "%*sitems[item&]\n%*s{\n", (int)level, "", (int)level, "");
            assert(written > 0 && (size_t)written < capacity - used);
            used += (size_t)written;
        }
        if (shape == 1) {
            written = snprintf(text + used, capacity - used, "%*s%%\n", (int)depth, "");
            assert(written > 0 && (size_t)written < capacity - used);
            used += (size_t)written;
            for (size_t level = depth; level > 0; level--) {
                written = snprintf(text + used, capacity - used, "%*s}\n", (int)(level - 1), "");
                assert(written > 0 && (size_t)written < capacity - used);
                used += (size_t)written;
            }
        }
        PlanStatus status = plan_text(text);
        if (status != PLAN_OK) { fprintf(stderr, "Deep structured IR: %s\n", plan.error.message); }
        assert(status == PLAN_OK && plan.valid);
        assert(plan.entry->frame_count == depth + (shape == 0));
        if (shape == 0) {
            assert(plan.entry->chains != NULL && plan.entry->chains->next == NULL);
            size_t selected = 0;
            for (const ControlNode *node = plan.entry->controls; node != NULL; node = node->allocated_next) {
                if (node->operation == CONTROL_SELECT_RESULT) {
                    assert(node->chain == plan.entry->chains && node->frame->operation_parent != NULL);
                    selected++;
                }
            }
            assert(selected == depth);
        } else {
            assert(plan.entry->cell_count == 1 && plan.entry->origin_count == 1);
            const PlannedLoop *previous = NULL;
            size_t count = 0;
            for (const PlannedLoop *loop = plan.entry->loops; loop != NULL; loop = loop->next) {
                assert(loop->parent == previous && loop->repeat->operation == CONTROL_FOREACH_NEXT);
                previous = loop;
                count++;
            }
            assert(count == depth && find_control(plan.entry, CONTROL_SAVE_POSITION)->resume == previous->repeat);
            assert(plan.entry->resume_entry->operation == CONTROL_VALIDATE_TRAVERSALS);
        }
        destroy_plan();
    }
    free(text);
}
