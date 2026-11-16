/** @file
 * @brief Define exact framed relocation journal output, with explicit integers, strings and binary snapshots.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * persists recovery/report authority independently of compiler object lifetimes.
 */
#ifndef AMA_RELOCATION_JOURNAL_FORMAT_H
#define AMA_RELOCATION_JOURNAL_FORMAT_H
#include <stdint.h>
#include "../internal.h"
#define RELOCATION_INTENT_MAGIC "AMA-RELOCATION-INTENT-1\n"
#define RELOCATION_JOURNAL_MAGIC "AMA-RELOCATION-PLAN-2\n"
/** Borrow the linear transaction owner and one open journal descriptor through a synchronous write. */
typedef struct { RelocationTransaction *transaction; int descriptor; const char *path; } RelocationJournalWriter;
/** Write complete raw data through the transaction's checked filesystem authority. */
bool relocation_journal_bytes(RelocationJournalWriter *, const unsigned char *, size_t);
/** Canonical decimal integers have a single terminating LF and no leading plus/zero padding. */
bool relocation_journal_unsigned(RelocationJournalWriter *, uintmax_t);
bool relocation_journal_signed(RelocationJournalWriter *, intmax_t);
/** Length-framed text preserves arbitrary non-NUL filesystem bytes, including embedded LF. */
bool relocation_journal_text(RelocationJournalWriter *, const char *);
/** Explicit presence preserves null separately from an empty string. */
bool relocation_journal_nullable(RelocationJournalWriter *, const char *);
/** Length-framed binary data includes any embedded NUL without interpreting it as source syntax. */
bool relocation_journal_blob(RelocationJournalWriter *, const SourceText *);
/** Write each relevant stat field explicitly; platform struct padding is never persisted. */
bool relocation_journal_metadata(RelocationJournalWriter *, const struct stat *);
/** Persist the independently owned complete scope and resulting configuration map. */
bool relocation_journal_report(RelocationJournalWriter *, const RelocationReport *);
/** Write one complete private slot identity tuple. */
bool relocation_journal_slot(RelocationJournalWriter *, const RelocationTransactionSlot *);
/** Persist ordered inverse actions, owned slot identities and prepared replacement snapshots. */
bool relocation_journal_actions(RelocationJournalWriter *);
#endif
