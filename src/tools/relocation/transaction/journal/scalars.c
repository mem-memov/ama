/** @file
 * @brief Encode exact canonical numeric/string/binary journal frames without locale-sensitive parsing or struct dumps.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains sufficient state for idempotent recovery without guessing record boundaries.
 */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "format.h"
/** All underlying write effects use the same checked real-I/O path as replacement data. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_bytes(RelocationJournalWriter *writer, const unsigned char *bytes, size_t length) {
    return relocation_transaction_write(writer->transaction,writer->descriptor,writer->path,bytes,length);
}
/** uintmax_t bounds every emitted unsigned field and its fixed maximum decimal buffer size. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_unsigned(RelocationJournalWriter *writer, uintmax_t value) {
    char bytes[3*sizeof(uintmax_t)+3];
    int length = snprintf(bytes,sizeof(bytes),"%ju\n",value);
    return (length > 0 && (size_t)length < sizeof(bytes) && relocation_journal_bytes(writer,(const unsigned char *)bytes,(size_t)length))
        || relocation_transaction_fail(writer->transaction,RELOCATION_IO_ERROR,"An unsigned journal frame could not be written",writer->path);
}
/** Signed time/size fields retain negative values without applying unsigned reinterpretation. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_signed(RelocationJournalWriter *writer, intmax_t value) {
    char bytes[3*sizeof(intmax_t)+4];
    int length = snprintf(bytes,sizeof(bytes),"%jd\n",value);
    return (length > 0 && (size_t)length < sizeof(bytes) && relocation_journal_bytes(writer,(const unsigned char *)bytes,(size_t)length))
        || relocation_transaction_fail(writer->transaction,RELOCATION_IO_ERROR,"A signed journal frame could not be written",writer->path);
}
/** The next record starts after exactly length bytes; text contents cannot inject records. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_text(RelocationJournalWriter *writer, const char *text) {
    size_t length = strlen(text);
    return relocation_journal_unsigned(writer,length) && relocation_journal_bytes(writer,(const unsigned char *)text,length);
}
/** Null is represented by a separate bit so an empty exclusion string retains its exact original meaning. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_nullable(RelocationJournalWriter *writer, const char *text) {
    return relocation_journal_unsigned(writer,text != NULL) && (text == NULL || relocation_journal_text(writer,text));
}
/** Snapshot bytes may be binary and are never passed through strlen or an Ama parser. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_blob(RelocationJournalWriter *writer, const SourceText *text) {
    return relocation_journal_unsigned(writer,source_length(text))
        && relocation_journal_bytes(writer,source_bytes(text),source_length(text));
}
