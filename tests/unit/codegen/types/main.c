#include "../type_common.c"
#include "../../../integration/process.c"
#include <inttypes.h>

static void declare_native_and_root(void) {
    type_count = 2;
    types[0] = (TypeModule){"root", &syntax[0], NULL};
    types[1] = (TypeModule){"native/buffer", NULL, "native.h"};
}

static void write_alias(FILE* output, const char* alias, GeneratedNameKind kind,
    const char* module, const char* member) {
    fprintf(output, "#define %s ", alias);
    emit_generated_name(output, kind, module, member);
    fputc('\n', output);
}

static void write_native_header(void) {
    FILE* output = fopen("generated/native.h", "wb");
    assert(output);
    fputs("#ifndef TEST_NATIVE_H\n#define TEST_NATIVE_H\n#include \"../project/native_api.h\"\n"
        "typedef TestBuffer ", output);
    emit_generated_name(output, NAME_STATE, "native/buffer", NULL);
    fputs(";\nextern const AmaType ", output);
    emit_generated_name(output, NAME_TYPE, "native/buffer", NULL);
    fputs(";\n", output);
    write_alias(output, "native_descriptor", NAME_TYPE, "native/buffer", NULL);
    fprintf(output, "#define TEST_NATIVE_IDENTITY UINT64_C(0x%016" PRIx64 ")\n#endif\n",
        source_identity_hash("native/buffer", NULL));
    close_file(output);
}

static void write_bindings(void) {
    FILE* output = fopen("generated/bindings.h", "wb");
    assert(output);
    write_alias(output, "RootState", NAME_STATE, "root", NULL);
    write_alias(output, "RootFrame", NAME_FRAME, "root", NULL);
    write_alias(output, "root_descriptor", NAME_TYPE, "root", NULL);
    write_alias(output, "frame_descriptor", NAME_FRAME_TYPE, "root", NULL);
    write_alias(output, "root_first", NAME_FIELD, "root", "first");
    write_alias(output, "root_copied", NAME_FIELD, "root", "copied");
    write_alias(output, "root_left", NAME_FIELD, "root", "left");
    write_alias(output, "root_unicode", NAME_FIELD, "root", "\u5546\u54c1");
    write_alias(output, "left_data", NAME_FIELD, "shop/item", "data");
    write_alias(output, "unicode_data", NAME_FIELD, "\u5546\u5e97/\u5546\u54c1", "data");
    write_alias(output, "frame_input", NAME_FIELD, "root", "input");
    write_alias(output, "frame_output", NAME_FIELD, "root", "output");
    write_alias(output, "NativeFrame", NAME_FRAME, "native/buffer", NULL);
    write_alias(output, "native_frame_type", NAME_FRAME_TYPE, "native/buffer", NULL);
    write_alias(output, "native_entry", NAME_TYPED_NATIVE, "native/buffer", NULL);
    write_alias(output, "invoke_native", NAME_INVOKE, "native/buffer", NULL);
    write_alias(output, "native_schema", NAME_CALL_SCHEMA, "native/buffer", NULL);
    close_file(output);
}

static void expect_process_success(char* const arguments[]) {
    int status = run_process(arguments, "generated/process.out", "generated/process.err");
    if (status) fprintf(stderr, "process failed with status %d; see generated/process.err\n", status);
    assert(status == 0);
}

static void it_compiles_separate_state_and_frame_layouts_and_runs_their_lifecycles(void) {
    setup_test();
    type_count = 5;
    types[0] = (TypeModule){"root", &syntax[0], NULL};
    types[1] = (TypeModule){"shop/item", &syntax[1], NULL};
    types[2] = (TypeModule){"other/item", &syntax[2], NULL};
    types[3] = (TypeModule){"\u5546\u5e97/\u5546\u54c1", &syntax[3], NULL};
    types[4] = (TypeModule){"native/buffer", &syntax[4], "native.h"};
    parse_interface(4, AST_PARAMETER_DECLARATION, "input\n.native.buffer\n |argument\n");
    parse_interface(4, AST_RESULT_DECLARATION, "output\n.native.buffer\n |result\n");
    parse_interface(0, AST_PARAMETER_DECLARATION, "input\n.native.buffer\n |input\n");
    parse_interface(0, AST_RESULT_DECLARATION,
        "output\n.native.buffer\n input\nsnapshot\n.root\n");
    parse_body(0, "first\n.native.buffer\n |first\ncopied\n.native.buffer\n first\n"
        "left\n.shop.item\nright\n.other.item\n\u5546\u54c1\n.\u5546\u5e97.\u5546\u54c1\n");
    parse_body(1, "data\n.native.buffer\n |left\n");
    parse_body(2, "data\n.native.buffer\n |right\n");
    parse_body(3, "data\n.native.buffer\n |\n");
    validate_types();
    close_file(header);
    close_file(implementation);
    header = fopen("generated/types.h", "wb");
    implementation = fopen("generated/types.c", "wb");
    assert(header && implementation && generate_types());
    assert(fflush(header) == 0 && fflush(implementation) == 0);
    write_native_header();
    write_bindings();
    char* compiler[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
#ifdef TEST_GENERATED_SANITIZERS
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
#endif
        "-Igenerated", "-I" TEST_ROOT "/src", "generated/types.c", "project/native.c",
        "project/driver.c", "project/second.c", TEST_ROOT "/src/runtime/value.c",
        TEST_ROOT "/src/runtime/record.c", TEST_ROOT "/src/runtime/call.c", "-o", "generated/program", NULL};
    expect_process_success(compiler);
    char* program[] = {"./generated/program", NULL};
    expect_process_success(program);
    destroy_test();
}

static void it_rejects_unvalidated_trees_without_writing_output(void) {
    setup_test();
    declare_native_and_root();
    parse_body(0, "data\n.native.buffer\n");
    assert(!generate_types() && error.error_code == ERROR_INVALID);
    expect_no_output();
    destroy_test();
}

static void it_rejects_direct_and_indirect_inline_ownership_cycles(void) {
    setup_test();
    type_count = 1;
    types[0] = (TypeModule){"root", &syntax[0], NULL};
    parse_body(0, "self\n.root\n");
    validate_types();
    assert(!generate_types() && error.error_code == ERROR_CYCLE);
    expect_no_output();
    ast_destroy(&syntax[0]);
    error_clear(&error);
    type_count = 2;
    types[1] = (TypeModule){"child", &syntax[1], NULL};
    parse_body(0, "child\n.child\n");
    parse_body(1, "parent\n.root\n");
    validate_types();
    assert(!generate_types() && error.error_code == ERROR_CYCLE);
    expect_no_output();
    destroy_test();
}

static void it_rejects_layouts_without_a_registered_type_provider(void) {
    setup_test();
    declare_native_and_root();
    parse_body(0, "data\n.native.buffer\n");
    validate_types();
    type_count = 1;
    assert(!generate_types() && strstr(error.message, "missing layout provider"));
    expect_no_output();
    destroy_test();
}

static void it_rejects_reference_and_list_layouts_until_their_abi_exists(void) {
    setup_test();
    const char* declarations[] = {"reference&\n.native.buffer\n", "items[]\n.native.buffer\n",
        "references&[]\n.native.buffer\n"};
    declare_native_and_root();
    for (size_t index = 0; index < 3; index++) {
        ast_destroy(&syntax[0]);
        error_clear(&error);
        parse_body(0, declarations[index]);
        validate_types();
        assert(!generate_types() && strstr(error.message, "owned scalar"));
        expect_no_output();
    }
    destroy_test();
}

static void it_rejects_initializers_crossing_the_state_frame_boundary(void) {
    setup_test();
    declare_native_and_root();
    parse_interface(0, AST_PARAMETER_DECLARATION, "input\n.native.buffer\n");
    parse_body(0, "saved\n.native.buffer\n input\n");
    validate_types();
    assert(!generate_types() && strstr(error.message, "same state or frame"));
    expect_no_output();
    destroy_test();
}

static void it_checks_duplicate_module_identities_and_include_names_before_emission(void) {
    setup_test();
    declare_native_and_root();
    validate_types();
    types[1].module_path = "root";
    assert(!generate_types() && strstr(error.message, "duplicate type module"));
    expect_no_output();
    error_clear(&error);
    types[1].module_path = "native/buffer";
    types[1].native_header = "native\n#error injected";
    assert(!generate_types());
    expect_no_output();
    error_clear(&error);
    types[1].native_header = "native.h";
    types[0].module_path = ".root";
    assert(!generate_types() && strstr(error.message, "normalized"));
    expect_no_output();
    destroy_test();
}

static void it_generates_empty_states_and_frames_without_zero_length_arrays(void) {
    setup_test();
    type_count = 1;
    types[0] = (TypeModule){"root", &syntax[0], NULL};
    validate_types();
    close_file(header);
    close_file(implementation);
    header = fopen("generated/types.h", "wb");
    implementation = fopen("generated/types.c", "wb");
    assert(header && implementation && generate_types());
    assert(fflush(header) == 0 && fflush(implementation) == 0);
    write_bindings();
    char* compiler[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
#ifdef TEST_GENERATED_SANITIZERS
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
#endif
        "-Igenerated", "-I" TEST_ROOT "/src", "generated/types.c", "project/empty.c",
        TEST_ROOT "/src/runtime/value.c", TEST_ROOT "/src/runtime/record.c",
        "-o", "generated/program", NULL};
    expect_process_success(compiler);
    char* program[] = {"./generated/program", NULL};
    expect_process_success(program);
    destroy_test();
}

static void it_rejects_missing_metadata_and_invalid_output_arguments(void) {
    setup_test();
    assert(!emit_type_layouts(header, header, "types.h", NULL, 0, &error));
    expect_no_output();
    error_clear(&error);
    assert(!emit_type_layouts(header, implementation, "types\".h", NULL, 0, &error));
    expect_no_output();
    error_clear(&error);
    assert(!emit_type_layouts(header, implementation, "types.h", NULL, 1, &error));
    assert(error.error_code == ERROR_INVALID);
    expect_no_output();
    error_clear(&error);
    type_count = 1;
    assert(!generate_types() && error.error_code == ERROR_INVALID);
    expect_no_output();
    destroy_test();
}

static void run(void) {
    it_compiles_separate_state_and_frame_layouts_and_runs_their_lifecycles();
    it_rejects_unvalidated_trees_without_writing_output();
    it_rejects_direct_and_indirect_inline_ownership_cycles();
    it_rejects_layouts_without_a_registered_type_provider();
    it_rejects_reference_and_list_layouts_until_their_abi_exists();
    it_rejects_initializers_crossing_the_state_frame_boundary();
    it_checks_duplicate_module_identities_and_include_names_before_emission();
    it_generates_empty_states_and_frames_without_zero_length_arrays();
    it_rejects_missing_metadata_and_invalid_output_arguments();
}

int main(void) {
    run();
    printf("unit/codegen/types: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
