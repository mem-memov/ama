/** @file
 * @brief Check native runtime fields selected from concrete static exports and independently compile their generated headers.
 * @see [T190 contract](../../../../../docs/implementation/native-template-composition.md)
 */

/** Probe source files own copied include text, never borrowed model pointers. */
static size_t native_header_probes;

/** The adapter has four runtime inputs and one output, irrespective of the
 * static export spelling. Checked reference access lasts only for this entry. */
static bool exported_entry(FILE *body, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".adapter") != 0) { return false; }
    const SemanticType *child = type->members->field->symbol->type;
    const ScaffoldType *resource = native_scaffold_type(&assembly, child);
    assert(resource);
    int written = fprintf(body,
        "AmaStatus AMA_NATIVE(inspect)(const %s *,void *);\n"
        "size_t observed=0; *status=parameters->AMA_INPUT(1)->with_read("
        "parameters->AMA_INPUT(1)->storage,AMA_NATIVE(inspect),&observed);\n"
        "if(status->code!=AMA_OK)return; assert(observed==7);\n"
        "assert(parameters->AMA_INPUT(2)->storage && parameters->AMA_INPUT(3)->storage);\n"
        "results->AMA_OUTPUT(0)->payload->number=parameters->AMA_INPUT(0)->payload->number;\n"
        "puts(\"selected-native\");\n", resource->names[SCAFFOLD_READ]->internal_name);
    assert(written >= 0);
    return true;
}

/** Verify resolved storage/ABI ordinals, then write independent header probes
 * and the concrete checked-reader callback after the fixture-owned entry. */
static void inspect_exported_native_model(NativeScaffold *model) {
    const StaticModule *definition = static_module_find(&frontend.static_inventory,
        project_find(&frontend.project, ".adapter"));
    assert(definition && definition->parameter_count == 0);
    bool adapter = false;
    for (const ScaffoldType *type = model->types; type; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        char path[96];
        int length = snprintf(path, sizeof(path), "generated/header-probe-%zu.c", native_header_probes++);
        assert(length > 0 && (size_t)length < sizeof(path));
        FILE *output = fopen(path, "wb");
        assert(output && scaffold_include(output, type->interface_name));
        assert(fputs("#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR) || defined(AMA_FIELD) || defined(AMA_INPUT) || defined(AMA_OUTPUT)\n#error Native context leaked\n#endif\n_Static_assert(1,\"header alone\");\n", output) != EOF);
        assert(fclose(output) == 0);
        if (type->plan->type->module != definition->definition) { continue; }
        adapter = true;
        assert(type->plan->type->counts[SYMBOL_PARAMETER] == 4);
        assert(type->plan->type->counts[SYMBOL_RESULT] == 1);
        assert(type->plan->type->counts[SYMBOL_STATE] == 0);
        const StorageForm forms[] = {STORAGE_OWNED, STORAGE_REFERENCE,
            STORAGE_OWNING_LIST, STORAGE_REFERENCE_LIST, STORAGE_OWNED};
        const SemanticType *resource = NULL;
        size_t count = 0;
        for (const ScaffoldMember *member = type->members; member; member = member->next) {
            assert(count < 5 && member->field->form == forms[count]);
            const SemanticSymbol *symbol = member->field->symbol;
            assert(symbol->role == (count < 4 ? SYMBOL_PARAMETER : SYMBOL_RESULT));
            assert(symbol->role_ordinal == (count < 4 ? count : 0));
            if (!resource) { resource = symbol->type; }
            assert(symbol->type == resource);
            count++;
        }
        assert(count == 5 && resource->module == project_find(&frontend.project, ".resource"));
        const ScaffoldType *child = native_scaffold_type(model, resource);
        FILE *body = fopen(type->body_path, "ab");
        assert(body && scaffold_include(body, type->begin_name));
        assert(fprintf(body, "AmaStatus AMA_NATIVE(inspect)(const %s *value,void *context) { *(size_t *)context=value->payload->number; return ama_status_success(); }\n",
            child->names[SCAFFOLD_READ]->internal_name) >= 0);
        assert(scaffold_include(body, type->end_name) && fclose(body) == 0);
    }
    assert(adapter && native_header_probes);
}

/** Each generated interface is the first include of its own strict translation
 * unit, checked after publication without retaining expired scaffold views. */
static void check_exported_headers(void) {
    for (size_t index = 0; index < native_header_probes; index++) {
        char path[96];
        int length = snprintf(path, sizeof(path), "generated/header-probe-%zu.c", index);
        assert(length > 0 && (size_t)length < sizeof(path));
        pid_t process = fork();
        assert(process >= 0);
        if (!process) {
            execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror",
                "-I", TEST_SDK, "-I", "generated/output", "-fsyntax-only", path, (char *)NULL);
            _exit(127);
        }
        int status;
        assert(waitpid(process, &status, 0) == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
    native_header_probes = 0;
}

/** Closed static outputs contribute only their ordinary runtime storage forms;
 * native references remain declared checked cells rather than opaque state. */
static void it_executes_native_owned_reference_and_list_fields_from_closed_static_exports(void) {
    directory("generated/project");
    native_fixture("resource", "@C\n");
    native_fixture("adapter", "@<\nowned\n.factory\n ...value\nreference&\n.factory\n ...value\nitems[]\n.factory\n ...value\nreferences&[]\n.factory\n ...value\n@>\nresult\n.factory\n ...value\n@C\n");
    native_fixture("consumer", "@<\ninput\n.resource\n@>\noutput\n.resource\n@C\n");
    file("generated/project/source", "@...\n...out\n.resource\n");
    file("generated/project/factory", "@...\n.source\n ...out > ...value\n...unused\n.resource\n");
    native_entry_override = exported_entry;
    native_fixture_completion = inspect_exported_native_model;
    execute_project("@!\nvalue\n.resource\n |7\nitems[]\n.resource\n |3\nreferences&[]\n.resource\n &value\nmethod\n.adapter\nmethod !\n owned < |7\n reference& < &value\n items[] < items[]\n references&[] < references&[]\n result > result\nprobe\n.consumer\nprobe !\n < result\n", "selected-native\n7:1:0\n", true);
    native_entry_override = NULL;
    native_fixture_completion = NULL;
    check_exported_headers();
    const char *companions[] = {"generated/project/adapter", "generated/project/adapter.h",
        "generated/project/adapter.c", "generated/project/adapter.abi"};
    for (size_t index = 0; index < sizeof(companions) / sizeof(companions[0]); index++) {
        assert(unlink(companions[index]) == 0);
    }
}
