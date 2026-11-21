/**
 * @file
 * @brief Exercise eligible versions, exact Unicode and real package-only alias expansion.
 * Contract: [selector model](../../../../../../docs/implementation/project-configuration.md#t263-selector-matching-contract).
 */

/** Eligibility is structural, unsuffixed versions stay exact, and names are never normalized. */
static void it_filters_namespace_version_and_unicode_identities_exactly(void) {
    begin();
    const DependencySelector *selector = compile("/acme/*/v2.4.14-*/src/mod1");
    assert(matches(selector, "/acme/graphs/v2.4.14-speed/src/mod1"));
    assert(matches(selector, "/acme/algorithms/sorting/v2.4.14-memory/src/mod1"));
    assert(!matches(selector, "/acme/graphs/v2.4.14/src/mod1"));
    assert(!matches(selector, "/acme/graphs/v2.4.15-speed/src/mod1"));
    selector = compile("/*");
    const char *invalid[] = {"/", "//item", "/acme", "/acme/pkg", "/acme/pkg/v1.0.0/src",
        "/acme/pkg/v01.0.0/src/item", "/acme/pkg/v1.0.0/src//item", "/acme/pkg/v1.0.0/src/\xff"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) { assert(!matches(selector, invalid[index])); }
    assert(matches(selector, "/acme/pkg/v1.0.0"));
    assert(matches(selector, "/acme/pkg/v1.0.0/tests/item"));
    selector = compile("/acme/pkg/v1.0.0/src/*界");
    assert(matches(selector, "/acme/pkg/v1.0.0/src/世界"));
    assert(!matches(selector, "/acme/pkg/v1.0.0/src/界面"));
    selector = compile("/acme/pkg/v1.0.0/src/é");
    assert(!matches(selector, "/acme/pkg/v1.0.0/src/e\xcc\x81"));
    selector = compile("/acme/pkg/v1.0.0");
    assert(!matches(selector, "/acme/pkg/v1.0.0-speed"));
    finish();
}

/** Package alias storage may retire after scalar copying; only source provenance is borrowed. */
static void it_copies_package_alias_components_without_cancelling_pattern_traversal(void) {
    begin();
    SourceArena aliases;
    source_arena_init(&aliases, sources.allocator);
    ConfigurationDocument document;
    SourceSpan source = configuration_test_text("[aliases]\n~tree = ./unused/../src\n~standard = //integer\n~version = ./unused/..\n~root = /a/..\n");
    configuration_document_init(&document, sources.allocator);
    assert(configuration_document_parse(&document, source.source, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *package = NULL;
    const char *version = "/acme/pkg/v1.0.0";
    assert(configuration_package_aliases(&aliases, &document, (const unsigned char *)version,
        strlen(version), &package, &issue) == CONFIGURATION_SCOPE_OK);
    const DependencySelector *selector = NULL;
    SourceSpan expression = configuration_test_text("~tree/*/item");
    assert(dependency_selector_compile(&patterns, expression, package, &selector, &issue) == DEPENDENCY_SELECTOR_OK);
    assert(selector->parts[selector->count - 1].origin.source == expression.source);
    assert(selector->parts[selector->count - 3].origin.source == source.source);
    const char *bad[] = {"~tree/*/../item", "~tree/./item", "~missing/item", "~standard", "//integer", "/a/../b", "~root", ""};
    for (size_t index = 0; index < sizeof(bad) / sizeof(*bad); index++) {
        const DependencySelector *output = selector;
        assert(dependency_selector_compile(&patterns, configuration_test_text(bad[index]), package,
            &output, &issue) == DEPENDENCY_SELECTOR_INVALID);
        assert(output == selector);
    }
    const ConfigurationAliasScope *local = NULL;
    assert(configuration_module_aliases(&aliases, NULL, package,
        (const unsigned char *)"/acme/pkg/v1.0.0/src/item", strlen("/acme/pkg/v1.0.0/src/item"),
        &local, &issue) == CONFIGURATION_SCOPE_OK);
    const DependencySelector *output = selector;
    assert(dependency_selector_compile(&patterns, expression, local, &output, &issue) == DEPENDENCY_SELECTOR_INVALID);
    assert(output == selector);
    assert(dependency_selector_compile(&patterns, (SourceSpan){0}, package, &output, &issue) == DEPENDENCY_SELECTOR_INVALID);
    assert(output == selector && issue.span.source == NULL);
    const DependencySelector *version_selector = NULL;
    assert(dependency_selector_compile(&patterns, configuration_test_text("~version"), package,
        &version_selector, &issue) == DEPENDENCY_SELECTOR_OK);
    assert(matches(version_selector, version));
    configuration_document_destroy(&document);
    source_arena_destroy(&aliases);
    assert(matches(selector, "/acme/pkg/v1.0.0/src/deep/tree/item"));
    assert(!matches(selector, "/acme/pkg/v1.0.0/src/item"));
    finish();
}

/** Deep component chains and long ordinary names remain bounded by inputs, not a semantic cap. */
static void it_matches_deep_and_long_identities_without_a_fixed_limit(void) {
    begin();
    char identity[5000] = "/a/pkg/v1.0.0/src";
    for (size_t index = 0; index < 1024; index++) { strcat(identity, "/a"); }
    assert(matches(compile("/a/pkg/v1.0.0/src/*"), identity));
    assert(matches(compile(identity), identity));
    const char *prefix = "/a/pkg/v1.0.0/src/";
    size_t start = strlen(prefix);
    memcpy(identity, prefix, start);
    memset(identity + start, 'a', 4096);
    identity[start + 4096] = 0;
    assert(matches(compile("/a/pkg/v1.0.0/src/a*"), identity));
    assert(!matches(compile("/a/pkg/v1.0.0/src/*b"), identity));
    finish();
}
