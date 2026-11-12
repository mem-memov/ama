/**
 * @file
 * @brief Verify source-backed semantic failures and related declarations.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../typed-common.c"

/**
 * Assert that the component reports both nominal types and original declarations. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_reports_both_nominal_types_and_original_declarations(void) {
    prepare_types();
    SemanticStatus status = resolve_text("@!\nsource\n.value\ndestination\n.other\ndestination\n source\n");
    assert(status == SEMANTIC_INVALID);
    assert(strcmp(semantic.error.subject, ".value") == 0);
    assert(strcmp(semantic.error.expected, ".other") == 0);
    assert(semantic.error.related_count == 2 && diagnostic_valid(&semantic.error));
    assert(semantic.error.related[0].location.span.start == 3);
    assert(layout_text_is(semantic.error.related[0].location.span, "source\n.value"));
    assert(layout_text_is(semantic.error.related[1].location.span, "destination\n.other"));
    destroy_semantic();
}

/**
 * Assert that the component preserves callee parameter context for literal capability errors.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_preserves_callee_parameter_context_for_literal_capability_errors(void) {
    prepare_types();
    file("generated/program/worker", "@<\ninput\n.resource\n@!\n");
    SemanticStatus status = resolve_text("@!\nworker\n.worker\nworker!\n input < |payload\n");
    assert(status == SEMANTIC_INVALID && strcmp(semantic.error.subject, ".resource") == 0);
    assert(semantic.error.related_count == 2);
    assert(layout_text_is(semantic.error.related[0].location.span, "input\n.resource"));
    assert(strcmp(semantic.error.related[1].name, ".resource") == 0);
    assert(diagnostic_valid(&semantic.error));
    destroy_semantic();
}

/**
 * Assert that the component identifies a noncopyable native seed inside a composed type. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_identifies_a_noncopyable_native_seed_inside_a_composed_type(void) {
    prepare_types();
    file("generated/program/worker", "@!\nchildren[]\n.worker\nresource\n.resource\n");
    SemanticStatus status = resolve_text("@!\nsource\n.worker\ndestination\n.worker\ndestination\n source\n");
    assert(status == SEMANTIC_INVALID && strcmp(semantic.error.subject, ".worker") == 0);
    assert(semantic.error.related_count == 2);
    assert(strcmp(semantic.error.related[1].name, ".resource") == 0);
    SemanticType *type = semantic_find_type(&semantic, ".worker");
    assert(type->clone_seed == semantic_find_type(&semantic, ".resource"));
    assert(type->clone_cause != NULL);
    destroy_semantic();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_reports_both_nominal_types_and_original_declarations();
    it_preserves_callee_parameter_context_for_literal_capability_errors();
    it_identifies_a_noncopyable_native_seed_inside_a_composed_type();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
