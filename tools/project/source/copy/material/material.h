/** @file
 * @brief Join complete physical copy membership with exact prepared text owners.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves opaque material and uses authorized parsed edits for changed files;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires a complete prepared image before publication.
 */
#ifndef AMA_SOURCE_COPY_MATERIAL_H
#define AMA_SOURCE_COPY_MATERIAL_H
#include "../../mapping/files/files.h"
#include "../../graph/sites.h"
/** Borrow the exact file mapping and either captured opaque or prepared bytes.
 * Directory entries retain NULL content, including empty directories. */
typedef struct {
    const SourceFileMapping *mapping;
    const SourceText *content;
} SourceCopyMaterialEntry;
/** Arena-owned array; all map/edit/source owners outlive its consumption. */
typedef struct { const SourceCopyMaterialEntry *entries; size_t count; } SourceCopyMaterial;
/** Distinguish malformed preparation, unrelated output, changed original bytes,
 * ambiguous overrides and resource refusal. No failed prefix is published. */
typedef enum { SOURCE_MATERIAL_OK, SOURCE_MATERIAL_INVALID, SOURCE_MATERIAL_UNMAPPED,
    SOURCE_MATERIAL_ORIGINAL, SOURCE_MATERIAL_DUPLICATE,
    SOURCE_MATERIAL_LIMIT, SOURCE_MATERIAL_MEMORY } SourceMaterialStatus;
/** Borrow the first exact mapping/preparation witnesses, or NULL if unavailable. */
typedef struct { const SourceFileMapping *mapping; const SourceGraphPreparedText *prepared; } SourceMaterialIssue;
/** Require a sealed successful P and a finite acyclic prepared-text list. Every
 * supplied edit owner must be successfully applied and target exactly one regular
 * P destination. Its original path AND bytes must equal that mapping's captured
 * input; equal bytes at another path never grant authority. Duplicate output
 * preparation refuses, even for equal bytes. Different SourceText handles with
 * the same captured path/bytes are permitted: compiler responses deep-own text.
 *
 * Emit exactly one row per P member in P order, replacing only explicitly
 * prepared files and retaining all other captured bytes/directories. Do not
 * interpret opaque contents, derive directories, fabricate filesystem identity,
 * grant edit authority or copy an unselected package configuration. Existing
 * destination configuration updates remain separate explicit image changes.
 * Caller proves P completeness and edit eligibility; final catalog/compiler,
 * graph/site, replacement/removal, freshness and transaction gates still apply.
 *
 * No I/O or mutation. Output changes only on complete success. One checked arena
 * allocation owns result rows; inputs and issue witnesses remain borrowed.
 * O(P*E + E squared + original comparison bytes) work and O(P) result storage.
 */
SourceMaterialStatus source_copy_material_prepare(SourceArena *, const SourceFileMap *,
    const SourceGraphPreparedText *, SourceCopyMaterial *, SourceMaterialIssue *);
#endif
