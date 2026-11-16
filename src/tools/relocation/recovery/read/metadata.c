/** @file
 * @brief Decode field-wise filesystem metadata with exact target-type range checks.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires recovery identities to remain exact rather than silently truncated.
 */
#include <string.h>
#include "../internal.h"

/** Each conversion must preserve its mathematical value; no struct padding enters the journal contract. */
bool relocation_journal_read_metadata(RelocationJournalReader *reader, struct stat *metadata) {
    uintmax_t unsigned_value;
    intmax_t signed_value;
    memset(metadata,0,sizeof(*metadata));
#define READ_UNSIGNED(field, type) \
    do { \
        if (!relocation_journal_read_unsigned(reader,&unsigned_value)) { return false; } \
        metadata->field = (type)unsigned_value; \
        if ((uintmax_t)metadata->field != unsigned_value) { \
            return relocation_journal_invalid(reader,"A filesystem metadata field exceeds its platform range"); \
        } \
    } while (0)
#define READ_SIGNED(field, type) \
    do { \
        if (!relocation_journal_read_signed(reader,&signed_value)) { return false; } \
        metadata->field = (type)signed_value; \
        if ((intmax_t)metadata->field != signed_value) { \
            return relocation_journal_invalid(reader,"A filesystem metadata field exceeds its platform range"); \
        } \
    } while (0)
    READ_UNSIGNED(st_dev,dev_t);
    READ_UNSIGNED(st_ino,ino_t);
    READ_UNSIGNED(st_mode,mode_t);
    READ_UNSIGNED(st_uid,uid_t);
    READ_UNSIGNED(st_gid,gid_t);
    READ_UNSIGNED(st_nlink,nlink_t);
    READ_SIGNED(st_size,off_t);
    READ_SIGNED(st_atim.tv_sec,time_t);
    READ_UNSIGNED(st_atim.tv_nsec,long);
    READ_SIGNED(st_mtim.tv_sec,time_t);
    READ_UNSIGNED(st_mtim.tv_nsec,long);
    READ_SIGNED(st_ctim.tv_sec,time_t);
    READ_UNSIGNED(st_ctim.tv_nsec,long);
#undef READ_UNSIGNED
#undef READ_SIGNED
    if (metadata->st_size < 0 || metadata->st_atim.tv_nsec < 0 || metadata->st_atim.tv_nsec >= 1000000000L
        || metadata->st_mtim.tv_nsec < 0 || metadata->st_mtim.tv_nsec >= 1000000000L
        || metadata->st_ctim.tv_nsec < 0 || metadata->st_ctim.tv_nsec >= 1000000000L) {
        return relocation_journal_invalid(reader,"Filesystem sizes and timestamp fractions must be valid");
    }
    return true;
}
