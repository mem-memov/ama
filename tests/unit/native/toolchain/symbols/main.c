/**
 * @file
 * @brief Reject malformed symbol listings and surviving exact native entry families.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "native/toolchain/symbols.h"

/**
 * Classify a bounded supplied symbol listing using the production checker and exact expected
 * entry name; no object/source-name heuristic.
 */
static AmaSymbolCheck check(const char *text) {
    const unsigned char name[] = "native_entry";
    AmaByteView entry = {name, sizeof(name) - 1};
    AmaByteView listing = {(const unsigned char *) text, strlen(text)};
    return ama_check_entry_symbols(listing, &entry, 1);
}

/**
 * Assert that the component accepts unrelated helpers and complete empty objects. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_accepts_unrelated_helpers_and_complete_empty_objects(void) {
    assert(check("") == AMA_SYMBOLS_CLEAR);
    assert(check(" helper T 0 10 \t") == AMA_SYMBOLS_CLEAR);
    assert(check("allowed_helper T 0 30\nnative_entry_extra t 30 10\n") == AMA_SYMBOLS_CLEAR);
}

/**
 * Assert that the component rejects entry definitions references and compiler specializations.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_entry_definitions_references_and_compiler_specializations(void) {
    assert(check("native_entry t 0 30\n") == AMA_SYMBOLS_ENTRY_REMAINS);
    assert(check("native_entry U\n") == AMA_SYMBOLS_ENTRY_REMAINS);
    assert(check("native_entry.constprop.0 t 0 10\n") == AMA_SYMBOLS_ENTRY_REMAINS);
}

/**
 * Assert that the component rejects malformed or truncated records instead of certifying them.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_rejects_malformed_or_truncated_records_instead_of_certifying_them(void) {
    const char *invalid[] = {"entry", "entry TT 0", "entry T xyz ", "entry T 0 1 2", "\n", "native_entry T 0 1\nentry T xyz"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        assert(check(invalid[index]) == AMA_SYMBOLS_MALFORMED);
    }
}

/** Check native and Ama control-entry registries without confusing longer unrelated identifiers.
 * A later forbidden record is still found; malformed evidence still takes precedence.
 */
static void it_checks_multiple_entry_kinds_using_complete_registered_names(void) {
    static const unsigned char native[] = "native_entry", control[] = "ama_label_1";
    const AmaByteView entries[] = {{native, sizeof(native) - 1}, {control, sizeof(control) - 1}};
    const char *listings[] = {
        "native_entry_helper T 0 10\nama_label_10 T 10 10\n",
        "helper T 0 10\nama_label_1 T 10 10\n",
        "ama_label_1.constprop.0 t 0 10\n",
        "ama_label_1 T 0 10\nmalformed\n"
    };
    const AmaSymbolCheck expected[] = {AMA_SYMBOLS_CLEAR, AMA_SYMBOLS_ENTRY_REMAINS,
        AMA_SYMBOLS_ENTRY_REMAINS, AMA_SYMBOLS_MALFORMED};
    for (size_t index = 0; index < sizeof(listings) / sizeof(*listings); index++) {
        AmaByteView listing = {(const unsigned char *)listings[index], strlen(listings[index])};
        assert(ama_check_entry_symbols(listing, entries, 2) == expected[index]);
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_accepts_unrelated_helpers_and_complete_empty_objects();
    it_rejects_entry_definitions_references_and_compiler_specializations();
    it_rejects_malformed_or_truncated_records_instead_of_certifying_them();
    it_checks_multiple_entry_kinds_using_complete_registered_names();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
