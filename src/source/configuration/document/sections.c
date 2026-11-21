/**
 * @file
 * @brief Admit each exact INI section once and finish pending required fields.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) permits reordering but
 * forbids duplicate sections; [PKG-06](../../../../spec/project/packages.md#pkg-06)
 * requires complete url/directory pairs before any document can succeed.
 */
#include "internal.h"

bool configuration_repository_complete(ConfigurationDocument *document, const ConfigurationRepository *repository) {
    if (repository == NULL) { return true; }
    if (repository->url.source == NULL || repository->directory.source == NULL) {
        return configuration_document_failure(document, repository->header,
            "Repository configuration is incomplete", "one url and one directory assignment");
    }
    return true;
}

bool configuration_section_begin(ConfigurationDocument *document, ConfigurationSection *section,
    const ConfigurationRecord *record) {
    if (document->profile != CONFIGURATION_PACKAGE) {
        return configuration_document_failure(document, record->span,
            "Module configuration has a section", "sectionless alias assignments");
    }
    if (!configuration_repository_complete(document, section->repository)) { return false; }
    if (record->kind == CONFIGURATION_DEPENDENCIES || record->kind == CONFIGURATION_ALIASES) {
        bool *seen = record->kind == CONFIGURATION_DEPENDENCIES
            ? &section->seen_dependencies : &section->seen_aliases;
        if (*seen) {
            return configuration_document_failure(document, record->span,
                "Duplicate configuration section", "each section exactly once");
        }
        *seen = true;
        section->repository = NULL;
    } else {
        for (const ConfigurationRepository *previous = document->repositories; previous != NULL; previous = previous->next) {
            if (configuration_text_equal(previous->prefix, record->key)) {
                return configuration_document_failure(document, record->key,
                    "Duplicate repository section", "one section per exact repository prefix");
            }
        }
        ConfigurationRepository *repository = configuration_document_allocate(document, sizeof(*repository), record->span);
        if (repository == NULL) { return false; }
        *repository = (ConfigurationRepository){.header = record->span, .prefix = record->key};
        if (document->last_repository == NULL) { document->repositories = repository; }
        else { document->last_repository->next = repository; }
        document->last_repository = repository;
        section->repository = repository;
    }
    section->kind = record->kind;
    return true;
}
