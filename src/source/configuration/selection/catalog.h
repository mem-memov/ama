/** @file
 * @brief Build dependency availability from one actual selected catalog without compiler state.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) includes actual empty
 * versions; [PKG-08](../../../../spec/project/packages.md#pkg-08) requires physical
 * test ancestry before '+' markers disappear from logical names.
 */
#ifndef AMA_DEPENDENCY_CATALOG_VIEW_H
#define AMA_DEPENDENCY_CATALOG_VIEW_H
#include "selection.h"
#include "source/module/catalog.h"
/** Require a successfully built immutable catalog and explicit arena/output/issue.
 * Own normalized identities and view in arena, including all failure scratch.
 * Output changes only on success and survives catalog teardown; a failure issue
 * may borrow original catalog source bytes. Include every ordinary version and
 * module exactly once; exclude standard endpoints. Retain actual test physical
 * provenance via the shared validator. No I/O, parsing, source mutation or
 * compiler state. Finite catalog scans plus normal-form/view construction bound
 * work and storage; no wildcard matching or acquisition occurs.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) requires the complete
 * actual universe; [PKG-08](../../../../spec/project/packages.md#pkg-08) supplies
 * test provenance independently of logical path depth.
 */
DependencySelectionStatus dependency_catalog_view(SourceArena *, const ModuleCatalog *,
    const DependencyView **, SourceIssue *);
#endif
