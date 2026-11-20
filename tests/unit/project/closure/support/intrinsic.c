/** @file
 * @brief Verify fixed Standard integer dependencies of structural list-service interfaces without wildcard or family fallback.
 */
/** Supply declared element/list results beside distinct same-spelled Project and Standard integer modules. */
static void prepare_intrinsic_closure(void) {
    fixture_package_prepare("generated/intrinsic", "fixture");
    directory("generated/intrinsic standard");
    file("generated/intrinsic/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/element\nproducer\n/test/fixture/v1.0.0/src/maker\nitems[]\n length !\n  > count\nproducer !\n values[]\n  append !\n   > element&\n");
    file("generated/intrinsic/packages/test/fixture/v1.0.0/src/element", "@!\n");
    file("generated/intrinsic/packages/test/fixture/v1.0.0/src/maker", "@>\nvalues[]\n/test/fixture/v1.0.0/src/element\n@!\n");
    file("generated/intrinsic/packages/test/fixture/v1.0.0/src/integer", "@!\n");
    file("generated/intrinsic standard/integer", "@!\n");
}
/** Each typed service parent requires the same fixed Standard identity at its real written target, never a synthetic source span. */
static void it_requires_the_fixed_standard_integer_once_for_intrinsic_interface_parents(void) {
    prepare_intrinsic_closure();
    setup_project();
    ModuleRoot roots[] = {{"generated/intrinsic", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/intrinsic standard", NULL, 0, MODULE_ROOT_STANDARD}};
    ProjectStatus status = project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main");
    assert(status == PROJECT_OK && module_count() == 4);
    const ProjectModule *integer = project_find(&project, "//integer");
    assert(integer != NULL && project_find(&project, "/test/fixture/v1.0.0/src/integer") == NULL);
    const ProjectDependency *first_implicit = project.entry->first->next->next;
    const ProjectDependency *second_implicit = first_implicit->next;
    assert(first_implicit->target == integer && second_implicit->target == integer && second_implicit->next == NULL);
    assert(layout_text_is(first_implicit->requirement, "items[]") && layout_text_is(second_implicit->requirement, "values[]"));
    assert(first_implicit->requirement.start < second_implicit->requirement.start);
    assert(first_implicit->implicit && second_implicit->implicit && !project.entry->first->implicit);
    destroy_project();
    setup_project();
    status = project_load(&project, roots, 1, "/test/fixture/v1.0.0/src/main");
    assert(status == PROJECT_MISSING && strcmp(project.error.subject, "//integer") == 0);
    assert(layout_text_is(project.error.primary.span, "items[]") && diagnostic_valid(&project.error));
    destroy_project();
    file("generated/intrinsic/packages/test/fixture/v1.0.0/src/main", "@!\nitems[]\n/test/fixture/v1.0.0/src/element\ncopies[]\n items[]\n");
    setup_project();
    status = project_load(&project, roots, 1, "/test/fixture/v1.0.0/src/main");
    assert(status == PROJECT_OK && module_count() == 2 && project_find(&project, "//integer") == NULL);
    destroy_project();
}
/** Every actual source/registry/implicit-request failure preserves an invalid destructible closure and the OOM cause. */
static void it_unwinds_every_implicit_interface_dependency_allocation_in_success_and_error_paths(void) {
    prepare_intrinsic_closure();
    ModuleRoot roots[] = {{"generated/intrinsic", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/intrinsic standard", NULL, 0, MODULE_ROOT_STANDARD}};
    for (size_t count = 1; count <= 2; count++) {
        setup_project();
        ProjectStatus status = project_load(&project, roots, count, "/test/fixture/v1.0.0/src/main");
        assert(status == (count == 1 ? PROJECT_MISSING : PROJECT_OK));
        size_t total = allocation_count;
        destroy_project();
        for (size_t failure = 1; failure <= total; failure++) {
            setup_project();
            failure_at = failure;
            status = project_load(&project, roots, count, "/test/fixture/v1.0.0/src/main");
            assert(status == PROJECT_OUT_OF_MEMORY && !project.valid && project.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
            destroy_project();
        }
    }
}
