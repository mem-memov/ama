/** @file
 * @brief Prepare structural closure facts from one admitted original source domain.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings; [PKG-07](../../../../spec/project/packages.md#pkg-07)
 * retains full units and context; [PKG-08](../../../../spec/project/packages.md#pkg-08)
 * keeps associated experiments. Preparation never publishes source changes.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates this preparation from compiler execution and mutation;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * still requires final transaction freshness.
 */
#ifndef AMA_PROJECT_SOURCE_EXPANSION_H
#define AMA_PROJECT_SOURCE_EXPANSION_H
#include "../capture/capture.h"
#include "../analysis/analysis.h"
#include "../inputs/declared.h"
struct SourceExpansionVersion;
/** Own structural observations; borrow both successful original owners through
 * closure destruction. Failed preparation is destructible but not publishable.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires a separate final freshness check after these observations.
 */
typedef struct {
    SourceArena arena;
    SourceManager files;
    const SourceRootCapture *capture;
    SourceAnalysis *analysis;
    const SourceDeclaredGroup *declared;
    struct SourceExpansionVersion *versions;
    const char *failed_path;
    SourceStatus file_status;
    bool declarations_checked;
} SourceExpansion;
/** Begin owned scratch. Borrowed owners and immutable global declarations must
 * outlive all retained facts. NULL declarations supplies no extra input groups.
 */
void source_expansion_init(SourceExpansion *, SourceAllocator, const SourceRootCapture *, SourceAnalysis *, const SourceDeclaredGroup *);
/** Retire metadata before captured bytes and before either borrowed owner. */
void source_expansion_destroy(SourceExpansion *);
/** Validate shared immutable original input ownership and global declarations,
 * including empty selections that never request a module fact. Perform no I/O or
 * allocation; cache successful declaration consistency under the immutable-list
 * contract. Clear issue on success; failure retains an exact stage and, for a
 * contradictory observation, its borrowed physical key. This proves no freshness.
 */
AcquisitionClosureStatus source_expansion_validate(SourceExpansion *, SourceIssue *);
/** T235 module/whole-version expansion callback over the exact original catalog.
 * Version requests retain a complete src tree even when empty. Capture complete
 * structural bytes, check repeated indexed observations against original bytes,
 * and retain source/version/context/test provenance. Only complete checked-export
 * absence identifies a draft. Output changes only on success; failures retain
 * scratch and an issue. No execution, acquisition, rewriting or publication occurs.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps these effects separate from compiler analysis and later mutation.
 */
AcquisitionClosureStatus source_expansion_prepare(void *, const char *, AcquisitionExpansion *, SourceIssue *);
/** Forward exact independent test admission to the retained original analysis. */
AcquisitionClosureStatus source_expansion_admit(void *, const DeliverySourceDefinition *, SourceIssue *);
#endif
