#include "../common.c"
#include "codegen/native.h"

static void declare_native_interface(void) {
    Token zero = {.type = TOKEN_LITERAL, .lexeme = "0"};
    Token alias_path = {.type = TOKEN_MODULE_PATH, .lexeme = ".sys.io.print"};
    Symbol* value = add_declarative_symbol(symbols, "math/scale", "value", SYMBOL_PARAMETER, zero, &error);
    Symbol* factor = add_declarative_symbol(symbols, "math/scale", "factor", SYMBOL_PARAMETER, zero, &error);
    Symbol* result = add_declarative_symbol(symbols, "math/scale", "answer", SYMBOL_RESULT, zero, &error);
    Symbol* alias = add_declarative_symbol(symbols, "math/scale", "print", SYMBOL_MODULE_ALIAS, alias_path, &error);
    assert(value && factor && result && alias);
}

static void expect_fragment(const char* expected) {
    int flushed = fflush(output);
    assert(flushed == 0);
    rewind(output);
    char text[8192];
    size_t length = fread(text, 1, sizeof(text) - 1, output);
    assert(!ferror(output));
    text[length] = '\0';
    assert(strstr(text, expected) != NULL);
}

static void it_builds_value_parameters_and_pointer_results_in_declaration_order(void) {
    setup_test();
    declare_native_interface();
    bool emitted = emit_native_header(output, symbols, "math/scale", &error);
    assert(emitted);
    expect_fragment("void ama_native_ba10b85aab5986ed(size_t ama_parameter_ad644811019d041a, size_t ama_parameter_9206860daed64081, size_t* ama_result_a0808ecf2b962110);");
    expect_fragment("/* Ama: math/scale */");
    expect_fragment("/* Ama: answer */");
    destroy_test();
}

static void it_passes_every_result_address_through_the_wrapper(void) {
    setup_test();
    declare_native_interface();
    bool emitted = emit_native_wrapper(&emitter, "math/scale", &error);
    assert(emitted);
    expect_fragment("void ama_call_ba10b85aab5986ed(ama_state_ba10b85aab5986ed* state)");
    expect_fragment("ama_native_ba10b85aab5986ed(state->ama_field_ad644811019d041a, state->ama_field_9206860daed64081, &state->ama_field_a0808ecf2b962110);");
    destroy_test();
}

static void it_uses_void_for_an_empty_native_signature(void) {
    setup_test();
    bool emitted = emit_native_header(output, symbols, "nothing", &error);
    assert(emitted);
    expect_fragment("void ama_native_1745d7fe6d23feec(void);");
    destroy_test();
}

static void it_accepts_unicode_interface_names_and_reports_small_name_buffers(void) {
    setup_test();
    Token zero = {.type = TOKEN_LITERAL, .lexeme = "0"};
    Symbol* value = add_declarative_symbol(symbols, "unicode", "\u03bbvalue", SYMBOL_PARAMETER, zero, &error);
    assert(value != NULL);
    bool emitted = emit_native_header(output, symbols, "unicode", &error);
    assert(emitted);
    expect_fragment("/* Ama: \u03bbvalue */");
    char name[2];
    bool named = native_function_name("unicode", name, sizeof(name), &error);
    assert(!named && error.error_code == ERROR_LIMIT);
    destroy_test();
}

static void it_hashes_interface_names_that_are_c_keywords(void) {
    setup_test();
    Token zero = {.type = TOKEN_LITERAL, .lexeme = "0"};
    Symbol* keyword = add_declarative_symbol(symbols, "keyword", "switch", SYMBOL_PARAMETER, zero, &error);
    assert(keyword != NULL);
    bool emitted = emit_native_header(output, symbols, "keyword", &error);
    assert(emitted);
    expect_fragment("/* Ama: switch */");
    expect_fragment("void ama_native_f72a39a84561d426(size_t ama_parameter_db12f96c9520ac1f);");
    destroy_test();
}

void run(void) {
    it_builds_value_parameters_and_pointer_results_in_declaration_order();
    it_passes_every_result_address_through_the_wrapper();
    it_uses_void_for_an_empty_native_signature();
    it_hashes_interface_names_that_are_c_keywords();
    it_accepts_unicode_interface_names_and_reports_small_name_buffers();
}

int main(void) {
    run();
    printf("unit/codegen/native: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
