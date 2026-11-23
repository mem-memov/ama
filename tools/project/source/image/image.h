/** @file
 * @brief Adapt one explicit finite prepared filesystem image to the common catalog reader.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * validates final package/topology resolution through common discovery;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps image production/freshness and publication separate from read-only indexing.
 */
#ifndef AMA_PROJECT_SOURCE_CATALOG_IMAGE_H
#define AMA_PROJECT_SOURCE_CATALOG_IMAGE_H
#include "source/module/view.h"
/** One exact prepared physical entry. Metadata absence means unavailable, not
 * missing: its name still belongs to its parent's complete listing. Likewise a
 * regular entry with NULL content has unavailable bytes, not an empty file.
 * children_complete asserts the supplied immediate child rows are the entire
 * prepared directory listing. Only observed directories can assert completeness.
 * Opaque/ignored file bytes need not be fetched merely to form an image. Parent
 * listings and root provenance must come from complete captures/projected edits,
 * never guessed filenames. Content borrows immutable captured SourceText storage.
 */
typedef struct {
    const char *path;
    struct stat metadata;
    const SourceText *content;
    bool metadata_captured, children_complete;
} SourceCatalogImageEntry;
/** Physical observation boundaries carry no namespace, exclusions or module
 * identity. Borrow exact absolute directory keys through all image consumers.
 * A zero count is the compact envelope consisting exactly of the semantic root
 * paths; it adds no ancestor or other inferred directory. Explicit paths replace
 * that envelope and may include stable parents without indexing them as modules.
 * Neither representation grants permission to modify observed material.
 */
typedef struct { const char *const *paths; size_t count; } SourceImageBounds;
/** Borrow immutable semantic roots, physical bounds and entries through every
 * consuming catalog's lifetime. Only roots determine module identities; bounds
 * determine available observations and ancestor provenance. Callbacks verify exact
 * declared ancestor directory keys and stable metadata. Neither host I/O nor
 * source writes occur. valid certifies input shape, not capture truth/freshness,
 * final semantic typing, edits or transaction safety.
 */
typedef struct {
    const SourceCatalogImageEntry *entries;
    size_t count;
    const ModuleRoot *roots;
    size_t root_count;
    SourceImageBounds bounds;
    ModuleCatalogReader reader;
    bool valid;
} SourceCatalogImage;
/** Distinguish inconsistent input structure from unsupported final catalog topology. */
typedef enum { SOURCE_IMAGE_OK, SOURCE_IMAGE_INVALID, SOURCE_IMAGE_DUPLICATE,
    SOURCE_IMAGE_OUTSIDE, SOURCE_IMAGE_PARENT } SourceImageStatus;
/** Borrow the first offending input key; entry is meaningful for row failures.
 * Root failures name the root key. Inputs outlive witness inspection. */
typedef struct { size_t entry; const char *path; } SourceImageIssue;
/** Compare exact absolute prepared keys at slash boundaries, without normalization
 * or I/O. '/' and repeated slash root prefixes keep their original spelling.
 * Return false for NULL, empty or relative inputs. This shared scope predicate
 * lets original observation and prepared-image admission use identical boundaries.
 */
bool source_image_below(const char *path, const char *root);
/** Return the finite physical envelope size, independently of catalog selection. */
size_t source_image_bound_count(const SourceCatalogImage *);
/** Borrow an exact physical boundary; index must be below bound_count. */
const char *source_image_bound_path(const SourceCatalogImage *, size_t);
/** Compare ordered physical envelopes, ignoring semantic roots/exclusions. */
bool source_image_same_bounds(const SourceCatalogImage *, const SourceCatalogImage *);
/** Prepare an allocation-free adapter over exact physical keys and separate domains.
 * Validate every row, duplicate, semantic root, physical coverage and immediate ancestor
 * before publishing output. Do not normalize keys or merge physical identities:
 * alias equivalence is explicit metadata checked by the common catalog. Expected
 * unknown/unread ignored entries remain unavailable observations. A callback that
 * needs unavailable metadata/content/list completeness fails instead of using disk
 * or treating it as absence. Missing children are established only by a complete
 * parent listing. Failure preserves output and sets issue. Reprepare only after
 * all catalog borrowers expire; a successful reader's context is this stable object.
 * O(N squared + N*(R+B)) exact path comparisons for R semantic roots and B
 * physical boundaries; O(1) adapter storage. Root provenance,
 * complete final image construction and transaction freshness are caller proofs.
 */
SourceImageStatus source_catalog_image_prepare(SourceCatalogImage *, const SourceCatalogImageEntry *, size_t,
    const ModuleRoot *, size_t, SourceImageBounds, SourceImageIssue *);
#endif
