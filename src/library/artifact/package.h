/** @file
 * @brief Package a caller-staged complete translation unit into isolated ELF libraries.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * component-private definitions before either static or dynamic host linking.
 */
#ifndef AMA_LIBRARY_ARTIFACT_PACKAGE_H
#define AMA_LIBRARY_ARTIFACT_PACKAGE_H
#include "library/model.h"
#include "native/scaffold/scaffold.h"
/** Explicit private paths and tools, borrowed through synchronous subprocess completion.
 * All output paths are distinct, absent caller-owned staging destinations. Source
 * includes are already staged. These files are unpublished scratch on failure.
 */
typedef struct {
    const NativeScaffold *assembly;
    const char *compiler, *archiver, *object_copy;
    const char *source, *sdk, *includes;
    const char *raw_object, *isolated_object, *symbols, *archive, *shared;
} LibraryPackageOptions;
/** Compile non-LTO PIC, localize every nonpublic definition, archive deterministically
 * and link with undefined dependencies rejected. No shell, executable launcher,
 * filesystem discovery, overwrite policy or final publication is implicit.
 */
bool library_package_build(const LibraryBuild *, const LibraryPackageOptions *, FILE *errors);
#endif
