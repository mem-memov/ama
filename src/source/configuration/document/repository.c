/**
 * @file
 * @brief Accept only unique repository fields and relative contained directory syntax.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) defines url, directory
 * and optional revision; raw values are not shell-expanded or unquoted.
 */
#include "internal.h"

bool configuration_document_repository_field(ConfigurationDocument *document,
    ConfigurationRepository *repository, const ConfigurationRecord *record) {
    if (record->kind != CONFIGURATION_ASSIGNMENT) {
        return configuration_document_failure(document, record->span,
            "Repository entry is not an assignment", "url, directory or revision = value");
    }
    SourceSpan *field;
    if (configuration_text_is(record->key, "url")) { field = &repository->url; }
    else if (configuration_text_is(record->key, "directory")) { field = &repository->directory; }
    else if (configuration_text_is(record->key, "revision")) { field = &repository->revision; }
    else {
        return configuration_document_failure(document, record->key,
            "Unknown repository key", "url, directory or revision");
    }
    if (field->source != NULL) {
        return configuration_document_failure(document, record->key,
            "Duplicate repository key", "one assignment per repository field");
    }
    if (field == &repository->directory && !configuration_repository_directory(record->value)) {
        return configuration_document_failure(document, record->value,
            "Repository directory escapes or is not relative", "a physical directory within the checkout");
    }
    *field = record->value;
    return true;
}
