/** @file
 * @brief Separate contained relative-directory normalization from version-path assembly.
 * [PKG-06](../../../../spec/project/packages.md#pkg-06) compares normalized
 * physical package-version locations without guessing repository equivalence.
 */
#ifndef AMA_PROJECT_ACQUISITION_LOCATION_INTERNAL_H
#define AMA_PROJECT_ACQUISITION_LOCATION_INTERNAL_H
#include "location.h"
/** Retain original bounded directory evidence and static failure explanations. */
AcquisitionMapStatus acquisition_map_failure(SourceIssue *, SourceSpan, AcquisitionMapStatus, const char *);
/** Normalize an original contained relative directory. Empty result denotes
 * checkout root; successful bytes/failed scratch belong to arena. */
AcquisitionMapStatus acquisition_directory(SourceArena *, SourceSpan, const char **, size_t *, SourceIssue *);
#endif
