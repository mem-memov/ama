/** @file
 * @brief Separate library source staging, verified packaging and owned publication.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) prohibits
 * publishing partially generated or unverified component artifacts.
 */
#ifndef AMA_DRIVER_LIBRARY_BUILD_INTERNAL_H
#define AMA_DRIVER_LIBRARY_BUILD_INTERNAL_H
#include "build.h"
#include "native/scaffold/internal.h"
/** Remove executable-only launcher/tooling artifacts and stage the remaining flat closure. */
bool driver_library_stage(NativeScaffold *, const LibraryBuild *, const char *stage);
/** Retain complete public source, manifest and binary artifacts for atomic owned publication. */
bool driver_library_retain(NativeScaffold *, const char *stage);
/** Best-effort cleanup of private flat scratch, including failed tool outputs. */
void driver_library_cleanup(const char *stage);
#endif
