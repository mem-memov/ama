/** @file
 * @brief Copy complete original/final module identities and active/draft roles without textual prefix matching.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * reports the actual simultaneous identity map established by resolved planning.
 */
#include "internal.h"
/** Preserve source record order while deep-copying every borrowed physical/logical string. */
bool relocation_report_modules(RelocationReport *report, const RelocationStaging *staging) {
    RelocationReportedModule **tail = &report->modules;
    for (const RelocationModule *module = staging->references->preview->modules; module != NULL; module = module->next) {
        RelocationReportedModule *row = source_arena_allocate(&report->arena,sizeof(*row));
        if (row == NULL) { return relocation_report_memory(report); }
        *row = (RelocationReportedModule){.active=module->active};
        row->before = source_arena_copy(&report->arena,module->before_path);
        row->after = source_arena_copy(&report->arena,module->after_path);
        row->before_identity = module->before_identity == NULL ? NULL : source_arena_copy(&report->arena,module->before_identity);
        row->after_identity = source_arena_copy(&report->arena,module->after_identity);
        if (row->before == NULL || row->after == NULL || row->after_identity == NULL
            || (module->before_identity != NULL && row->before_identity == NULL)) { return relocation_report_memory(report); }
        *tail = row;
        tail = &row->next;
    }
    return true;
}
