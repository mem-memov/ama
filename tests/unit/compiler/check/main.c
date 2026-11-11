#include "../common.c"

static void prepare_project(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.value\n |literal\n");
    file("generated/project/value", "@C\n");
    file("generated/project/value.h", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.c", "#error Checking must not invoke the C compiler\n");
    file("generated/project/value.abi", "+ initialize\n- destroy\n| literal\n= clone\n");
}

static void it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c(void) {
    prepare_project();
    setup_frontend();
    bool valid = check_project();
    assert(valid && frontend.valid && frontend.plan.valid && frontend.semantic.constants_valid);
    assert(frontend.plan.entry->first->symbol->initializers->transfer == TRANSFER_LITERAL);
    assert(frontend.plan.entry->type->module == frontend.project.entry);
    destroy_frontend();
}

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

void run(void) {
    it_returns_the_same_typed_graph_that_compilation_will_consume_without_running_c();
    it_preserves_source_semantic_and_relation_diagnostics();
    it_unwinds_every_frontend_allocation_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
