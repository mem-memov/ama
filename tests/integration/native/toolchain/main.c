#include "../common.c"

static void it_inlines_native_entries_and_runs_label_returns_with_native_recursion(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/positive.c", options[index]);
        assert(result == 0);
        AmaSymbolCheck symbols = inspect_entry();
        assert(symbols == AMA_SYMBOLS_CLEAR);
        link_and_execute();
    }
}

static void it_rejects_surviving_entry_bodies_even_if_the_c_program_would_run(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/noninline.c", options[index]);
        assert(result == 0);
        AmaSymbolCheck symbols = inspect_entry();
        assert(symbols == AMA_SYMBOLS_ENTRY_REMAINS);
    }
}

static void it_requires_a_compiler_error_for_uninlinable_native_entries(void) {
    const char *options[] = {"-O0", "-O2"};
    for (size_t index = 0; index < 2; index++) {
        int result = compile_fixture("project/uninlinable.c", options[index]);
        assert(result != 0);
    }
}

static void it_rejects_incompatible_native_conversion_signatures(void) {
    int result = compile_fixture("project/wrong-signature.c", "-O0");
    assert(result != 0);
}

void run(void) {
    it_inlines_native_entries_and_runs_label_returns_with_native_recursion();
    it_rejects_surviving_entry_bodies_even_if_the_c_program_would_run();
    it_requires_a_compiler_error_for_uninlinable_native_entries();
    it_rejects_incompatible_native_conversion_signatures();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
