/** @file
 * @brief Verify lexical bounds independently of invocation depth, element count and runtime loop repetition.
 */
/** Long selected chains and nested reused traversals each reserve a finite slot per written site without recursive planning. */
static void it_bounds_deep_chain_and_traversal_storage_by_lexical_sites(void) {
    const size_t depth = 128;
    size_t capacity = depth * depth * 3 + depth * 48 + 256;
    char *text = malloc(capacity);
    assert(text != NULL);
    for (size_t shape = 0; shape < 2; shape++) {
        prepare_types();
        if (shape == 0) { file("generated/program/packages/test/fixture/v1.0.0/src/node", "@>\nnext&\n/test/fixture/v1.0.0/src/node\n@!\n"); }
        int written = snprintf(text, capacity, "%s", shape == 0 ? "@!\nroot\n/test/fixture/v1.0.0/src/node\nroot !\n" : "@!\nitems[]\n/test/fixture/v1.0.0/src/value\n");
        assert(written > 0);
        size_t used = (size_t)written;
        for (size_t level = 0; level < depth; level++) {
            written = shape == 0
                ? snprintf(text + used, capacity - used, "%*snext& !\n", (int)(level + 1), "")
                : snprintf(text + used, capacity - used, "%*sitems[item&]\n%*s{\n", (int)level, "", (int)level, "");
            assert(written > 0 && (size_t)written < capacity - used);
            used += (size_t)written;
        }
        if (shape == 1) {
            for (size_t level = depth; level > 0; level--) {
                written = snprintf(text + used, capacity - used, "%*s}\n", (int)(level - 1), "");
                assert(written > 0 && (size_t)written < capacity - used);
                used += (size_t)written;
            }
        }
        SemanticStatus status = resolve_text(text);
        assert(status == SEMANTIC_OK);
        status = semantic_relations(&semantic);
        assert(status == SEMANTIC_OK);
        status = semantic_constants(&semantic);
        assert(status == SEMANTIC_OK);
        program_plan_init(&plan, &semantic, sources.allocator);
        bool built = plan_storage(&plan);
        assert(built && plan.entry->frame_count == depth + (shape == 0));
        assert(plan.entry->cell_count == (shape == 1) && plan.entry->origin_count == (shape == 1));
        const StorageFrame *previous = NULL;
        for (const StorageFrame *frame = plan.entry->frames; frame != NULL; frame = frame->next) {
            assert((shape == 0 ? frame->operation_parent : frame->traversal_parent) == previous);
            previous = frame;
        }
        destroy_plan();
    }
    free(text);
}
/** Intrinsic result storage belongs to caller operation frames, never a synthetic method pool or module variable. */
static void it_keeps_intrinsic_results_in_caller_frames_without_method_instances(void) {
    prepare_types();
    directory("generated/standard");
    file("generated/standard/integer", "@C\n");
    file("generated/standard/integer.h", "#error Storage must not compile native companions\n");
    file("generated/standard/integer.c", "#error Storage must not execute native companions\n");
    file("generated/standard/integer.abi", "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nitems[]\n append !\n  > created&\n  element& !\n length !\n  > size\n");
    setup_project();
    ModuleRoot roots[] = {{"generated/program", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus loaded = project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main");
    assert(loaded == PROJECT_OK);
    semantic_program_init(&semantic, &project, sources.allocator);
    SemanticStatus status = semantic_fixture_check(&semantic);
    assert(status == SEMANTIC_OK);
    program_plan_init(&plan, &semantic, sources.allocator);
    bool built = plan_storage(&plan);
    assert(built && plan.entry->frame_count == 4 && plan.context_storage.pools == 3);
    assert(frame_at(0)->contents == (FRAME_OUTPUT_CONTINUATION | FRAME_TARGET_PROTECTION));
    assert((frame_at(1)->contents & FRAME_INTRINSIC_RESULT) != 0);
    assert((frame_at(1)->contents & FRAME_BODY_RETURN) == 0);
    assert(frame_at(2)->operation_parent == frame_at(1));
    assert((frame_at(2)->contents & FRAME_BODY_RETURN) != 0);
    assert((frame_at(3)->contents & FRAME_INTRINSIC_RESULT) != 0);
    destroy_plan();
}
