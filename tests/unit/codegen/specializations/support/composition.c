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

/** A known producer's finite structural result participates in ordinary call
 * equations; no unknown constructor or discarded input is guessed.
 */
static void it_infers_static_producer_inputs_from_exact_structural_call_constraints(void) {
    prepare_nested_definitions();
    execute_project("@!\nvalue\n/test/fixture/v1.0.0/src/value\n |31\ninner\n/test/fixture/v1.0.0/src/inner\ninner !\n < value\nouter\n/test/fixture/v1.0.0/src/factory\n .result\nouter !\n < inner\n > copied\ncopied\n >> retained\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < retained\n", "31:1:0\n", true);
}
