/** @file
 * @brief Execute equal-key and distinct-key instances through references, yields and explicit continuation copies.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** A shared body cannot merge native state or saved positions; ordinary copies clear positions while explicit snapshots preserve them. */
static void it_preserves_independent_captured_instances_references_and_explicit_continuation_copies(void) {
    directory("generated/project");
    native_fixture("other", "@C\n");
    file("generated/project/factory", "@...\n...numbers\n.task\n ...slot < .value\n...others\n.task\n ...slot < .other\n");
    file("generated/project/task", "@<\ninput\n...slot\n@!\nprobe\n.worker\nprobe !\n < |10\n%\nprobe !\n < |20\n%\nprobe !\n < |30\n");
    execute_project("@!\nnumber\n.value\n |1\nother\n.other\n |2\noriginal\n.factory\n ...numbers\ncopy\n.factory\n ...numbers\nplain\n.factory\n ...numbers\nsecond\n.factory\n ...others\nfresh\n.factory\n ...numbers\nalias&\n.factory\n ...numbers\nalias&\n &original\noriginal !\n < number\nsecond !\n < other\ncopy\n % original\nplain\n original\ncopy %\nalias& %\nsecond %\ncopy %\nplain %\n:\n observer\n .worker\n observer !\n  < |90\nplain !\n < number\nalias& %\nsecond %\nfresh !\n < number\n", "10:1:0\n10:1:0\n20:2:0\n20:2:0\n20:2:0\n30:3:0\n90:1:10\n10:2:0\n30:3:0\n30:3:0\n10:1:0\n", true);
}
