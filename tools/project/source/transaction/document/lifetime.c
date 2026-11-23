/** @file
 * @brief Keep image facts in one epoch owner and retire every failed prefix.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * separates retained names/bytes from temporary JSON syntax.
 */
#include <string.h>
#include "document.h"
void source_image_document_init(SourceImageDocument *document, SourceAllocator allocator) {
    memset(document, 0, sizeof(*document)); source_json_capture_init(&document->capture, allocator);
}
void source_image_document_destroy(SourceImageDocument *document) {
    source_json_capture_destroy(&document->capture); memset(document, 0, sizeof(*document));
}
