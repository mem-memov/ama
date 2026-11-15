/** @file
 * @brief Copy explicit candidate source bytes without changing physical module identity or touching files.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) preserves source ownership;
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) lets normal parser/resolver validate candidates.
 */
#include "source/module/catalog.h"
#include "source/unicode/unicode.h"
/** Catalog preparation is exclusive: callers must override before creating AST/native consumers. */
ModuleStatus module_catalog_override(ModuleCatalog *catalog, const ModuleSource *selected,
    const unsigned char *bytes, size_t length) {
    if (catalog == NULL) { return MODULE_INVALID_PATH; }
    if (!catalog->built || selected == NULL || (bytes == NULL && length != 0)) {
        catalog->issue = (ModuleIssue){.status=MODULE_INVALID_PATH};
        return MODULE_INVALID_PATH;
    }
    ModuleSource *record = catalog->first;
    while (record != NULL && record != selected) { record = record->next; }
    if (record == NULL || record->native_loaded) {
        catalog->issue = (ModuleIssue){.status=MODULE_INVALID_PATH,.path=record == NULL ? NULL : source_path(record->text)};
        return MODULE_INVALID_PATH;
    }
    const SourceText *replacement = NULL;
    SourceStatus copied = source_manager_add(&catalog->sources,source_path(record->text),bytes,length,&replacement);
    if (copied != SOURCE_OK) {
        catalog->issue = (ModuleIssue){.status=MODULE_OUT_OF_MEMORY,.path=source_path(record->text)};
        return MODULE_OUT_OF_MEMORY;
    }
    size_t error = 0;
    if (!utf8_validate(source_bytes(replacement),source_length(replacement),&error)) {
        catalog->issue = (ModuleIssue){.status=MODULE_INVALID_UTF8,.path=source_path(replacement),.source=replacement,.offset=error};
        return MODULE_INVALID_UTF8;
    }
    record->text = replacement;
    catalog->issue = (ModuleIssue){0};
    return MODULE_OK;
}
