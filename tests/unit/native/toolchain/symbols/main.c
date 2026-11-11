#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "native/toolchain/symbols.h"

static AmaSymbolCheck check(const char *text) {
    const unsigned char name[] = "native_entry";
    AmaByteView entry = {name, sizeof(name) - 1};
    AmaByteView listing = {(const unsigned char *) text, strlen(text)};
    return ama_check_native_symbols(listing, &entry, 1);
}

static void it_accepts_unrelated_helpers_and_complete_empty_objects(void) {
    assert(check("") == AMA_SYMBOLS_CLEAR);
    assert(check(" helper T 0 10 \t") == AMA_SYMBOLS_CLEAR);
    assert(check("allowed_helper T 0 30\nnative_entry_extra t 30 10\n") == AMA_SYMBOLS_CLEAR);
}

static void it_rejects_entry_definitions_references_and_compiler_specializations(void) {
    assert(check("native_entry t 0 30\n") == AMA_SYMBOLS_ENTRY_REMAINS);
    assert(check("native_entry U\n") == AMA_SYMBOLS_ENTRY_REMAINS);
    assert(check("native_entry.constprop.0 t 0 10\n") == AMA_SYMBOLS_ENTRY_REMAINS);
}

static void it_rejects_malformed_or_truncated_records_instead_of_certifying_them(void) {
    const char *invalid[] = {"entry", "entry TT 0", "entry T xyz ", "entry T 0 1 2", "\n", "native_entry T 0 1\nentry T xyz"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        assert(check(invalid[index]) == AMA_SYMBOLS_MALFORMED);
    }
}

void run(void) {
    it_accepts_unrelated_helpers_and_complete_empty_objects();
    it_rejects_entry_definitions_references_and_compiler_specializations();
    it_rejects_malformed_or_truncated_records_instead_of_certifying_them();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
