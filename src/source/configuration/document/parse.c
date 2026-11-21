/**
 * @file
 * @brief Compose physical lines, record recognition and complete typed INI validation.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) has one deterministic
 * profile; [ARCH-01](../../../../spec/implementation/compiler.md#arch-01)
 * keeps orchestration independent of each record/state operation.
 */
#include <assert.h>
#include "internal.h"

/** Dispatch one non-layout entry through its established section, never infer a section from keys.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids inferring a missing package section from its keys.
 */
static bool entry(ConfigurationDocument *document, ConfigurationSection *section,
    const ConfigurationRecord *record) {
    if (section->kind == CONFIGURATION_ALIASES) { return configuration_document_alias(document, record); }
    if (section->kind == CONFIGURATION_DEPENDENCIES) { return configuration_document_selector(document, record); }
    if (section->kind == CONFIGURATION_REPOSITORY) {
        return configuration_document_repository_field(document, section->repository, record);
    }
    return configuration_document_failure(document, record->span,
        "Package entry precedes a section", "an explicit package section header");
}

ConfigurationStatus configuration_document_parse(ConfigurationDocument *document,
    const SourceText *source, ConfigurationProfile profile) {
    assert(!document->attempted);
    document->attempted = true;
    document->source = source;
    document->profile = profile;
    ConfigurationLines lines;
    if (!configuration_lines_begin(&lines, source)) {
        document->issue = lines.issue;
        document->status = CONFIGURATION_INVALID;
        return document->status;
    }
    if (profile != CONFIGURATION_PACKAGE && profile != CONFIGURATION_MODULE) {
        (void)configuration_document_failure(document, (SourceSpan){source, 0, 0},
            "Invalid configuration profile", "package or module configuration");
        return document->status;
    }
    ConfigurationSection section = {.kind = profile == CONFIGURATION_MODULE
        ? CONFIGURATION_ALIASES : CONFIGURATION_BLANK};
    for (;;) {
        SourceSpan line;
        ConfigurationLineStatus status = configuration_line_next(&lines, &line);
        if (status == CONFIGURATION_END) { break; }
        if (status == CONFIGURATION_INVALID_LINE) {
            document->issue = lines.issue;
            document->status = CONFIGURATION_INVALID;
            return document->status;
        }
        ConfigurationRecord record;
        if (!configuration_record_read(line, &record, &document->issue)) {
            document->status = CONFIGURATION_INVALID;
            return document->status;
        }
        if (record.kind == CONFIGURATION_BLANK || record.kind == CONFIGURATION_COMMENT) { continue; }
        bool accepted = record.kind == CONFIGURATION_DEPENDENCIES || record.kind == CONFIGURATION_ALIASES
            || record.kind == CONFIGURATION_REPOSITORY
            ? configuration_section_begin(document, &section, &record) : entry(document, &section, &record);
        if (!accepted) { return document->status; }
    }
    if (!configuration_repository_complete(document, section.repository)) { return document->status; }
    document->valid = true;
    document->status = CONFIGURATION_OK;
    return document->status;
}
