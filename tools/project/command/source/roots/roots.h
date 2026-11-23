/** @file
 * @brief Translate explicit command roots through genuine raw copy placement before original augmentation.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves outside roots and carried families; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * still requires common original analysis of newly covered existing inputs.
 */
#ifndef AMA_PROJECT_SOURCE_ROOT_SETTINGS_H
#define AMA_PROJECT_SOURCE_ROOT_SETTINGS_H
#include "../request.h"
#include "../../../source/operation/roots/roots.h"
/** Keep argument, physical, namespace and bounded storage refusals distinct. */
typedef enum { PROJECT_SOURCE_ROOTS_OK, PROJECT_SOURCE_ROOTS_INVALID,
    PROJECT_SOURCE_ROOTS_PHYSICAL, PROJECT_SOURCE_ROOTS_KIND,
    PROJECT_SOURCE_ROOTS_CONFLICT, PROJECT_SOURCE_ROOTS_MEMORY,
    PROJECT_SOURCE_ROOTS_LIMIT } ProjectSourceRootSettingsStatus;
/** Own rows and explicitly normalized keys; borrow placement keys, configured
 * exclusions and explicit request strings through destruction. Placement must be genuine over that configured
 * capture. A valid proposal is not a captured catalog, original augmentation,
 * final root certificate or permission to publish. Never copy a live owner. */
typedef struct {
    SourceArena arena;
    const SourceRootCapture *configured;
    const SourceTreePlacement *placement;
    ModuleRoot *final;
    SourceOperationRootDisposition *originals;
    SourceOperationRootAddition *additions;
    size_t final_count, addition_count, capacity;
    SourcePhysicalIssue physical_issue;
    const char *failed_path;
    ProjectSourceRootSettingsStatus status;
    bool attempted, valid;
} ProjectSourceRootSettings;
/** Initialize empty effect-free proposal ownership. */
void project_source_root_settings_init(ProjectSourceRootSettings *, SourceAllocator);
/** Retire proposal rows before borrowed configured/request owners expire. */
void project_source_root_settings_destroy(ProjectSourceRootSettings *);
/** Prepare COPY/MOVE settings once. Keep outside roots. A root's actual raw
 * directory row is the sole carry source: COPY keeps it and adds the carry;
 * MOVE carries its setting. Preserve kind and relative exclusions. Explicit
 * destination descriptors select the complete final scope at an equal canonical
 * path, including its exclusions; a family conflict refuses. Additional explicit
 * paths are ADD choices. Normalize only with the neutral creation-destination
 * resolver; no source names/roots are inferred from path components.
 * Actual C/P material, replacements, common augmented original, final selection,
 * independent compiler and shared complete root certificate remain mandatory.
 * No creation or execution. For O originals, E explicit roots, P raw rows and B
 * spelling bytes, finite O(OP+(O+E)^2) key comparisons plus physical traversal;
 * owned O(O+E+B) storage with checked 2O+E capacity. Failed prefixes remain owned.
 */
ProjectSourceRootSettingsStatus project_source_root_settings_build(ProjectSourceRootSettings *,
    const SourceRootCapture *, const SourceTreePlacement *, SourceOperationKind,
    const ProjectSourceRootRequest *, size_t);
#endif
