/** @file
 * @brief Install owned captured native companions before parsing a projected source registry.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * checks staged sources without publishing their paths or executing native bodies;
 * [DEP-03](../../../../spec/implementation/compiler.md#dep-03) retains each captured ABI once.
 */
#include "../internal.h"
#include "source/unicode/unicode.h"

/** Prepare every companion copy before publishing any field; failed scratch stays catalog-owned. */
ModuleStatus module_catalog_snapshot_native(ModuleCatalog *catalog, const ModuleSource *selected,
    const char *body, const char *header, const SourceText *abi) {
    if (catalog == NULL || !catalog->built || selected == NULL) { return MODULE_INVALID_PATH; }
    ModuleSource *record = catalog->first;
    while (record != NULL && record != selected) { record = record->next; }
    if (record == NULL || record->native_loaded) { return MODULE_INVALID_PATH; }
    const char *copied_body = body == NULL ? NULL : source_arena_copy(&catalog->arena,body);
    const char *copied_header = header == NULL ? NULL : source_arena_copy(&catalog->arena,header);
    if ((body != NULL && copied_body == NULL) || (header != NULL && copied_header == NULL)) {
        return module_fail(catalog,MODULE_OUT_OF_MEMORY,source_path(record->text));
    }
    const SourceText *copied_abi = NULL;
    if (abi != NULL) {
        if (source_manager_add(&catalog->sources,source_path(abi),source_bytes(abi),
            source_length(abi),&copied_abi) != SOURCE_OK) {
            return module_fail(catalog,MODULE_OUT_OF_MEMORY,source_path(record->text));
        }
        size_t offset;
        if (!utf8_validate(source_bytes(copied_abi),source_length(copied_abi),&offset)) {
            catalog->issue.source = copied_abi;
            catalog->issue.offset = offset;
            return module_fail(catalog,MODULE_INVALID_UTF8,source_path(copied_abi));
        }
    }
    record->native_c = copied_body;
    record->native_header = copied_header;
    record->native_abi = copied_abi;
    record->native_loaded = true;
    catalog->issue = (ModuleIssue){0};
    return MODULE_OK;
}
