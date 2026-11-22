/** @file
 * @brief Emit exact concrete dependency roots while retaining surrounding configuration bytes.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits finite concrete re-expression, never a guessed mask rewrite.
 */
#include "reexpress.h"
#include <stdint.h>
#include <string.h>
/** Identity is established by membership in the actual immutable view, not a
 * foreign entry whose strings merely happen to be equal. */
static bool selected(const DependencyViewEntry *entry, const DependencyViewEntry *const *roots, size_t count) {
    for (size_t i = 0; i < count; i++) { if (roots[i] == entry) { return true; } }
    return false;
}
/** Locate a strict parsed header only when no selector provides an edit anchor. */
static bool header(const ConfigurationDocument *document, SourceSpan *output) {
    ConfigurationLines lines; SourceSpan line; SourceIssue issue;
    if (!configuration_lines_begin(&lines, document->source)) { return false; }
    ConfigurationLineStatus status;
    while ((status = configuration_line_next(&lines, &line)) == CONFIGURATION_LINE) {
        ConfigurationRecord record;
        if (!configuration_record_read(line, &record, &issue)) { return false; }
        if (record.kind == CONFIGURATION_DEPENDENCIES) { *output = line; return true; }
    }
    return status == CONFIGURATION_END;
}
/** Size arithmetic refuses before allocation or writing a partial replacement. */
static bool measure(size_t *size, size_t add) {
    if (add > SIZE_MAX - *size) { return false; }
    *size += add; return true;
}
/** Copy one measured segment; the caller has checked the total before allocation. */
static void append(unsigned char *bytes, size_t *cursor, const void *segment, size_t length) {
    memcpy(bytes + *cursor, segment, length); *cursor += length;
}
SourceEditsStatus source_copy_reexpress_selectors(SourceTextEdits *edits, const ConfigurationDocument *document,
    const DependencyView *view, const DependencyViewEntry *const *roots, size_t count) {
    if (!edits || edits->attempted) { return SOURCE_EDITS_INVALID; }
    if (edits->status != SOURCE_EDITS_OK) { return edits->status; }
    if (!document || !document->valid || document->profile != CONFIGURATION_PACKAGE
        || document->source != edits->original || !view || (view->count && !view->entries) || (count && !roots)) {
        return edits->status = SOURCE_EDITS_INVALID;
    }
    for (size_t i = 0; i < count; i++) {
        bool found = false;
        for (size_t j = 0; j < view->count; j++) { if (roots[i] == &view->entries[j]) { found = true; break; } }
        if (!found) { return edits->status = SOURCE_EDITS_INVALID; }
    }
    size_t input_length = source_length(document->source);
    const unsigned char *input = source_bytes(document->source);
    const char *newline = "\n"; size_t newline_length = 1;
    for (size_t i = 0; i < input_length; i++) {
        if (input[i] == '\n') { if (i && input[i - 1] == '\r') { newline = "\r\n"; newline_length = 2; } break; }
    }
    SourceSpan anchor = document->selectors ? document->selectors->pattern : (SourceSpan){0};
    SourceSpan section = {0};
    if (!anchor.source && count && !header(document, &section)) { return edits->status = SOURCE_EDITS_INVALID; }
    bool insert = !anchor.source;
    bool new_section = insert && !section.source;
    bool prefix = insert && (section.source || (input_length && input[input_length - 1] != '\n'));
    bool suffix = insert && new_section && (!input_length || input[input_length - 1] == '\n');
    if (insert) { anchor = (SourceSpan){document->source, section.source ? section.end : input_length,
        section.source ? section.end : input_length}; }
    size_t size = 0, distinct = 0;
    for (size_t i = 0; i < view->count; i++) {
        if (!selected(&view->entries[i], roots, count)) { continue; }
        if ((distinct && !measure(&size, newline_length)) || !measure(&size, view->entries[i].identity->length)) {
            return edits->status = SOURCE_EDITS_LIMIT;
        }
        distinct++;
    }
    if (distinct) {
        if ((prefix && !measure(&size, newline_length)) || (suffix && !measure(&size, newline_length))
            || (new_section && (!measure(&size, sizeof("[dependencies]") - 1) || !measure(&size, newline_length)))) {
            return edits->status = SOURCE_EDITS_LIMIT;
        }
        unsigned char *bytes = source_arena_allocate(&edits->arena, size);
        if (!bytes) { return edits->status = SOURCE_EDITS_MEMORY; }
        size_t cursor = 0;
        if (prefix) { append(bytes, &cursor, newline, newline_length); }
        if (new_section) { append(bytes, &cursor, "[dependencies]", sizeof("[dependencies]") - 1); append(bytes, &cursor, newline, newline_length); }
        bool first = true;
        for (size_t i = 0; i < view->count; i++) {
            if (!selected(&view->entries[i], roots, count)) { continue; }
            if (!first) { append(bytes, &cursor, newline, newline_length); }
            append(bytes, &cursor, view->entries[i].identity->text, view->entries[i].identity->length); first = false;
        }
        if (suffix) { append(bytes, &cursor, newline, newline_length); }
        SourceEditsStatus status = source_text_edits_add(edits, anchor, bytes, cursor);
        if (status != SOURCE_EDITS_OK) { return status; }
    } else if (!insert) {
        SourceEditsStatus status = source_text_edits_add(edits, anchor, NULL, 0);
        if (status != SOURCE_EDITS_OK) { return status; }
    }
    for (const ConfigurationSelector *row = document->selectors ? document->selectors->next : NULL; row; row = row->next) {
        SourceEditsStatus status = source_text_edits_add(edits, row->pattern, NULL, 0);
        if (status != SOURCE_EDITS_OK) { return status; }
    }
    return SOURCE_EDITS_OK;
}
