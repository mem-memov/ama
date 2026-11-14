/** @file
 * @brief Compile inferred list receivers, independent values and aliased targets.
 */
#include "../execution/common.c"

/** Inferred owned results are independent copies; duplicate reference results
 * select the same actual continuation without cloning or inventing an instance.
 */
static void it_copies_inferred_lists_and_preserves_duplicate_reference_targets(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    file("generated/project/producer", "@>\nseed\n.task\nvalues[]\n.value\n |3\n |5\n"
        "references&[]\n.task\n &seed\n &seed\n@!\nseed !\n");
    execute_project("@!\nproducer\n.producer\nindex\n.value\n |0\nnext\n.value\n |1\n"
        "changed\n.value\n |41\nprobe\n.worker\nproducer !\n values[] > copies[]\n references&[] > aliases&[]\n"
        "copies[index]\n changed\nproducer\n values[] >> retained[]\nprobe !\n < copies[index]\n"
        "probe !\n < retained[index]\naliases&[index] %\naliases&[next] %\n:\n probe !\n  < |99\n",
        "1:1:0\n41:1:0\n3:2:0\n2:2:0\n99:3:10\n", true);
}

/** Empty and stale list targets refuse locally before their bodies run, while
 * failed native index conversion is a technical error and bypasses handlers.
 */
static void it_checks_list_targets_and_index_conversion_before_execution(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nprobe\n.worker\nprobe !\n < |1\n");
    execute_project("@!\nempty&\n.task\nreferences&[]\n.task\n empty&\nindex\n.value\n |0\n"
        "probe\n.worker\nreferences&[index] !\n:\n probe !\n  < |7\n", "7:1:7\n", true);
    file("generated/project/box", "@>\nchild&\n.task\n@!\nowned\n.task\nchild&\n &owned\n");
    execute_project("@!\nbox\n.box\nreplacement\n.box\nempty&\n.task\nreferences&[]\n.task\n empty&\n"
        "index\n.value\n |0\nprobe\n.worker\nbox !\n > saved&\nreferences&[index]\n saved&\n"
        "box\n replacement\nreferences&[index] !\n:\n probe !\n  < |8\n", "8:1:8\n", true);
    execute_project("@!\nitems[]\n.worker\nindex\n.value\n |9999\nprobe\n.worker\nitems[index] !\n:\n probe !\n",
        "", false);
    expect_diagnostic("Index decoder failed");
}

/** Run every source-to-compiled-runtime list value scenario explicitly. */
void run(void) {
    it_copies_inferred_lists_and_preserves_duplicate_reference_targets();
    it_checks_list_targets_and_index_conversion_before_execution();
}

/** Return success after all generated programs and expected diagnostics pass. */
int main(void) { run(); return EXIT_SUCCESS; }
