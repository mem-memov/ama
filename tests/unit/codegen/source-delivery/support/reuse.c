/** @file
 * @brief Compare copied canonical identities and project C across independent source-library consumers.
 * @see [T191 contract](../../../../../docs/implementation/template-source-delivery.md)
 */

/** Copied evidence remains owned after each frontend and scaffold expires. */
static unsigned char *delivery_encoding;
static size_t delivery_encoding_length;
static unsigned char *delivery_c;
static size_t delivery_c_length;

/** A private concrete counter demonstrates instance state without constraining
 * the ordinary template's input/output parameter to that counter's Type. */
static bool delivery_entry(FILE *body, const ScaffoldType *type) {
    if (type->plan->type->module != project_find(&frontend.project, ".lib.counter")) { return false; }
    assert(fputs("state->calls++; printf(\"calls=%zu\\n\",state->calls);\n", body) != EOF);
    return true;
}

/** Compare exact bytes across expired owners; no digest or pointer equality
 * stands in for cross-build nominal identity or deterministic generated output. */
static void compare_delivery_bytes(unsigned char **saved, size_t *size, const unsigned char *bytes, size_t length) {
    if (*saved) { assert(*size == length && memcmp(*saved, bytes, length) == 0); return; }
    *saved = malloc(length ? length : 1);
    assert(*saved);
    memcpy(*saved, bytes, length);
    *size = length;
}

/** Only explicit fixture-owned imported native definitions are completed. Their
 * generated contexts remain the same concrete protocol used by project natives. */
static void complete_delivery_natives(NativeScaffold *model) {
    const char *natives[] = {".lib.value", ".lib.counter", ".lib.sink"};
    for (size_t index = 0; index < sizeof(natives) / sizeof(natives[0]); index++) {
        const ProjectModule *definition = project_find(&frontend.project, natives[index]);
        assert(definition);
        bool emitted = false;
        for (const ScaffoldType *type = model->types; type; type = type->next) {
            if (type->plan->type->module != definition) { continue; }
            assert(!emitted && definition->syntax.sections.kind == MODULE_NATIVE);
            executable_native(type);
            emitted = true;
        }
        assert(emitted);
    }
}

/** Validate complete source identity and deterministic output after imported
 * native fixture bodies have been completed. */
static void complete_delivery_model(NativeScaffold *model) {
    complete_delivery_natives(model);
    const ProjectModule *origin = project_find(&frontend.project, ".lib.forward");
    size_t owners = 0;
    for (const ScaffoldType *type = model->types; type; type = type->next) {
        if (type->plan->type->module != origin) { continue; }
        owners++;
        const StaticTerm *owner = type->plan->type->specialization->context->registry_use->candidate->key->owner_type;
        assert(owner->as.nominal.origin->definition == origin && owner->as.nominal.capture_count == 1);
        const StaticTermCapture *capture = owner->as.nominal.first_capture;
        assert(capture->formal->owner->definition == origin);
        assert(capture->value->as.nominal.origin->definition == project_find(&frontend.project, ".lib.value"));
        compare_delivery_bytes(&delivery_encoding, &delivery_encoding_length,
            type->native_prefix->encoding, type->native_prefix->encoding_length);
    }
    assert(owners == 1);
    char project_path[1024];
    int length = snprintf(project_path, sizeof(project_path), "%s/project.c", model->directory);
    assert(length > 0 && (size_t)length < sizeof(project_path));
    bool emitted = false;
    for (const ScaffoldArtifact *artifact = model->first; artifact; artifact = artifact->next) {
        if (strcmp(artifact->path, project_path) != 0) { continue; }
        assert(!emitted);
        compare_delivery_bytes(&delivery_c, &delivery_c_length,
            source_bytes(artifact->content), source_length(artifact->content));
        emitted = true;
    }
    assert(emitted);
}

/** Both configured root orders resolve one complete Type and exact C assembly;
 * three real calls observe distinct private counters and shared code only. */
static void it_reuses_source_library_types_and_code_across_root_orders_without_sharing_instances(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        prepare_delivery_sources();
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
        if (reverse) { ModuleRoot swap = roots[0]; roots[0] = roots[1]; roots[1] = swap; }
        project_roots_override = roots;
        project_roots_override_count = 3;
        native_entry_override = delivery_entry;
        native_fixture_completion = complete_delivery_model;
        execute_project(delivery_program, "calls=1\n17:1:0\ncalls=1\n17:2:0\ncalls=2\n23:3:0\n", true);
        project_roots_override = NULL;
        project_roots_override_count = 0;
        native_entry_override = NULL;
        native_fixture_completion = NULL;
    }
    free(delivery_encoding);
    free(delivery_c);
    delivery_encoding = delivery_c = NULL;
    delivery_encoding_length = delivery_c_length = 0;
}

/** A concrete imported native protocol still refuses invalid literals before
 * any private template counter is entered, preserving its original error. */
static void it_reports_imported_native_literal_failure_before_entering_the_library_template(void) {
    prepare_delivery_sources();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
        {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
    project_roots_override = roots;
    project_roots_override_count = 3;
    native_entry_override = delivery_entry;
    native_fixture_completion = complete_delivery_natives;
    execute_project("@!\nmethod\n.lib.assembly\n ...result\nsink\n.lib.sink\nmethod !\n < |invalid\n", "", false);
    expect_diagnostic("Not a decimal value");
    project_roots_override = NULL;
    project_roots_override_count = 0;
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
