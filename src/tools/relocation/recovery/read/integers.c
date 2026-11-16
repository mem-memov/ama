/** @file
 * @brief Decode canonical integer frames without permissive whitespace, locale rules or arithmetic overflow.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects malformed recovery authority before any source mutation.
 */
#include <inttypes.h>
#include "../internal.h"

/** Every accepted digit advances the cursor; checked multiply/add bounds the exact numeric value. */
static bool magnitude(RelocationJournalReader *reader, uintmax_t limit, uintmax_t *value) {
    const unsigned char *bytes = source_bytes(reader->source);
    size_t length = source_length(reader->source), start = reader->offset;
    uintmax_t result = 0;
    if (start >= length || bytes[start] < '0' || bytes[start] > '9') {
        reader->truncated = start >= length;
        return relocation_journal_invalid(reader,"A journal integer needs a decimal digit");
    }
    while (reader->offset < length && bytes[reader->offset] >= '0' && bytes[reader->offset] <= '9') {
        unsigned digit = bytes[reader->offset]-'0';
        if (result > (limit-digit)/10) { return relocation_journal_invalid(reader,"A journal integer exceeds its supported range"); }
        result = result*10+digit;
        reader->offset++;
    }
    if (reader->offset-start > 1 && bytes[start] == '0') {
        return relocation_journal_invalid(reader,"Journal integers cannot contain leading zero padding");
    }
    if (reader->offset >= length || bytes[reader->offset] != '\n') {
        reader->truncated = reader->offset >= length;
        return relocation_journal_invalid(reader,"A journal integer must end with exactly one LF");
    }
    reader->offset++;
    *value = result;
    return true;
}
/** Unsigned frames accept no sign or optional surrounding whitespace. */
bool relocation_journal_read_unsigned(RelocationJournalReader *reader, uintmax_t *value) {
    return magnitude(reader,UINTMAX_MAX,value);
}
/** INTMAX_MIN is constructed from magnitude-1 so no signed intermediate overflows. */
bool relocation_journal_read_signed(RelocationJournalReader *reader, intmax_t *value) {
    const unsigned char *bytes = source_bytes(reader->source);
    bool negative = reader->offset < source_length(reader->source) && bytes[reader->offset] == '-';
    if (negative) { reader->offset++; }
    uintmax_t result;
    if (!magnitude(reader,negative ? (uintmax_t)INTMAX_MAX+1 : (uintmax_t)INTMAX_MAX,&result)) { return false; }
    if (negative && result == 0) { return relocation_journal_invalid(reader,"Negative zero is not a canonical journal integer"); }
    *value = negative ? -(intmax_t)(result-1)-1 : (intmax_t)result;
    return true;
}
/** This is a necessary byte-count bound, not an estimate of the decoded collection's size. */
bool relocation_journal_read_count(RelocationJournalReader *reader, size_t *count) {
    uintmax_t value;
    if (!relocation_journal_read_unsigned(reader,&value)) { return false; }
    size_t remaining = source_length(reader->source)-reader->offset;
    if (value > SIZE_MAX || value > remaining/2) {
        return relocation_journal_invalid(reader,"A journal collection count exceeds its remaining framing bytes");
    }
    *count = (size_t)value;
    return true;
}
/** Boolean role/phase fields have no implicit nonzero interpretation. */
bool relocation_journal_read_boolean(RelocationJournalReader *reader, bool *value) {
    uintmax_t integer;
    if (!relocation_journal_read_unsigned(reader,&integer)) { return false; }
    if (integer > 1) { return relocation_journal_invalid(reader,"A journal boolean must be 0 or 1"); }
    *value = integer != 0;
    return true;
}
