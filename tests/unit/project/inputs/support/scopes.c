/** @file
 * @brief Exercise real required-module alias preparation and normalized dependency admission.
 * This leaf owns retained project/source contexts and counted allocation failures.
 */

/** Prepare one explicit package whose graph uses package/local and relative operands. */
static void prepare_scoped_project(void) {
    fixture_package_prepare("generated/scope-project", "fixture");
    file("generated/scope-project/packages/test/fixture/v1.0.0/package.ini",
        "[aliases]\n~common = ./src\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main",
        "@!\nleft\n~local/left\nright\n./right\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main.ini", "~local = ~common\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/left", "@!\nshared\n./shared\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/right", "@!\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/shared", "@!\n");
}

/** Own lexical scopes share only the exact version table; raw spans survive normalization. */
static void it_resolves_actual_defining_scopes_and_relative_dependencies_without_rewriting_sources(void) {
    prepare_scoped_project();
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_OK);
    const ProjectModule *main_module = project.entry;
    const ProjectModule *left = project_find(&project, "/test/fixture/v1.0.0/src/left");
    assert(main_module->aliases->count == 1 && left->aliases->count == 0);
    assert(main_module->aliases->package == left->aliases->package);
    assert(main_module->aliases->package->count == 1);
    const ProjectDependency *edge = main_module->first;
    assert(edge != NULL && edge->resolved_path != NULL && edge->target == left);
    assert(strcmp(edge->logical_path, "/test/fixture/v1.0.0/src/left") == 0);
    assert(edge->requirement.end - edge->requirement.start == strlen("~local/left"));
    assert(memcmp(source_bytes(edge->requirement.source) + edge->requirement.start,
        "~local/left", strlen("~local/left")) == 0);
    assert(edge->resolved_path->expression.source == edge->requirement.source);
    assert(edge->resolved_path->expression.start == edge->requirement.start);
    assert(strcmp(left->first->logical_path, "/test/fixture/v1.0.0/src/shared") == 0);
    assert(strcmp(main_module->first->next->logical_path, "/test/fixture/v1.0.0/src/right") == 0);
    assert(source_length(main_module->source->text) == strlen("@!\nleft\n~local/left\nright\n./right\n"));
    destroy_project();
}

/** Invalid shared/local configuration and unknown aliases retain original source locations. */
static void it_rejects_actual_scope_collisions_unknown_aliases_and_malformed_profiles(void) {
    const char *package = "generated/scope-project/packages/test/fixture/v1.0.0/package.ini";
    const char *local = "generated/scope-project/packages/test/fixture/v1.0.0/src/main.ini";
    const char *paths[] = {package, local, local};
    const char *payloads[] = {"[unknown]\n", "~common = ./shared\n", "~local = ~unknown\n"};
    for (size_t index = 0; index < 3; index++) {
        prepare_scoped_project();
        file(paths[index], payloads[index]);
        setup_project();
        assert(load_root("generated/scope-project") == PROJECT_INVALID_CONFIGURATION);
        assert(!project.valid && project.error.primary.span.source != NULL);
        assert(strcmp(source_path(project.error.primary.span.source), paths[index]) == 0);
        assert(diagnostic_valid(&project.error));
        destroy_project();
    }
    prepare_scoped_project();
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/left", "@!\nshared\n~local/shared\n");
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_INVALID_CONFIGURATION);
    assert(strstr(project.error.message, "alias") != NULL);
    assert(strcmp(source_path(project.error.primary.span.source),
        "generated/scope-project/packages/test/fixture/v1.0.0/src/left") == 0);
    assert(project.error.related_count == 1 && diagnostic_valid(&project.error));
    destroy_project();
}

/** Every actual scope/normalization/loading allocation fails under one ordinary destructor. */
static void it_unwinds_every_actual_scope_and_normalization_allocation_failure(void) {
    prepare_scoped_project();
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_OK);
    size_t measured = allocation_count;
    destroy_project();
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        setup_project();
        failure_at = ordinal;
        assert(load_root("generated/scope-project") == PROJECT_OUT_OF_MEMORY);
        assert(!project.valid);
        destroy_project();
    }
}

/** Real selectors grant only actual units; source/test isolation precedes same-version privilege. */
static void it_checks_real_external_selectors_and_explicit_source_test_dependencies(void) {
    for (size_t permitted = 0; permitted < 2; permitted++) {
        prepare_scoped_project();
        fixture_package_prepare("generated/scope-project", "external");
        file("generated/scope-project/packages/test/external/v1.0.0/src/value", "@!\n");
        file("generated/scope-project/packages/test/fixture/v1.0.0/src/main",
            "@!\nvalue\n/test/external/v1.0.0/src/value\n");
        if (permitted) {
            file("generated/scope-project/packages/test/fixture/v1.0.0/package.ini",
                "[dependencies]\n/test/external/v1.0.0/src/*\n[aliases]\n~common = ./src\n");
        }
        setup_project();
        assert(load_root("generated/scope-project") == (permitted ? PROJECT_OK : PROJECT_INVALID_CONFIGURATION));
        if (!permitted) { assert(strstr(project.error.message, "selectors") != NULL); }
        destroy_project();
    }
    prepare_scoped_project();
    file("generated/scope-project/packages/test/fixture/v1.0.0/tests/helper", "@!\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main",
        "@!\nvalue\n../tests/helper\n");
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_INVALID_CONFIGURATION);
    assert(strstr(project.error.message, "tests") != NULL && diagnostic_valid(&project.error));
    destroy_project();
}

/** Source-unit selectors cover mirrored tests and their structural helpers, not unrelated tests. */
static void it_admits_actual_associated_test_units_using_retained_physical_ancestry(void) {
    const char *targets[] = {"check", "check/nested", NULL};
    for (size_t index = 0; index < 3; index++) {
        prepare_scoped_project();
        fixture_package_prepare("generated/scope-project", "external");
        file("generated/scope-project/packages/test/external/v1.0.0/src/data", "@!\n");
        directory("generated/scope-project/packages/test/external/v1.0.0/tests/data");
        file("generated/scope-project/packages/test/external/v1.0.0/tests/data/check", "@!\n");
        directory("generated/scope-project/packages/test/external/v1.0.0/tests/data/check+");
        file("generated/scope-project/packages/test/external/v1.0.0/tests/data/check+/nested", "@!\n");
        directory("generated/scope-project/packages/test/external/v1.0.0/tests/other");
        file("generated/scope-project/packages/test/external/v1.0.0/tests/other/check", "@!\n");
        file("generated/scope-project/packages/test/fixture/v1.0.0/package.ini",
            "[dependencies]\n/test/external/v1.0.0/src/data\n");
        char program[256];
        int length = index < 2 ? snprintf(program, sizeof(program),
            "@!\nvalue\n/test/external/v1.0.0/tests/data/%s\n", targets[index])
            : snprintf(program, sizeof(program), "@!\nvalue\n/test/external/v1.0.0/tests/other/check\n");
        assert(length > 0 && (size_t)length < sizeof(program));
        file("generated/scope-project/packages/test/fixture/v1.0.0/tests/main", program);
        setup_project();
        ModuleRoot root = {.path = "generated/scope-project", .kind = MODULE_ROOT_PROJECT};
        ProjectStatus status = project_load(&project, &root, 1, "/test/fixture/v1.0.0/tests/main");
        assert(status == (index < 2 ? PROJECT_OK : PROJECT_INVALID_CONFIGURATION));
        if (index < 2) { assert(project.entry->first && project.entry->first->target); }
        else { assert(strstr(project.error.message, "selectors") != NULL); }
        destroy_project();
    }
}

/** Actual absence remains a missing-module failure even when a wildcard has no match. */
static void it_reports_actual_missing_wildcard_targets_without_inventing_availability(void) {
    prepare_scoped_project();
    file("generated/scope-project/packages/test/fixture/v1.0.0/package.ini",
        "[dependencies]\n/test/missing/v1.0.0/src/*\n[aliases]\n~common = ./src\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main",
        "@!\nvalue\n/test/missing/v1.0.0/src/value\n");
    setup_project();
    assert(load_root("generated/scope-project") == PROJECT_MISSING);
    assert(project.error.code == DIAGNOSTIC_MISSING_MODULE && diagnostic_valid(&project.error));
    destroy_project();
}

/** Standard local aliases remain nonimports until an actual authored use crosses namespaces. */
static void it_checks_standard_scope_isolation_at_actual_alias_use(void) {
    prepare_scoped_project();
    directory("generated/scope-standard");
    file("generated/scope-standard/adapter", "@!\n");
    file("generated/scope-standard/adapter.ini",
        "~ordinary = /test/fixture/v1.0.0/src/shared\n");
    file("generated/scope-project/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n//adapter\n");
    ModuleRoot roots[] = {
        {.path = "generated/scope-project", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/scope-standard", .kind = MODULE_ROOT_STANDARD}
    };
    setup_project();
    assert(project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main") == PROJECT_OK);
    assert(project_find(&project, "/test/fixture/v1.0.0/src/shared") == NULL);
    assert(project_find(&project, "//adapter")->aliases->package == NULL);
    destroy_project();
    const char *uses[] = {"@!\nvalue\n~ordinary\n", "@!\nvalue\n ~ordinary/member\n"};
    const char *operands[] = {"~ordinary", "~ordinary/member"};
    for (size_t index = 0; index < 2; index++) {
        file("generated/scope-standard/adapter", uses[index]);
        setup_project();
        assert(project_load(&project, roots, 2, "/test/fixture/v1.0.0/src/main") == PROJECT_INVALID_CONFIGURATION);
        assert(strstr(project.error.message, "standard module") != NULL && diagnostic_valid(&project.error));
        assert(strcmp(source_path(project.error.primary.span.source), "generated/scope-standard/adapter") == 0);
        assert(project.error.primary.span.end - project.error.primary.span.start == strlen(operands[index]));
        destroy_project();
    }
}
