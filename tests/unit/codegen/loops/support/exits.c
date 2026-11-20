/** @file
 * @brief Exercise nested break and invocation return after header protection has ended.
 */
/** Break targets only the innermost loop; early return completes its invocation
 * with no retained outgoing target and returns to the correct dynamic caller.
 */
static void it_exits_nested_loops_and_returns_from_header_bodies(void) {
    prepare_loop_header();
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/container\nremaining\n/test/fixture/v1.0.0/src/value\n |3\nouter\n/test/fixture/v1.0.0/src/value\n |1\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "outer\n{\n object !\n  child !\n   < remaining\n   output\n   {\n    probe !\n     < |8\n    ^\n    probe !\n     < |999\n   }\n probe !\n  < |9\n ^\n}\nprobe !\n < |10\n",
        "truth=1\n100:1:0\ncount=3\ntruth=2\n8:1:0\n9:2:0\n10:3:0\n", true);
    file("generated/project/packages/test/fixture/v1.0.0/src/returner", "@!\nobject\n/test/fixture/v1.0.0/src/container\nremaining\n/test/fixture/v1.0.0/src/value\n |3\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "object !\n child !\n  < remaining\n  output\n  {\n   >\n  }\nprobe !\n < |999\n");
    execute_project("@!\nearly\n/test/fixture/v1.0.0/src/returner\nprobe\n/test/fixture/v1.0.0/src/worker\nearly !\nprobe !\n < |20\n",
        "100:1:0\ncount=3\ntruth=2\n20:1:0\n", true);
}
