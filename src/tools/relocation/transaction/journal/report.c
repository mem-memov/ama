/** @file
 * @brief Persist the complete explicit relocation result scope so recovery can report without rerunning a compiler.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * reports final roots, identities, rewritten files, drafts and rebuild requirements after interruption.
 */
#include "format.h"

/** Root kinds/paths/exclusions remain explicit supplied settings, including source roots that physically move. */
static bool roots(RelocationJournalWriter *writer, const ModuleRoot *items, size_t count) {
    if (!relocation_journal_unsigned(writer,count)) { return false; }
    for (size_t index = 0; index < count; index++) {
        if (!relocation_journal_text(writer,items[index].path) || !relocation_journal_unsigned(writer,items[index].kind)
            || !relocation_journal_unsigned(writer,items[index].excluded_count)) { return false; }
        for (size_t item = 0; item < items[index].excluded_count; item++) {
            if (!relocation_journal_text(writer,items[index].excluded[item])) { return false; }
        }
    }
    return true;
}
/** Source scope includes inactive drafts and explicitly introduced files, rather than only a selected entry closure. */
static bool modules(RelocationJournalWriter *writer, const RelocationReport *report) {
    size_t count = 0;
    for (const RelocationReportedModule *row = report->modules; row != NULL; row = row->next) { count++; }
    if (!relocation_journal_unsigned(writer,count)) { return false; }
    for (const RelocationReportedModule *row = report->modules; row != NULL; row = row->next) {
        if (!relocation_journal_text(writer,row->before) || !relocation_journal_text(writer,row->after)
            || !relocation_journal_nullable(writer,row->before_identity) || !relocation_journal_text(writer,row->after_identity)
            || !relocation_journal_unsigned(writer,row->active)) { return false; }
    }
    return true;
}
/** Preserve role flags explicitly; no recovery code searches C, literals or paths for old names. */
static bool files(RelocationJournalWriter *writer, const RelocationReport *report) {
    size_t count = 0;
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) { count++; }
    if (!relocation_journal_unsigned(writer,count)) { return false; }
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        if (!relocation_journal_text(writer,row->before) || !relocation_journal_text(writer,row->after)
            || !relocation_journal_unsigned(writer,row->mode) || !relocation_journal_unsigned(writer,row->existed)
            || !relocation_journal_unsigned(writer,row->rewritten) || !relocation_journal_unsigned(writer,row->draft)
            || !relocation_journal_unsigned(writer,row->opaque) || !relocation_journal_unsigned(writer,row->invalidated)) { return false; }
    }
    return true;
}
/** The report is already independent of the compiler-side snapshot before application publication can begin. */
bool relocation_journal_report(RelocationJournalWriter *writer, const RelocationReport *report) {
    return relocation_journal_text(writer,report->source) && relocation_journal_text(writer,report->destination)
        && relocation_journal_nullable(writer,report->entry) && relocation_journal_unsigned(writer,report->rebuild_required)
        && roots(writer,report->before_roots,report->before_root_count) && roots(writer,report->after_roots,report->after_root_count)
        && modules(writer,report) && files(writer,report);
}
