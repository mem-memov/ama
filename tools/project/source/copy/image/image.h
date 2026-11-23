/** @file
 * @brief Assemble mapped source material into one immutable image with explicitly selected final semantic roots.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact physical material; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates image preparation from deletion authority and recoverable publication.
 */
#ifndef AMA_SOURCE_COPY_IMAGE_H
#define AMA_SOURCE_COPY_IMAGE_H
#include "../material/material.h"
#include "../../image/projection.h"
/** Keep semantic preparation and original-observation failures distinct. */
typedef enum { SOURCE_COPY_IMAGE_OK, SOURCE_COPY_IMAGE_INVALID,
    SOURCE_COPY_IMAGE_MATERIAL, SOURCE_COPY_IMAGE_MEMORY, SOURCE_COPY_IMAGE_LIMIT,
    SOURCE_COPY_IMAGE_ORIGINAL, SOURCE_COPY_IMAGE_COLLISION, SOURCE_COPY_IMAGE_SCOPE,
    SOURCE_COPY_IMAGE_PROJECTION } SourceCopyImageStatus;
/** Borrow exact witnesses and child failures; all inputs outlive diagnostics. */
typedef struct {
    const char *path;
    const SourceFileMapping *mapping;
    SourceMaterialStatus material_status;
    SourceMaterialIssue material;
    SourceProjectionStatus projection_status;
    SourceProjectionIssue projection;
} SourceCopyImageIssue;
/** Require an immutable valid original image, sealed P and applied prepared texts.
 * Supplemental changes explicitly supply configuration changes and authorized
 * removals; this function grants no permission to remove or edit those inputs.
 * Existing P destinations require an explicit supplemental REMOVE at that exact
 * key. Such a pair becomes one UPDATE; a conflicting supplemental ADD/UPDATE
 * refuses. Complete replacement/removal sets remain an outer graph/unit proof.
 *
 * Check each P input against original observed kind/bytes (complete directory
 * membership for directories). Preserve opaque content via the material join.
 * Add exactly the absent physical ancestor directories needed by final additions
 * within the original physical observation envelope; reuse unchanged observed directories.
 * New rows receive distinct symbolic device/inode classes, not predicted host
 * identities. Their permission modes derive from the captured input, or the
 * containing physical boundary for new scaffolding. No host metadata or I/O is consulted.
 *
 * Project material and final_roots simultaneously: removed original semantic
 * roots need not exist in the final rows. An empty final root set selects no
 * modules; it never means reuse original roots. The physical envelope remains
 * unchanged, including when the original used compact bounds. Final roots are
 * borrowed and must outlive image consumers. This operation grants no root-setting
 * authorization; the outer complete operation must prove that independently.
 *
 * Return an arena-owned stable image pointer only after shared simultaneous
 * projection succeeds. Borrow roots/bytes/paths through all catalog consumers;
 * generated ancestor paths and result rows belong to arena. Failed scratch stays
 * owned and output remains unchanged. Final catalog/compiler, exact configuration
 * and binding/site checks, original safe deletion, freshness, exclusive access
 * and recoverable publication remain mandatory. No filesystem mutation occurs.
 * Finite scans and strictly shorter ancestor paths bound work; storage is
 * O(original rows + P + supplemental rows + destination path components) rows,
 * plus retained ancestor/scratch key bytes.
 */
SourceCopyImageStatus source_copy_image_prepare(SourceArena *, const SourceCatalogImage *,
    const SourceFileMap *, const SourceGraphPreparedText *, const SourceImageChange *, size_t,
    const ModuleRoot *final_roots, size_t final_root_count, const SourceCatalogImage **output, SourceCopyImageIssue *);
#endif
