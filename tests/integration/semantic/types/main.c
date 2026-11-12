#include "../common.c"

static void it_renders_distinct_unicode_module_types_and_both_declarations(void) {
    setup_test();
    SemanticStatus status = semantic_analyze(&semantic);
    assert(status == SEMANTIC_INVALID && !semantic.resolved);
    expect_diagnostic();
    destroy_test();
}

void run(void) {
    it_renders_distinct_unicode_module_types_and_both_declarations();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
