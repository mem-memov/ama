/** @file
 * @brief Declare one component-private code owner shared by native services, diagnostics and exported values.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires every
 * independent creator-code dependency to contribute to the same unload counter.
 */
#include "internal.h"

/** Emit before lifecycle setup or standalone diagnostic/snapshot adapters; code ownership is not an Ama instance. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06). */
void library_lifecycle_retention(LibraryHeader *header) {
    library_header_print(header, "#include \"runtime/library/native.h\"\nstatic AmaLibraryCodePins %s_code;\n", header->prefix);
}
