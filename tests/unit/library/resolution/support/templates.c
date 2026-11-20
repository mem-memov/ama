/** @file
 * @brief Refuse compile-time definitions and fictitious specialization selectors as public runtime roots.
 */
/** A discovered closed worker does not turn its open source definition or its
 * compile-time producer into an ordinary concrete export selector. */
static void it_rejects_open_static_and_synthetic_template_export_roots(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/generic", "@<\ninput\n.element\n@>\noutput\n.element\n input\n@!\noutput\n input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly", "@...\n.texts\n/test/fixture/v1.0.0/src/generic\n .element < //text\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/facade", "@!\nworker\n/test/fixture/v1.0.0/src/assembly\n .texts\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/missingclosure", "@!\nworker\n/test/fixture/v1.0.0/src/unavailable\n");
    const char *invalid[]={
        "export | facade | /test/fixture/v1.0.0/src/facade | entry\nexport | open | /test/fixture/v1.0.0/src/generic | entry\n",
        "export | run | /test/fixture/v1.0.0/src/assembly | entry\n",
        "export | facade | /test/fixture/v1.0.0/src/facade | entry\nexport | static | /test/fixture/v1.0.0/src/assembly | read\n",
        "export | run | /test/fixture/v1.0.0/src/assembly...texts | entry\n",
        "export | run | /test/fixture/v1.0.0/src/generic(..text) | entry\n",
        "export | run | /test/fixture/v1.0.0/src/facade | entry\nstatic-input | run | element | //text\n",
        "export | run | /test/fixture/v1.0.0/src/missingclosure | entry\n"
    };
    for (size_t index=0;index<sizeof(invalid)/sizeof(*invalid);index++) {
        setup_test(); LibraryBuild build;
        bool valid=resolve(&build,invalid[index]);
        assert(!valid && !build.valid && diagnostic_valid(&build.error));
        library_build_destroy(&build); destroy_test();
    }
}
