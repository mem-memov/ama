/** @file
 * @brief Prepare a root-preserving removal image after original consumer checks.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks original graph/input/client cuts first; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * still requires final independent semantic/reference/freshness and transaction proof.
 */
#ifndef AMA_SOURCE_REMOVAL_IMAGE_H
#define AMA_SOURCE_REMOVAL_IMAGE_H
#include "../selection/selection.h"
#include "../../observation/observation.h"
#include "../../image/projection.h"
#include "../../capture/image.h"
#include "../../graph/sites.h"
/** Keep original review, material, projection and final discovery refusals distinct. */
typedef enum { SOURCE_REMOVAL_IMAGE_OK, SOURCE_REMOVAL_IMAGE_INVALID,
    SOURCE_REMOVAL_IMAGE_REVIEW, SOURCE_REMOVAL_IMAGE_MEMORY,
    SOURCE_REMOVAL_IMAGE_OBSERVATION, SOURCE_REMOVAL_IMAGE_CHANGED,
    SOURCE_REMOVAL_IMAGE_PROJECTION, SOURCE_REMOVAL_IMAGE_CAPTURE } SourceRemovalImageStatus;
/** Final compiler facts must match prepared inputs before binding/site proof. */
typedef enum { SOURCE_REMOVAL_PROOF_OK, SOURCE_REMOVAL_PROOF_INVALID,
    SOURCE_REMOVAL_PROOF_INPUTS, SOURCE_REMOVAL_PROOF_SITES } SourceRemovalProofStatus;
/** Own prepared rows/observations/final compiler inputs; borrow the valid removal
 * selection and its original capture/analysis through teardown. Review records
 * belong to selection. valid proves original cuts, unchanged removed bytes and
 * final common discovery/capture only, never semantic admission or permission to
 * mutate. Missing final observations refuse. This boundary preserves root keys;
 * removal of an explicit root requires a separate root-changing outer plan.
 */
typedef struct {
    SourceArena arena;
    SourceObservedImage observed;
    SourceCatalogImage image;
    SourceImageCapture capture;
    SourceRemovalSelection *selection;
    SourceRemovalReview review;
    SourceRemovalStatus review_status;
    SourceProjectionStatus projection_status;
    SourceProjectionIssue projection_issue;
    const char *failed_path;
    SourceRemovalImageStatus status;
    bool attempted, valid;
} SourceRemovalImage;
/** Initialize independent owners without source reads or mutation. */
void source_removal_image_init(SourceRemovalImage *, SourceAllocator);
/** Retire final input/catalog borrowers before projected rows and observations. */
void source_removal_image_destroy(SourceRemovalImage *);
/** Build once from complete original removal membership. Check all original cuts,
 * demand each removed file's raw bytes and each removed directory's membership,
 * compare demanded material against selection, project simultaneous removals and
 * capture common final compiler inputs. No compiler/test/native execution, source
 * mutation or acquisition. Repeat refusal preserves previous success. Failure
 * retains destructible scratch/review witnesses, never valid. Finite list scans
 * and the existing observer/projection/catalog bounds determine work/storage.
 */
SourceRemovalImageStatus source_removal_image_build(SourceRemovalImage *, SourceRemovalSelection *,
    const SourceRemovalInputUse *, const SourceClientRoot *);
/** Admit an already successful independent whole-domain client against exactly
 * this prepared capture, then check all surviving original kinds/edges/sites with
 * an empty clone map. Reset issue, retaining borrowed graph/site witnesses on
 * failure. No allocation, I/O or mutation; repeated proofs are harmless. Semantic
 * proof still grants no physical freshness/journal/publication authority. */
SourceRemovalProofStatus source_removal_image_prove(const SourceRemovalImage *, const SourceCompilerClient *, SourceSitesIssue *);
#endif
