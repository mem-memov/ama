/** @file
 * @brief Check constant visibility and every allocation in nested shared construction, cycle diagnostics and startup-plan publication.
 */
/** Resolve one complete nested-construction or actual-value cycle fixture before graph allocation injection. */
static SemanticStatus resolve_constant_failure_shape(bool cyclic) {
    prepare_types();
    file("generated/program/worker", "@!\nvalue\n.value\n .palette.value\n");
    file("generated/program/palette", cyclic
        ? "@=\nvalue\n.value\n .main.first\n"
        : "@=\nvalue\n.value\n |4\n");
    return resolve_text(cyclic
        ? "@=\nfirst\n.value\n .palette.value\n@!\n"
        : "@=\nfirst\n.worker\nsecond\n.worker\n first\n@!\n");
}
/** Fail each graph/work-list/notes/plan allocation after successful source/native/typed resolution; partial owners never become valid. */
static void it_unwinds_every_nested_shared_graph_cycle_and_plan_allocation(void) {
    for (size_t cyclic = 0; cyclic < 2; cyclic++) {
        SemanticStatus status = resolve_constant_failure_shape(cyclic != 0);
        assert(status == SEMANTIC_OK);
        size_t before = allocation_count;
        status = semantic_relations(&semantic);
        assert(status == (cyclic ? SEMANTIC_INVALID : SEMANTIC_OK));
        if (!cyclic) {
            status = semantic_constants(&semantic);
            assert(status == SEMANTIC_OK && semantic.constants_valid);
        }
        size_t count = allocation_count - before;
        destroy_semantic();
        for (size_t failure = 1; failure <= count; failure++) {
            status = resolve_constant_failure_shape(cyclic != 0);
            assert(status == SEMANTIC_OK);
            failure_at = allocation_count + failure;
            status = semantic_relations(&semantic);
            if (status == SEMANTIC_OK) { status = semantic_constants(&semantic); }
            assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.constants_valid);
            assert(diagnostic_valid(&semantic.error));
            destroy_semantic();
        }
    }
}
/** Qualification cannot bypass local initialization order, expose instance fields or reinterpret shared storage flags. */
static void it_rejects_invalid_constant_members_forms_and_forward_reads(void) {
    prepare_types();
    file("generated/program/worker", "@=\nshared\n.value\n |1\nitems[]\n.value\n |2\n@>\nresult\n.value\n@!\nstate\n.value\n");
    const char *invalid[] = {
        "@=\nfirst\n.value\n .main.later\nlater\n.value\n |2\n@!\n",
        "@=\nfirst\n.value\n later\nlater\n.value\n |2\n@!\n",
        "@=\nfirst\n.value\n .worker.result\n@!\n",
        "@=\nfirst\n.value\n .worker.state\n@!\n",
        "@=\nfirst\n.value\n .worker.absent\n@!\n",
        "@=\nfirst\n.value\n .worker.items\n@!\n",
        "@=\nfirst\n.value\n .worker.shared[]\n@!\n",
        "@=\nfirst\n.value\n &second\n@!\nsecond\n.value\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) { expect_resolved(invalid[index], false); }
}
