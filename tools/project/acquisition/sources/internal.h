/** @file
 * @brief Keep snapshot/version lookup and owned preparation private to one acquisition operation.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) compares exact source
 * keys; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete and failed allocations under one destructor.
 */
#ifndef AMA_PROJECT_ACQUISITION_SOURCES_INTERNAL_H
#define AMA_PROJECT_ACQUISITION_SOURCES_INTERNAL_H
#include "sources.h"
/** Private mutable index node; consumers receive the const public value only. */
typedef struct AcquisitionSnapshotRecord {
    struct AcquisitionSnapshotRecord *next;
    AcquisitionSnapshot value;
} AcquisitionSnapshotRecord;
/** Private version index node; published public values never change. */
typedef struct AcquisitionVersionRecord {
    struct AcquisitionVersionRecord *next;
    AcquisitionVersionSource value;
} AcquisitionVersionRecord;
/** Populate static diagnostics and an optional previous source without allocation. */
AcquisitionSourcesStatus acquisition_sources_failure(AcquisitionSourcesIssue *, SourceSpan,
    AcquisitionSourcesStatus, const char *, const AcquisitionVersionSource *);
/** Exact C-string/original-span equality, with NULL revision distinct from text. */
bool acquisition_source_text_equal(const char *, SourceSpan);
/** Copy nonempty original bytes into the operation arena; failure retains scratch. */
char *acquisition_source_text(AcquisitionSources *, SourceSpan);
/** Find a public snapshot by address without dereferencing an unknown handle. */
AcquisitionSnapshotRecord *acquisition_snapshot_record(AcquisitionSources *, const AcquisitionSnapshot *);
/** Intern one successful selected route's exact transport key; no resolution. */
AcquisitionSourcesStatus acquisition_snapshot_request(AcquisitionSources *, const AcquisitionRoute *,
    const AcquisitionSnapshot **, bool *, AcquisitionSourcesIssue *);
#endif
