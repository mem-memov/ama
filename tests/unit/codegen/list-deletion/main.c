/** @file
 * @brief Compile actual remove/clear effects, stale aliases and native failure routing.
 */
#include "../execution/common.c"
#include "../list-insertion/support/integer.c"
#include "support/native.c"

/** Ordered removal preserves survivors; clear removes the whole remainder, and
 * a second empty clear succeeds without a disguised remove-at-zero operation.
 */
static void it_removes_exact_owned_entries_and_clears_the_remaining_order(void) {
    deletion_standard();
    execute_project("@!\nzero\n..integer\n |0\none\n..integer\n |1\nitems[]\n.resource\n |1\n |2\n |3\n"
        "saved&\n.resource\n &items[one]\ninspect\n.inspect\nprobe\n.worker\nitems[]\n remove !\n  < one\n"
        "inspect !\n < items[zero]\ninspect !\n < items[one]\nsaved& !\n:\n probe !\n  < |8\n"
        "items[]\n clear !\nitems[zero] !\n:\n probe !\n  < |12\nitems[]\n clear !\nitems[zero] !\n:\n probe !\n  < |13\n",
        "value=1\nvalue=3\n8:1:8\n12:2:12\n13:3:12\n", true);
}

/** A reference list can contain the running caller twice. Removing/clearing its
 * occurrences never acquires or destroys that caller and needs no copy capability.
 */
static void it_retires_reference_occurrences_without_touching_the_running_target(void) {
    deletion_standard();
    execute_project("@!\nreferences&[]\n.main\n &\n &\nprobe\n.worker\nreferences&[]\n remove !\n clear !\n clear !\nprobe !\n < |7\n",
        "7:1:0\n", true);
}

/** All three native credits are consumed in reverse order despite Busy from the
 * middle destructor; the own method handler and duplicate cleanup never execute.
 */
static void it_keeps_native_destruction_busy_technical_and_continues_terminal_cleanup(void) {
    deletion_standard();
    trace_resource_destruction = true;
    execute_project("@!\nitems[]\n.resource\n |1\n |2\n |3\nprobe\n.worker\nitems[]\n clear !\n :\n  probe !\n   < |999\n",
        "drop=3\ndrop=2\ndrop=1\n", false);
    expect_diagnostic("Ama runtime failure: 9");
    trace_resource_destruction = false;
}

/** Removal and clear never require cloning the element type, including their
 * compiled owning default elements and an empty final clear.
 */
static void it_deletes_noncopyable_default_elements_without_cloning(void) {
    deletion_standard();
    native_fixture("limited", "@C\n");
    file("generated/project/limited.abi", "+ initialize\n- destroy\n");
    execute_project("@!\nitems[]\n.limited\nprobe\n.worker\nitems[]\n append !\n  > saved&\n append !\n remove !\n clear !\n clear !\nsaved& !\n:\n probe !\n  < |8\n",
        "8:1:8\n", true);
}

/** Run each source/IR/C/runtime deletion contract explicitly. */
void run(void) {
    it_removes_exact_owned_entries_and_clears_the_remaining_order();
    it_retires_reference_occurrences_without_touching_the_running_target();
    it_keeps_native_destruction_busy_technical_and_continues_terminal_cleanup();
    it_deletes_noncopyable_default_elements_without_cloning();
    native_hook_override = NULL;
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
/** Return success after every compiled program and exact diagnostic check passes. */
int main(void) { run(); return EXIT_SUCCESS; }
