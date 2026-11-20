/** @file
 * @brief Observe exact full-header repeats and release before body/false successors.
 */
/** Body reads reacquire both header owners, proving release; prefix counters
 * prove every repeat starts at the written root rather than only its last call.
 */
static void it_repeats_the_complete_header_and_releases_before_body_and_exit(void) {
    prepare_loop_header();
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/container\nremaining\n/test/fixture/v1.0.0/src/value\n |3\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "object !\n child !\n  < remaining\n  output\n  {\n"
        "   object\n    child\n     output >> remaining\n   probe !\n    < remaining\n  }\n"
        "object\n child\n  output >> last\nprobe !\n < last\n",
        "100:1:0\ncount=3\ntruth=2\n2:1:0\n100:2:0\ncount=2\ntruth=1\n1:2:0\n100:3:0\ncount=1\ntruth=0\n0:3:0\n", true);
}

/** Inner backedges repeat neither the enclosing named loop nor its branch. */
static void it_keeps_nested_loop_and_branch_repeat_boundaries_independent(void) {
    prepare_loop_header();
    execute_project("@!\nobject\n/test/fixture/v1.0.0/src/container\nremaining\n/test/fixture/v1.0.0/src/value\n |3\nenabled\n/test/fixture/v1.0.0/src/value\n |8\nzero\n/test/fixture/v1.0.0/src/value\n |0\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "enabled\n{\n enabled\n ?\n  object !\n   child !\n    < remaining\n    output\n    {\n"
        "     object\n      child\n       output >> remaining\n     probe !\n      < remaining\n    }\n enabled\n  zero\n}\nprobe !\n < |99\n",
        "truth=8\ntruth=8\n100:1:0\ncount=3\ntruth=2\n2:1:0\n100:2:0\ncount=2\ntruth=1\n1:2:0\n100:3:0\ncount=1\ntruth=0\ntruth=0\n99:3:0\n", true);
}
