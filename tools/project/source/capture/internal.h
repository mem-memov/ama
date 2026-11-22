/** @file
 * @brief Separate physical unit coherence from immutable compiler-input row construction.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids publishing incomplete capture authority.
 */
#ifndef AMA_PROJECT_SOURCE_CAPTURE_INTERNAL_H
#define AMA_PROJECT_SOURCE_CAPTURE_INTERNAL_H
#include "capture.h"
/** Own exact absolute root paths and copied exclusions before catalog discovery
 * under [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
SourceCaptureStatus source_capture_roots(SourceRootCapture *, const ModuleRoot *, size_t);
/** Capture one catalog member and its opaque companions, retaining owned partial
 * data on refusal under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
SourceCaptureStatus source_capture_unit(SourceRootCapture *, const ModuleSource *, SourceCapturedUnit **);
/** Construct one complete borrowed protocol view from an admitted catalog and
 * complete units, allocating table metadata in arena. Output changes only on
 * success; failed prefixes stay owned. Both physical-original and prepared-image
 * owners use the same unconditional native/version tables under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
SourceCaptureStatus source_capture_tables(SourceArena *, const ModuleCatalog *,
    const SourceCapturedUnit *, DeliverySourceSnapshotView *);
#endif
