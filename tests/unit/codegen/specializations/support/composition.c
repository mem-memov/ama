/** @file
 * @brief Compile nested static composition through a closed assembly and reject runtime inference of static producer inputs.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Define a parameterized producer independently of its caller; only explicit static bindings may close its input. */
static void prepare_nested_definitions(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/inner", "@<\ninput\n.element\n@>\noutput\n.element\n input\n@!\noutput\n input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/outer", "@<\ninput\n.element\n@>\noutput\n.element\n input\n@!\noutput\n input\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.inner\n/test/fixture/v1.0.0/src/inner\n .element < .element\n.result\n/test/fixture/v1.0.0/src/outer\n .element < .inner\n");
}

/** A closed assembly explicitly specializes the factory; nested nominal state retains its inner result through ordinary cloning. */
static void it_executes_nested_static_composition_through_an_explicitly_closed_static_assembly(void) {
    prepare_nested_definitions();
    file("generated/project/packages/test/fixture/v1.0.0/src/assembly", "@...\n/test/fixture/v1.0.0/src/factory\n .element < /test/fixture/v1.0.0/src/value\n .result > .nested\n");
    execute_project("@!\nvalue\n/test/fixture/v1.0.0/src/value\n |31\ninner\n/test/fixture/v1.0.0/src/inner\ninner !\n < value\nouter\n/test/fixture/v1.0.0/src/assembly\n .nested\nouter !\n < inner\n > copied\ncopied\n >> retained\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < retained\n", "31:1:0\n", true);
}

/** Ordinary call arguments cannot fill omitted static producer inputs, even when a mathematical type equation could otherwise determine them. */
static void it_refuses_to_infer_static_producer_inputs_from_ordinary_calls(void) {
    prepare_nested_definitions();
    native_fixture("value", "@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n/test/fixture/v1.0.0/src/value\n |31\ninner\n/test/fixture/v1.0.0/src/inner\ninner !\n < value\nouter\n/test/fixture/v1.0.0/src/factory\n .result\nouter !\n < inner\n > copied\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    assert(!frontend.valid && !frontend.plan_initialized && diagnostic_valid(&frontend.error));
    assert(frontend.error.primary.span.source == project_find(&frontend.project, "/test/fixture/v1.0.0/src/factory")->source->text);
    destroy_frontend();
}
