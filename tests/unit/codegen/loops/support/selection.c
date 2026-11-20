/** @file
 * @brief Resolve terminal owned-list conditions in their owner with one caller-local index conversion.
 */
/** The caller chooses true element one while the parent has a same-name index
 * choosing false element zero. Decoder output proves selection happens once.
 */
static void it_selects_an_indexed_terminal_condition_using_the_caller_index_once(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/producer", "@>\nconditions[]\n/test/fixture/v1.0.0/src/value\n |0\n |1\n@!\nindex\n/test/fixture/v1.0.0/src/value\n |1000\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/producer\nindex\n/test/fixture/v1.0.0/src/value\n |1001\nprobe\n/test/fixture/v1.0.0/src/worker\nsource\n conditions[index]\n {\n  probe !\n   < |88\n  ^\n }\nprobe !\n < |99\n",
        "index=1001\ntruth=1\n88:1:0\n99:2:0\n", true);
}
