/**
 * @file
 * @brief Compose standard-root scalar library modules through real generated execution.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../execution/common.c"

/**
 * Assert that the component composes scalar operations with consistent positional interfaces.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_composes_scalar_operations_with_consistent_positional_interfaces(void) {
    directory("generated/project");
    native_fixture("showindex", "@<\nvalue\n..std.index\n@C\n");
    execute_project("@!\nleft\n..std.index\n |12\nright\n..std.index\n |30\nresult\n..std.index\ncondition\n..std.condition\nadd\n..std.index.add\nnext\n..std.index.next\nbefore\n..std.index.before\nshow\n.showindex\nadd !\n < left\n < right\n > result\nshow !\n < result\nnext !\n < result\n > result\nbefore !\n < left\n < result\n > condition\ncondition\n?\n show !\n  < result\n", "value=42\nvalue=43\n", true);
}

/**
 * Assert that the component rejects checked scalar overflow and invalid conditions. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_rejects_checked_scalar_overflow_and_invalid_conditions(void) {
    execute_project("@!\nindex\n..std.index\n |99999999999999999999999999999999999999\n", "", false);
    expect_diagnostic("field construction, type ..std.index");
    execute_project("@!\ncondition\n..std.condition\n |2\n", "", false);
    expect_diagnostic("field construction, type ..std.condition");
}

/**
 * Assert that the component keeps result first and named inputs independent of textual order.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_keeps_result_first_and_named_inputs_independent_of_textual_order(void) {
    directory("generated/project");
    native_fixture("showindex", "@<\nvalue\n..std.index\n@C\n");
    execute_project("@!\nleft\n..std.index\n |2\nright\n..std.index\n |3\nresult\n..std.index\ncondition\n..std.condition\nnext\n..std.index.next\nbefore\n..std.index.before\nshow\n.showindex\nnext !\n <\n > result\nshow !\n < result\nbefore !\n right < left\n left < right\n result > condition\ncondition\n?\n show !\n  < left\n:\n show !\n  < right\n", "value=1\nvalue=3\n", true);
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_keeps_result_first_and_named_inputs_independent_of_textual_order();
    it_composes_scalar_operations_with_consistent_positional_interfaces();
    it_rejects_checked_scalar_overflow_and_invalid_conditions();
}
/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) { run(); return EXIT_SUCCESS; }
