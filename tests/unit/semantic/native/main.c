/**
 * @file
 * @brief Validate explicit ABI companion protocols and native type obligations.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <unistd.h>
#include "../common.c"

/**
 * Write the explicit native module and companion contract fixtures for this semantic/authoring
 * scenario.
 */
static void prepare_native(unsigned mask) {
    directory("generated/native");
    file("generated/native/main", "@!\nchild\n.value\n");
    file("generated/native/value", "@C\n");
    const char *paths[] = {"generated/native/value.h", "generated/native/value.c",
        "generated/native/value.abi"};
    for (size_t index = 0; index < 3; index++) {
        int removed = unlink(paths[index]);
        assert(removed == 0 || errno == ENOENT);
        if ((mask & (1u << index)) != 0) {
            file(paths[index], index == 2 ? "+ initialize\n- destroy\n| decode\n" : "/* native input */\n");
        }
    }
}

/**
 * Assert that the component checks every combination of missing required companions. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_checks_every_combination_of_missing_required_companions(void) {
    for (unsigned mask = 0; mask < 8; mask++) {
        prepare_native(mask);
        setup_semantic();
        SemanticStatus status = load_semantic("generated/native");
        assert(status == SEMANTIC_OK && module_count() == 2);
        status = semantic_native(&semantic);
        assert(status == (mask == 7 ? SEMANTIC_OK : SEMANTIC_INVALID));
        assert(semantic.native_valid == (mask == 7));
        if (mask != 7) {
            assert(diagnostic_valid(&semantic.error));
            assert(layout_text_is(semantic.error.primary.span, "@C"));
            assert(strcmp(semantic.error.subject, ".value") == 0);
        }
        destroy_semantic();
    }
}

/**
 * Assert that the component parses required abi and keeps helper names module local. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_parses_required_abi_and_keeps_helper_names_module_local(void) {
    prepare_native(7);
    file("generated/native/main", "@<\nchild\n.value\n@C\n");
    file("generated/native/main.h", "/* header */\n");
    file("generated/native/main.c", "/* body */\n");
    file("generated/native/main.abi", "+ initialize\n- destroy\n");
    file("generated/native/draft", "unfinished");
    file("generated/native/draft.abi", "invalid but unused ABI grammar\n");
    setup_semantic();
    SemanticStatus status = load_semantic("generated/native");
    assert(status == SEMANTIC_OK && module_count() == 2);
    status = semantic_native(&semantic);
    assert(status == SEMANTIC_OK);
    assert(layout_text_is(semantic_entry()->abi.bindings[ABI_DEFAULT].function, "initialize"));
    assert(layout_text_is(semantic.last->abi.bindings[ABI_DEFAULT].function, "initialize"));
    assert(semantic_entry()->abi.bindings[ABI_DEFAULT].function.source
        != semantic.last->abi.bindings[ABI_DEFAULT].function.source);
    destroy_semantic();
    file("generated/native/main", "@!\nchild\n.value\n");
    file("generated/native/value.abi", "+ initialize\n- destroy\n= copy\n= other\n");
    setup_semantic();
    status = load_semantic("generated/native");
    assert(status == SEMANTIC_OK);
    status = semantic_native(&semantic);
    assert(status == SEMANTIC_INVALID && diagnostic_valid(&semantic.error));
    assert(semantic.error.related_count == 1);
    destroy_semantic();
}

/**
 * Assert that the component cleans every native contract allocation failure. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_cleans_every_native_contract_allocation_failure(void) {
    prepare_native(7);
    setup_semantic();
    SemanticStatus status = load_semantic("generated/native");
    assert(status == SEMANTIC_OK);
    size_t before = allocation_count;
    status = semantic_native(&semantic);
    assert(status == SEMANTIC_OK);
    size_t count = allocation_count - before;
    destroy_semantic();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_semantic();
        status = load_semantic("generated/native");
        assert(status == SEMANTIC_OK);
        failure_at = allocation_count + failure;
        status = semantic_native(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.native_valid);
        destroy_semantic();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_checks_every_combination_of_missing_required_companions();
    it_parses_required_abi_and_keeps_helper_names_module_local();
    it_cleans_every_native_contract_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
