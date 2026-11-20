/**
 * @file
 * @brief Compose complete root-aware registry seeds with exact physical catalog ownership.
 */

/** Explicit seeds exercise current registry identities without relying on the pending path-token grammar. */
static void it_shares_complete_multiword_seeds_across_root_orders(void) {
    fixture_package_prepare("generated/registry project", "fixture");
    directory("generated/registry standard");
    file("generated/registry project/packages/test/fixture/v1.0.0/src/main", "@!\n");
    file("generated/registry project/packages/test/fixture/v1.0.0/src/item state", "@C\n");
    file("generated/registry standard/item state", "@C\n");
    ModuleRoot roots[] = {
        {"generated/registry project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/registry standard", NULL, 0, MODULE_ROOT_STANDARD}
    };
    const char *required[] = {"//item state", "/test/fixture/v1.0.0/src/item state", "//item state", "/test/fixture/v1.0.0/src/main", "/test/fixture/v1.0.0/src/item state"};
    const char *expected[] = {"/test/fixture/v1.0.0/src/main", "//item state", "/test/fixture/v1.0.0/src/item state"};
    for (size_t order = 0; order < 2; order++) {
        setup_project();
        ProjectStatus status = project_load_required(&project, roots, 2, "/test/fixture/v1.0.0/src/main", required, 5);
        assert(status == PROJECT_OK && module_count() == 3);
        const ProjectModule *module = project.first;
        for (size_t index = 0; index < 3; index++) {
            assert(module != NULL && strcmp(module->source->logical_path, expected[index]) == 0);
            assert(module == project_find(&project, expected[index]));
            assert(module->state == PROJECT_MODULE_READY);
            const ModuleSource *catalog_source = NULL;
            assert(module_catalog_find(&project.catalog, expected[index], &catalog_source) == MODULE_OK);
            assert(module->source == catalog_source);
            module = module->next;
        }
        assert(module == NULL);
        assert(strcmp(source_path(project_find(&project, "//item state")->source->text),
            "generated/registry standard/item state") == 0);
        assert(strcmp(source_path(project_find(&project, "/test/fixture/v1.0.0/src/item state")->source->text),
            "generated/registry project/packages/test/fixture/v1.0.0/src/item state") == 0);
        destroy_project();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/** No missing standard seed can silently resolve an existing project module with the same components. */
static void it_preserves_exact_missing_seed_identity_without_family_fallback(void) {
    ModuleRoot root = {"generated/registry project", NULL, 0, MODULE_ROOT_PROJECT};
    char missing[] = "//item state";
    const char *required[] = {missing};
    setup_project();
    ProjectStatus status = project_load_required(&project, &root, 1, "/test/fixture/v1.0.0/src/main", required, 1);
    assert(status == PROJECT_MISSING && !project.valid);
    missing[2] = 'X';
    assert(strcmp(project.error.subject, "//item state") == 0);
    assert(diagnostic_valid(&project.error));
    assert(project_find(&project, "/test/fixture/v1.0.0/src/main") == NULL);
    destroy_project();
}

/** Every allocation in the combined root-aware closure is failed once; no partial graph becomes public. */
static void it_unwinds_every_complete_identity_registry_allocation_failure(void) {
    ModuleRoot roots[] = {
        {"generated/registry project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/registry standard", NULL, 0, MODULE_ROOT_STANDARD}
    };
    const char *required[] = {"//item state", "/test/fixture/v1.0.0/src/item state", "//item state"};
    setup_project();
    assert(project_load_required(&project, roots, 2, "/test/fixture/v1.0.0/src/main", required, 3) == PROJECT_OK);
    size_t count = allocation_count;
    destroy_project();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_project();
        failure_at = failure;
        assert(project_load_required(&project, roots, 2, "/test/fixture/v1.0.0/src/main", required, 3) == PROJECT_OUT_OF_MEMORY);
        assert(!project.valid && project_find(&project, "/test/fixture/v1.0.0/src/main") == NULL);
        assert(diagnostic_valid(&project.error));
        destroy_project();
    }
}
