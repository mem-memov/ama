/** @file
 * @brief Own finite local compiler-input captures without classifying or executing modules.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * includes every selected original source;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates prepared bytes from later transaction freshness;
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps discovery outside the compiler;
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains opaque companions.
 */
#ifndef AMA_PROJECT_SOURCE_CAPTURE_H
#define AMA_PROJECT_SOURCE_CAPTURE_H
#include "delivery/source/snapshot/snapshot.h"
#include "source/path/physical/physical.h"
#include "../../acquisition/closure/unit/unit.h"
/** Failure stage retains typed catalog/file details instead of guessing meaning
 * from diagnostic strings under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef enum { SOURCE_CAPTURE_OK, SOURCE_CAPTURE_INVALID, SOURCE_CAPTURE_CATALOG,
    SOURCE_CAPTURE_ROOT, SOURCE_CAPTURE_FILES, SOURCE_CAPTURE_CHANGED, SOURCE_CAPTURE_MEMORY,
    SOURCE_CAPTURE_LIMIT } SourceCaptureStatus;
/** One indexed main and all same-name companions; no recursive opaque subtree
 * claim is made by this compiler-input view under
 * [PKG-07](../../../../spec/project/packages.md#pkg-07).
 */
typedef struct SourceCapturedUnit {
    struct SourceCapturedUnit *next;
    const ModuleSource *source;
    const AcquisitionUnitEntry *files;
} SourceCapturedUnit;
/** Single-attempt owner. Catalog owns canonical names/main/INI/version bytes;
 * files owns raw companions, arena owns rows and units. Failed prefixes are
 * inspectable only for diagnostics, never as accepted input under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
typedef struct {
    ModuleCatalog catalog;
    SourceArena arena;
    SourceManager files;
    DeliverySourceSnapshotView view;
    const ModuleRoot *roots;
    size_t root_count;
    SourcePhysicalIssue root_issue;
    const SourceCapturedUnit *units;
    const char *failed_path;
    SourceStatus file_status;
    SourceCaptureStatus status;
    bool attempted, valid;
} SourceRootCapture;
/** Begin independent owners under
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01).
 */
void source_root_capture_init(SourceRootCapture *, SourceAllocator);
/** Release metadata before borrowed source bytes under
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
void source_root_capture_destroy(SourceRootCapture *);
/** Capture every selected source and version through the common catalog and
 * structural unit reader. Roots/exclusions are explicit and stable for this call;
 * no parser, native activation, test execution or mutation occurs. Require exact
 * main/INI equality across component observations and matching main device/inode.
 * Normalize exact physical roots and own their exclusion strings without changing
 * namespace kind. C/header/ABI are unconditional opaque captures. Publish valid only after all
 * rows succeed; failures own destructible prefixes and typed diagnostics. Repeat
 * calls refuse without replacing a prior result. Successful data survives later
 * physical edits but does not certify freshness at transaction application.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires whole selected-domain preparation, not entry reachability.
 */
SourceCaptureStatus source_root_capture_build(SourceRootCapture *, const ModuleRoot *, size_t);
#endif
