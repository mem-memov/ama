/** @file
 * @brief Check original source witnesses for definition cycles, occurs refusal, changing-key recursion and equality receipts.
 * @see [T189 contract](../../../../../docs/implementation/specialization-diagnostics.md)
 */

/** An actual static definition cycle is not a harmless equality receipt and must retain its source and the selecting caller. */
static void it_reports_static_definition_cycles_without_fabricating_runtime_invocations(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n/test/fixture/v1.0.0/src/factory\n .first\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.first\n.second\n.second\n.first\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error) && frontend.specializations.guards.rejected_definition);
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
    assert(references_site(error, project_find(&frontend.project, "/test/fixture/v1.0.0/src/factory")->source->text, 2, 5));
    assert(references_site(error, frontend.project.entry->source->text, 4, 4));
    destroy_frontend();
}

/** A finite equation requesting T = Box(T) must be refused by the occurs check and identify original variable and constructor sources. */
static void it_reports_occurs_refusal_with_original_variable_and_constructor_definitions(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@<\ninput\n.slot\n@!\nbox\n/test/fixture/v1.0.0/src/box\nbox !\n < input\ninput\n box\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\ninput\n.element\n@!\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error) && strstr(error->message, "infinite substitution"));
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
    assert(references_site(error, frontend.project.entry->source->text, 2, 10));
    assert(references_site(error, project_find(&frontend.project, "/test/fixture/v1.0.0/src/box")->source->text, 1, SIZE_MAX));
    destroy_frontend();
}

/** Re-entering a source template under a different key is a creation error, distinct from ordinary nominal reference cycles. */
static void it_reports_changing_key_creation_at_its_actual_original_required_path(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@<\ninput\n.slot\n@!\nanchor\n//integer\ninput\n anchor\ntext\n//text\nchild&\n/test/fixture/v1.0.0/src/main\nchild& !\n < text\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error) && frontend.specializations.guards.rejected_origin);
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
    assert(references_site(error, frontend.project.entry->source->text, 11, 14));
    destroy_frontend();
}

/** A result received into an existing pair is an equality check; its conflict must preserve the first definition and actual receipt. */
static void it_reports_pair_definition_and_conflicting_equality_receipt_as_distinct_original_sites(void) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n/test/fixture/v1.0.0/src/factory\n .known\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.known\n//integer\n/test/fixture/v1.0.0/src/identity\n .same > .known\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/identity", "@...\n.same\n//text\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STANDARD_ROOT, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(!compiler_frontend_check(&frontend, roots, 2, "/test/fixture/v1.0.0/src/main"));
    const Diagnostic *error = &frontend.error;
    assert(diagnostic_valid(error));
    assert(diagnostic_render(stderr, error) == DIAGNOSTIC_RENDER_OK);
    const SourceText *factory = project_find(&frontend.project, "/test/fixture/v1.0.0/src/factory")->source->text;
    assert(references_site(error, factory, 2, 2));
    assert(references_site(error, factory, 5, 5));
    destroy_frontend();
}
