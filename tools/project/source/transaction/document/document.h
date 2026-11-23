/** @file
 * @brief Persist exact physical image facts without compiler state or source mutation authority.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires recoverable original/final bytes, identities and observation precision;
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * keeps independent epoch ownership and failed decode prefixes explicit.
 */
#ifndef AMA_SOURCE_TRANSACTION_IMAGE_DOCUMENT_H
#define AMA_SOURCE_TRANSACTION_IMAGE_DOCUMENT_H
#include "../../image/image.h"
#include "source/json/capture/capture.h"
#include <stdio.h>
/** Own decoded physical facts; separate owners represent separate before/after
 * epochs even when a path has different bytes. Borrow input JSON through error
 * diagnostics. Never copy a live owner or expire it before image consumers. */
typedef struct {
    SourceJsonCapture capture;
    SourceCatalogImage image;
    SourceJsonIssue issue;
    SourceImageIssue image_issue;
    SourceJsonStatus status;
    bool attempted, valid;
} SourceImageDocument;
/** Initialize storage without I/O. */
void source_image_document_init(SourceImageDocument *, SourceAllocator);
/** Retire all decoded names, rows and bytes, including failed prefixes. */
void source_image_document_destroy(SourceImageDocument *);
/** Read once from the closed ama.source-image.v2 schema. Preserve byte paths,
 * semantic roots and independent physical bounds,
 * empty/binary content, unknown metadata/content, complete directory flags and
 * exact device/inode/mode integers. Reject unknown/missing fields, duplicate keys,
 * lossy/narrowing scalars, NUL paths and invalid image topology. Scratch syntax
 * expires before returning; decoded facts remain independently owned. No host
 * access, module discovery or mutation. valid publishes only on complete success.
 * Work/storage are bounded by JSON/capture/topology children and payload bytes.
 */
SourceJsonStatus source_image_document_read(SourceImageDocument *, const SourceText *);
/** Encode one valid image into a borrowed stream using exact byte/unsigned values.
 * Content is associated with its image row path, not a SourceText's earlier origin.
 * Flush but do not close/fsync; durable journal ownership belongs to the caller.
 * A false result can leave partial output and grants no publication authority.
 * No source reads or mutation, O(rows + roots + payload bytes) emission work.
 */
bool source_image_document_write(FILE *, const SourceCatalogImage *);
#endif
