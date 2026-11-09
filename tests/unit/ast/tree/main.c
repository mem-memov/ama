#include "../common.c"
#include <stddef.h>

static void it_keeps_owned_nodes_stable_and_aligned_during_growth(void) {
    setup_test();
    AstStatement* first = ast_allocate(&module, sizeof(*first), &error);
    assert(first != NULL && (uintptr_t)first % _Alignof(max_align_t) == 0);
    first->kind = AST_RETURN;
    ast_append_statement(&module.body, first);
    module.validated = true;
    for (int index = 0; index < 1000; index++) {
        AstStatement* next = ast_allocate(&module, sizeof(*next), &error);
        assert(next != NULL);
        ast_append_statement(&module.body, next);
    }
    assert(module.body.first == first && first->kind == AST_RETURN && !module.validated);
    destroy_test();
}

static void it_destroys_one_module_without_affecting_another(void) {
    setup_test();
    AstModule other = {0};
    int* first = ast_allocate(&module, sizeof(*first), &error);
    int* second = ast_allocate(&other, sizeof(*second), &error);
    assert(first && second);
    *first = 5;
    *second = 7;
    ast_destroy(&module);
    assert(module.allocations == NULL && module.body.first == NULL && *second == 7);
    ast_destroy(&other);
    destroy_test();
}

static void it_preserves_existing_nodes_on_rejected_allocations_and_resets_for_reuse(void) {
    setup_test();
    int* value = ast_allocate(&module, sizeof(*value), &error);
    assert(value != NULL);
    *value = 42;
    void* rejected = ast_allocate(&module, SIZE_MAX, &error);
    assert(rejected == NULL && error.error_code == ERROR_LIMIT && *value == 42);
    ast_destroy(&module);
    error_clear(&error);
    value = ast_allocate(&module, sizeof(*value), &error);
    assert(value && *value == 0);
    destroy_test();
}

void run(void) {
    it_keeps_owned_nodes_stable_and_aligned_during_growth();
    it_destroys_one_module_without_affecting_another();
    it_preserves_existing_nodes_on_rejected_allocations_and_resets_for_reuse();
}

int main(void) {
    run();
    printf("unit/ast/tree: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
