/** @file
 * @brief Fail every ordinary-binding semantic allocation after real source/native setup and verify complete owner teardown.
 */
/** Every counted semantic stage allocation has one independently injected failure; literal and indexed input/output views are included. */
static void it_unwinds_each_allocation_in_complete_ordinary_binding_resolution(void) {
    prepare_types();
    file("generated/program/packages/test/fixture/v1.0.0/src/worker", "@<\ninput\n/test/fixture/v1.0.0/src/value\nunused\n/test/fixture/v1.0.0/src/resource\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@!\n");
    const char *text = "@!\nworker\n/test/fixture/v1.0.0/src/worker\nindex\n/test/fixture/v1.0.0/src/value\n |0\nvalues[]\n/test/fixture/v1.0.0/src/value\n |1\n"
        "worker !\n < values[index]\n <\n > values[index]\nworker !\n < |bytes\n > values[index]\n"
        "worker %\n > values[index]\nworker\n >> values[index]\n";
    SemanticStatus status = resolve_text(text);
    assert(status == SEMANTIC_OK);
    size_t count = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        file("generated/program/packages/test/fixture/v1.0.0/src/main", text);
        setup_semantic();
        status = load_semantic("generated/program");
        assert(status == SEMANTIC_OK);
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OK);
        if (failure <= allocation_count) { destroy_semantic(); continue; }
        failure_at = failure;
        status = semantic_resolve(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.resolved && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
