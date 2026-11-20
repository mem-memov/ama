/** @file
 * @brief Route refused root/intermediate/resume header links to their own caller-side handlers.
 */
/** Refusal exits the complete unentered loop, releases held prefixes and does
 * not perform terminal conversion or execute an ancestor's alternative.
 */
static void it_handles_refusal_at_each_header_depth_without_running_the_loop(void) {
    execute_project("@>\ncondition\n/test/fixture/v1.0.0/src/value\n |1\n@!\nself&\n/test/fixture/v1.0.0/src/main\n &\nprobe\n/test/fixture/v1.0.0/src/worker\n"
        "self& !\n condition\n {\n  probe !\n   < |999\n }\n:\n probe !\n  < |55\nprobe !\n < |66\n",
        "55:1:9\n66:2:9\n", true);
    file("generated/project/packages/test/fixture/v1.0.0/src/branch", "@>\ncondition\n/test/fixture/v1.0.0/src/value\n |1\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/producer", "@>\nchild&\n/test/fixture/v1.0.0/src/branch\n@!\ntrace\n/test/fixture/v1.0.0/src/worker\ntrace !\n < |10\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/producer\nprobe\n/test/fixture/v1.0.0/src/worker\nsource !\n child& !\n  condition\n  {\n   probe !\n    < |999\n  }\n :\n  probe !\n   < |55\n:\n probe !\n  < |888\nprobe !\n < |66\n",
        "10:1:0\n55:1:7\n66:2:7\n", true);
    file("generated/project/packages/test/fixture/v1.0.0/src/middle", "@>\nchild\n/test/fixture/v1.0.0/src/branch\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/producer", "@>\nmiddle\n/test/fixture/v1.0.0/src/middle\n@!\n");
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/producer\nprobe\n/test/fixture/v1.0.0/src/worker\nsource\n middle\n  child %\n   condition\n   {\n    probe !\n     < |999\n   }\n  :\n   probe !\n    < |55\n"
        "probe !\n < |66\nsource\n middle\n  child !\n   condition\n   {\n   }\n",
        "55:1:10\n66:2:10\ntruth=1\n", true);
    /* The refused header never entered its own body. Its handler's break must
     * exit the enclosing named loop, without checking that outer condition again. */
    execute_project("@!\nsource\n/test/fixture/v1.0.0/src/producer\nprobe\n/test/fixture/v1.0.0/src/worker\nouter\n/test/fixture/v1.0.0/src/value\n |1\nouter\n{\n"
        " source\n  middle\n   child %\n    condition\n    {\n     probe !\n      < |999\n    }\n   :\n    ^\n"
        " probe !\n  < |888\n}\nprobe !\n < |77\n", "truth=1\n77:1:10\n", true);
}
