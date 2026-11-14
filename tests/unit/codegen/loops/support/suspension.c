/** @file
 * @brief Resume inside a header-loop body before starting its next complete test.
 */
/** Root-prefix counters separate body resumption from an incorrectly replayed
 * header, while caller output proves each yield returns normally to its caller.
 */
static void it_resumes_inside_the_body_before_repeating_the_header(void) {
    prepare_loop_header();
    file("generated/project/controller", "@!\nobject\n.container\nremaining\n.value\n |3\nprobe\n.worker\n"
        "object !\n child !\n  < remaining\n  output\n  {\n   object\n    child\n     output >> remaining\n   probe !\n    < remaining\n   %\n   probe !\n    < |99\n  }\n");
    execute_project("@!\ntask\n.controller\nprobe\n.worker\ntask !\nprobe !\n < |1\ntask %\nprobe !\n < |2\ntask %\nprobe !\n < |3\n",
        "100:1:0\ncount=3\ntruth=2\n2:1:0\n1:1:0\n99:2:0\n100:2:0\ncount=2\ntruth=1\n1:3:0\n2:2:0\n99:4:0\n100:3:0\ncount=1\ntruth=0\n3:3:0\n", true);
}
