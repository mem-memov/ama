/** @file
 * @brief Choose finite private sibling names before creation so durable reservations can precede mkdir.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * records ownership explicitly instead of recovering by directory-name guesses.
 */
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Reserved public inputs/final paths cannot become temporary storage even when currently absent. */
static bool public_path(const RelocationReport *report, const char *path) {
    if (relocation_contains(path,report->source) || relocation_contains(report->source,path)
        || relocation_contains(path,report->destination) || relocation_contains(report->destination,path)) { return true; }
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        if (relocation_contains(path,row->before) || relocation_contains(path,row->after)) { return true; }
    }
    return false;
}
/** Occupied names and the finite explicit public domain are skipped; ordinal exhaustion is an exact checked failure. */
const char *relocation_transaction_slot_name(RelocationTransaction *transaction, const char *parent) {
    for (;;) {
        if (transaction->slot_ordinal == SIZE_MAX) {
            relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Private relocation slot identity space is exhausted",parent);
            return NULL;
        }
        size_t ordinal = ++transaction->slot_ordinal;
        char name[3*sizeof(uintmax_t)*3+64];
        int length = snprintf(name,sizeof(name),".ama-relocation-%ju-%ju-%ju",
            (uintmax_t)transaction->recovery_slot->metadata.st_dev,(uintmax_t)transaction->recovery_slot->metadata.st_ino,(uintmax_t)ordinal);
        if (length < 0 || (size_t)length >= sizeof(name)) {
            relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Private relocation slot name formatting failed",parent);
            return NULL;
        }
        const char *path = source_arena_join(&transaction->arena,parent,strcmp(parent,"/") == 0 ? "" : "/",name);
        if (path == NULL) { relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Private relocation slot path allocation failed",parent); return NULL; }
        if (public_path(&transaction->report,path)) { continue; }
        struct stat metadata;
        if (lstat(path,&metadata) == 0) { continue; }
        if (errno == ENOENT) { return path; }
        relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A reserved private slot candidate could not be inspected",path);
        return NULL;
    }
}
