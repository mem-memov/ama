/** @file
 * @brief Verify exact public definitions and forbidden entry families in ELF objects.
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04) requires
 * unstripped non-LTO object evidence before publication of any component.
 */
#ifndef AMA_LIBRARY_ARTIFACT_VERIFY_H
#define AMA_LIBRARY_ARTIFACT_VERIFY_H
#include "package.h"
/** Inspect raw entry families and the complete isolated public definition set. */
bool library_package_verify(const LibraryBuild *, const LibraryPackageOptions *, FILE *errors);
#endif
