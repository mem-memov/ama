/** @file
 * @brief Assemble the checked public boundary and one complete private project.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires
 * one translation unit per component, with no executable launcher in its ABI.
 */
#ifndef AMA_CODEGEN_LIBRARY_ASSEMBLY_H
#define AMA_CODEGEN_LIBRARY_ASSEMBLY_H
#include "library/model.h"
#include "native/scaffold/scaffold.h"
/** Emit a complete component C translation unit for a ready matching scaffold.
 * header and project are valid C include path spellings supplied by the caller;
 * directive-breaking bytes are rejected before any output. The model
 * and scaffold remain borrowed. Failure leaves only unpublished partial output.
 * Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
bool library_assembly_write(const LibraryBuild *, const NativeScaffold *,
    const char *header, const char *project, FILE *);
#endif
