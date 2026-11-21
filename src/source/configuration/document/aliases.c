/**
 * @file
 * @brief Retain exact unique alias spellings with profile-appropriate raw targets.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) distinguishes package
 * direct targets from local targets; scope expansion and cross-table collisions
 * belong to the later alias environment, never a textual substitution here.
 */
#include "internal.h"

bool configuration_document_alias(ConfigurationDocument *document, const ConfigurationRecord *record) {
    if (record->kind != CONFIGURATION_ASSIGNMENT || !configuration_alias_key(record->key)
        || !configuration_alias_target(record->value, document->profile == CONFIGURATION_MODULE)) {
        return configuration_document_failure(document, record->span,
            "Invalid alias configuration", "~name = path with a permitted complete raw target");
    }
    for (const ConfigurationAlias *alias = document->aliases; alias != NULL; alias = alias->next) {
        if (configuration_text_equal(alias->name, record->key)) {
            return configuration_document_failure(document, record->key,
                "Duplicate alias key", "one assignment per exact alias name");
        }
    }
    ConfigurationAlias *alias = configuration_document_allocate(document, sizeof(*alias), record->span);
    if (alias == NULL) { return false; }
    *alias = (ConfigurationAlias){.name = record->key, .target = record->value};
    if (document->last_alias == NULL) { document->aliases = alias; }
    else { document->last_alias->next = alias; }
    document->last_alias = alias;
    return true;
}
