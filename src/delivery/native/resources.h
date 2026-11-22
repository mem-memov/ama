/** @file
 * @brief Capture module-owned companion and subtree resources as exact bytes.
 * [PKG-07](../../../spec/project/packages.md#pkg-07) defines same-basename files
 * and the entire sibling module-plus subtree as the declared delivery unit;
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01)
 * prohibits executing CMake or guessing neighboring ownership to discover assets.
 */
#ifndef AMA_DELIVERY_NATIVE_RESOURCES_H
#define AMA_DELIVERY_NATIVE_RESOURCES_H
#include "files/snapshot.h"
#include "source/module/catalog.h"
/** Arena-owned resource record; directory entries retain presence without file
 * bytes under [PKG-07](../../../spec/project/packages.md#pkg-07). Paths and
 * regular-file snapshots borrow the supplied arena/source-manager lifetimes.
 */
typedef struct DeliveryNativeResource {
    struct DeliveryNativeResource *next;
    DeliveryFileSnapshot file;
    bool directory;
} DeliveryNativeResource;
/** Capture all additional same-basename companion files and the full module-plus
 * tree. Core .c/.h/.abi/.cmake/.ini inputs are retained by their existing owners
 * and excluded here. Include an absent module-plus root explicitly, distinguishing
 * absent from present-empty storage. Results are sorted by complete physical
 * path; no symlink following or external-file inference occurs. All supplied
 * resources must be usable. Failure preserves output, retains partial scratch
 * and reports a stable failed path where available. Filesystem inputs must remain
 * stable during capture. Both owners outlive the result.
 * [PKG-07](../../../spec/project/packages.md#pkg-07) supplies the finite ownership
 * domain; [BUILD-01](../../../spec/implementation/application-builds.md#build-01)
 * separates it from external resources declared during explicit configuration.
 */
SourceStatus delivery_native_resources(SourceArena *, SourceManager *, const ModuleSource *,
    const DeliveryNativeResource **, const char **failed_path);
#endif
