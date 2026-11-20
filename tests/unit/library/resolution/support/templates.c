/** @file
 * @brief Refuse compile-time definitions and fictitious specialization selectors as public runtime roots.
 */
/** A discovered closed worker does not turn its open source definition or its
 * compile-time producer into an ordinary concrete export selector. */
static void it_rejects_open_static_and_synthetic_template_export_roots(void) {
    directory("generated/project");
    file("generated/project/generic", "@<\ninput\n...element\n@>\noutput\n...element\n input\n@!\noutput\n input\n");
    file("generated/project/assembly", "@...\n...texts\n.generic\n ...element < ..text\n");
    file("generated/project/facade", "@!\nworker\n.assembly\n ...texts\n");
    file("generated/project/missingclosure", "@!\nworker\n.unavailable\n");
    const char *invalid[]={
        "export | facade | .facade | entry\nexport | open | .generic | entry\n",
        "export | run | .assembly | entry\n",
        "export | facade | .facade | entry\nexport | static | .assembly | read\n",
        "export | run | .assembly...texts | entry\n",
        "export | run | .generic(..text) | entry\n",
        "export | run | .facade | entry\nstatic-input | run | element | ..text\n",
        "export | run | .missingclosure | entry\n"
    };
    for (size_t index=0;index<sizeof(invalid)/sizeof(*invalid);index++) {
        setup_test(); LibraryBuild build;
        bool valid=resolve(&build,invalid[index]);
        assert(!valid && !build.valid && diagnostic_valid(&build.error));
        library_build_destroy(&build); destroy_test();
    }
}
