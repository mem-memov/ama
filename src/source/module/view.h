/** @file
 * @brief Supply an immutable explicit filesystem view to the common complete catalog pipeline.
 * [PKG-02](../../../spec/project/packages.md#pkg-02) requires one whole-version
 * selector; [SRC-02](../../../spec/language/source-and-names.md#src-02) and
 * [SRC-05](../../../spec/language/source-and-names.md#src-05) require the same
 * exact-name, alias and plus-anchor checks for prepared views as physical inputs.
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02) uses
 * final source discovery rather than a separate guessed placement recognizer.
 */
#ifndef AMA_SOURCE_MODULE_VIEW_H
#define AMA_SOURCE_MODULE_VIEW_H
#include "packages/reader.h"
/** Borrow this immutable callback tuple/context until catalog destruction,
 * including lazy configuration/native companion observations after build. tree
 * supplies complete sorted names, exact child/root metadata and stable physical
 * alias equivalence. Its root operation accepts any requested directory key,
 * verifying every component and rejecting unsupported spelling/symlinks just as
 * physical discovery does. No callback may delegate missing prepared inputs to
 * an unrelated physical view. Metadata/names obey ModulePackageReader's ownership
 * and diagnostic contract. read supplies a borrowed complete immutable SourceText
 * only for the duration of a successful call; the catalog copies its exact bytes
 * under the requested key before publishing them. The borrowed text's original
 * path need not equal that key: copied source bytes retain a new physical path.
 * Every operation is read-only and synchronous; failed/partial catalog scratch
 * remains owned by the catalog under [ARCH-03](../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct ModuleCatalogReader {
    ModulePackageReader tree;
    SourceStatus (*read)(void *, const char *path, const SourceText **output);
} ModuleCatalogReader;
/** Build once over an explicit immutable view, retaining the reader through lazy
 * observations and destruction. Require every callback; invalid reader/root-array
 * arguments leave the owner unattempted. Once build starts, failure is final for
 * that owner and never falls back to disk. The common selector, scanner, canonical
 * validators, UTF-8 checks and alias registries remain authoritative. Successful
 * records and bytes are catalog-owned, while the reader/context stays borrowed.
 * Root descriptors/exclusion arrays are borrowed during build only. This API
 * validates the supplied view; its producer separately proves original capture
 * completeness, projected contents and freshness before publication under
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
ModuleStatus module_catalog_build_view(ModuleCatalog *, const ModuleRoot *, size_t, const ModuleCatalogReader *);
#endif
