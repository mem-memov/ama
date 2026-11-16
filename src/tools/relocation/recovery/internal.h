/** @file
 * @brief Separate strict journal decoding, physical ownership validation and outcome-directed replay.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids guessing missing state or overwriting conflicting paths during recovery.
 */
#ifndef AMA_RELOCATION_RECOVERY_INTERNAL_H
#define AMA_RELOCATION_RECOVERY_INTERNAL_H
#include <stdint.h>
#include "recovery.h"
#include "../transaction/internal.h"
/** A borrowed immutable record and monotonically advancing byte cursor; decoding never mutates files. */
typedef struct {
    RelocationRecovery *recovery;
    const SourceText *source;
    size_t offset;
    bool truncated;
} RelocationJournalReader;
/** Record malformed data at the exact journal byte without allocating error prose. */
bool relocation_journal_invalid(RelocationJournalReader *, const char *);
/** Consume an exact marker; only a proper incomplete prefix marks truncated. */
bool relocation_journal_read_token(RelocationJournalReader *, const char *);
/** Decode canonical checked unsigned and signed decimal LF frames. */
bool relocation_journal_read_unsigned(RelocationJournalReader *, uintmax_t *);
bool relocation_journal_read_signed(RelocationJournalReader *, intmax_t *);
/** Bound collection counts by representable size and remaining framing bytes. */
bool relocation_journal_read_count(RelocationJournalReader *, size_t *);
/** Accept only exact 0 or 1. */
bool relocation_journal_read_boolean(RelocationJournalReader *, bool *);
/** Copy length-framed non-NUL text into the given owning arena. */
bool relocation_journal_read_text(RelocationJournalReader *, SourceArena *, const char **);
/** Preserve null separately from empty text. */
bool relocation_journal_read_nullable(RelocationJournalReader *, SourceArena *, const char **);
/** Copy opaque binary payload into independently owned source storage. */
bool relocation_journal_read_blob(RelocationJournalReader *, const char *, const SourceText **);
/** Decode exact target-width metadata fields and bounded timestamp fractions. */
bool relocation_journal_read_metadata(RelocationJournalReader *, struct stat *);
/** Decode and link one acknowledged private slot identity. */
RelocationTransactionSlot *relocation_journal_read_slot(RelocationJournalReader *);
/** Decode complete independent scope/settings/module/file report records. */
bool relocation_journal_read_report(RelocationJournalReader *);
/** Decode ordered actions and prepared replacement payloads without filesystem authority. */
bool relocation_journal_read_actions(RelocationJournalReader *);
/** Decode a complete prepared inventory and exact decision/finalization tail. */
bool relocation_journal_decode(RelocationRecovery *, const SourceText *);
/** Decode preparation scope and complete append records; a truncated suffix authorizes no new effects. */
bool relocation_intent_decode(RelocationRecovery *, const SourceText *, bool *);
/** Validate the finite report/action/slot geometry before filesystem replay. */
bool relocation_journal_validate(RelocationRecovery *);
/** Require canonical absolute spelling without dot components or repeated separators. */
bool relocation_recovery_path(const char *);
/** Validate report settings and logical identities independently of actions. */
bool relocation_recovery_report_shape(const RelocationReport *);
/** Validate a slot tuple; an unacknowledged reservation omits identity authority. */
bool relocation_recovery_slot_shape(const RelocationTransactionSlot *, bool);
/** Prove prepared tree rows have unique names and exact source/destination projection. */
bool relocation_recovery_tree_shape(RelocationRecovery *, const RelocationTransactionAction *);
/** Require prepared report/storage authority to equal its earlier intent exactly. */
bool relocation_recovery_same_intent(RelocationRecovery *, const RelocationReport *, const RelocationTransactionSlot *);
/** Verify all physical directory components before acquiring the corresponding lease. */
bool relocation_recovery_directory(RelocationTransaction *, const char *);
/** Resolve, lease, load and validate the complete private recovery authority. */
bool relocation_recovery_load(RelocationRecovery *, const char *);
/** Validate all retained slots and inverse preconditions before any replay mutation. */
bool relocation_recovery_preflight(RelocationRecovery *);
/** Compare published replacements with exact bytes and complete tree membership. */
bool relocation_recovery_replacement(RelocationRecovery *, const RelocationTransactionAction *);
/** Synchronize the read decision/terminal record before deleting originals or acknowledging historical completion. */
bool relocation_recovery_acknowledge(RelocationRecovery *);
/** Durably retract incomplete uncommitted acknowledgement bytes before inverse execution. */
bool relocation_recovery_retract(RelocationRecovery *);
/** Retire only registered preparation copies; unacknowledged directories must be absent or empty. */
bool relocation_recovery_preparation(RelocationRecovery *);
#endif
