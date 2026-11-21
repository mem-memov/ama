/**
 * @file
 * @brief Check namespace-isolated standard policy and normalized local-alias handoffs.
 * Contract: [standard policy](../../../../../../docs/implementation/project-configuration.md#t265-standard-origin-policy-contract).
 */

/** Root kinds partition permission; words such as src/tests do not change the namespace. */
static void it_checks_standard_dependency_partitions_without_package_scope_or_effects(void) {
    begin();
    const ModulePathValue *source = path("//算法/适配器");
    const char *targets[] = {"//integer", "//tests/helper", "//src/value", "//名字/有 空格",
        "/acme/pkg/v1.0.0/src/value", "/acme/pkg/v1.0.0/tests/value", "/厂/包/v1.0.0/src/数据"};
    for (size_t index = 0; index < sizeof(targets) / sizeof(*targets); index++) {
        const ModulePathValue *target = path(targets[index]);
        size_t allocations = allocation_count, live = live_allocations;
        failure_at = allocations + 1;
        DependencyEdgeDecision decision = dependency_standard_edge_check(source, target);
        failure_at = 0;
        assert(decision == (index < 4 ? DEPENDENCY_EDGE_ALLOWED : DEPENDENCY_EDGE_STANDARD_ORDINARY));
        assert(allocation_count == allocations && live_allocations == live);
        assert(strcmp(source->text, "//算法/适配器") == 0 && strcmp(target->text, targets[index]) == 0);
    }
    assert(dependency_standard_edge_check(NULL, source) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_standard_edge_check(source, NULL) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_standard_edge_check(path("//"), source) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_standard_edge_check(source, path("/acme/pkg/v1.0.0")) == DEPENDENCY_EDGE_INVALID);
    assert(dependency_standard_edge_check(path("/acme/pkg/v1.0.0/src/item"), source) == DEPENDENCY_EDGE_INVALID);
    finish();
}

/** Legal alias declarations import nothing; actual normalized uses determine the policy. */
static void it_checks_standard_local_alias_targets_after_full_normalization(void) {
    begin();
    ConfigurationDocument document;
    SourceSpan ini = configuration_test_text("~outside = /acme/pkg/v1.0.0/src/value\n~inside = ./helper\n");
    configuration_document_init(&document, sources.allocator);
    assert(configuration_document_parse(&document, ini.source, CONFIGURATION_MODULE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *scope = NULL;
    const char *module = "//group/adapter";
    assert(configuration_module_aliases(&metadata, &document, NULL, (const unsigned char *)module,
        strlen(module), &scope, &issue) == CONFIGURATION_SCOPE_OK);
    const ModulePathValue *source = path(module), *target = NULL;
    SourceSpan expression = configuration_test_text("~outside/../value");
    assert(module_path_normalize(&metadata, expression, scope->base, configuration_alias_lookup,
        (void *)scope, &target, &issue) == MODULE_PATH_OK);
    assert(target->expression.source == expression.source);
    assert(dependency_standard_edge_check(source, target) == DEPENDENCY_EDGE_STANDARD_ORDINARY);
    expression = configuration_test_text("~inside/../helper");
    assert(module_path_normalize(&metadata, expression, scope->base, configuration_alias_lookup,
        (void *)scope, &target, &issue) == MODULE_PATH_OK);
    assert(strcmp(target->text, "//group/helper") == 0);
    assert(dependency_standard_edge_check(source, target) == DEPENDENCY_EDGE_ALLOWED);
    configuration_document_destroy(&document);
    finish();
}
