/** @file
 * @brief Prepare zero-byte configuration and required areas for an absent version.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * initializes new destinations atomically; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps carried whole-version configurations and destination permission planning separate.
 */
#ifndef AMA_SOURCE_VERSION_INITIALIZATION_H
#define AMA_SOURCE_VERSION_INITIALIZATION_H
#include "../../observation/observation.h"
#include "../../image/projection.h"
#include "../../mapping/files/files.h"
#include "../../mapping/physical/version.h"
#include "../../mapping/identity.h"
/** CARRIED means P explicitly supplies package.ini: use copied configuration rules,
 * never replace that input with an initialized empty environment. */
typedef enum { SOURCE_VERSION_INITIALIZATION_OK, SOURCE_VERSION_INITIALIZATION_INVALID,
    SOURCE_VERSION_INITIALIZATION_ABSENCE, SOURCE_VERSION_INITIALIZATION_CARRIED,
    SOURCE_VERSION_INITIALIZATION_PLACEMENT, SOURCE_VERSION_INITIALIZATION_CONFLICT,
    SOURCE_VERSION_INITIALIZATION_MEMORY, SOURCE_VERSION_INITIALIZATION_LIMIT } SourceVersionInitializationStatus;
/** Own change array/path bytes in arena and zero-byte configuration in texts;
 * borrow original observer, final-root table and version spelling through all image consumers. */
typedef struct {
    const SourceObservedImage *original;
    const ModuleRoot *final_roots;
    size_t final_root_count;
    SourceVersionPlacement version;
    const SourceText *configuration;
    const SourceImageChange *changes;
    size_t count;
} SourceVersionInitialization;
/** Keep structural projection, source construction and exact path failures typed. */
typedef struct {
    const char *path;
    SourceIdentityStatus identity_status;
    SourceStatus source_status;
} SourceVersionInitializationIssue;
/** Require a valid sealed original observer, ready P, a canonical version placement,
 * explicit final semantic roots and one actual known regular original image
 * row as file-mode prototype. Derive
 * the version identity directly through those final roots/exclusions; do not test
 * a guessed module filename. Prove its complete physical directory absent.
 *
 * Prepare empty package.ini, and src/tests directories only where P does not
 * already provide an area or a descendant (the image assembler supplies those
 * exact ancestors). Required empty areas are explicit changes. Reject conflicting
 * regular area entries or explicit carried configuration. Source tables are never
 * imported. Reserved changes supply prior symbolic classes and cannot collide
 * with the changes emitted here. New rows have distinct symbolic metadata using
 * captured regular/directory modes, without predicting host inodes. An existing
 * final root supplies its original directory mode; a missing final root uses the
 * closest containing explicit physical boundary, matching scaffold preparation.
 * Unknown or non-directory mode observations refuse; no host fallback occurs.
 * Final root rows/strings remain borrowed through all consumers.
 *
 * Output changes only on full success; failed arena/text-manager prefixes remain
 * owned. No I/O/source mutation. Finite root/image/P/reserved scans, path lengths
 * and the shared symbolic-class algorithm bound work. Storage is O(reserved + 3)
 * change rows plus generated paths and one empty text. The caller joins changes
 * with P, validates final discovery/configuration/compiler/reference laws, and
 * publishes the entire operation under TOOL-04; this is not command acceptance.
 */
SourceVersionInitializationStatus source_version_initialize(SourceArena *, SourceManager *,
    const SourceObservedImage *, const ModuleRoot *final_roots, size_t final_root_count,
    const SourceVersionPlacement *, const SourceFileMap *,
    const SourceCatalogImageEntry *file_mode, const SourceImageChange *reserved, size_t,
    SourceVersionInitialization *, SourceVersionInitializationIssue *);
#endif
