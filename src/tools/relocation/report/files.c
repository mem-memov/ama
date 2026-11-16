/** @file
 * @brief Report physical copy/rewrite/draft/output roles from actual plan records, never from path-looking contents.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * distinguishes preserved opaque/draft data from rewritten active references.
 */
#include <string.h>
#include "internal.h"
#include "../internal.h"

/** Output subtree membership is exact slash-delimited containment under the validated explicit output list. */
static bool invalidated(const RelocationStaging *staging, const char *after) {
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) {
        if (relocation_contains(output->after,after)) { return true; }
    }
    return false;
}
/** Classify an actual source/configuration path through retained records; arbitrary C and literal bytes are never searched. */
static void roles(RelocationReportedFile *row, const RelocationStaging *staging) {
    row->opaque = true;
    for (const RelocationModule *module = staging->references->preview->modules; module != NULL; module = module->next) {
        if (strcmp(module->before_path,row->before) == 0) { row->draft = !module->active; row->opaque = false; break; }
    }
    for (const RelocationStagedConfiguration *configuration = staging->configurations; configuration != NULL; configuration = configuration->next) {
        if (strcmp(configuration->file->before,row->before) == 0) { row->opaque = false; break; }
    }
    row->invalidated = invalidated(staging,row->after);
}
/** Deep-copy every contributing entry; explicit outside outputs have separate rows even when their contents were never parsed. */
bool relocation_report_files(RelocationReport *report, const RelocationStaging *staging) {
    RelocationReportedFile **tail = &report->files;
    for (const RelocationStagedFile *file = staging->files; file != NULL; file = file->next) {
        RelocationReportedFile *row = source_arena_allocate(&report->arena,sizeof(*row));
        if (row == NULL) { return relocation_report_memory(report); }
        *row = (RelocationReportedFile){.mode=file->metadata.st_mode,.existed=true};
        row->before = source_arena_copy(&report->arena,file->before);
        row->after = source_arena_copy(&report->arena,file->after);
        if (row->before == NULL || row->after == NULL) { return relocation_report_memory(report); }
        row->rewritten = file->original != NULL && file->replacement != NULL
            && (source_length(file->original) != source_length(file->replacement)
                || memcmp(source_bytes(file->original),source_bytes(file->replacement),source_length(file->original)) != 0);
        roles(row,staging);
        *tail = row;
        tail = &row->next;
    }
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) {
        bool present = false;
        for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
            if (strcmp(row->before,output->before) == 0) { present = true; break; }
        }
        if (present) { continue; }
        RelocationReportedFile *row = source_arena_allocate(&report->arena,sizeof(*row));
        if (row == NULL) { return relocation_report_memory(report); }
        *row = (RelocationReportedFile){.mode=output->metadata.st_mode,.existed=output->existed,.invalidated=true,.opaque=true};
        row->before = source_arena_copy(&report->arena,output->before);
        row->after = source_arena_copy(&report->arena,output->after);
        if (row->before == NULL || row->after == NULL) { return relocation_report_memory(report); }
        *tail = row;
        tail = &row->next;
    }
    return true;
}
