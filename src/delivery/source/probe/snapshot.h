/** @file
 * @brief Classify one independently checked entry over exact captured source authority.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) requires actual root eligibility;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * excludes future-path source/native reads.
 */
#ifndef AMA_DELIVERY_SOURCE_PROBE_SNAPSHOT_H
#define AMA_DELIVERY_SOURCE_PROBE_SNAPSHOT_H
#include "../domain/domain.h"
#include "../entry.h"
/** Prepare a fresh frontend from a valid decoded snapshot, classify native
 * definitions with the production parser, then independently check the canonical
 * selected entry. Return exact plan/root-key evidence; Invalid retains normal
 * diagnostics. Neither successful nor open classification seals physical inputs
 * or sets composite frontend.valid. Frontend/arena retain failed metadata; input
 * snapshots outlive diagnostic consumption. No execution, guessed types or
 * diagnostic-text classification under
 * [PKG-08](../../../../spec/project/packages.md#pkg-08).
 */
DeliveryEntryState delivery_source_snapshot_entry(CompilerFrontend *, SourceArena *,
    const DeliverySourceSnapshot *, const char *entry);
#endif
