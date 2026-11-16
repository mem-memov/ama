/** @file
 * @brief Decode independent scope settings before any filesystem recovery operation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * reports the exact affected scope even when original compiler owners no longer exist.
 */
#include "../internal.h"

/** Checked arrays belong to the independent report arena and survive decoder failure. */
static void *allocate(RelocationJournalReader *reader, size_t count, size_t size) {
    if (count == 0) { return NULL; }
    if (count > SIZE_MAX/size) {
        relocation_journal_invalid(reader,"A report array exceeds the supported allocation range");
        return NULL;
    }
    void *result = source_arena_allocate(&reader->recovery->transaction.report.arena,count*size);
    if (result == NULL) {
        relocation_transaction_fail(&reader->recovery->transaction,RELOCATION_OUT_OF_MEMORY,"Decoded report ownership allocation failed",NULL);
    }
    return result;
}
/** Preserve the supplied order, root family and literal exclusion strings. */
static bool roots(RelocationJournalReader *reader, const ModuleRoot **output, size_t *count) {
    if (!relocation_journal_read_count(reader,count)) { return false; }
    ModuleRoot *items = allocate(reader,*count,sizeof(*items));
    if (*count != 0 && items == NULL) { return false; }
    *output = items;
    for (size_t index = 0; index < *count; index++) {
        uintmax_t kind;
        items[index] = (ModuleRoot){0};
        if (!relocation_journal_read_text(reader,&reader->recovery->transaction.report.arena,&items[index].path)
            || !relocation_journal_read_unsigned(reader,&kind)) { return false; }
        if (kind != MODULE_ROOT_PROJECT && kind != MODULE_ROOT_STANDARD) {
            return relocation_journal_invalid(reader,"A report source root has an unknown family");
        }
        items[index].kind = (ModuleRootKind)kind;
        if (!relocation_journal_read_count(reader,&items[index].excluded_count)) { return false; }
        const char **excluded = allocate(reader,items[index].excluded_count,sizeof(*excluded));
        if (items[index].excluded_count != 0 && excluded == NULL) { return false; }
        items[index].excluded = excluded;
        for (size_t item = 0; item < items[index].excluded_count; item++) {
            if (!relocation_journal_read_text(reader,&reader->recovery->transaction.report.arena,&excluded[item])) { return false; }
        }
    }
    return true;
}
/** Link initialized rows only; a failed suffix never exposes uninitialized pointers. */
static bool modules(RelocationJournalReader *reader, RelocationReport *report) {
    size_t count;
    if (!relocation_journal_read_count(reader,&count)) { return false; }
    RelocationReportedModule **tail = &report->modules;
    for (size_t index = 0; index < count; index++) {
        RelocationReportedModule *row = allocate(reader,1,sizeof(*row));
        if (row == NULL) { return false; }
        *row = (RelocationReportedModule){0};
        *tail = row;
        tail = &row->next;
        if (!relocation_journal_read_text(reader,&report->arena,&row->before)
            || !relocation_journal_read_text(reader,&report->arena,&row->after)
            || !relocation_journal_read_nullable(reader,&report->arena,&row->before_identity)
            || !relocation_journal_read_text(reader,&report->arena,&row->after_identity)
            || !relocation_journal_read_boolean(reader,&row->active)) { return false; }
    }
    return true;
}
/** Role flags remain explicit; mode values must survive the platform conversion exactly. */
static bool files(RelocationJournalReader *reader, RelocationReport *report) {
    size_t count;
    if (!relocation_journal_read_count(reader,&count)) { return false; }
    RelocationReportedFile **tail = &report->files;
    for (size_t index = 0; index < count; index++) {
        RelocationReportedFile *row = allocate(reader,1,sizeof(*row));
        if (row == NULL) { return false; }
        *row = (RelocationReportedFile){0};
        *tail = row;
        tail = &row->next;
        uintmax_t mode;
        if (!relocation_journal_read_text(reader,&report->arena,&row->before)
            || !relocation_journal_read_text(reader,&report->arena,&row->after)
            || !relocation_journal_read_unsigned(reader,&mode)) { return false; }
        row->mode = (mode_t)mode;
        if ((uintmax_t)row->mode != mode) { return relocation_journal_invalid(reader,"A reported file mode exceeds its platform range"); }
        if (!relocation_journal_read_boolean(reader,&row->existed)
            || !relocation_journal_read_boolean(reader,&row->rewritten)
            || !relocation_journal_read_boolean(reader,&row->draft)
            || !relocation_journal_read_boolean(reader,&row->opaque)
            || !relocation_journal_read_boolean(reader,&row->invalidated)) { return false; }
    }
    return true;
}
/** Readiness is published only after complete journal framing and authority validation. */
bool relocation_journal_read_report(RelocationJournalReader *reader) {
    RelocationReport *report = &reader->recovery->transaction.report;
    report->attempted = true;
    return relocation_journal_read_text(reader,&report->arena,&report->source)
        && relocation_journal_read_text(reader,&report->arena,&report->destination)
        && relocation_journal_read_nullable(reader,&report->arena,&report->entry)
        && relocation_journal_read_boolean(reader,&report->rebuild_required)
        && roots(reader,&report->before_roots,&report->before_root_count)
        && roots(reader,&report->after_roots,&report->after_root_count)
        && modules(reader,report) && files(reader,report);
}
