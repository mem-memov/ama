/**
 * @file
 * @brief Check all measured scope allocation refusals and preserved owners.
 * Contract: [configuration](../../../../../../docs/implementation/project-configuration.md).
 */
/** Fail every measured module preparation allocation and retain shared inputs and output. */
static void it_cleans_all_module_scope_allocation_refusals(void) {
    begin("[aliases]\n~shared = ./src\n");
    document(&local_document, "~local = ~shared/child\n~other = ./other\n", CONFIGURATION_MODULE);
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    const ConfigurationAliasScope *output = NULL;
    size_t before = allocation_count;
    assert(configuration_module_aliases(&temporary, &local_document, package,
        (const unsigned char *)"/acme/pkg/v1.0.0/src/item", sizeof("/acme/pkg/v1.0.0/src/item") - 1, &output, &issue) == CONFIGURATION_SCOPE_OK);
    size_t allocations = allocation_count - before;
    source_arena_destroy(&temporary);
    for (size_t i = 1; i <= allocations; i++) {
        size_t live = live_allocations;
        source_arena_init(&temporary, sources.allocator);
        output = package;
        failure_at = allocation_count + i;
        assert(configuration_module_aliases(&temporary, &local_document, package,
            (const unsigned char *)"/acme/pkg/v1.0.0/src/item", sizeof("/acme/pkg/v1.0.0/src/item") - 1, &output, &issue) == CONFIGURATION_SCOPE_OUT_OF_MEMORY);
        failure_at = 0;
        assert(output == package);
        source_arena_destroy(&temporary);
        assert(live_allocations == live);
    }
    configuration_document_destroy(&local_document);
    finish();
}

/** Every package allocation refusal leaves output unpublished and parsed inputs intact. */
static void it_cleans_all_package_scope_allocation_refusals_and_rejects_invalid_profiles(void) {
    begin("[aliases]\n~z = //z\n~a = ./src\n");
    SourceArena temporary;
    source_arena_init(&temporary, sources.allocator);
    const ConfigurationAliasScope *output = NULL;
    const char *version = "/acme/pkg/v1.0.0";
    size_t before = allocation_count;
    assert(configuration_package_aliases(&temporary, &package_document,
        (const unsigned char *)version, strlen(version), &output, &issue) == CONFIGURATION_SCOPE_OK);
    size_t allocations = allocation_count - before;
    source_arena_destroy(&temporary);
    for (size_t i = 1; i <= allocations; i++) {
        size_t live = live_allocations;
        source_arena_init(&temporary, sources.allocator);
        output = package;
        failure_at = allocation_count + i;
        assert(configuration_package_aliases(&temporary, &package_document,
            (const unsigned char *)version, strlen(version), &output, &issue) == CONFIGURATION_SCOPE_OUT_OF_MEMORY);
        failure_at = 0;
        assert(output == package && package_document.valid);
        source_arena_destroy(&temporary);
        assert(live_allocations == live);
    }
    output = package;
    assert(configuration_package_aliases(&arena, NULL, (const unsigned char *)version,
        strlen(version), &output, &issue) == CONFIGURATION_SCOPE_INVALID);
    assert(module(&package_document, package, "/acme/pkg/v1.0.0/src/item", &output) == CONFIGURATION_SCOPE_INVALID);
    assert(configuration_package_aliases(&arena, &package_document, (const unsigned char *)"//version",
        strlen("//version"), &output, &issue) == CONFIGURATION_SCOPE_INVALID);
    assert(output == package);
    finish();
}
