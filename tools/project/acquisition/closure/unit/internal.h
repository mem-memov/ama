/** @file
 * @brief Share only structural capture scratch between selection and traversal.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) keeps context selection
 * separate from complete subtree traversal; [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * retains failed scratch under the caller's owners.
 * Directory identities reject aliased traversal before scheduling.
 * [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 */
#ifndef AMA_ACQUISITION_UNIT_INTERNAL_H
#define AMA_ACQUISITION_UNIT_INTERNAL_H
#include "unit.h"
#include "source/module/catalog.h"
/** Borrowed capture owners and the private exact-directory identity registry.
 * [SRC-02](../../../../../spec/language/source-and-names.md#src-02) requires
 * rejecting repeated physical directories before recursively enumerating them.
 */
typedef struct {
    SourceArena *arena;
    SourceManager *texts;
    ModuleCatalog entries;
    AcquisitionUnitEntry *first;
    const char *failed;
} AcquisitionUnitCapture;
/** Capture one verified named file/directory and insert it in byte order. Optional
 * missing roots add no membership. Non-directory roots can be required to be files.
 * Publish scratch only after success; whole-output publication belongs to caller.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) requires exact owned bytes.
 */
SourceStatus acquisition_unit_child(AcquisitionUnitCapture *, const char *, const char *, bool optional,
    bool directory, AcquisitionUnitEntry **);
/** Capture a verified root directory and all its descendants iteratively; return
 * original I/O/shape failure, never infer absence after failed enumeration under
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
SourceStatus acquisition_unit_tree(AcquisitionUnitCapture *, const char *, const char *);
#endif
