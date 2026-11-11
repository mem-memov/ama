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
