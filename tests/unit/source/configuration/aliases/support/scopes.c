/**
 * @file
 * @brief Check scope identity, visibility and source provenance.
 * Contract: [configuration](../../../../../../docs/implementation/project-configuration.md).
 */
/** Package aliases survive parsed-node retirement and locals never leak to siblings. */
static void it_preserves_package_bases_local_isolation_and_original_ownership(void) {
    begin("[aliases]\n~shared = ./src\n~root = //a/..\n~shared long = //long\n");
    document(&local_document, "~local = ./helper\n~copy = ~shared/child\n", CONFIGURATION_MODULE);
    const ConfigurationAliasScope *local = NULL, *sibling = NULL;
    assert(module(&local_document, package, "/acme/pkg/v1.0.0/src/deep/worker", &local) == CONFIGURATION_SCOPE_OK);
    configuration_document_destroy(&local_document);
    configuration_document_destroy(&package_document);
    configuration_document_init(&package_document, sources.allocator);
    target(local, "~local", "/acme/pkg/v1.0.0/src/deep/helper");
    target(local, "~copy", "/acme/pkg/v1.0.0/src/child");
    target(local, "~root", "//");
    target(local, "~shared long", "//long");
    assert(local->package == package && local->count == 2);
    assert(module(NULL, package, "/acme/pkg/v1.0.0/src/sibling", &sibling) == CONFIGURATION_SCOPE_OK);
    assert(configuration_alias_find(sibling, configuration_test_text("~local")) == NULL);
    assert(configuration_alias_find(local, configuration_test_text("~share")) == NULL);
    assert(module(NULL, local, "/acme/pkg/v1.0.0/src/child", &sibling) == CONFIGURATION_SCOPE_INVALID);
    finish();
}

/** Resolve against package only, rejecting local/self chains in either source order. */
static void it_rejects_shadowing_chains_and_foreign_version_scopes(void) {
    begin("[aliases]\n~shared = ./src\n");
    const char *invalid[] = {"~shared = ~missing/x\n", "~a = ~a/x\n",
        "~a = ~b/x\n~b = ./src\n", "~b = ./src\n~a = ~b/x\n"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(*invalid); i++) {
        document(&local_document, invalid[i], CONFIGURATION_MODULE);
        const ConfigurationAliasScope *output = package;
        assert(module(&local_document, package, "/acme/pkg/v1.0.0/src/item", &output) == CONFIGURATION_SCOPE_INVALID);
        assert(output == package && issue.span.source == local_document.source);
        if (i == 0) { assert(strstr(issue.message, "collides") != NULL); }
        configuration_document_destroy(&local_document);
    }
    const char *foreign[] = {"/acme/pkg/v1.0.1/src/item", "/acme/pkg extra/v1.0.0/src/item", "//item", "/acme/pkg/v1.0.0/src"};
    for (size_t i = 0; i < sizeof(foreign) / sizeof(*foreign); i++) {
        const ConfigurationAliasScope *output = package;
        assert(module(NULL, package, foreign[i], &output) == CONFIGURATION_SCOPE_INVALID);
        assert(output == package);
    }
    const ConfigurationAliasScope *standard = NULL;
    assert(module(NULL, NULL, "//item", &standard) == CONFIGURATION_SCOPE_OK);
    assert(strcmp(standard->base->text, "//") == 0);
    assert(module(NULL, NULL, "/acme/pkg/v1.0.0/src/item", &standard) == CONFIGURATION_SCOPE_INVALID);
    finish();
}

/** Preparation depends on complete names, not record order, and propagates target origins. */
static void it_composes_source_operands_with_exact_sorted_scope_lookup(void) {
    const char *orders[] = {
        "[aliases]\n~z = //z\n~a = ./src\n~имя с пробелом = //unicode\n",
        "[aliases]\n~имя с пробелом = //unicode\n~a = ./src\n~z = //z\n"
    };
    for (size_t order = 0; order < 2; order++) {
        begin(orders[order]);
        target(package, "~a", "/acme/pkg/v1.0.0/src");
        target(package, "~z", "//z");
        target(package, "~имя с пробелом", "//unicode");
        assert(configuration_alias_find(package, configuration_test_text("~имя")) == NULL);
        assert(configuration_alias_find(package, configuration_test_text("~A")) == NULL);
        SourceSpan expression = configuration_test_text("~a/item");
        const ModulePathValue *value = NULL;
        assert(module_path_normalize(&arena, expression, NULL, configuration_alias_lookup,
            (void *)package, &value, &issue) == MODULE_PATH_OK);
        assert(strcmp(value->text, "/acme/pkg/v1.0.0/src/item") == 0);
        assert(value->parts[value->count - 1].origin.source == expression.source);
        assert(value->parts[value->count - 2].origin.source == package_document.source);
        finish();
    }
}

/** Normalization precedes endpoint validation, while first root underflow remains fatal. */
static void it_preserves_cancelled_targets_and_standard_scope_independence(void) {
    begin("[aliases]\n~repair = ./not source\n");
    document(&local_document, "~fixed = ~repair/../src/item\n", CONFIGURATION_MODULE);
    const ConfigurationAliasScope *output = NULL;
    assert(module(&local_document, package, "/acme/pkg/v1.0.0/src/item", &output) == CONFIGURATION_SCOPE_OK);
    target(output, "~fixed", "/acme/pkg/v1.0.0/src/item");
    configuration_document_destroy(&local_document);
    document(&local_document, "~own = ./helper\n", CONFIGURATION_MODULE);
    assert(module(&local_document, NULL, "//group/item", &output) == CONFIGURATION_SCOPE_OK);
    target(output, "~own", "//group/helper");
    assert(configuration_alias_find(output, configuration_test_text("~repair")) == NULL);
    configuration_document_destroy(&local_document);
    document(&local_document, "~own = ../outside\n", CONFIGURATION_MODULE);
    const ConfigurationAliasScope *unchanged = output;
    assert(module(&local_document, NULL, "//item", &output) == CONFIGURATION_SCOPE_INVALID);
    assert(output == unchanged && issue.span.source == local_document.source);
    configuration_document_destroy(&local_document);
    document(&local_document, "~own = ~repair/child\n", CONFIGURATION_MODULE);
    assert(module(&local_document, NULL, "//item", &output) == CONFIGURATION_SCOPE_INVALID);
    assert(output == unchanged);
    configuration_document_destroy(&local_document);
    finish();
}
