/** @file
 * @brief Check constant visibility and every allocation in nested shared construction, cycle diagnostics and startup-plan publication.
 */
/** Resolve one complete nested-construction or actual-value cycle fixture before graph allocation injection. */
static SemanticStatus resolve_constant_failure_shape(bool cyclic) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@!\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/value\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/palette", cyclic
        ? "@=\nvalue\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/first\n"
        : "@=\nvalue\n/test/fixture/v1.0.0/src/value\n |4\n");
    return resolve_text(cyclic
        ? "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/palette/value\n@!\n"
        : "@=\nfirst\n/test/fixture/v1.0.0/src/worker\nsecond\n/test/fixture/v1.0.0/src/worker\n first\n@!\n");
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
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@=\nshared\n/test/fixture/v1.0.0/src/value\n |1\nitems[]\n/test/fixture/v1.0.0/src/value\n |2\n@>\nresult\n/test/fixture/v1.0.0/src/value\n@!\nstate\n/test/fixture/v1.0.0/src/value\n");
    const char *invalid[] = {
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/main/later\nlater\n/test/fixture/v1.0.0/src/value\n |2\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n later\nlater\n/test/fixture/v1.0.0/src/value\n |2\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/result\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/state\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/absent\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/items\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n /test/fixture/v1.0.0/src/worker/shared[]\n@!\n",
        "@=\nfirst\n/test/fixture/v1.0.0/src/value\n & second\n@!\nsecond\n/test/fixture/v1.0.0/src/value\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) { expect_resolved(invalid[index], false); }
}
