/** @file
 * @brief Project explicit finite material edits without manufacturing original observations.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact prepared material; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps projection separate from freshness, removal permission and publication.
 */
#ifndef AMA_SOURCE_IMAGE_PROJECTION_H
#define AMA_SOURCE_IMAGE_PROJECTION_H
#include "image.h"
#include "source/memory/arena.h"
/** Each path occurs once. ADD/UPDATE carry the final row; REMOVE uses only path.
 * Paths, contents and original roots remain immutable through all consumers. */
typedef enum { SOURCE_IMAGE_ADD, SOURCE_IMAGE_UPDATE, SOURCE_IMAGE_REMOVE } SourceImageChangeKind;
/** One explicit operation and its borrowed final row or removal key. */
typedef struct { SourceImageChangeKind kind; SourceCatalogImageEntry entry; } SourceImageChange;
/** A refusal never changes the published output. Invalid final shapes use SHAPE;
 * unknown original observations use UNOBSERVED, never inferred absence. */
typedef enum { SOURCE_PROJECTION_OK, SOURCE_PROJECTION_INVALID, SOURCE_PROJECTION_DUPLICATE,
    SOURCE_PROJECTION_EXISTS, SOURCE_PROJECTION_MISSING, SOURCE_PROJECTION_UNOBSERVED,
    SOURCE_PROJECTION_ROOT, SOURCE_PROJECTION_SHAPE, SOURCE_PROJECTION_ALLOCATION } SourceProjectionStatus;
/** Borrow the offending change/final key; SIZE_MAX means no particular change. */
typedef struct { size_t change; const char *path; } SourceProjectionIssue;
/** Build one root-preserving image; output must be distinct from original and
 * stable through consumers. Arena owns result rows, not borrowed row payloads.
 * Failure may retain scratch allocations until arena teardown; source and output
 * remain unchanged. All changes are checked against the original simultaneously.
 * Known regular ancestors prove descendant absence too: file-to-directory edits
 * can add children, but final shape still requires explicit parent replacement.
 * No I/O, allocation of filesystem identities, recursive deletion permission or
 * content/type checking occurs. New directories must have complete membership;
 * directory updates require/preserve existing complete membership authority.
 * O((N+M)^2) comparisons, O(N+M) arena storage; checked size arithmetic. */
SourceProjectionStatus source_image_project(SourceArena *, const SourceCatalogImage *,
    const SourceImageChange *, size_t, SourceCatalogImage *, SourceProjectionIssue *);
#endif
