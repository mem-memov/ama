/**
 * @file
 * @brief Write an exact qualified native symbol from its registered prefix and original local bytes.
 *
 * [NAME-05](../../../spec/implementation/native-interface.md#name-05)
 * Do not transliterate Unicode, strip UCNs or infer a global C symbol from helper spelling.
 */
#include "namespace.h"

bool native_namespace_symbol(FILE *output, const IdentityName *name, SourceSpan local) {
    if (output == NULL || name == NULL || name->category != IDENTITY_NATIVE_NAMESPACE
        || !source_span_valid(local) || local.start == local.end) {
        return false;
    }
    size_t length = local.end - local.start;
    return fprintf(output, "%s_", name->internal_name) >= 0
        && fwrite(source_bytes(local.source) + local.start, 1, length, output) == length;
}
