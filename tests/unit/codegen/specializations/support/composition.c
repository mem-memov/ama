/** @file
 * @brief Compile nested static composition through a closed assembly and reject runtime inference of static producer inputs.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */

/** Define a parameterized producer independently of its caller; only explicit static bindings may close its input. */
static void prepare_nested_definitions(void) {
    directory("generated/project");
    file("generated/project/inner", "@<\ninput\n...element\n@>\noutput\n...element\n input\n@!\noutput\n input\n");
    file("generated/project/outer", "@<\ninput\n...element\n@>\noutput\n...element\n input\n@!\noutput\n input\n");
    file("generated/project/factory", "@...\n...inner\n.inner\n ...element < ...element\n...result\n.outer\n ...element < ...inner\n");
}

/** A closed assembly explicitly specializes the factory; nested nominal state retains its inner result through ordinary cloning. */
static void it_executes_nested_static_composition_through_an_explicitly_closed_static_assembly(void) {
    prepare_nested_definitions();
    file("generated/project/assembly", "@...\n.factory\n ...element < .value\n ...result > ...nested\n");
    execute_project("@!\nvalue\n.value\n |31\ninner\n.inner\ninner !\n < value\nouter\n.assembly\n ...nested\nouter !\n < inner\n > copied\ncopied\n >> retained\nprobe\n.worker\nprobe !\n < retained\n", "31:1:0\n", true);
}

/** Ordinary call arguments cannot fill omitted static producer inputs, even when a mathematical type equation could otherwise determine them. */
static void it_refuses_to_infer_static_producer_inputs_from_ordinary_calls(void) {
    prepare_nested_definitions();
    native_fixture("value", "@C\n");
    file("generated/project/main", "@!\nvalue\n.value\n |31\ninner\n.inner\ninner !\n < value\nouter\n.factory\n ...result\nouter !\n < inner\n > copied\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
    assert(!compiler_frontend_check(&frontend, roots, 2, ".main"));
    assert(!frontend.valid && !frontend.plan_initialized && diagnostic_valid(&frontend.error));
    assert(frontend.error.primary.span.source == project_find(&frontend.project, ".factory")->source->text);
    destroy_frontend();
}
