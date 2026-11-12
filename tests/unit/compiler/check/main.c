/**
 * @file
 * @brief Verify shared frontend outputs, source-backed diagnostics and every allocation failure prefix.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Write this scenario's explicit source/native fixtures in test-owned storage before loading or
 * compiling them; the caller controls their lifetime and expected result.
 */
static void prepare_project(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.value\n |literal\n");
    file("generated/project/value", "@C\n");
    file("generated/project/value.h", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.c", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n");
}

/**
 * Assert that the component returns the same typed graph that compilation will consume without
 * running c. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid && frontend.valid && frontend.plan.valid && frontend.semantic.constants_valid);
    assert(frontend.plan.entry->first->symbol->initializers->transfer == TRANSFER_LITERAL);
    assert(frontend.plan.entry->type->module == frontend.project.entry);
    destroy_frontend();
}

/**
 * Assert that the component preserves source semantic and relation diagnostics. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_preserves_source_semantic_and_relation_diagnostics(void) {
    const char *cases[] = {"@!\n invalid\n", "@!\nmissing\n.missing\n", "@!\n^\n", "@!\nself\n.main\n"};
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        prepare_project();
        file("generated/project/main", cases[index]);
        setup_frontend();
        bool valid = check_project();
        assert(!valid && !frontend.valid && diagnostic_valid(&frontend.error));
        destroy_frontend();
    }
}

/**
 * Assert that the component unwinds every frontend allocation failure. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_unwinds_every_frontend_allocation_failure(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid);
    size_t total = allocation_count;
    destroy_frontend();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_frontend();
        failure_at = failure;
        valid = check_project();
        assert(!valid && frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_frontend();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c();
    it_preserves_source_semantic_and_relation_diagnostics();
    it_unwinds_every_frontend_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
