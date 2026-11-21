/** @file
 * @brief Compile typed append/insert chains with actual direct result execution.
 */
#include "../execution/common.c"
#include "support/integer.c"

/** Direct structural source borrowing uses one index conversion and independent
 * ordinary clones; insertion at zero shifts exact order before later siblings.
 */
static void it_executes_sibling_append_insert_and_direct_structural_sources(void) {
    insertion_standard();
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\n |5\nselector\n/test/fixture/v1.0.0/src/value\n |1000\nzero\n//integer\n |0\none\n//integer\n |1\ntwo\n//integer\n |2\n"
        "replacement\n/test/fixture/v1.0.0/src/value\n |9\nprobe\n/test/fixture/v1.0.0/src/worker\nitems[]\n append !\n  < & items[selector]\n  > added&\n"
        " insert !\n  < zero\n  < added&\n  > inserted&\nitems[one]\n replacement\n"
        "probe !\n < items[zero]\nprobe !\n < items[one]\nprobe !\n < items[two]\n",
        "index=1000\n5:1:0\n9:2:0\n5:3:0\n", true);
}

/** A method's reference result executes its stored yielded instance without an
 * intermediate variable or copied working state, then a captured alias resumes it.
 */
static void it_executes_and_resumes_actual_new_elements_through_method_results(void) {
    insertion_standard();
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    execute_project("@!\ntasks[]\n/test/fixture/v1.0.0/src/task\ntasks[]\n append !\n  element& !\n  > saved&\n"
        " append !\n  element& !\nsaved& %\n", "1:1:0\n1:1:0\n2:2:0\n", true);
}

/** An owning append clears the source's saved position. A nested NoResume handler
 * runs only after the service grant is released; committed insertion remains live.
 */
static void it_clears_cloned_positions_and_releases_the_service_before_a_local_handler(void) {
    insertion_standard();
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/task\ntasks[]\n/test/fixture/v1.0.0/src/task\nzero\n//integer\n |0\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\n"
        "tasks[]\n append !\n  < & source\n  element& %\n  :\n   probe !\n    < |99\n"
        "source %\ntasks[zero] !\ntasks[zero] %\n", "1:1:0\n99:1:10\n2:2:0\n1:2:0\n2:3:0\n", true);
}

/** Invalid insertion position refuses before output transfer and skips siblings. */
static void it_preserves_receivers_on_position_refusal_and_skips_pending_siblings(void) {
    insertion_standard();
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nzero\n//integer\n |0\nprobe\n/test/fixture/v1.0.0/src/worker\nitems[]\n insert !\n  < |1\n  > untouched&\n :\n  probe !\n   < |12\n append !\n"
        "untouched& !\n:\n probe !\n  < |7\nitems[zero] !\n:\n probe !\n  < |88\n"
        "items[]\n append !\n  > kept&\nkept& %\n", "12:1:12\n7:2:7\n88:3:12\n", false);
    expect_diagnostic("Ama runtime failure: 10");
}

/** Native Busy during element construction is terminal, never the method's
 * preflight refusal handler, even though its numeric code is the same.
 */
static void it_does_not_catch_native_constructor_busy_as_a_method_refusal(void) {
    insertion_standard();
    insertion_busy_value = true;
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/value\nprobe\n/test/fixture/v1.0.0/src/worker\nitems[]\n append !\n :\n  probe !\n   < |999\n",
        "construct\n", false);
    expect_diagnostic("Ama runtime failure: 9");
    insertion_busy_value = false;
}

/** Omitted/default source is executable for a genuinely noncopyable native type;
 * explicitly supplying even an empty reference is rejected by static capability.
 */
static void it_accepts_noncopyable_default_elements_and_rejects_explicit_empty_copy_sources(void) {
    insertion_standard();
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("limited", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/limited.abi", "+ initialize\n- destroy\n");
    execute_project("@!\nitems[]\n/test/fixture/v1.0.0/src/limited\nitems[]\n append !\n  element& !\n append !\n  <\n  element& !\n insert !\n  < |0\n  <\n  element& !\n", "", true);
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/limited\nempty&\n/test/fixture/v1.0.0/src/limited\nitems[]\n append !\n  < empty&\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},
        {"generated/standard",NULL,0,MODULE_ROOT_STANDARD}};
    bool checked = compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main");
    assert(!checked && frontend.semantic_initialized && frontend.semantic.status == SEMANTIC_INVALID);
    assert(diagnostic_valid(&frontend.error));
    assert(strstr(frontend.error.message, "An explicit owning-list source requires independently copyable elements") != NULL);
    destroy_frontend();
}

/** Run all actual frontend/IR/C/runtime method scenarios explicitly. */
void run(void) {
    it_executes_sibling_append_insert_and_direct_structural_sources();
    it_executes_and_resumes_actual_new_elements_through_method_results();
    it_clears_cloned_positions_and_releases_the_service_before_a_local_handler();
    it_preserves_receivers_on_position_refusal_and_skips_pending_siblings();
    it_does_not_catch_native_constructor_busy_as_a_method_refusal();
    it_accepts_noncopyable_default_elements_and_rejects_explicit_empty_copy_sources();
    native_hook_override = NULL;
    native_fixture_completion = NULL;
}
/** Return success after emitted programs and cleanup/diagnostics match exactly. */
int main(void) { run(); return EXIT_SUCCESS; }
