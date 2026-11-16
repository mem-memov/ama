/**
 * @file
 * @brief Reject malformed or stripped symbol evidence before exact entry verification.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Require supported POSIX nm records with main before verifying native and IR control entries.
 */
#include <string.h>
#include "internal.h"
#include "native/toolchain/listing/internal.h"

/**
 * Run supported nm output inspection, require complete valid records and main evidence, then
 * reject every surviving exact entry/compiler-family symbol. Verification is specific to the
 * supported unstripped non-LTO workflow.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
bool codegen_verify_object(NativeScaffold *scaffold, const char *stage, FILE *errors) {
    const char *object = scaffold_join(scaffold, stage, "/", "program.o");
    const char *capture = scaffold_join(scaffold, stage, "/", "symbols.txt");
    if (object == NULL || capture == NULL) { return false; }
    char *arguments[] = {"nm", "--format=posix", (char *)object, NULL};
    if (!codegen_run_tool(scaffold, arguments, capture, errors)) { return false; }
    const SourceText *listing = NULL;
    SourceStatus loaded = source_manager_load(&scaffold->texts, capture, &listing);
    if (loaded != SOURCE_OK) { return scaffold_error(scaffold, loaded == SOURCE_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_IO_FAILURE, "Cannot inspect the native object symbol table", capture); }
    AmaByteView bytes = {source_bytes(listing), source_length(listing)};
    bool main_found = false;
    for (size_t start = 0; start < bytes.length;) {
        size_t end = start;
        while (end < bytes.length && bytes.data[end] != '\n') { end++; }
        AmaByteView symbol = {0};
        if (!ama_listing_record((AmaByteView){bytes.data + start, end - start}, &symbol)) {
            return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "The native object symbol table is malformed", capture);
        }
        if (symbol.length == 4 && memcmp(symbol.data, "main",4) == 0) { main_found = true; }
        start = end + 1;
    }
    if (!main_found) { return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Native verification requires an unstripped object containing main", object); }
    return codegen_verify_entries(scaffold, bytes);
}
