/**
 * @file
 * @brief Append source-owned static occurrences with checked count and atomic linking.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) requires complete
 * introduction/use records; allocation refusal cannot publish a partial record.
 */
#include <stdint.h>
#include "internal.h"

/** Prepare before touching the chain/count; the arena retains all prior work
 * on failure. Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
bool static_occurrence_append(StaticInventory *inventory, StaticModule *module, StaticOccurrence value) {
    if (module->occurrence_count == SIZE_MAX) { return static_memory_failure(inventory); }
    StaticOccurrence *record = static_allocate(inventory, sizeof(*record));
    if (record == NULL) { return false; }
    *record = value;
    record->next = NULL;
    if (module->last_occurrence == NULL) { module->first_occurrence = record; }
    else { module->last_occurrence->next = record; }
    module->last_occurrence = record;
    module->occurrence_count++;
    return true;
}
