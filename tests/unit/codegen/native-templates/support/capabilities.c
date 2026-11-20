/** @file
 * @brief Check actual native operations selected by a closed ordinary template.
 * @see [T190 contract](../../../../../docs/implementation/native-template-composition.md)
 */

/** The output starts as a copied input, then a native condition chooses an
 * indexed list element. Each operation has observable concrete value semantics. */
static const char *const capability_program =
    "@!\nmethod\n/test/fixture/v1.0.0/src/factory\n .algorithm\nmethod !\n < |4\n > result\nprobe\n/test/fixture/v1.0.0/src/consumer\nprobe !\n < result\n";

/** Install only concrete native companions; the Ama algorithm alone has a formal. */
static void prepare_capability_sources(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    native_fixture("resource", "@C\n");
    native_fixture("consumer", "@<\ninput\n/test/fixture/v1.0.0/src/resource\n@>\noutput\n/test/fixture/v1.0.0/src/resource\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.algorithm\n/test/fixture/v1.0.0/src/algorithm\n .slot < /test/fixture/v1.0.0/src/resource\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/algorithm", "@<\ninput\n.slot\n@>\noutput\n.slot\n input\n@!\nvalues[]\n.slot\n |3\n |9\nindex\n.slot\n |1\noutput\n?\n output\n  values[index]\n");
}

/** Actual generated execution must use the selected native protocol, including
 * address-sensitive lifecycle hooks, without any generic native companion. */
static void it_executes_all_required_native_capabilities_after_template_substitution(void) {
    prepare_capability_sources();
    execute_project(capability_program, "9:1:0\n", true);
}

/** Remove one real ABI capability at a time. Supplying a concrete argument to
 * the ordinary template must not synthesize the missing native operation. */
static void it_refuses_missing_native_capabilities_without_generating_specialized_companions(void) {
    const char *bindings[] = {"+ initialize\n", "- destroy\n", "| literal\n",
        "= clone\n", "? condition\n", "[] index\n"};
    for (size_t absent = 0; absent < ABI_BINDING_COUNT; absent++) {
        if (absent == ABI_DESTROY) { continue; }
        prepare_capability_sources();
        file("generated/project/packages/test/fixture/v1.0.0/src/main", capability_program);
        FILE *abi = fopen("generated/project/packages/test/fixture/v1.0.0/src/resource.abi", "wb");
        assert(abi);
        for (size_t index = 0; index < ABI_BINDING_COUNT; index++) {
            if (index != absent) { assert(fputs(bindings[index], abi) != EOF); }
        }
        assert(fclose(abi) == 0);
        setup_frontend();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
        bool checked = compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main");
        if (checked) { fprintf(stderr, "Accepted absent native capability %zu\n", absent); }
        assert(!checked && !frontend.valid && !frontend.plan_initialized);
        assert(diagnostic_valid(&frontend.error));
        assert(frontend.proposed_sources.first == NULL);
        destroy_frontend();
    }
}

/** A refused native literal prevents the specialized body and consumer from
 * executing, and is reported through the ordinary source call boundary. */
static void it_reports_native_literal_failure_before_entering_a_specialized_body(void) {
    prepare_capability_sources();
    execute_project("@!\nmethod\n/test/fixture/v1.0.0/src/factory\n .algorithm\nmethod !\n < |invalid\n", "", false);
    expect_diagnostic("Not a decimal value");
}
