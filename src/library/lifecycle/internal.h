/** @file
 * @brief Compose private lifecycle definitions after the checked public and project headers.
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) separates
 * host factories from generated Ama body execution.
 */
#ifndef AMA_LIBRARY_LIFECYCLE_EMITTER_INTERNAL_H
#define AMA_LIBRARY_LIFECYCLE_EMITTER_INTERNAL_H
#include "library/header/internal.h"
/** Emit code-pinned diagnostic conversion and initialized-empty release operations. */
void library_lifecycle_diagnostics(LibraryHeader *);
/** Emit exact numeric-control conversions and public lifecycle adapters. */
void library_lifecycle_operations(LibraryHeader *);
#endif
