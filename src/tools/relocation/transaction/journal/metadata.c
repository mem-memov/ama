/** @file
 * @brief Persist portable field-wise filesystem identities, permissions and times without uninitialized padding.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * lets recovery validate physical ownership before restoring or removing paths.
 */
#include "format.h"
/** Values are explicit integer frames; the corresponding reader must validate target-type round trips and nanosecond ranges. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_metadata(RelocationJournalWriter *writer, const struct stat *metadata) {
    return relocation_journal_unsigned(writer,(uintmax_t)metadata->st_dev)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_ino)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_mode)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_uid)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_gid)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_nlink)
        && relocation_journal_signed(writer,(intmax_t)metadata->st_size)
        && relocation_journal_signed(writer,(intmax_t)metadata->st_atim.tv_sec)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_atim.tv_nsec)
        && relocation_journal_signed(writer,(intmax_t)metadata->st_mtim.tv_sec)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_mtim.tv_nsec)
        && relocation_journal_signed(writer,(intmax_t)metadata->st_ctim.tv_sec)
        && relocation_journal_unsigned(writer,(uintmax_t)metadata->st_ctim.tv_nsec);
}
