/** @file
 * @brief Separate closed image schema recognition from retained filesystem facts.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves exact record values; no schema decoder may perform source operations.
 */
#ifndef AMA_SOURCE_TRANSACTION_IMAGE_DOCUMENT_INTERNAL_H
#define AMA_SOURCE_TRANSACTION_IMAGE_DOCUMENT_INTERNAL_H
#include "document.h"
#include "source/json/schema/schema.h"
/** Decoder scratch owns temporary syntax/byte conversion; output owns retained facts. */
typedef struct { SourceImageDocument *output; SourceArena *scratch; } SourceImageReader;
/** Preserve a typed first schema/resource issue at the exact original token. */
SourceJsonStatus source_image_document_error(SourceImageReader *, const SourceJsonValue *, const char *, SourceJsonStatus);
/** Decode a NUL-free byte string; empty is permitted only for exclusion values. */
SourceJsonStatus source_image_document_path(SourceImageReader *, const SourceJsonValue *, bool, const char **);
/** Check array category/count arithmetic before allocating retained rows. */
SourceJsonStatus source_image_document_array(SourceImageReader *, const SourceJsonValue *, size_t, size_t *);
/** Decode actual ordered root settings and complete exact physical rows. */
SourceJsonStatus source_image_document_roots(SourceImageReader *, const SourceJsonValue *, const ModuleRoot **, size_t *);
/** Decode owned physical boundaries with no namespace or exclusion interpretation. */
SourceJsonStatus source_image_document_bounds(SourceImageReader *, const SourceJsonValue *, SourceImageBounds *);
/** Decode filesystem observations without inventing metadata or empty bytes. */
SourceJsonStatus source_image_document_entries(SourceImageReader *, const SourceJsonValue *, const SourceCatalogImageEntry **, size_t *);
#endif
