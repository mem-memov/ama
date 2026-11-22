/** @file
 * @brief Verify raw selected members belong to the prepared original physical analysis domain.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * rejects ignored-version selection and reports the supplied root boundary;
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) selects entire versions.
 */
#ifndef AMA_PROJECT_SOURCE_SELECTION_DOMAIN_H
#define AMA_PROJECT_SOURCE_SELECTION_DOMAIN_H
#include "tree.h"
/** Exact source-domain failures, distinct from raw byte-capture success. */
typedef enum { SOURCE_TREE_DOMAIN_OK, SOURCE_TREE_DOMAIN_INVALID,
    SOURCE_TREE_DOMAIN_OUTSIDE, SOURCE_TREE_DOMAIN_EXCLUDED,
    SOURCE_TREE_DOMAIN_UNSELECTED_VERSION } SourceTreeDomainStatus;
/** Check every raw member against the union of explicit root scopes minus their
 * exclusions and ordinary ignored-version copies. Use the shared canonical
 * version-component predicate and exact selected physical directories; never
 * borrow a same-named version's environment. An independently explicit covering
 * root can provide authority; no namespace is inferred from its physical name.
 * Return the first out-of-domain path in raw byte order; output borrows original
 * selection lifetime, NULL on success. No allocation/I/O/mutation or reanalysis.
 * Success establishes physical scope only, not complete associated material,
 * dependency/removal safety, copied identities or final freshness.
 */
SourceTreeDomainStatus source_tree_selection_domain(const SourceTreeSelection *, const char **failed_path);
#endif
