/**
 * @file
 * @brief Check one exact nominal namespace and declaration identity.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"

/**
 * Assert that the component rejects all pairs of duplicate storage forms. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_rejects_all_pairs_of_duplicate_storage_forms(void) {
    directory("generated/names");
    file("generated/names/value", "@C\n");
    const char *suffixes[] = {"", "&", "[]", "&[]"};
    for (size_t first = 0; first < 4; first++) {
        for (size_t second = 0; second < 4; second++) {
            char content[256];
            int length = snprintf(content, sizeof(content),
                "@<\nitem%s\n.value\n@!\nflag\n?\n item%s\n .value\n", suffixes[first], suffixes[second]);
            assert(length > 0 && (size_t)length < sizeof(content));
            file("generated/names/main", content);
            setup_semantic();
            SemanticStatus status = load_semantic("generated/names");
            assert(status == SEMANTIC_INVALID && !semantic.names_valid);
            assert(diagnostic_valid(&semantic.error) && semantic.error.related_count == 1);
            assert(semantic.error.primary.span.start > semantic.error.related[0].location.span.start);
            destroy_semantic();
        }
    }
}

/**
 * Write exact namespace declarations whose duplicate/nominal behavior is checked by the
 * semantic-name cases.
 */
static void prepare_names(void) {
    directory("generated/valid");
    file("generated/valid/main", "@=\nconstant\n.value\n@<\nparameter\n.value\n@>\nresult\n.value\n"
        "@!\nflag\n?\n nested\n .value\n:\n other\n .value\nworker !\n:\n fallback\n .value\n");
    file("generated/valid/value", "@!\nconstant&\n.value\n");
}

/**
 * Assert that the component preserves section and nested declaration order without cross module
 * conflicts. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_preserves_section_and_nested_declaration_order_without_cross_module_conflicts(void) {
    prepare_names();
    setup_semantic();
    SemanticStatus status = load_semantic("generated/valid");
    assert(status == SEMANTIC_OK && semantic.names_valid && module_count() == 2);
    SemanticType *type = semantic.first;
    assert(type->symbol_count == 6 && type->counts[SYMBOL_STATE] == 3);
    const char *names[] = {"constant", "parameter", "result", "nested", "other", "fallback"};
    SemanticSymbol *symbol = type->first;
    for (size_t index = 0; index < 6; index++) {
        assert(symbol != NULL && symbol->ordinal == index);
        assert(layout_text_is(symbol->declaration->as.declaration.name->name, names[index]));
        symbol = symbol->next;
    }
    assert(symbol == NULL && semantic.last->symbol_count == 1);
    destroy_semantic();
}

/**
 * Assert that the component cleans partial namespace storage on every allocation failure. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_cleans_partial_namespace_storage_on_every_allocation_failure(void) {
    prepare_names();
    setup_semantic();
    SemanticStatus status = load_semantic("generated/valid");
    assert(status == SEMANTIC_OK);
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_semantic();
        ProjectStatus loaded = load_root("generated/valid");
        assert(loaded == PROJECT_OK);
        if (failure <= allocation_count) {
            destroy_project();
            continue;
        }
        semantic_program_init(&semantic, &project, sources.allocator);
        failure_at = failure;
        status = semantic_names(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_rejects_all_pairs_of_duplicate_storage_forms();
    it_preserves_section_and_nested_declaration_order_without_cross_module_conflicts();
    it_cleans_partial_namespace_storage_on_every_allocation_failure();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
