/** @file
 * @brief Share bounded image assembly state between membership and ancestor operations.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires one simultaneous final image rather than sequential host mutations.
 */
#ifndef AMA_SOURCE_COPY_IMAGE_INTERNAL_H
#define AMA_SOURCE_COPY_IMAGE_INTERNAL_H
#include "image.h"
/** Own change storage in arena; borrow original observation and issue owner. */
typedef struct {
    SourceArena *arena;
    const SourceCatalogImage *original;
    SourceImageChange *changes;
    size_t count, capacity;
    SourceCopyImageIssue *issue;
} SourceCopyImageBuilder;
/** Exact key lookups; no normalization, allocation or host fallback. */
const SourceCatalogImageEntry *source_copy_image_original(const SourceCopyImageBuilder *, const char *);
/** Return a mutable planned row at one unique key, or NULL. */
SourceImageChange *source_copy_image_change(const SourceCopyImageBuilder *, const char *);
/** Assign the least available symbolic inode in device class zero, retaining
 * mode and other supplied fields. Refuse finite representation exhaustion.
 * New classes differ from every original and previously prepared class. */
SourceCopyImageStatus source_copy_image_identity(const SourceCopyImageBuilder *, struct stat *);
/** Admit every captured member and reject collisions without explicit removal. */
SourceCopyImageStatus source_copy_image_members(SourceCopyImageBuilder *, const SourceCopyMaterial *);
/** Add exact missing parent keys for every planned final row, stopping at known
 * unchanged/planned directories or configured roots; never invent outside scope. */
SourceCopyImageStatus source_copy_image_parents(SourceCopyImageBuilder *);
#endif
