/** @file
 * @brief Own captured source-command prerequisites through the single complete operation certificate.
 * [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 * keeps preparation outside compiler execution; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires original and final independently checked bytes before publication.
 */
#ifndef AMA_PROJECT_SOURCE_PREPARATION_H
#define AMA_PROJECT_SOURCE_PREPARATION_H
#include "../request.h"
#include "../augmentation/augmentation.h"
#include "../seeds/seeds.h"
#include "../../../source/workspace/workspace.h"
#include "../../../source/operation/complete/complete.h"
/** Preserve the actual failed composed boundary rather than interpreting stderr. */
typedef enum { PROJECT_SOURCE_PREPARATION_OK, PROJECT_SOURCE_PREPARATION_INVALID,
    PROJECT_SOURCE_PREPARATION_MEMORY, PROJECT_SOURCE_PREPARATION_CAPTURE,
    PROJECT_SOURCE_PREPARATION_SELECTION, PROJECT_SOURCE_PREPARATION_DESTINATION,
    PROJECT_SOURCE_PREPARATION_PLACEMENT, PROJECT_SOURCE_PREPARATION_ROOTS,
    PROJECT_SOURCE_PREPARATION_AUGMENTATION, PROJECT_SOURCE_PREPARATION_WORKSPACE,
    PROJECT_SOURCE_PREPARATION_ANALYSIS, PROJECT_SOURCE_PREPARATION_REMOVAL,
    PROJECT_SOURCE_PREPARATION_MAPPING, PROJECT_SOURCE_PREPARATION_OBSERVATION,
    PROJECT_SOURCE_PREPARATION_COMPLETE, PROJECT_SOURCE_PREPARATION_COMPILER,
    PROJECT_SOURCE_PREPARATION_SEAL, PROJECT_SOURCE_PREPARATION_SEEDS } ProjectSourcePreparationStatus;
/** Borrow the parsed request and explicit external records through destruction.
 * temporary_parent is existing scratch, never an inferred semantic root. Inputs
 * and clients remain explicit; absent records do not claim outside-project safety. */
typedef struct {
    const ProjectSourceRequest *request;
    const char *temporary_parent;
    const SourceDeclaredGroup *declared;
    const SourceRemovalInputUse *inputs;
    const SourceClientRoot *clients;
    SourceTreeControl control;
} ProjectSourcePreparationInputs;
/** Noncopyable owner. Destruction retires borrowers before capture/text owners;
 * explicit workspace cleanup is separate, preserving retained-path evidence.
 * Only complete.sealed establishes final read-only operation authority. */
typedef struct {
    SourceArena arena;
    ProjectSourcePreparationInputs inputs;
    SourceRootCapture configured;
    SourceTreeSelection source;
    SourcePhysicalPath destination;
    SourcePhysicalIssue destination_issue;
    SourceTreePlacement initial_placement, placement;
    ProjectSourceRootSettings root_settings;
    ProjectSourceAugmentation augmentation;
    const SourceRootCapture *original_capture;
    const SourceTreeSelection *selected;
    SourceWorkspace workspace;
    SourceAnalysis original;
    ProjectSourceSeeds seeds;
    SourceRemovalSelection removal;
    SourceCloneMap mains;
    SourceFileMap files;
    SourceObservedImage observed;
    SourceOperationIntent intent;
    SourceOperationWritesInputs writes;
    SourceOperationComplete complete;
    SourceCompilerClient final;
    ModuleRoot *final_roots;
    SourceOperationRootDisposition *root_originals;
    size_t final_root_count;
    const char *failed_path;
    ProjectSourcePreparationStatus status;
    bool attempted, captured, prepared;
} ProjectSourcePreparation;
/** Initialize all child owners without I/O or processes. */
void project_source_preparation_init(ProjectSourcePreparation *, SourceAllocator);
/** Capture once from a valid immutable parsed request and explicit scratch parent.
 * Admit arguments before I/O, then retain exact O/U/F proposals, original raw
 * material and independent compiler facts. captured certifies original preparation
 * only; prepared/sealed remain false until later complete-operation gates.
 * No source/native/test/CMake/network execution or source mutation. Failed
 * prefixes and primary child causes survive explicit workspace cleanup. Shared
 * finite capture/root/region/serialization/compiler bounds determine work/storage.
 * Repeated calls refuse without overwriting accepted or failed prior evidence.
 */
ProjectSourcePreparationStatus project_source_preparation_capture(ProjectSourcePreparation *, const ProjectSourcePreparationInputs *);
/** Retire all borrowers, close workspace descriptors and return its close errno;
 * never implicitly delete retained private files. Call workspace cleanup first. */
int project_source_preparation_destroy(ProjectSourcePreparation *);
#endif
