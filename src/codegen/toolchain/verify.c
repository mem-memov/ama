#include <string.h>
#include "internal.h"
#include "native/toolchain/listing/internal.h"

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
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        const char *name = scaffold_join(scaffold, type->native_prefix->internal_name, "", "_entry");
        if (name == NULL) { return false; }
        AmaByteView entry = {(const unsigned char *)name, strlen(name)};
        if (ama_check_native_symbols(bytes, &entry, 1) != AMA_SYMBOLS_CLEAR) {
            return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "A native entry was not inlined; no executable was published", type->body_path);
        }
    }
    return true;
}
