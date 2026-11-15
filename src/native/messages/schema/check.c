/** @file
 * @brief Validate exact bounded data-only schema declarations before codec work.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) makes
 * version, field types and limits explicit rather than inferring payload layouts.
 */
#include "../internal.h"

/** Empty records are valid; no list is unbounded and no scalar silently becomes a list. */
bool ama_message_schema_valid(const AmaNativeMessageSchema *schema) {
    if (schema == NULL || schema->name.data == NULL || schema->name.length == 0
        || schema->version == 0 || schema->maximum_wire_bytes == 0
        || schema->name.length > schema->maximum_wire_bytes
        || (schema->field_count != 0 && schema->fields == NULL)
        || schema->field_count > SIZE_MAX / sizeof(*schema->fields)) { return false; }
    for (size_t index = 0; index < schema->field_count; index++) {
        const AmaNativeMessageField *field = &schema->fields[index];
        if (field->kind < AMA_MESSAGE_CONDITION || field->kind > AMA_MESSAGE_BYTES
            || field->maximum_count == 0 || (!field->list && field->maximum_count != 1)
            || (field->kind == AMA_MESSAGE_CONDITION && field->maximum_bytes != 0)
            || (field->kind == AMA_MESSAGE_INTEGER && field->maximum_bytes == 0)) { return false; }
    }
    return true;
}
