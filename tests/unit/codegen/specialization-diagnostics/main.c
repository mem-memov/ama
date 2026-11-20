/** @file
 * @brief Observe original Unicode source paths and active caller locations through a real captured module.
 * @see [T189 contract](../../../../docs/implementation/specialization-diagnostics.md)
 */
#include "../execution/common.c"

/** Native failure in a captured caller must report its real source file and ordinary caller, without inventing a static producer frame. */
static void it_preserves_unicode_multiword_hyphen_paths_through_captured_runtime_failure(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/src/данные");
    file("generated/project/packages/test/fixture/v1.0.0/src/данные/метод-обработки данных", "@<\ninput\n.slot\n@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < |500\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.result\n/test/fixture/v1.0.0/src/данные/метод-обработки данных\n .slot < /test/fixture/v1.0.0/src/value\n");
    execute_project("@!\ntarget\n/test/fixture/v1.0.0/src/factory\n .result\ntarget !\n", "native-entered\n", false);
    expect_diagnostic("Body failed");
    expect_diagnostic("generated/project/packages/test/fixture/v1.0.0/src/данные/метод-обработки данных:7:1");
    expect_diagnostic("generated/project/packages/test/fixture/v1.0.0/src/main:5:1");
    FILE *errors = fopen("generated/errors.txt", "rb");
    assert(errors);
    char text[8192] = {0};
    size_t length = fread(text, 1, sizeof(text) - 1, errors);
    assert(!ferror(errors) && length < sizeof(text) - 1 && fgetc(errors) == EOF);
    assert(fclose(errors) == 0);
    assert(!strstr(text, "generated/project/packages/test/fixture/v1.0.0/src/factory:") && !strstr(text, "ama_t_"));
}

/** Actual C preprocessing failure must retain the native Unicode filename and close publication without altering authored bytes or previous output. */
static void it_maps_native_c_errors_to_original_unicode_companions_and_preserves_previous_output(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/src/данные");
    native_fixture("данные/нативный-метод", "@<\ninput\n/test/fixture/v1.0.0/src/value\n@>\noutput\n/test/fixture/v1.0.0/src/value\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/task", "@<\ninput\n.slot\n@!\nleaf\n/test/fixture/v1.0.0/src/данные/нативный-метод\nleaf !\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.result\n/test/fixture/v1.0.0/src/task\n .slot < /test/fixture/v1.0.0/src/value\n");
    prepare_project("@!\ntarget\n/test/fixture/v1.0.0/src/factory\n .result\ntarget !\n");
    const ScaffoldType *native = NULL;
    for (const ScaffoldType *type = assembly.types; type; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/данные/нативный-метод") == 0) { native = type; }
    }
    assert(native);
    FILE *body = fopen(native->body_path, "ab");
    assert(body && fputs("\n#error original_native_source_marker\n", body) != EOF && fclose(body) == 0);
    const SourceText *before, *after;
    assert(source_manager_load(&sources, native->body_path, &before) == SOURCE_OK);
    directory("generated/output");
    file("generated/output/project.c", "previous successful source\n");
    FILE *errors = tmpfile();
    assert(errors && !project_codegen_compile(&assembly, errors) && !assembly.ready);
    assert(fseek(errors, 0, SEEK_SET) == 0);
    char text[32768] = {0};
    size_t length = fread(text, 1, sizeof(text) - 1, errors);
    assert(!ferror(errors) && length < sizeof(text) - 1 && fgetc(errors) == EOF && fclose(errors) == 0);
    assert(strstr(text, "original_native_source_marker") && strstr(text, "данные/нативный-метод.c:"));
    assert(!native_scaffold_publish(&assembly));
    assert(source_manager_load(&sources, native->body_path, &after) == SOURCE_OK);
    assert(source_length(before) == source_length(after) && memcmp(source_bytes(before), source_bytes(after), source_length(before)) == 0);
    const SourceText *previous;
    assert(source_manager_load(&sources, "generated/output/project.c", &previous) == SOURCE_OK);
    const char *expected = "previous successful source\n";
    assert(source_length(previous) == strlen(expected) && memcmp(source_bytes(previous), expected, strlen(expected)) == 0);
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/** Run every independent diagnostic composition explicitly. */
static void run(void) {
    it_preserves_unicode_multiword_hyphen_paths_through_captured_runtime_failure();
    it_maps_native_c_errors_to_original_unicode_companions_and_preserves_previous_output();
}

/** Return success only after original-source observations and counted teardown. */
int main(void) { run(); return 0; }
