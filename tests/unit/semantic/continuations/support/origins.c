/** @file
 * @brief Check exact positive origin proofs and boundaries where reference identity remains runtime state.
 */
/** Fresh direct handle stores and handle-only chains prove alias identity; lexical declarations do not execute between them. */
static void it_rejects_positive_direct_reference_aliases_and_current_root_overlap(void) {
    prepare_types();
    const char *invalid[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\nsecond&\n alias&\nvalue\n % second&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\nlater\n/test/fixture/v1.0.0/src/value\n |7\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nvalue\n?\n alias&\n  &value\n value\n  % alias&\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        SemanticStatus status = resolve_text(invalid[index]);
        assert(status == SEMANTIC_INVALID && !semantic.resolved && semantic.error.related_count == 2);
        assert(strstr(semantic.error.message, "known reference") != NULL && diagnostic_valid(&semantic.error));
        assert(layout_text_is(semantic.error.related[1].location.span, "&value"));
        assert(strcmp(semantic.error.related[1].role, "reference formed here") == 0);
        destroy_semantic();
    }
    SemanticStatus status = resolve_text("@!\nitems[]\n/test/fixture/v1.0.0/src/main\nindex\n/test/fixture/v1.0.0/src/value\n |0\nself&\n &\nitems[index]\n % self&\n");
    assert(status == SEMANTIC_INVALID && strstr(semantic.error.message, "current instance") != NULL);
    assert(semantic.error.related_count == 2 && layout_text_is(semantic.error.related[1].location.span, "&"));
    destroy_semantic();
}
/** Initializers are not current body-state proofs; calls, yields, branch joins and unknown stores discard facts rather than inventing aliases. */
static void it_does_not_infer_aliases_across_blocks_calls_yields_or_unknown_reference_stores(void) {
    prepare_types();
    native_fixture("worker", "+ initialize\n- destroy\n");
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nsource&\n/test/fixture/v1.0.0/src/value\n@>\noutput&\n/test/fixture/v1.0.0/src/value\n@C\n");
    const char *valid[] = {
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n/test/fixture/v1.0.0/src/value\n &value\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < &value\n > alias&\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\n%\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n/test/fixture/v1.0.0/src/value\nvalue\n?\n alias&\n  &value\nvalue\n % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n/test/fixture/v1.0.0/src/value\nvalue\n?\n alias&\n  &value\n:\n value\n  % alias&\n",
        "@!\nvalue\n/test/fixture/v1.0.0/src/value\nalias&\n &value\nunknown&\n/test/fixture/v1.0.0/src/value\nalias&\n unknown&\nvalue\n % alias&\n",
        "@!\nitems[]\n/test/fixture/v1.0.0/src/value\nindex\n/test/fixture/v1.0.0/src/value\n |0\nitems[index]\n % items[index]\n",
        "@!\nitems&[]\n/test/fixture/v1.0.0/src/value\nindex\n/test/fixture/v1.0.0/src/value\n |0\nvalue\n/test/fixture/v1.0.0/src/value\nvalue\n % items&[index]\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) { expect_resolved(valid[index], true); }
}
/** External reference links do not require cloning their resources and are not owned overlap between independently owned peers. */
static void it_keeps_external_references_outside_clone_capability_and_ownership_boundaries(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\nresource&\n/test/fixture/v1.0.0/src/resource\n@!\n");
    const char *valid[] = {
        "@!\nresource\n/test/fixture/v1.0.0/src/resource\nfirst&\n &resource\nsecond&\n first&\n",
        "@!\nfirst\n/test/fixture/v1.0.0/src/worker\nsecond\n/test/fixture/v1.0.0/src/worker\nreference&\n &first\nsecond\n % reference&\n",
        "@!\nfirst\n/test/fixture/v1.0.0/src/worker\nsecond\n/test/fixture/v1.0.0/src/worker\nreference&\n &first\nreference&\n &second\nfirst\n % reference&\n"
    };
    for (size_t index = 0; index < sizeof(valid) / sizeof(*valid); index++) { expect_resolved(valid[index], true); }
}
