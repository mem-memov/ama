/**
 * @file
 * @brief Verify standalone typed native declarations, authoring models and complete ABI round trips.
 */
#include "../common.c"
#include "native/scaffold/internal.h"

/** Compile each generated interface as the first project include, verify repeated inclusion/macro cleanup and untouched authored skeleton syntax. */
static void it_generates_complete_cycle_safe_headers_and_standalone_native_scaffolds(void) {
    prepare_native();
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) {
            continue;
        }
        char probe[4096];
        int length = snprintf(probe, sizeof(probe),
            "#include \"%s\"\n#include \"%s\"\n"
            "#if defined(AMA_NATIVE) || defined(AMA_INPUT) || defined(AMA_OUTPUT) || defined(AMA_FIELD)\n#error Leaked namespace\n#endif\n"
            "int main(void) { return 0; }\n", type->interface_name, type->interface_name);
        assert(length > 0 && (size_t)length < sizeof(probe));
        file("generated/probe.c", probe);
        int compiled = compile_file("generated/probe.c");
        assert(compiled == 0);
    }
    int compiled = compile_file("generated/project/main.c");
    assert(compiled == 0);
    end_scaffold();
}

/** Parse the actual compilation database independently and execute its argv records without shell reconstruction. */
static void it_exports_a_database_that_an_independent_json_consumer_can_execute(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("python3", "python3", "-c",
            "import json,subprocess; "
            "db=json.load(open('generated/native/compile_commands.json')); "
            "assert len(db)==4; "
            "[subprocess.run(row['arguments'],cwd=row['directory'],check=True) for row in db]",
            (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Inject each measured allocation failure into model/type/artifact generation and require safe owner teardown. */
static void it_unwinds_each_scaffold_model_allocation_failure(void) {
    begin_scaffold();
    size_t start = allocation_count;
    bool built = build_scaffold(false);
    assert(built);
    size_t count = allocation_count - start;
    end_scaffold();
    for (size_t failure = 1; failure <= count; failure++) {
        begin_scaffold();
        failure_at = allocation_count + failure;
        built = build_scaffold(false);
        assert(!built && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        end_scaffold();
    }
}

static const ScaffoldType *value_type(void) {
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, ".value") == 0) {
            return type;
        }
    }
    assert(false);
    return NULL;
}

static void write_composed_harness(FILE *output) {
    const ScaffoldType *type = scaffold.selected;
    const ScaffoldType *value = value_type();
    bool emitted = scaffold_emit_body(&scaffold, output, type)
        && scaffold_emit_body(&scaffold, output, value);
    assert(emitted);
    int written = fprintf(output,
        "#include <assert.h>\n#include \"%s\"\n"
        "static void transfer(const %s *parameters, %s *results) {\n"
        "    results->AMA_OUTPUT(0)->payload->number = parameters->AMA_INPUT(0)->payload->number;\n"
        "}\n#include \"%s\"\n"
        "#if defined(AMA_NATIVE) || defined(AMA_FIELD) || defined(AMA_INPUT) || defined(AMA_OUTPUT)\n"
        "#error Leaked naming context\n#endif\n"
        "int main(void) {\n"
        "    %s_State input = {255}, output = {0};\n"
        "    %s read = {.payload = &input};\n"
        "    %s write = {.payload = &output};\n"
        "    %s parameters = {0};\n    %s results = {0};\n"
        "    parameters.%s = &read;\n    results.%s = &write;\n"
        "    transfer(&parameters, &results);\n    assert(output.number == 255);\n"
        "    %s_State state = {0};\n    AmaNativeServices services = {0};\n"
        "    AmaStatus status = ama_status_success();\n"
        "    %s_entry(&state, &parameters, &results, &services, &status);\n"
        "    assert(status.code == AMA_NATIVE_FAILURE);\n"
        "    %s value_parameters = {0};\n    %s value_results = {0};\n"
        "    %s_entry(&input, &value_parameters, &value_results, &services, &status);\n"
        "    assert(status.code == AMA_NATIVE_FAILURE);\n    return 0;\n}\n",
        type->begin_name, type->names[SCAFFOLD_PARAMETERS]->internal_name,
        type->names[SCAFFOLD_RESULTS]->internal_name, type->end_name,
        value->native_prefix->internal_name, value->names[SCAFFOLD_READ]->internal_name,
        value->names[SCAFFOLD_WRITE]->internal_name, type->names[SCAFFOLD_PARAMETERS]->internal_name,
        type->names[SCAFFOLD_RESULTS]->internal_name, type->members->identity->internal_name,
        type->members->next->next->next->next->identity->internal_name,
        type->native_prefix->internal_name, type->native_prefix->internal_name,
        value->names[SCAFFOLD_PARAMETERS]->internal_name, value->names[SCAFFOLD_RESULTS]->internal_name,
        value->native_prefix->internal_name);
    assert(written > 0);
}

/** Compile/run qualified typed C contributions with equal local names, proving namespace and data-view agreement. */
static void it_composes_typed_views_and_identically_named_native_helpers_in_one_c_unit(void) {
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    FILE *output = fopen("generated/composed.c", "wb");
    assert(output != NULL);
    write_composed_harness(output);
    int closed = fclose(output);
    assert(closed == 0);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-O2", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
            "-I" TEST_SDK, "-Igenerated/native", "generated/composed.c", "-o", "generated/composed", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    process = fork();
    assert(process >= 0);
    if (process == 0) {
        execl("generated/composed", "generated/composed", (char *)NULL);
        _exit(127);
    }
    waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    end_scaffold();
}

/** Require authoring and check-only paths to consume the same validated semantic/plan structure. */
static void it_uses_the_same_strict_frontend_for_checking_and_generation(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0};
    bool checked = compiler_frontend_check(&frontend, &root, 1, ".main");
    assert(checked && !frontend.authoring && frontend.plan.valid);
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
    bool built = build_scaffold(false);
    assert(built && scaffold.selected->plan == frontend.plan.entry);
    end_scaffold();
}

/** Deduplicate only compatible C lifecycle helpers and reject reserved names/incompatible typed signatures. */
static void it_handles_shared_compatible_hooks_and_reports_incompatible_c_roles(void) {
    const char *saved = contract;
    contract = "+ shared\n- shared\n";
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    size_t definitions = 0;
    const char *pattern = "AmaStatus AMA_NATIVE(shared)(";
    for (const ScaffoldArtifact *artifact = scaffold.first; artifact != NULL; artifact = artifact->next) {
        if (strlen(artifact->path) < strlen(scaffold.selected->template_name)
            || strcmp(artifact->path + strlen(artifact->path) - strlen(scaffold.selected->template_name),
                scaffold.selected->template_name) != 0) {
            continue;
        }
        const unsigned char *bytes = source_bytes(artifact->content);
        size_t length = source_length(artifact->content), needed = strlen(pattern);
        for (size_t offset = 0; offset + needed <= length; offset++) {
            definitions += memcmp(bytes + offset, pattern, needed) == 0;
        }
    }
    assert(definitions == 1);
    end_scaffold();
    const char *invalid[] = {"+ entry\n- destroy\n", "+ helper\n- destroy\n| helper\n"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        contract = invalid[index];
        begin_scaffold();
        built = build_scaffold(false);
        assert(!built && scaffold.error.code == DIAGNOSTIC_TYPE_MISMATCH);
        end_scaffold();
    }
    contract = saved;
}

/** Deduplicate compatible C definitions without dropping mandatory protocol records. */
static void it_retains_both_lifecycle_records_for_a_shared_compatible_c_helper(void) {
    const char *saved = contract;
    contract = "+ shared\n- shared\n";
    begin_scaffold();
    bool built = build_scaffold(true);
    assert(built);
    size_t found = 0;
    for (const ScaffoldArtifact *artifact = scaffold.first; artifact != NULL; artifact = artifact->next) {
        size_t length = strlen(artifact->path);
        if (!artifact->authored || length < 4 || strcmp(artifact->path + length - 4, ".abi") != 0) { continue; }
        AbiTable table;
        abi_table_init(&table, sources.allocator);
        ParseStatus status = parser_abi_parse(&table, artifact->content);
        assert(status == PARSE_OK && table.valid);
        assert(table.bindings[ABI_DEFAULT].present && table.bindings[ABI_DESTROY].present);
        assert(layout_text_is(table.bindings[ABI_DEFAULT].function, "shared"));
        assert(layout_text_is(table.bindings[ABI_DESTROY].function, "shared"));
        found++;
        abi_table_destroy(&table);
    }
    assert(found == 1);
    end_scaffold();
    contract = saved;
}

/** Enumerate every measured proposed-ABI/frontend allocation failure and require a retained out-of-memory diagnostic. */
static void it_unwinds_every_proposed_frontend_allocation_failure(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0};
    bool checked = compiler_frontend_authoring(&frontend, &root, 1, ".main", contract);
    assert(checked);
    size_t count = allocation_count;
    destroy_frontend();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_frontend();
        failure_at = failure;
        checked = compiler_frontend_authoring(&frontend, &root, 1, ".main", contract);
        assert(!checked && frontend.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_frontend();
    }
}

void run(void) {
    it_generates_complete_cycle_safe_headers_and_standalone_native_scaffolds();
    it_exports_a_database_that_an_independent_json_consumer_can_execute();
    it_unwinds_each_scaffold_model_allocation_failure();
    it_composes_typed_views_and_identically_named_native_helpers_in_one_c_unit();
    it_uses_the_same_strict_frontend_for_checking_and_generation();
    it_handles_shared_compatible_hooks_and_reports_incompatible_c_roles();
    it_unwinds_every_proposed_frontend_allocation_failure();
    it_retains_both_lifecycle_records_for_a_shared_compatible_c_helper();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
