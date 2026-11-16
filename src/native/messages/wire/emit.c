/** @file
 * @brief Use one deterministic traversal for envelope sizing and exact wire emission.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) binds
 * a complete value to its explicit schema name/version and data-only field shapes.
 */
#include "../internal.h"

/** Emit only already validated values; every write checks the same complete-envelope bound. Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_emit(const AmaNativeMessageSchema *schema, const AmaNativeMessageFieldValue *fields,
    AmaMessageWriter *writer) {
    static const unsigned char magic[] = {'A', 'm', 'M', 1};
    if (!ama_message_write(writer, (AmaByteView){magic, sizeof(magic)})
        || !ama_message_write_size(writer, schema->version)
        || !ama_message_write_size(writer, schema->name.length)
        || !ama_message_write(writer, schema->name)
        || !ama_message_write_size(writer, schema->field_count)) { return false; }
    for (size_t index = 0; index < schema->field_count; index++) {
        unsigned char shape[] = {(unsigned char)schema->fields[index].kind, schema->fields[index].list ? 1 : 0};
        if (!ama_message_write(writer, (AmaByteView){shape, sizeof(shape)})
            || !ama_message_write_size(writer, fields[index].count)) { return false; }
        for (size_t item = 0; item < fields[index].count; item++) {
            AmaNativeMessageValue value = fields[index].items[item];
            unsigned char flag = (unsigned char)value.flag;
            if (!ama_message_write(writer, (AmaByteView){&flag, 1})
                || !ama_message_write_size(writer, value.bytes.length)
                || !ama_message_write(writer, value.bytes)) { return false; }
        }
    }
    return true;
}
