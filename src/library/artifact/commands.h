/** @file
 * @brief Apply declared compiler/link inputs as exact ordered tool arguments.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) binds
 * the verified target assumptions and manifest settings to actual artifacts.
 */
#ifndef AMA_LIBRARY_ARTIFACT_COMMANDS_H
#define AMA_LIBRARY_ARTIFACT_COMMANDS_H
#include "package.h"
/** Compile with declared flags followed by mandatory non-LTO/PIC/profile requirements. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool library_package_compile(const LibraryBuild *, const LibraryPackageOptions *, FILE *);
/** Link declared C dependencies after the isolated object, rejecting unresolved imports. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool library_package_link(const LibraryBuild *, const LibraryPackageOptions *, FILE *);
#endif
