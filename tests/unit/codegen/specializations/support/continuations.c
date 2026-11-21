/** @file
 * @brief Execute equal-key and distinct-key instances through references, yields and explicit continuation copies.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** A shared body cannot merge native state or saved positions; ordinary copies clear positions while explicit snapshots preserve them. */
static void it_preserves_independent_captured_instances_references_and_explicit_continuation_copies(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("other", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/task\n .slot < /test/fixture/v1.0.0/src/value\n.others\n/test/fixture/v1.0.0/src/task\n .slot < /test/fixture/v1.0.0/src/other\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@<\ninput\n.slot\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |10\n%\nprobe !\n < |20\n%\nprobe !\n < |30\n");
    execute_project("@!\nnumber\n/test/fixture/v1.0.0/src/value\n |1\nother\n/test/fixture/v1.0.0/src/other\n |2\noriginal\n/test/fixture/v1.0.0/src/factory\n .numbers\ncopy\n/test/fixture/v1.0.0/src/factory\n .numbers\nplain\n/test/fixture/v1.0.0/src/factory\n .numbers\nsecond\n/test/fixture/v1.0.0/src/factory\n .others\nfresh\n/test/fixture/v1.0.0/src/factory\n .numbers\nalias&\n/test/fixture/v1.0.0/src/factory\n .numbers\nalias&\n & original\noriginal !\n < number\nsecond !\n < other\ncopy\n % original\nplain\n original\ncopy %\nalias& %\nsecond %\ncopy %\nplain %\n:\n observer\n /test/fixture/v1.0.0/src/worker\n observer !\n  < |90\nplain !\n < number\nalias& %\nsecond %\nfresh !\n < number\n", "10:1:0\n10:1:0\n20:2:0\n20:2:0\n20:2:0\n30:3:0\n90:1:10\n10:2:0\n30:3:0\n30:3:0\n10:1:0\n", true);
}
