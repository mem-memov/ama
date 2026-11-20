/** @file
 * @brief Refuse open native field suppliers, invalid concrete ABI signatures and malformed selected lifecycle results.
 * @see [T190 contract](../../../../../docs/implementation/native-template-composition.md)
 */

/** A selected export adds no native formal, but its supplying producer must
 * still close independently before any native layout or header is emitted. */
static void it_refuses_open_static_suppliers_for_every_native_storage_form(void) {
    const char *forms[] = {"value", "value&", "value[]", "value&[]"};
    for (size_t index = 0; index < sizeof(forms) / sizeof(forms[0]); index++) {
        directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
        char text[128];
        int length = snprintf(text, sizeof(text), "@<\n%s\n/test/fixture/v1.0.0/src/factory\n .value\n@C\n", forms[index]);
        assert(length > 0 && (size_t)length < sizeof(text));
        native_fixture("main", text);
        file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.value\n.missing\n");
        setup_frontend();
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        assert(!compiler_frontend_check(&frontend, &root, 1, "/test/fixture/v1.0.0/src/main"));
        assert(frontend.static_inventory.valid && !frontend.valid && !frontend.plan_initialized);
        assert(static_module_find(&frontend.static_inventory, frontend.project.entry)->parameter_count == 0);
        assert(diagnostic_valid(&frontend.error) && frontend.proposed_sources.first == NULL);
        destroy_frontend();
    }
    const char *companions[] = {"generated/project/packages/test/fixture/v1.0.0/src/main.h", "generated/project/packages/test/fixture/v1.0.0/src/main.c", "generated/project/packages/test/fixture/v1.0.0/src/main.abi"};
    for (size_t index = 0; index < sizeof(companions) / sizeof(companions[0]); index++) {
        assert(unlink(companions[index]) == 0);
    }
}

/** A real C type disagreement cannot be repaired by template application;
 * failed compilation preserves authored bytes and previous published source. */
static void it_refuses_invalid_native_entry_types_and_preserves_authored_and_previous_outputs(void) {
    prepare_capability_sources();
    wrong_entry_result = true;
    prepare_project(capability_program);
    wrong_entry_result = false;
    const SourceText *before;
    assert(source_manager_load(&sources, "generated/project/packages/test/fixture/v1.0.0/src/resource.c", &before) == SOURCE_OK);
    const SourceText *original;
    assert(source_manager_load(&sources, "generated/output/project.c", &original) == SOURCE_OK);
    FILE *errors = tmpfile();
    assert(errors);
    assert(!project_codegen_compile(&assembly, errors) && !assembly.ready);
    rewind(errors);
    char message[4096] = {0};
    size_t length = fread(message, 1, sizeof(message) - 1, errors);
    assert(length && !ferror(errors) && strstr(message, "conflicting types for"));
    assert(fclose(errors) == 0 && !native_scaffold_publish(&assembly));
    const SourceText *after, *previous;
    assert(source_manager_load(&sources, "generated/project/packages/test/fixture/v1.0.0/src/resource.c", &after) == SOURCE_OK);
    assert(source_length(before) == source_length(after));
    assert(memcmp(source_bytes(before), source_bytes(after), source_length(before)) == 0);
    assert(source_manager_load(&sources, "generated/output/project.c", &previous) == SOURCE_OK);
    assert(source_length(previous) == source_length(original));
    assert(memcmp(source_bytes(previous), source_bytes(original), source_length(previous)) == 0);
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/** Selected constructors and destructors keep the ordinary checked status
 * protocol. Invalid success payloads abort safely instead of entering Ama. */
static void it_checks_selected_native_lifecycle_statuses_through_template_execution(void) {
    for (int operation = ABI_DEFAULT; operation <= ABI_CLONE; operation++) {
        prepare_capability_sources();
        malformed_lifecycle = operation;
        malformed_lifecycle_module = "/test/fixture/v1.0.0/src/resource";
        /* Replacing initial native state can invoke destruction before the
         * algorithm; malformed lifecycle status must prevent consumer output. */
        execute_project(capability_program, "", false);
        expect_diagnostic("Malformed native status");
    }
    malformed_lifecycle = -1;
    malformed_lifecycle_module = NULL;
}
