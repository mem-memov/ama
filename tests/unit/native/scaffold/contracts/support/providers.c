/** @file
 * @brief Integrate current-source full-root identity planning with unchanged scoped native header/body error construction.
 */
/** Same local declarations deliberately appear in root-distinct Unicode/multiword modules and a legal reference cycle. */
static void prepare_providers(void) {
    directory("generated/project");
    directory("generated/project/магазин");
    directory("generated/standard");
    directory("generated/standard/магазин");
    const char *header = "struct AMA_NATIVE(State) { size_t number; };\n"
        "enum { AMA_NATIVE(reason) = 1 };\n"
        "static inline AmaStatus AMA_NATIVE(header_error)(void) { return AMA_NATIVE_ERROR(AMA_NATIVE(reason),\"header\",6); }\n";
    contract_native("generated/project/main", "@<\nproject&\n.магазин.товар данные\nstandard&\n..магазин.товар данные\n@C\n", header);
    contract_native("generated/project/магазин/товар данные", "@<\npeer&\n..магазин.товар данные\n@C\n", header);
    contract_native("generated/standard/магазин/товар данные", "@<\npeer&\n.магазин.товар данные\n@C\n", header);
}
/** The test writes native implementation code explicitly; scaffold publication has already been checked to preserve its original authored body. */
static void write_provider_body(const ScaffoldType *type) {
    FILE *output = fopen(type->body_path, "wb");
    assert(output != NULL);
    bool included = scaffold_include(output, type->interface_name) && scaffold_include(output, type->begin_name);
    assert(included);
    int written = fputs("static AmaStatus AMA_NATIVE(body_error)(unsigned int counts[3]) {\n"
        "return AMA_NATIVE_ERROR((counts[0]++,AMA_NATIVE(reason)),(counts[1]++,\"body\"),(counts[2]++,4)); }\n", output);
    included = scaffold_include(output, type->end_name);
    int closed = fclose(output);
    assert(written != EOF && included && closed == 0);
}
/** Current checked source keys, not the old vector harness, determine three noncolliding component namespaces in matching headers and bodies. */
static void it_emits_matching_current_source_error_providers_for_root_distinct_reference_cycles(void) {
    prepare_providers();
    begin_contract(".main", false);
    bool built = build_contract(false, false);
    assert(built && scaffold.providers.ready && scaffold.providers.count == 3);
    const ScaffoldType *main = contract_type(".main"), *project = contract_type(project_peer), *standard = contract_type(standard_peer);
    assert(main->error_provider == 1 && project->error_provider == 2 && standard->error_provider == 3);
    assert(project->native_prefix != standard->native_prefix && strcmp(project->native_prefix->internal_name, standard->native_prefix->internal_name) != 0);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) { assert_authored_bytes(type->body_path, original_body); }
    check_contract_headers();
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) { write_provider_body(type); }
    FILE *output = fopen("generated/provider-probe.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <assert.h>\n", output);
    assert(written != EOF);
    const ScaffoldType *types[] = {standard, project, main};
    for (size_t index = 0; index < 3; index++) {
        bool included = scaffold_include(output, types[index]->interface_name);
        assert(included);
    }
    for (size_t index = 3; index != 0;) {
        bool included = scaffold_include(output, types[--index]->body_path);
        assert(included);
    }
    written = fputs("#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR) || defined(AMA_FIELD)\n#error Native context leaked\n#endif\nint main(void) {\n", output);
    assert(written != EOF);
    for (size_t index = 0; index < 3; index++) {
        const ScaffoldType *type = types[index];
        const char *prefix = type->native_prefix->internal_name;
        written = fprintf(output, "{ unsigned int counts[3]={0}; AmaStatus header=%s_header_error(); AmaStatus body=%s_body_error(counts);\n"
            "assert(header.code==((UINT32_C(%u)<<16)|1) && body.code==header.code);\n"
            "assert(header.message_length==6 && body.message_length==4 && counts[0]==1 && counts[1]==1 && counts[2]==1); }\n",
            prefix, prefix, (unsigned)type->error_provider);
        assert(written > 0);
    }
    written = fputs("return 0; }\n", output);
    int closed = fclose(output);
    assert(written != EOF && closed == 0);
    compile_contract("generated/provider-probe.c", false, true);
    run_contract();
    end_contract();
}
