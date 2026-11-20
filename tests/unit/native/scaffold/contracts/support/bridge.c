/** @file
 * @brief Compile the fixed standard integer bridge against actual authored declarations, exact root identities and explicit skeleton policy.
 */
/** Write ordinary and standard same-basename modules; only the requested entry participates in each checked closure. */
static void prepare_bridge(const char *standard_header) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/standard");
    directory("generated/standard/nested");
    const char *unrelated = "struct AMA_NATIVE(State) { size_t value; };\nvoid AMA_NATIVE(from_size)(void);\n";
    contract_native("generated/project/packages/test/fixture/v1.0.0/src/integer", "@C\n", unrelated);
    contract_native("generated/standard/integer", "@C\n", standard_header);
    contract_native("generated/standard/nested/integer", "@C\n", unrelated);
    contract_native("generated/standard/Integer", "@C\n", unrelated);
}
/** Emit a header-first isolated source without assuming the generated C symbol spelling. */
static void bridge_header_source(void) {
    FILE *output = fopen("generated/bridge-header.c", "wb");
    assert(output != NULL);
    bool included = scaffold_include(output, scaffold.selected->interface_name);
    assert(included);
    int written = fputs("#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR)\n#error Native context leaked\n#endif\n", output);
    int closed = fclose(output);
    assert(written != EOF && closed == 0);
}
/** Canonical ..integer gets a typed alias callable across the full host-size range; other identical basenames/layouts need no such declaration. */
static void it_binds_only_the_exact_standard_integer_header_bridge_and_keeps_six_abi_records(void) {
    assert(ABI_BINDING_COUNT == 6);
    const char *header = "struct AMA_NATIVE(State) { size_t value; };\nAmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t value);\n";
    const char *entries[] = {"/test/fixture/v1.0.0/src/integer", "//integer", "//nested/integer", "//Integer"};
    for (size_t index = 0; index < 4; index++) {
        prepare_bridge(header);
        begin_contract(entries[index], false);
        bool built = build_contract(false, false);
        assert(built && scaffold_size_bridge(scaffold.selected) == (index == 1));
        for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
            if (scaffold.selected->plan->type->abi.bindings[operation].present) {
                assert(!layout_text_is(scaffold.selected->plan->type->abi.bindings[operation].function, "from_size"));
            }
        }
        bool published = native_scaffold_publish(&scaffold);
        assert(published);
        check_contract_headers();
        if (index == 1) {
            const ScaffoldType *type = scaffold.selected;
            FILE *body = fopen(type->body_path, "wb");
            assert(body != NULL);
            bool included = scaffold_include(body, type->interface_name) && scaffold_include(body, type->begin_name);
            int written = fputs("AmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t value) { destination->value=value; return ama_status_success(); }\n"
                "AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }\n", body);
            included = included && scaffold_include(body, type->end_name);
            int closed = fclose(body);
            assert(included && written != EOF && closed == 0);
            FILE *output = fopen("generated/bridge-probe.c", "wb");
            assert(output != NULL);
            included = scaffold_include(output, type->interface_name) && scaffold_include(output, type->body_path);
            written = fprintf(output, "#include <assert.h>\n#include <stdint.h>\nint main(void) { %s_State zero={0}, maximum={0};\n"
                "AmaStatus a=%s_FromSize(&zero,0); AmaStatus b=%s_FromSize(&maximum,SIZE_MAX);\n"
                "assert(a.code==AMA_OK && b.code==AMA_OK && zero.value==0 && maximum.value==SIZE_MAX);\n"
                "a=%s_destroy(&zero); b=%s_destroy(&maximum); assert(a.code==AMA_OK && b.code==AMA_OK); return 0; }\n",
                type->native_prefix->internal_name, type->names[SCAFFOLD_WRITE]->internal_name,
                type->names[SCAFFOLD_WRITE]->internal_name, type->native_prefix->internal_name, type->native_prefix->internal_name);
            closed = fclose(output);
            assert(included && written > 0 && closed == 0);
            compile_contract("generated/bridge-probe.c", false, true);
            run_contract();
        }
        end_contract();
    }
}
/** Missing, wrong-return and const-destination declarations are native C build errors, never silently repaired by a generated prototype. */
static void it_rejects_missing_or_incompatible_standard_integer_bridge_declarations(void) {
    const char *headers[] = {
        "struct AMA_NATIVE(State) { size_t value; };\n",
        "struct AMA_NATIVE(State) { size_t value; };\nvoid AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t value);\n",
        "struct AMA_NATIVE(State) { size_t value; };\nAmaStatus AMA_NATIVE(from_size)(const AMA_NATIVE(State) *destination,size_t value);\n"
    };
    for (size_t index = 0; index < 3; index++) {
        prepare_bridge(headers[index]);
        begin_contract("//integer", false);
        bool built = build_contract(false, false);
        assert(built);
        bool published = native_scaffold_publish(&scaffold);
        assert(published);
        bridge_header_source();
        compile_contract("generated/bridge-header.c", true, false);
        FILE *errors = fopen("generated/contract-errors.txt", "rb");
        assert(errors != NULL);
        char bytes[8192] = {0};
        size_t count = fread(bytes, 1, sizeof(bytes) - 1, errors);
        assert(!ferror(errors) && count != 0);
        int closed = fclose(errors);
        assert(closed == 0 && strstr(bytes, "from_size") != NULL);
        end_contract();
    }
    prepare_bridge(headers[0]);
    file("generated/standard/integer.abi", "+ from_size\n- destroy\n");
    begin_contract("//integer", false);
    bool built = build_contract(false, false);
    assert(!built && scaffold.error.code == DIAGNOSTIC_TYPE_MISMATCH);
    assert(strstr(scaffold.error.message, "from_size") != NULL);
    end_contract();
}
/** An explicitly requested new standard module gets its required prototype and honest fail-first body, while existing companions keep their exact bytes without overwrite permission. */
static void it_generates_an_explicit_unwritten_standard_bridge_without_overwriting_existing_files(void) {
    const char *existing_header = "struct AMA_NATIVE(State) { size_t value; };\nAmaStatus AMA_NATIVE(from_size)(AMA_NATIVE(State) *destination,size_t value);\n";
    prepare_bridge(existing_header);
    begin_contract("//integer", false);
    bool built = build_contract(true, false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    assert_authored_bytes(scaffold.selected->body_path, original_body);
    assert_authored_bytes(scaffold.selected->header_path, existing_header);
    assert_authored_bytes("generated/standard/integer.abi", minimal_abi);
    end_contract();
    int removed = unlink("generated/standard/integer.h");
    assert(removed == 0);
    removed = unlink("generated/standard/integer.c");
    assert(removed == 0);
    removed = unlink("generated/standard/integer.abi");
    assert(removed == 0);
    begin_contract("//integer", true);
    built = build_contract(true, false);
    assert(built);
    published = native_scaffold_publish(&scaffold);
    assert(published);
    check_contract_headers();
    compile_contract(scaffold_join(&scaffold, scaffold.directory, "/", scaffold.selected->template_name), true, true);
    assert_authored_bytes("generated/standard/integer.abi", minimal_abi);
    end_contract();
}
