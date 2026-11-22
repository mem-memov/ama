/** @file
 * @brief Edit only authoritative owned alias target spans from one resolved original scope.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids importing a foreign package table or rewriting alias keys. An explicitly
 * copied package configuration retains its own table. Original
 * canonical targets make owned aliases independent of a relocated relative base.
 */
#include "rebase.h"
/** Parsed/source ownership is exact even for byte-identical independent inputs. */
static bool same_span(SourceSpan left, SourceSpan right) {
    return left.source == right.source && left.start == right.start && left.end == right.end;
}
SourceCopyAliasesStatus source_copy_rebase_aliases(SourceTextEdits *edits, const ConfigurationDocument *document,
    const ConfigurationAliasScope *scope, SourceCopyAliasesIssue *issue) {
    if (!issue) { return SOURCE_COPY_ALIASES_INVALID; }
    *issue = (SourceCopyAliasesIssue){0};
    if (!edits || edits->attempted) { return SOURCE_COPY_ALIASES_INVALID; }
    if (edits->status != SOURCE_EDITS_OK) { issue->edit_status = edits->status; return SOURCE_COPY_ALIASES_EDITS; }
    if (!document || !document->valid
        || (document->profile != CONFIGURATION_MODULE && document->profile != CONFIGURATION_PACKAGE)
        || document->source != edits->original || !scope || scope->kind != (document->profile == CONFIGURATION_MODULE ? CONFIGURATION_MODULE_SCOPE : CONFIGURATION_PACKAGE_SCOPE)
        || scope->source != document->source || (scope->count && !scope->entries)) {
        edits->status = SOURCE_EDITS_INVALID;
        return SOURCE_COPY_ALIASES_INVALID;
    }
    size_t count = 0;
    for (const ConfigurationAlias *alias = document->aliases; alias; alias = alias->next) { count++; }
    if (count != scope->count) { edits->status = SOURCE_EDITS_INVALID; return SOURCE_COPY_ALIASES_SCOPE; }
    for (const ConfigurationAlias *alias = document->aliases; alias; alias = alias->next) {
        *issue = (SourceCopyAliasesIssue){.alias = alias};
        for (size_t i = 0; i < scope->count; i++) {
            if (same_span(alias->name, scope->entries[i].name)) { issue->entry = &scope->entries[i]; }
        }
        const ConfigurationAliasEntry *entry = issue->entry;
        if (!entry || entry->local != (document->profile == CONFIGURATION_MODULE) || !entry->target || !entry->target->text || !entry->target->length) {
            edits->status = SOURCE_EDITS_INVALID;
            return SOURCE_COPY_ALIASES_SCOPE;
        }
        issue->edit_status = source_text_edits_add(edits, alias->target,
            (const unsigned char *)entry->target->text, entry->target->length);
        if (issue->edit_status != SOURCE_EDITS_OK) { return SOURCE_COPY_ALIASES_EDITS; }
    }
    *issue = (SourceCopyAliasesIssue){0};
    return SOURCE_COPY_ALIASES_OK;
}
