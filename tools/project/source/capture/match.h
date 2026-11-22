/** @file
 * @brief Share exact original-unit observation checks between selection and structural expansion.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * refuses changed observations; [PKG-07](../../../../spec/project/packages.md#pkg-07)
 * determines the complete same-module companion set.
 */
#ifndef AMA_PROJECT_SOURCE_CAPTURE_MATCH_H
#define AMA_PROJECT_SOURCE_CAPTURE_MATCH_H
#include "capture.h"
/** Require every original main/companion byte observation, no new same-module
 * companion, and unchanged original main device/inode in an independently captured
 * larger set. Inputs are valid finite structural captures with stable ancestors.
 * Never mutate either owner or allocate. A false result refuses admission; it does
 * not authorize rereading and replacing the original snapshot. This is a unit
 * check, not final whole-tree membership/freshness certification.
 */
bool source_captured_unit_matches(const SourceCapturedUnit *, const AcquisitionUnitEntry *);
#endif
