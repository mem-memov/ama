/**
 * @file
 * @brief Retain unique canonical selector patterns without importing their matches.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) gives dependencies their
 * sectionless-per-record pattern syntax; [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * separates prepared selection authorization from hidden acquisition effects.
 */
#include "internal.h"

bool configuration_document_selector(ConfigurationDocument *document, const ConfigurationRecord *record) {
    if (record->kind != CONFIGURATION_SELECTOR || !configuration_selector(record->value)) {
        return configuration_document_failure(document, record->span,
            "Invalid dependency selector", "one absolute ordinary or package-alias selector per line");
    }
    for (const ConfigurationSelector *selector = document->selectors; selector != NULL; selector = selector->next) {
        if (configuration_text_equal(selector->pattern, record->value)) {
            return configuration_document_failure(document, record->value,
                "Duplicate dependency selector", "one record per exact selector pattern");
        }
    }
    ConfigurationSelector *selector = configuration_document_allocate(document, sizeof(*selector), record->span);
    if (selector == NULL) { return false; }
    *selector = (ConfigurationSelector){.pattern = record->value};
    if (document->last_selector == NULL) { document->selectors = selector; }
    else { document->last_selector->next = selector; }
    document->last_selector = selector;
    return true;
}
