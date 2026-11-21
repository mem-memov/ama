/**
 * @file
 * @brief Check explicit-edge origins and policy independently of selected coverage.
 * Contract: [selection](../../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */

/** Source/test refusal precedes internal privilege; caller-owned test origins stay valid. */
static void it_checks_origin_versions_and_source_test_policy_before_privileges(void) {
    prepare();
    const DependencySelector *patterns[] = {pattern("/ext/pkg/v1.0.0")};
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(&results, view, inputs[OWNER].identity, patterns, 1,
        &selection, &issue) == DEPENDENCY_SELECTION_OK);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, inputs[OWN_TEST].identity) == DEPENDENCY_EDGE_SOURCE_TEST);
    assert(dependency_edge_check(selection, inputs[OWN_TEST].identity, inputs[MAIN].identity) == DEPENDENCY_EDGE_ALLOWED);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, inputs[A_TEST].identity) == DEPENDENCY_EDGE_SOURCE_TEST);
    assert(dependency_edge_check(selection, inputs[OWN_TEST].identity, inputs[A_TEST].identity) == DEPENDENCY_EDGE_ALLOWED);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, inputs[A].identity) == DEPENDENCY_EDGE_ALLOWED);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, inputs[VARIANT_A].identity) == DEPENDENCY_EDGE_UNDECLARED);
    assert(dependency_edge_check(selection, inputs[A].identity, inputs[B].identity) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, path("//tests/item")) == DEPENDENCY_EDGE_ALLOWED);
    assert(dependency_edge_check(selection, path("//item"), inputs[A].identity) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_edge_check(selection, inputs[MAIN].identity, path("/home/app/v1.0.0/src/missing")) == DEPENDENCY_EDGE_ALLOWED);
    assert(!dependency_selection_covers(selection, path("/ext/pkg/v1.0.0/src/missing")));
    finish();
}
