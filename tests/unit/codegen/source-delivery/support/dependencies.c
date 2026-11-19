/** @file
 * @brief Rebuild actual external C effects and refuse missing delivered native inputs without publishing partial output.
 * @see [T191 contract](../../../../../docs/implementation/template-source-delivery.md)
 */

/** Keep external helper definitions inside the concrete native namespace. The
 * actual preprocessor, not an Ama include parser, loads this authored header. */
static void prepare_delivery_external_header(size_t offset) {
    directory("generated/library/lib/value+");
    char text[192];
    int length = snprintf(text, sizeof(text), "static inline size_t AMA_NATIVE(offset)(size_t value) { return value+%zu; }\n", offset);
    assert(length > 0 && (size_t)length < sizeof(text));
    file("generated/library/lib/value+/offset.h", text);
    file("generated/library/lib/value.h", "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; };\n#include \"value+/offset.h\"\n");
}

/** A changed external helper has an actual observable compiled effect on the
 * native literal protocol, without changing any ordinary template source. */
static const char *delivery_shifted_literal(const ScaffoldType *type, AbiOperation operation) {
    if (operation != ABI_LITERAL || type->plan->type->module != project_find(&frontend.project, ".lib.value")) { return NULL; }
    return " { *destination=(AMA_NATIVE(State)){.address=destination}; for(size_t index=0;index<literal.length;index++) { if(literal.data[index]<'0'||literal.data[index]>'9') return ama_status_make(AMA_INVALID_LITERAL,\"Not a decimal value\",19); destination->number=destination->number*10+literal.data[index]-'0'; } destination->number=AMA_NATIVE(offset)(destination->number); return ama_status_success(); }\n";
}

/** Normal project compilation must consume new external C bytes even when
 * the delivered Ama definitions and generated include operands are unchanged. */
static void it_rebuilds_changed_external_native_headers_in_actual_consumers(void) {
    for (size_t changed = 0; changed < 2; changed++) {
        prepare_delivery_sources();
        prepare_delivery_external_header(changed ? 5 : 0);
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
        project_roots_override = roots;
        project_roots_override_count = 3;
        native_entry_override = delivery_entry;
        native_hook_override = delivery_shifted_literal;
        native_fixture_completion = complete_delivery_natives;
        execute_project(delivery_program, changed
            ? "calls=1\n22:1:0\ncalls=1\n22:2:0\ncalls=2\n28:3:0\n"
            : "calls=1\n17:1:0\ncalls=1\n17:2:0\ncalls=2\n23:3:0\n", true);
        project_roots_override = NULL;
        project_roots_override_count = 0;
        native_entry_override = NULL;
        native_hook_override = NULL;
        native_fixture_completion = NULL;
    }
}

/** Missing files remain missing: compilation refuses at its actual C boundary,
 * emits the responsible path and preserves the previous usable source bytes. */
static void it_refuses_missing_imported_native_and_external_c_inputs_without_publication(void) {
    const char *missing[] = {"generated/library/lib/value.c", "generated/library/lib/value+/offset.h"};
    const char *names[] = {"value.c", "offset.h"};
    for (size_t index = 0; index < sizeof(missing) / sizeof(missing[0]); index++) {
        prepare_delivery_sources();
        if (index) { prepare_delivery_external_header(0); }
        ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/library", NULL, 0, MODULE_ROOT_PROJECT},
            {TEST_STDLIB, NULL, 0, TEST_STDLIB_KIND}};
        project_roots_override = roots;
        project_roots_override_count = 3;
        native_entry_override = delivery_entry;
        native_fixture_completion = complete_delivery_natives;
        prepare_project(delivery_program);
        project_roots_override = NULL;
        project_roots_override_count = 0;
        native_entry_override = NULL;
        native_fixture_completion = NULL;
        assert(unlink(missing[index]) == 0);
        const SourceText *before;
        assert(source_manager_load(&sources, "generated/output/project.c", &before) == SOURCE_OK);
        FILE *errors = tmpfile();
        assert(errors);
        assert(!project_codegen_compile(&assembly, errors) && !assembly.ready);
        rewind(errors);
        char bytes[4096] = {0};
        size_t length = fread(bytes, 1, sizeof(bytes) - 1, errors);
        assert(length && !ferror(errors) && strstr(bytes, names[index]) && strstr(bytes, "No such file or directory"));
        assert(fclose(errors) == 0 && !native_scaffold_publish(&assembly));
        const SourceText *after;
        assert(source_manager_load(&sources, "generated/output/project.c", &after) == SOURCE_OK);
        assert(source_length(before) == source_length(after));
        assert(memcmp(source_bytes(before), source_bytes(after), source_length(before)) == 0);
        assert(access(missing[index], F_OK) != 0 && errno == ENOENT);
        native_scaffold_destroy(&assembly);
        destroy_frontend();
    }
}
